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
 *  Base64 encoding and decoding.
 */

#include <cassert>

#include <memory>

#include "src/common/base64.h"
#include "src/common/ustring.h"
#include "src/common/error.h"
#include "src/common/readstream.h"
#include "src/common/memreadstream.h"
#include "src/common/memwritestream.h"

namespace Common {

static const char kBase64Char[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const uint8_t kBase64Values[128] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xFF, 0xFF, 0x3F,
	0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
	0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
	0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/** Write a character into our base64 string, and update the remaining
 *  string length.
 *
 *  Returns false if we ran out of remaining characters in this string.
 */
static bool writeCharacter(UString &base64, uint32_t c, size_t &maxLength) {
	assert(maxLength > 0);

	base64 += c;

	return --maxLength > 0;
}

/** Write multiple characters into our base64 string, and update the remaining
 *  string length.
 *
 *  Returns false if we ran out of remaining characters in this string.
 */
static bool writeCharacters(UString &base64, UString &str, size_t &lineLength) {
	while (!str.empty()) {
		writeCharacter(base64, *str.begin(), lineLength);
		str.erase(str.begin());

		if (lineLength == 0)
			return false;
	}

	return lineLength > 0;
}

/** Find the raw value of a base64-encoded character. */
static uint8_t findCharacterValue(uint32_t c) {
	if ((c >= 128) || (kBase64Values[c] > 0x3F))
		throw Exception("Invalid base64 character");

	return kBase64Values[c];
}

/** Encode data into base64 and write the result into the string, but only up
 *  to maxLength characters.
 *
 *  The string overhang is and input/output string of both the overhang from
 *  the previous run of this function (which will get written into the base64
 *  string first) and the newly produced overhang.
 *
 *  Returns false if we have written all data there is to write, both from the
 *  overhang and the input data stream.
 */
static bool encodeBase64(ReadStream &data, UString &base64, size_t maxLength, UString &overhang) {
	if (maxLength == 0)
		throw Exception("Invalid base64 max line length");

	// First write the overhang, and return if we already maxed out the length there
	if (!writeCharacters(base64, overhang, maxLength))
		return true;

	overhang.clear();

	uint8_t n;
	byte input[3];

	// Read up to 3 characters at a time
	while ((n = data.read(input, 3)) != 0) {
		uint32_t code = 0;

		// Concat the input characters
		for (uint8_t i = 0; i < n; i++)
			code |= input[i] << (24 - i * 8);

		// Create up to 4 6-bit base64-characters out of them
		for (uint8_t i = 0; i < (n + 1); i++) {
			overhang += kBase64Char[(code >> 26) & 0x0000003F];
			code <<= 6;
		}

		// Add padding
		for (int i = 0; i < (3 - n); i++)
			overhang += '=';

		// Write the base64 characters into the string, and return if we maxed out the length
		if (!writeCharacters(base64, overhang, maxLength))
			return true;

		overhang.clear();
	}

	// We reached the end of input the data
	return false;
}

static void decodeBase64(WriteStream &data, const UString &base64, UString &overhang) {
	assert(overhang.size() < 4);

	for (UString::iterator c = base64.begin(); c != base64.end(); ++c) {
		if ((*c != '=') && ((*c >= 128) || (kBase64Values[*c] > 0x3F)))
			continue;

		overhang += *c;

		if (overhang.size() == 4) {
			uint32_t code = 0;

			uint8_t n = 0;
			for (UString::iterator o = overhang.begin(); o != overhang.end(); ++o) {
				code <<= 6;

				if (*o != '=') {
					code += findCharacterValue(*o);
					n    += 6;
				}
			}

			for (size_t i = 0; i < (n / 8); i++, code <<= 8)
				data.writeByte((byte) ((code & 0x00FF0000) >> 16));

			overhang.clear();
		}
	}
}

static size_t countLength(const UString &str, bool partial = false) {
	size_t dataLength = 0;
	for (const auto &c : str) {
		if ((c == '=') || ((c < 128) && kBase64Values[c] <= 0x3F))
			++dataLength;
	}

	if (!partial)
		if ((dataLength % 4) != 0)
			throw Exception("Invalid length for a base64-encoded string");

	return dataLength;
}

static size_t countLength(const std::list<UString> &str) {
	size_t dataLength = 0;
	for (const auto &s : str)
		dataLength += countLength(s, true);

	if ((dataLength % 4) != 0)
		throw Exception("Invalid length for a base64-encoded string");

	return dataLength;
}


void encodeBase64(ReadStream &data, UString &base64) {
	UString overhang;

	encodeBase64(data, base64, SIZE_MAX, overhang);
}

void encodeBase64(ReadStream &data, std::list<UString> &base64, size_t lineLength) {
	UString overhang;

	// Base64-encode the data, creating a new string after every lineLength characters
	do {
		base64.push_back(UString());
	} while (encodeBase64(data, base64.back(), lineLength, overhang));

	// Trim empty strings from the back
	while (!base64.empty() && base64.back().empty())
		base64.pop_back();
}

SeekableReadStream *decodeBase64(const UString &base64) {
	const size_t dataLength = (countLength(base64) / 4) * 3;
	std::unique_ptr<byte[]> data = std::make_unique<byte[]>(dataLength);

	MemoryWriteStream output(data.get(), dataLength);

	UString overhang;
	decodeBase64(output, base64, overhang);

	return new MemoryReadStream(data.release(), output.pos(), true);
}

SeekableReadStream *decodeBase64(const std::list<UString> &base64) {
	const size_t dataLength = (countLength(base64) / 4) * 3;
	std::unique_ptr<byte[]> data = std::make_unique<byte[]>(dataLength);

	MemoryWriteStream output(data.get(), dataLength);

	UString overhang;

	for (std::list<UString>::const_iterator b = base64.begin(); b != base64.end(); ++b)
		decodeBase64(output, *b, overhang);

	return new MemoryReadStream(data.release(), output.pos(), true);
}

} // End of namespace Common
