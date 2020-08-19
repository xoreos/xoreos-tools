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

#include <ctime>

#include "src/common/filepath.h"
#include "src/common/encoding.h"

#include "src/aurora/keywriter.h"
#include "src/aurora/util.h"

static const uint32_t kKEYID = MKTAG('K', 'E', 'Y', ' ');
static const uint32_t kV1ID  = MKTAG('V', '1', ' ', ' ');

namespace Aurora {

KEYWriter::KEYWriter() {

}

void KEYWriter::addBIF(const Common::UString &fileName, const std::list<Common::UString> &files, uint32_t size) {
	_entries.emplace_back();

	_entries.back().fileName = fileName;
	_entries.back().files = files;
	_entries.back().fileSize = size;
}

void KEYWriter::write(Common::WriteStream &writeStream) {
	writeStream.writeUint32BE(kKEYID);
	writeStream.writeUint32BE(kV1ID);

	// Number of BIF/BZF files, this KEY file controls
	writeStream.writeUint32LE(_entries.size());

	uint32_t totalCount = 0;
	uint32_t totalFilenameSize = 0;
	for (const auto &entry : _entries) {
		totalCount += entry.files.size();
		totalFilenameSize += entry.fileName.size();
	}

	// Number of resources in all BIF/BZF files linked to this file
	writeStream.writeUint32LE(totalCount);

	// Constant offset to the file table
	writeStream.writeUint32LE(64);

	// Calculate the key table offset.
	uint32_t keyTableOffset = 64;
	keyTableOffset += _entries.size() * 12;
	keyTableOffset += totalFilenameSize;

	// Offset to the keytable
	writeStream.writeUint32LE(keyTableOffset);

	// Write the creation time of the file.
	std::time_t now = std::time(0);
	std::tm *timepoint = std::localtime(&now);
	writeStream.writeUint32LE(timepoint->tm_year);
	writeStream.writeUint32LE(timepoint->tm_yday);

	// Reserved padding
	writeStream.writeZeros(32);

	// Write file table.
	uint32_t filenameOffset = 64 + _entries.size() * 12;
	for (const auto &entry : _entries) {
		writeStream.writeUint32LE(entry.fileSize);
		writeStream.writeUint32LE(filenameOffset);
		writeStream.writeUint16LE(entry.fileName.size());
		writeStream.writeUint16LE(0); // Drive Letter

		filenameOffset += entry.fileName.size();
	}

	// Write file name table.
	for (const auto &entry : _entries) {
		Common::writeString(writeStream, entry.fileName, Common::kEncodingASCII, false);
	}

	// Write Key table.
	uint32_t x = 0, y = 0;
	for (const auto &entry : _entries) {
		y = 0;
		for (const auto &file : entry.files) {
			Common::writeStringFixed(writeStream, Common::FilePath::getStem(file), Common::kEncodingASCII, 16);
			writeStream.writeUint16LE(TypeMan.getFileType(file));
			writeStream.writeUint32LE((x << 20) + y);
			++y;
		}
		++x;
	}
}

} // End of namespace Aurora
