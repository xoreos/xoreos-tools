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
 * A writer for BZF (lzma compressed BIF) archive files
 */

#ifndef AURORA_BZFWRITER_H
#define AURORA_BZFWRITER_H

#include "src/common/writestream.h"

#include "src/aurora/keydatawriter.h"

namespace Aurora {

/**
 * This class handles the writing of
 */
class BZFWriter : public KEYDataWriter {
public:
	BZFWriter(uint32_t fileCount, Common::SeekableWriteStream &writeStream);

	void add(Common::SeekableReadStream &data, Aurora::FileType type);

	uint32_t size();

private:
	const uint32_t _maxFiles;
	uint32_t _currentFiles;
	uint32_t _dataOffset;
	Common::SeekableWriteStream &_writer;
};

} // End of namespace Aurora

#endif // AURORA_BZFWRITER_H
