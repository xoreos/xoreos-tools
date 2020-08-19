/* xoreos-tools - Tools to help with xoreos development
 *
 * xoreos-tools is the legal property of its developers, whose names
 * can be found in the AUTHORS file distributed with this source
 * distribution.
 *
 * xoreos-tools is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * xoreos-tools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xoreos-tools. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 * A writer for KEY index files files.
 */

#ifndef AURORA_KEYWRITER_H
#define AURORA_KEYWRITER_H

#include <list>

#include "src/common/ustring.h"
#include "src/common/writestream.h"

namespace Aurora {

/**
 * This class handles the writing of KEY files, which store the name
 * of resource contained inside BIF/BZF files and associated BIF/BZF files.
 */
class KEYWriter {
public:
	KEYWriter();

	/**
	 * Add a reference to a specific BIF/BZF file to this KEY file.
	 * @param fileName the filename of the specific BIF/BZF file
	 * @param files the files contained in this BIF/BZF file
	 * @param size total size of the bif file
	 */
	void addBIF(const Common::UString &fileName, const std::list<Common::UString> &files, uint32_t size);

	/**
	 * Write the collected information as KEY file to the specified.
	 * WriteStream
	 * @param writeStream The stream to write to
	 */
	void write(Common::WriteStream &writeStream);

private:
	struct Entry {
		Common::UString fileName;
		std::list<Common::UString> files;
		uint32_t fileSize;
	};

	std::list<Entry> _entries;
};

} // End of namespace Aurora

#endif // AURORA_KEYWRITER_H
