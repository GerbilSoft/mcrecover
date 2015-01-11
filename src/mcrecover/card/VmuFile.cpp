/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * VmuFile.cpp: Dreamcast VMU file entry class.                            *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include "VmuFile.hpp"

#include "vmu.h"
#include "util/byteswap.h"

// VmuCard class.
#include "VmuCard.hpp"

// GcImage class.
#include "GcImage.hpp"
#include "GcToolsQt.hpp"

// C includes. (C++ namespace)
#include <cerrno>
#include <cstdlib>

// C++ includes.
#include <string>
#include <vector>
using std::string;
using std::vector;

// Qt includes.
#include <QtCore/QByteArray>
#include <QtCore/QTextCodec>
#include <QtCore/QFile>
#include <QtCore/QIODevice>

#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** VmuFilePrivate **/

#include "File_p.hpp"
class VmuFilePrivate : public FilePrivate
{
	public:
		/**
		 * Initialize the VmuFile private class.
		 * This constructor is for valid files.
		 * @param q VmuFile.
		 * @param card VmuCard.
		 * @param direntry Directory Entry pointer.
		 * @param mc_fat VMU FAT.
		 */
		VmuFilePrivate(VmuFile *q, VmuCard *card,
			const vmu_dir_entry *dirEntry,
			const vmu_fat *mc_fat);

		virtual ~VmuFilePrivate();

	protected:
		Q_DECLARE_PUBLIC(VmuFile)
	private:
		Q_DISABLE_COPY(VmuFilePrivate)

		/**
		 * Load the file information.
		 */
		void loadFileInfo(void);

		/**
		 * Attempt to decode text as Shift-JIS.
		 * If that fails, use cp1252.
		 * @param str Text data.
		 * @param len Length of str.
		 * @return Unicode QString.
		 */
		static QString decodeText_SJISorCP1252(const char *str, int len);

	public:
		const vmu_fat *mc_fat;	// VMU FAT. (TODO: Do we need to store this?)

		/**
		 * Directory entry.
		 * This points to an entry within card's Directory Table.
		 * NOTE: If this is a lost file, this was allocated by us,
		 * and needs to be freed in the destructor.
		 */
		const vmu_dir_entry *dirEntry;

		// File descriptions.
		QString vmu_desc;
		QString dc_desc;

		// GcImages.
		GcImage *gcBanner;
		QVector<GcImage*> gcIcons;

		/**
		 * Load the banner image.
		 * @return GcImage containing the banner image, or nullptr on error.
		 */
		GcImage *loadBannerImage(void);

		/**
		 * Load the icon images.
		 * @return QVector<GcImage*> containing the icon images, or empty QVector on error.
		 */
		QVector<GcImage*> loadIconImages(void);

		/**
		 * Load the banner and icon images.
		 */
		void loadImages(void);
};

/**
 * Initialize the VmuFile private class.
 * This constructor is for valid files.
 * @param q VmuFile.
 * @param card VmuCard.
 * @param direntry Directory Entry pointer.
 * @param mc_fat VMU FAT.
 */
VmuFilePrivate::VmuFilePrivate(VmuFile *q, VmuCard *card,
		const vmu_dir_entry *dirEntry,
		const vmu_fat *mc_fat)
	: FilePrivate(q, card)
	, mc_fat(mc_fat)
	, dirEntry(dirEntry)
	, gcBanner(nullptr)
{
	if (!dirEntry || !mc_fat) {
		// Invalid data.
		this->dirEntry = nullptr;
		this->mc_fat = nullptr;

		// This file is basically useless now...
		return;
	}

	// Clamp file length to the size of the memory card.
	// This shouldn't happen, but it's possible if either
	// the filesystem is heavily corrupted, or the file
	// isn't actually a GCN Memory Card image.
	int size = dirEntry->size;
	if (size > card->totalUserBlocks())
		size = card->totalUserBlocks();

	// Load the FAT entries.
	fatEntries.clear();
	fatEntries.reserve(size);
	// TODO: Add a 'lastValidBlock' function?
	uint16_t totalUserBlocks = card->totalUserBlocks();
	uint16_t next_block = dirEntry->address;
	if (next_block < totalUserBlocks && next_block != VMU_FAT_BLOCK_LAST_IN_FILE) {
		fatEntries.append(next_block);

		// Go through the rest of the blocks.
		for (int i = size; i > 1; i--) {
			next_block = mc_fat->fat[next_block];
			if (next_block < totalUserBlocks && next_block != VMU_FAT_BLOCK_LAST_IN_FILE) {
				// Next block is invalid.
				break;
			}
			fatEntries.append(next_block);
		}
	}

	// Load the file information.
	loadFileInfo();
}

VmuFilePrivate::~VmuFilePrivate()
{
	if (lostFile) {
		// dirEntry was allocated by us.
		// Free it.
		free((void*)dirEntry);
	}

	// Delete GcImages.
	delete gcBanner;
	qDeleteAll(gcIcons);
	gcIcons.clear();
}

/**
 * Load the file information.
 */
void VmuFilePrivate::loadFileInfo(void)
{
	// Clear stuff that isn't used by the VMU.
	gameID.clear();

	// Filename.
	// TODO: Are Shift-JIS filenames allowed?
	// TODO: Trim filenames?
	filename = QString::fromLatin1(dirEntry->filename, sizeof(dirEntry->filename));

	// Load the header.
	const int blockSize = card->blockSize();
	char *data = (char*)malloc(blockSize);
	int ret = card->readBlock(data, blockSize, fileBlockAddrToPhysBlockAddr(dirEntry->header_addr));
	if (ret != blockSize) {
		// Read error.
		// File is probably invalid.
		free(data);
		return;
	}
	const vmu_file_header *fileHeader = (vmu_file_header*)data;

	// File description.
	// TODO: Get both VMS and DC descriptions?
	// Currently only using DC description.
	// TODO: Heuristic to determine if the description is Japanese.
	vmu_desc = decodeText_SJISorCP1252(fileHeader->desc_vmu, sizeof(fileHeader->desc_vmu)).trimmed();
	dc_desc  = decodeText_SJISorCP1252(fileHeader->desc_dc,  sizeof(fileHeader->desc_dc)).trimmed();

	// NOTE: The DC file manager shows filename and DC description,
	// so we'll show the same thing.
	description = filename + QChar(L'\0') + dc_desc;

	// Timestamp.
	// NOTE: This might be ctime, not mtime...
	// TODO: Convert from BCD.
	//mtime.setGcnTimestamp(dirEntry->lastmodified);

	// Mode. (TODO)
	//this->mode = dirEntry->permission;

	// Load the banner and icon images.
	loadImages();
}

/**
 * Attempt to decode text as Shift-JIS.
 * If that fails, use cp1252.
 * @param str Text data.
 * @param len Length of str.
 * @return Unicode QString.
 */
QString VmuFilePrivate::decodeText_SJISorCP1252(const char *str, int len)
{
	// Static codec initialization.
	// NOTE: Assuming cp1252 always works.
	static QTextCodec *shiftJis = QTextCodec::codecForName("Shift_JIS");
	static QTextCodec *cp1252 = QTextCodec::codecForName("cp1252");

	if (!shiftJis) {
		// Shift-JIS isn't available.
		// Always use cp1252.
		if (!cp1252) {
			// Should not happen...
			return QString::fromLatin1(str, len);
		}
		return cp1252->toUnicode(str, len);
	}

	// Attempt to decode as Shift-JIS.
	// TODO: There should be a faster way to check if the text isn't valid...
	// (iconv can return an error if an invalid character is encountered.)
	QTextCodec::ConverterState state(QTextCodec::ConvertInvalidToNull);
	QString text = shiftJis->toUnicode(str, len);
	// U+FFFD: REPLACEMENT CHARACTER
	// QTextCodec uses this if it encounters
	// an invalid Shift-JIS sequence.
	if (text.contains(QChar(0xFFFD))) {
		// Invalid characters detected.
		// Use cp1252 instead.
		if (!cp1252) {
			// Should not happen...
			return QString::fromLatin1(str, len);
		}
		return cp1252->toUnicode(str, len);
	}

	// Text decoded as Shift-JIS.
	return text;
}

/**
 * Load the banner image.
 * @return GcImage* containing the banner image, or nullptr on error.
 */
GcImage *VmuFilePrivate::loadBannerImage(void)
{
	// TODO
	return nullptr;
}

/**
 * Load the icon images.
 * @return QVector<GcImage*> containing the icon images, or empty QVector on error.
 */
QVector<GcImage*> VmuFilePrivate::loadIconImages(void)
{
	// TODO
	return QVector<GcImage*>();
}

/**
 * Load the banner and icon images.
 */
void VmuFilePrivate::loadImages(void)
{
	// Load the banner.
	this->gcBanner = loadBannerImage();
	if (gcBanner) {
		// Set the new banner image.
		QImage qBanner = gcImageToQImage(gcBanner);
		if (!qBanner.isNull())
			banner = QPixmap::fromImage(qBanner);
		else
			banner = QPixmap();
	} else {
		// No banner image.
		banner = QPixmap();
	}

	// Load the icons.
	this->gcIcons = loadIconImages();
	icons.clear();
	icons.reserve(gcIcons.size());
	foreach (GcImage *gcIcon, gcIcons) {
		if (gcIcon) {
			QImage qIcon = gcImageToQImage(gcIcon);
			if (!qIcon.isNull())
				icons.append(QPixmap::fromImage(qIcon));
			else
				icons.append(QPixmap());
		} else {
			// No icon image.
			icons.append(QPixmap());
		}
	}
}

/** VmuFile **/

/**
 * Create a VmuFile for a VmuCard.
 * This constructor is for valid files.
 * @param q VmuFile.
 * @param card VmuCard.
 * @param direntry Directory Entry pointer.
 * @param mc_fat VMU FAT.
 */
VmuFile::VmuFile(VmuCard *card,
		const vmu_dir_entry *dirEntry,
		const vmu_fat *mc_fat)
	: File(new VmuFilePrivate(this, card, dirEntry, mc_fat), card)
{ }

// TODO: Maybe not needed?
VmuFile::~VmuFile()
{ }

/**
 * Get the file's mode as a string.
 * This is system-specific.
 * @return File mode as a string.
 */
QString VmuFile::modeAsString(void) const
{
	// TODO
	return QString();
}

/** Export **/

/**
 * Get the default export filename.
 * @return Default export filename.
 */
QString VmuFile::defaultExportFilename(void) const
{
	// TODO
	return QString();
}

/**
 * Export the file.
 * @param filename Filename for the exported file.
 * @return 0 on success; non-zero on error.
 * TODO: Error code constants.
 */
int VmuFile::exportToFile(const QString &filename)
{
	// NOTE: This function doesn't actually do anything different
	// from the base class function, but gcc-4.9.2 attempts to use
	// the QIODevice version when using a VmuFile* instead of a
	// File*. Hence, we need this virtual function.
	return File::exportToFile(filename);
}

/**
 * Export the file.
 * @param qioDevice QIODevice to write the data to.
 * @return 0 on success; non-zero on error.
 * TODO: Error code constants.
 */
int VmuFile::exportToFile(QIODevice *qioDevice)
{
	// TODO
	Q_UNUSED(qioDevice);
	return -ENOSYS;
}

/**
 * Save the banner image.
 * @param qioDevice QIODevice to write the banner image to.
 * @return 0 on success; non-zero on error.
 * TODO: Error code constants.
 */
int VmuFile::saveBanner(QIODevice *qioDevice) const
{
	Q_D(const VmuFile);
	if (!d->gcBanner)
		return -EINVAL;

	GcImageWriter gcImageWriter;
	int ret = gcImageWriter.write(d->gcBanner, GcImageWriter::IMGF_PNG);
	if (!ret) {
		const vector<uint8_t> *pngData = gcImageWriter.memBuffer();
		ret = qioDevice->write(reinterpret_cast<const char*>(pngData->data()), pngData->size());
		if (ret != (qint64)pngData->size())
			return -EIO;
		ret = 0;
	}

	// Saved the banner image.
	return ret;
}

/**
 * Save the icon.
 * @param filenameNoExt Filename for the icon, sans extension.
 * @param animImgf Animated image format to use for animated icons.
 * @return 0 on success; non-zero on error.
 * TODO: Error code constants.
 */
int VmuFile::saveIcon(const QString &filenameNoExt,
	GcImageWriter::AnimImageFormat animImgf) const
{
	Q_D(const VmuFile);
	if (d->gcIcons.isEmpty())
		return -EINVAL;

	// Append the correct extension.
	const char *ext;
	if (d->gcIcons.size() > 1) {
		// Animated icon.
		ext = GcImageWriter::extForAnimImageFormat(animImgf);
	} else {
		// Static icon.
		ext = GcImageWriter::extForImageFormat(GcImageWriter::IMGF_PNG);
	}

	// NOTE: Due to PNG_FPF saving multiple files, we can't simply
	// call a version of saveIcon() that takes a QIODevice.
	GcImageWriter gcImageWriter;
	int ret;
	if (d->gcIcons.size() > 1) {
		// Animated icon.
		vector<const GcImage*> gcImages;
		const int maxIcons = (d->gcIcons.size() * 2 - 2);
		gcImages.reserve(maxIcons);
		gcImages.resize(d->gcIcons.size());
		for (int i = 0; i < d->gcIcons.size(); i++)
			gcImages[i] = d->gcIcons[i];

		// Icon speed.
		vector<int> gcIconDelays;
		gcIconDelays.reserve(maxIcons);
		gcIconDelays.resize(d->gcIcons.size());
		for (int i = 0; i < d->gcIcons.size(); i++)
			gcIconDelays[i] = iconDelay(i);

		if (gcImages.size() > 1 && iconAnimMode() == CARD_ANIM_BOUNCE) {
			// BOUNCE animation.
			int src = (gcImages.size() - 2);
			int dest = gcImages.size();
			gcImages.resize(maxIcons);
			gcIconDelays.resize(maxIcons);
			for (; src >= 1; src--, dest++) {
				gcImages[dest] = gcImages[src];
				gcIconDelays[dest] = gcIconDelays[src];
			}
		}

		ret = gcImageWriter.write(&gcImages, &gcIconDelays, animImgf);
	} else {
		// Static icon.
		ret = gcImageWriter.write(d->gcIcons.at(0), GcImageWriter::IMGF_PNG);
	}

	if (ret != 0) {
		// Error writing the icon.
		return ret;
	}

	// Icon written successfully.
	// Save it to a file.
	for (int i = 0; i < gcImageWriter.numFiles(); i++) {
		QString filename = filenameNoExt;
		if (gcImageWriter.numFiles() > 1) {
			// Multiple files.
			// Append the file number.
			char tmp[8];
			snprintf(tmp, sizeof(tmp), "%02d", i+1);
			filename += QChar(L'.') + QLatin1String(tmp);
		}

		// Append the file extension.
		if (ext)
			filename += QChar(L'.') + QLatin1String(ext);

		QFile file(filename);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
			// Error opening the file.
			// TODO: Convert QFileError to a POSIX error code.
			// TODO: Delete previous files?
			return -EIO;
		}

		const vector<uint8_t> *pngData = gcImageWriter.memBuffer(i);
		ret = file.write(reinterpret_cast<const char*>(pngData->data()), pngData->size());
		file.close();

		if (ret != (qint64)pngData->size()) {
			// Error saving the icon.
			file.remove();
			return -EIO;
		}

		ret = 0;
	}

	return ret;
}
