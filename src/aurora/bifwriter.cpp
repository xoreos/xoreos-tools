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

#include "src/aurora/bifwriter.h"
#include "src/aurora/types.h"

static const uint32_t kBIFFID = MKTAG('B', 'I', 'F', 'F');
static const uint32_t kV1ID  = MKTAG('V', '1', ' ', ' ');

namespace Aurora {

BIFWriter::BIFWriter(uint32_t fileCount, Common::SeekableWriteStream &writeStream) :
		_maxFiles(fileCount), _currentFiles(0), _dataOffset(0), _writer(writeStream) {
	// Write id and version.
	writeStream.writeUint32BE(kBIFFID);
	writeStream.writeUint32BE(kV1ID);

	writeStream.writeUint32LE(fileCount);
	writeStream.writeUint32LE(0);
	writeStream.writeUint32LE(20);

	writeStream.writeZeros(fileCount * 16);
}

uint32_t BIFWriter::size() {
	_writer.seek(0, Common::SeekableWriteStream::kOriginEnd);
	return _writer.pos();
}

void BIFWriter::add(Common::SeekableReadStream &data, Aurora::FileType type) {
	if (_currentFiles >= _maxFiles)
		throw Common::Exception("BIFWriter::add() Attempt to write more files than maximum");

	_writer.seek(0, Common::SeekableWriteStream::kOriginEnd);
	size_t fileSize = _writer.writeStream(data);
	data.seek(0);

	_writer.seek(20 + _currentFiles * 16);

	_writer.writeUint32LE(_currentFiles); // Index
	_writer.writeUint32LE(20 + _maxFiles * 16 + _dataOffset); // Data offset
	_writer.writeUint32LE(fileSize); // File size
	_writer.writeUint32LE(type); // Type

	++_currentFiles;
	_dataOffset += fileSize;
}

} // End of namespace Aurora
