/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * FileComments.hpp: File comments class                                   *
 *                                                                         *
 * Copyright (c) 2012-2013 by David Korth.                                 *
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

#ifndef __MCRECOVER_FILECOMMENTS_HPP__
#define __MCRECOVER_FILECOMMENTS_HPP__

// Qt includes.
#include <QtCore/QMetaType>
#include <QtCore/QString>
#include <QtCore/QDataStream>

class FileComments
{
	public:
		FileComments() { };
		FileComments(QString gameDesc, QString fileDesc)
			: m_gameDesc(gameDesc)
			, m_fileDesc(fileDesc)
			{ }

		/**
		 * Get the Game Description field.
		 * @return Game Description field.
		 */
		QString gameDesc(void) const;

		/**
		 * Set the Game Description field.
		 * @param gameDesc Game Description field.
		 */
		void setGameDesc(QString gameDesc);

		/**
		 * Get the File Description field.
		 * @return File Description field.
		 */
		QString fileDesc(void) const;

		/**
		 * Set the File Description field.
		 * @param fileDesc File Description field.
		 */
		void setFileDesc(QString fileDesc);

		/**
		 * Convert the file comments to a single string.
		 * @return File comments as a single string.
		 */
		QString toString(void) const;

	private:
		QString m_gameDesc;
		QString m_fileDesc;
};

Q_DECLARE_METATYPE(FileComments)

inline QString FileComments::gameDesc(void) const
	{ return m_gameDesc; }
inline void FileComments::setGameDesc(QString gameDesc)
	{ m_gameDesc = gameDesc; }

inline QString FileComments::fileDesc(void) const
	{ return m_fileDesc; }
inline void FileComments::setFileDesc(QString fileDesc)
	{ m_fileDesc = fileDesc; }

#endif /* __MCRECOVER_FILECOMMENTS_HPP__ */
