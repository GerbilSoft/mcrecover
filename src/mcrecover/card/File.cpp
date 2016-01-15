/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * File.cpp: Memory Card file entry. [base class]                          *
 *                                                                         *
 * Copyright (c) 2012-2015 by David Korth.                                 *
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

#include "File.hpp"
#include "File_p.hpp"
#include "Card.hpp"

// GcImage.
#include "GcImage.hpp"
#include "GcToolsQt.hpp"
#include "GcImageWriter.hpp"

// C includes. (C++ namespace)
#include <cerrno>
#include <cassert>

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

/** FilePrivate **/

/**
 * Initialize the FilePrivate private class.
 * @param q File object.
 * @param card Card this file belongs to.
 */
FilePrivate::FilePrivate(File *q, Card *card)
	: q_ptr(q)
	, card(card)
	, mode(0)
	, gcBanner(nullptr)
	, iconAnimMode(0)
	, lostFile(false)
{ }

FilePrivate::~FilePrivate()
{
	// Delete the GcImages.
	delete gcBanner;
	qDeleteAll(gcIcons);
	gcIcons.clear();
}

/**
 * Get the file size, in blocks.
 * @return File size, in blocks.
 */
int FilePrivate::size(void) const
{
	// (old GCN-specific note)
	// NOTE: We're using fatEntries.size() instead of
	// dirEntry.length, since the directory entry may
	// contain invalid data.
	return fatEntries.size();
}

/**
 * Convert a file block number to a physical block number.
 * @param fileBlock File block number.
 * @return Physical block number, or negative on error.
 */
uint16_t FilePrivate::fileBlockAddrToPhysBlockAddr(uint16_t fileBlock) const
{
	if ((int)fileBlock >= fatEntries.size())
		return -1;
	return fatEntries.at((int)fileBlock);
}

/**
 * Load the file data.
 * @return QByteArray with file data, or empty QByteArray on error.
 */
QByteArray FilePrivate::loadFileData(void)
{
	// TODO: Combine with readBlocks()?
	// TODO: Add a generic read() function?
	const int blockSize = card->blockSize();
	if (this->size() > card->totalUserBlocks()) {
		// File is larger than the card.
		// This shouldn't happen...
		return QByteArray();
	}

	QByteArray fileData;
	// FIXME: Optimize blockSize multiplication by using shifts.
	fileData.resize(this->size() * blockSize);

	uint8_t *fileDataPtr = (uint8_t*)fileData.data();
	for (int i = 0; i < this->size(); i++, fileDataPtr += blockSize) {
		const uint16_t physBlockAddr = fileBlockAddrToPhysBlockAddr(i);
		card->readBlock(fileDataPtr, blockSize, physBlockAddr);
	}
	return fileData;
}

/**
 * Read the specified range from the file.
 * @param blockStart First block.
 * @param len Length, in blocks.
 * @return QByteArray with file data, or empty QByteArray on error.
 */
QByteArray FilePrivate::readBlocks(uint16_t blockStart, int len)
{
	// Check if the starting block is valid.
	if (blockStart >= this->size()) {
		// Starting block is larger than the filesize.
		return QByteArray();
	}

	// Check if the length is valid.
	// FIXME: Check for overflow?
	uint16_t blockEnd = (uint16_t)(blockStart + len);
	if (blockEnd > this->size()) {
		// Reading too much data.
		// Truncate it to the available data.
		blockEnd = this->size();
		len = (uint16_t)(this->size() - blockStart);
	}

	const int blockSize = card->blockSize();
	QByteArray blockData;
	blockData.resize(len * blockSize);

	uint8_t *blockDataPtr = (uint8_t*)blockData.data();
	for (int i = 0; i < len; i++, blockDataPtr += blockSize) {
		const uint16_t physBlockAddr = fileBlockAddrToPhysBlockAddr(i);
		card->readBlock(blockDataPtr, blockSize, physBlockAddr);
	}
	return blockData;
}

/**
 * Strip invalid DOS characters from a filename.
 * @param filename Filename.
 * @param replaceChar Replacement character.
 * @return Filename with invalid DOS characters replaced with replaceChar.
 */
QString FilePrivate::StripInvalidDosChars(
				const QString &filename,
				const QChar replaceChar)
{
	QString ret(filename);
	for (int i = (ret.size() - 1); i > 0; i--) {
		QCharRef chr = ret[i];

		// Reference: http://en.wikipedia.org/wiki/8.3_filename#Directory_table
		switch (chr.unicode()) {
			case '"': case '*': case '/': case ':':
			case '<': case '>': case '?': case '\\':
			case '[': case ']': case '|':
				// Invalid DOS character.
				// (Technically, '[' and ']' are legal on Win32,
				//  but we'll exclude them anyway.)
				chr = replaceChar;
				break;
			default:
				// Valid character.
				break;
		}
	}

	// Return the adjusted filename.
	return ret;
}

/**
 * Attempt to decode text as Shift-JIS.
 * If that fails, use cp1252.
 * @param str Text data.
 * @param len Length of str.
 * @return Unicode QString.
 */
QString FilePrivate::decodeText_SJISorCP1252(const char *str, int len)
{
	// Static codec initialization.
	// NOTE: Assuming cp1252 always works.
	static QTextCodec *shiftJis = QTextCodec::codecForName("Shift_JIS");
	static QTextCodec *cp1252 = QTextCodec::codecForName("cp1252");

	// Remove trailing NULL characters before converting to UTF-8.
	for (int i = len-1; i >= 0; i--) {
		if (str[i] == 0) {
			// Found a NULL.
			len--;
		} else {
			// No more NULLs.
			break;
		}
	}

	if (!shiftJis) {
		// Shift-JIS isn't available.
		// Always use cp1252.
		assert(cp1252 != nullptr);
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

/** Images **/

/**
 * Load the banner and icon images.
 * TODO: Move to File?
 */
void FilePrivate::loadImages(void)
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

/** Checksums **/

/**
 * Calculate the file checksum.
 */
void FilePrivate::calculateChecksum(void)
{
	checksumValues.clear();

	if (checksumDefs.empty()) {
		// No checksum definitions were set.
		return;
	}

	// Load the file data.
	QByteArray fileData = loadFileData();
	if (fileData.isEmpty()) {
		// File is empty.
		return;
	}

	// Pointer to fileData's internal data array.
	uint8_t *data = reinterpret_cast<uint8_t*>(fileData.data());

	// Process all of the checksum definitions.
	for (int i = 0; i < (int)checksumDefs.size(); i++) {
		const Checksum::ChecksumDef &checksumDef = checksumDefs.at(i);

		if (checksumDef.algorithm == Checksum::CHKALG_NONE ||
		    checksumDef.algorithm >= Checksum::CHKALG_MAX ||
		    checksumDef.length == 0)
		{
			// No algorithm or invalid algorithm set,
			// or the checksum data has no length.
			continue;
		}

		// Make sure the checksum definition is in range.
		if (fileData.size() < (int)checksumDef.address ||
		    fileData.size() < (int)(checksumDef.start + checksumDef.length))
		{
			// File is too small...
			// TODO: Also check the size of the checksum itself.
			continue;
		}

		// Get the expected checksum.
		// NOTE: Assuming big-endian for all values.
		uint32_t expected = 0;
		Checksum::ChaoGardenChecksumData chaoChk_orig;

		switch (checksumDef.algorithm) {
			case Checksum::CHKALG_CRC16:
			case Checksum::CHKALG_DREAMCASTVMU:
				if (checksumDef.endian != Checksum::CHKENDIAN_LITTLE) {
					// Big-endian.
					expected = (data[checksumDef.address+0] << 8) |
						   (data[checksumDef.address+1]);
				} else {
					// Little-endian.
					expected = (data[checksumDef.address+1] << 8) |
						   (data[checksumDef.address+0]);
				}
				break;

			case Checksum::CHKALG_CRC32:
			case Checksum::CHKALG_ADDINVDUAL16:
			case Checksum::CHKALG_ADDBYTES32:
				if (checksumDef.endian != Checksum::CHKENDIAN_LITTLE) {
					// Big-endian.
					expected = (data[checksumDef.address+0] << 24) |
						   (data[checksumDef.address+1] << 16) |
						   (data[checksumDef.address+2] << 8) |
						   (data[checksumDef.address+3]);
				} else {
					// Little-endian.
					expected = (data[checksumDef.address+3] << 24) |
						   (data[checksumDef.address+2] << 16) |
						   (data[checksumDef.address+1] << 8) |
						   (data[checksumDef.address+0]);
				}
				break;

			case Checksum::CHKALG_SONICCHAOGARDEN: {
				memcpy(&chaoChk_orig, &data[checksumDef.address], sizeof(chaoChk_orig));

				// Temporary working copy.
				Checksum::ChaoGardenChecksumData chaoChk = chaoChk_orig;
				if (checksumDef.endian != Checksum::CHKENDIAN_LITTLE) {
					// Big-endian.
					expected = (chaoChk.checksum_3 << 24) |
						   (chaoChk.checksum_2 << 16) |
						   (chaoChk.checksum_1 << 8) |
						   (chaoChk.checksum_0);
				} else {
					// Little-endian.
					// TODO: Is this correct?
					expected = (chaoChk.checksum_0 << 24) |
						   (chaoChk.checksum_1 << 16) |
						   (chaoChk.checksum_2 << 8) |
						   (chaoChk.checksum_3);
				}

				// Clear some fields that must be 0 when calculating the checksum.
				chaoChk.checksum_3 = 0;
				chaoChk.checksum_2 = 0;
				chaoChk.checksum_1 = 0;
				chaoChk.checksum_0 = 0;
				chaoChk.random_3 = 0;
				memcpy(&data[checksumDef.address], &chaoChk, sizeof(chaoChk));
				break;
			}

			case Checksum::CHKALG_NONE:
			default:
				// Unsupported algorithm.
				expected = 0;
				break;
		}

		// Calculate the checksum.
		const char *const start = (fileData.constData() + checksumDef.start);
		uint32_t actual = Checksum::Exec(checksumDef.algorithm,
				start, checksumDef.length, checksumDef.endian, checksumDef.param);

		if (checksumDef.algorithm == Checksum::CHKALG_SONICCHAOGARDEN) {
			// Restore the Chao Garden checksum data.
			memcpy(&data[checksumDef.address], &chaoChk_orig, sizeof(chaoChk_orig));
		}

		// Save the checksums.
		Checksum::ChecksumValue checksumValue;
		checksumValue.expected = expected;
		checksumValue.actual = actual;
		checksumValues.push_back(checksumValue);
	}
}

/** File **/

/**
 * Create a File for a Card.
 * This Card object is NOT valid by itself, and must
 * be subclassed by a system-specific class. The subclass
 * constructor must then initialize the File, including
 * fatEntries and other properties.
 * @param d FilePrivate-derived private class.
 * @param card Card object.
 */
File::File(FilePrivate *d, Card *card)
	: QObject(card)
	, d_ptr(d)
{ }

File::~File()
{
	Q_D(File);
	delete d;
}

/**
 * Get the internal filename.
 * @return internal filename.
 */
QString File::filename(void) const
{
	Q_D(const File);
	return d->filename;
}

/**
 * Get this file's FAT entries.
 * @return FAT entries.
 */
QVector<uint16_t> File::fatEntries(void) const
{
	Q_D(const File);
	return d->fatEntries;
}

/**
 * Load the file data.
 * @return QByteArray with file data, or empty QByteArray on error.
 */
QByteArray File::loadFileData(void)
{
	Q_D(File);
	return d->loadFileData();
}

/**
 * Get the game ID.
 * @return Game ID
 */
QString File::gameID(void) const
{
	Q_D(const File);
	return d->gameID;
}

/**
 * Get the last modified time.
 * @return Last modified time.
 */
GcnDateTime File::mtime(void) const
{
	Q_D(const File);
	return d->mtime;
}

/**
 * Get the file's description.
 * This is for UI purposes only.
 * Description may contain a '\0' to separate
 * the main description from a sub-description.
 * @return File's description.
 */
QString File::description(void) const
{
	Q_D(const File);
	return d->description;
}

/**
 * Get the file's mode. (attribute, permissions)
 * NOTE: Values may be system-specific.
 * FIXME: Use system-independent values?
 * @return File mode.
 */
uint32_t File::mode(void) const
{
	Q_D(const File);
	return d->mode;
}

/**
 * Get the file size, in blocks.
 * @return File size, in blocks.
 */
int File::size(void) const
{
	Q_D(const File);
	return d->size();
}

/** Icon and banner **/

/**
 * Get the banner image.
 * @return Banner image, or null QPixmap on error.
 */
QPixmap File::banner(void) const
{
	Q_D(const File);
	return d->banner;
}

/**
 * Get the number of icons in the file.
 * @return Number of icons.
 */
int File::iconCount(void) const
{
	Q_D(const File);
	return d->icons.size();
}

/**
 * Get an icon from the file.
 * @param idx Icon number.
 * @return Icon, or null QPixmap on error.
 */
QPixmap File::icon(int idx) const
{
	Q_D(const File);
	if (idx < 0 || idx >= d->icons.size())
		return QPixmap();
	return d->icons.at(idx);
}

/**
 * Get the delay for a given icon.
 * FIXME: Make this use system-independent values.
 * @param idx Icon number.
 * @return Icon delay.
 */
int File::iconDelay(int idx) const
{
	Q_D(const File);
	if (idx < 0 || idx >= d->iconSpeed.size())
		return 0x0;
	return d->iconSpeed.at(idx);
}

/**
 * Get the icon animation mode.
 * FIXME: Use system-independent values.
 * Currently uses GCN values.
 * @return Icon animation mode.
 */
int File::iconAnimMode(void) const
{
	Q_D(const File);
	return (d->iconAnimMode & 0x4);
}

/** Lost File information **/

/**
 * Is this a lost file?
 * @return True if lost; false if file is in the directory table.
 */
bool File::isLostFile(void) const
{
	Q_D(const File);
	return (d->lostFile);
}

/** Export **/

/**
 * Export the file.
 * @param filename Filename for the exported file.
 * @return 0 on success; non-zero on error.
 * TODO: Error code constants.
 */
int File::exportToFile(const QString &filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		// Error opening the file.
		return -1;
	}

	// Write the save data.
	int ret = exportToFile(&file);
	file.close();

	if (ret != 0) {
		// Error exporting the save file.
		file.remove();
	}

	return ret;
}

/** Images **/

/**
 * Save the banner image.
 * @param filenameNoExt Filename for the GCI file, sans extension.
 * @return 0 on success; non-zero on error.
 * TODO: Error code constants.
 */
int File::saveBanner(const QString &filenameNoExt) const
{
	Q_D(const File);
	// TODO: Make GcImageWriter more generic and move the
	// internal image data here.
	if (d->banner.isNull())
		return -EINVAL;

	// Append the correct extension.
	QString filename = filenameNoExt;
	const char *ext = GcImageWriter::extForImageFormat(GcImageWriter::IMGF_PNG);
	if (ext)
		filename += QChar(L'.') + QLatin1String(ext);

	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		// Error opening the file.
		// TODO: Convert QFileError to a POSIX error code.
		return -EIO;
	}

	// Write the banner image.
	int ret = saveBanner(&file);
	file.close();

	if (ret != 0) {
		// Error saving the banner image.
		file.remove();
	}

	return ret;
}

/**
 * Save the banner image.
 * @param qioDevice QIODevice to write the banner image to.
 * @return 0 on success; non-zero on error.
 * TODO: Error code constants.
 */
int File::saveBanner(QIODevice *qioDevice) const
{
	Q_D(const File);
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
int File::saveIcon(const QString &filenameNoExt,
	GcImageWriter::AnimImageFormat animImgf) const
{
	Q_D(const File);
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

/** Checksum **/

/**
 * Get the checksum definitions.
 * @return Checksum definitions.
 */
QVector<Checksum::ChecksumDef> File::checksumDefs(void) const
{
	Q_D(const File);
	return d->checksumDefs;
}

/**
 * Set the checksum definitions.
 * @param checksumDefs Checksum definitions.
 */
void File::setChecksumDefs(const QVector<Checksum::ChecksumDef> &checksumDefs)
{
	Q_D(File);
	d->checksumDefs = checksumDefs;
	d->calculateChecksum();
}

/**
 * Get the checksum values.
 * @return Checksum values, or empty QVector if no checksum definitions were set.
 */
QVector<Checksum::ChecksumValue> File::checksumValues(void) const
{
	Q_D(const File);
	return d->checksumValues;
}

/**
 * Get the checksum algorithm.
 * NOTE: We're assuming each file only uses one algorithm...
 * @return Checksum algorithm.
 */
Checksum::ChkAlgorithm File::checksumAlgorithm(void) const
{
	Q_D(const File);
	if (d->checksumDefs.isEmpty())
		return Checksum::CHKALG_NONE;
	return d->checksumDefs.at(0).algorithm;
}

/**
 * Get the checksum status.
 * @return Checksum status.
 */
Checksum::ChkStatus File::checksumStatus(void) const
{
	Q_D(const File);
	return Checksum::ChecksumStatus(d->checksumValues.toStdVector());
}

/**
 * Format checksum values as HTML for display purposes.
 * @return QVector containing one or two HTML strings.
 * - String 0 contains the actual checksums.
 * - String 1, if present, contains the expected checksums.
 */
QVector<QString> File::checksumValuesFormatted(void) const
{
	Q_D(const File);
	vector<string> vs = Checksum::ChecksumValuesFormatted(d->checksumValues.toStdVector());
	QVector<QString> ret;
	ret.reserve((int)vs.size());
	for (int i = 0; i < (int)vs.size(); i++) {
		ret.append(QString::fromStdString(vs[i]));
	}
	return ret;
}
