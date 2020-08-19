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

/** @file
 *  Writing BioWare's RIM files.
 */

#ifndef AURORA_RIMWRITER_H
#define AURORA_RIMWRITER_H

#include "src/common/writestream.h"

#include "src/aurora/types.h"

namespace Aurora {

class RIMWriter {
public:
	/** Create a new RIM writer.
	 *
	 *  @param fileCount the number of files to pack in this RIM file.
	 *  @param stream the stream to write this RIM file to.
	 */
	RIMWriter(uint32_t fileCount, Common::SeekableWriteStream &stream);

	/** Add a new stream to this archive to be packed. */
	void add(const Common::UString &resRef, FileType resType, Common::ReadStream &stream);

private:
	const uint32_t _fileCount;
	uint32_t _currentFileCount;

	uint32_t _offsetToResourceTable;
	uint32_t _offsetToResourceData;

	Common::SeekableWriteStream &_stream;
};

} // End of namespace Aurora

#endif // AURORA_RIMWRITER_H
