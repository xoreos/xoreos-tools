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

#include "src/common/encoding.h"

#include "src/aurora/rimwriter.h"

static const uint32_t kRIMID     = MKTAG('R', 'I', 'M', ' ');
static const uint32_t kVersion1  = MKTAG('V', '1', '.', '0');

namespace Aurora {

RIMWriter::RIMWriter(uint32_t fileCount, Common::SeekableWriteStream &stream) :
	_fileCount(fileCount), _currentFileCount(0), _stream(stream) {

	// Write magic id
	_stream.writeUint32BE(kRIMID);

	// Write version
	_stream.writeUint32BE(kVersion1);

	// Write reserved data
	_stream.writeZeros(4);

	// Write number of files
	_stream.writeUint32LE(fileCount);

	// Write constant offset to resource table
	_stream.writeUint32LE(120);

	// Write reserved header data
	_stream.writeZeros(104);

	// Reserve space for the resource table
	_stream.writeZeros(fileCount * 32);

	// Save offsets
	_offsetToResourceData = 120 + fileCount * 32;
	_offsetToResourceTable = 120;
}

void RIMWriter::add(const Common::UString &resRef, FileType resType, Common::ReadStream &stream) {
	if (_currentFileCount >= _fileCount)
		throw Common::Exception("RIMWriter::add() exceeded file count");

	// Write resource data
	_stream.seek(_offsetToResourceData);

	uint32_t size = _stream.writeStream(stream);

	// Write resource table entry
	_stream.seek(_offsetToResourceTable);

	Common::writeStringFixed(_stream, resRef, Common::kEncodingASCII, 16);
	_stream.writeUint16LE(resType);
	_stream.writeZeros(2);
	_stream.writeUint32LE(_currentFileCount);
	_stream.writeUint32LE(_offsetToResourceData);
	_stream.writeUint32LE(size);

	_offsetToResourceData += size;
	_offsetToResourceTable += 32;

	_currentFileCount += 1;
}

} // End of namespace Aurora
