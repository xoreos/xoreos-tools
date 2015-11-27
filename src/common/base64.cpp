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

#include "src/common/base64.h"
#include "src/common/ustring.h"
#include "src/common/error.h"
#include "src/common/readstream.h"

namespace Common {

static const char kBase64Char[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/** Write a character into our base64 string, and update the remaining
 *  string length.
 *
 *  Returns false if we ran out of remaining characters in this string.
 */
static bool writeCharacter(UString &base64, uint32 c, size_t &maxLength) {
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

	uint8 n;
	byte input[3];

	// Read up to 3 characters at a time
	while ((n = data.read(input, 3)) != 0) {
		uint32 code = 0;

		// Concat the input characters
		for (uint8 i = 0; i < n; i++)
			code |= input[i] << (24 - i * 8);

		// Create up to 4 6-bit base64-characters out of them
		for (uint8 i = 0; i < (n + 1); i++) {
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

} // End of namespace Common
