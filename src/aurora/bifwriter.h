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
 * A writer for BIF archive files.
 */

#ifndef AURORA_BIFWRITER_H
#define AURORA_BIFWRITER_H

#include "src/common/writestream.h"
#include "src/common/readstream.h"
#include "src/common/types.h"

#include "src/aurora/keydatawriter.h"
#include "src/aurora/types.h"

namespace Aurora {

/**
 * The purpose of this class is to write a BIF file containing
 * every data added by add().
 */
class BIFWriter : public KEYDataWriter {
public:
	/**
	 * Create a new BIF writer reserving place for fileCount files.
	 * @param fileCount the count of files to reserve
	 * @param writeStream the stream to write to
	 */
	BIFWriter(uint32_t fileCount, Common::SeekableWriteStream &writeStream);

	/**
	 * Add new data by stream to this BIF file and write also offset,
	 * size and type.
	 * @param data the data to add to this archive
	 * @param type the file type of the given data
	 */
	void add(Common::SeekableReadStream &data, Aurora::FileType type);

	/**
	 * The current total size of this file, needed for the KEY file.
	 * @return the current total size of this file
	 */
	uint32_t size();

private:
	const uint32_t _maxFiles;
	uint32_t _currentFiles;
	uint32_t _dataOffset;
	Common::SeekableWriteStream &_writer;
};

} // End of namespace Aurora

#endif // AURORA_BIFWRITER_H
