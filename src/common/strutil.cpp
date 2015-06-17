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
 *  Utility templates and functions for working with strings.
 */

#include <cstdio>

#include "src/common/strutil.h"
#include "src/common/util.h"
#include "src/common/error.h"
#include "src/common/memreadstream.h"
#include "src/common/memwritestream.h"

namespace Common {

void printDataHex(SeekableReadStream &stream) {
	size_t pos  = stream.pos();
	size_t size = stream.size() - pos;

	if (size == 0)
		return;

	uint32 offset = 0;
	byte rowData[16];

	while (size > 0) {
		// At max 16 bytes printed per row
		uint32 n = MIN<size_t>(size, 16);
		if (stream.read(rowData, n) != n)
			throw Exception(kReadError);

		// Print an offset
		std::fprintf(stderr, "%08X  ", offset);

		// 2 "blobs" of each 8 bytes per row
		for (uint32 i = 0; i < 2; i++) {
			for (uint32 j = 0; j < 8; j++) {
				uint32 m = i * 8 + j;

				if (m < n)
					// Print the data
					std::fprintf(stderr, "%02X ", rowData[m]);
				else
					// Last row, data count not aligned to 16
					std::fprintf(stderr, "   ");
			}

			// Separate the blobs by an extra space
			std::fprintf(stderr, " ");
		}

		std::fprintf(stderr, "|");

		// If the data byte is a printable character, print it. If not, substitute a '.'
		for (uint32 i = 0; i < n; i++)
			std::fprintf(stderr, "%c", std::isprint(rowData[i]) ? rowData[i] : '.');

		std::fprintf(stderr, "|\n");

		size   -= n;
		offset += n;
	}

	// Seek back
	stream.seek(pos);
}

void printDataHex(const byte *data, size_t size) {
	if (!data || (size == 0))
		return;

	MemoryReadStream stream(data, size);
	printDataHex(stream);
}

void printStream(SeekableReadStream &stream) {
	uint32 c;
	while ((c = stream.readChar()) != ReadStream::kEOF)
		std::printf("%c", (char) c);
}

void printStream(MemoryWriteStreamDynamic &stream) {
	MemoryReadStream readStream(stream.getData(), stream.size());

	printStream(readStream);
}

static bool tagToString(uint32 tag, bool trim, UString &str) {
	tag = TO_BE_32(tag);

	const char *tS = (const char *) &tag;
	if (!std::isprint(tS[0]) || !std::isprint(tS[1]) || !std::isprint(tS[2]) || !std::isprint(tS[3]))
		return false;

	str = UString::format("%c%c%c%c", tS[0], tS[1], tS[2], tS[3]);
	if (trim)
		str.trim();

	return true;
}

UString tagToString(uint32 tag, bool trim) {
	UString str;
	if (tagToString(tag, trim, str))
		return str;

	return UString::format("0x%08X", FROM_BE_32(tag));
}

UString debugTag(uint32 tag, bool trim) {
	UString str;
	if (tagToString(tag, trim, str))
		return UString::format("0x%08X ('%s')", FROM_BE_32(tag), str.c_str());

	return UString::format("0x%08X", FROM_BE_32(tag));
}

} // End of namespace Common
