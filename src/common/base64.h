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
 *
 *  Base64 encodes binary data into printable ASCII, by representing data in
 *  in radix 64. It maps each 3 input bytes onto 4 printable characters.
 *
 *  This implements the most commonly found Base64 variant, as also used by,
 *  for example, MIME. It uses, in order, the 26 uppercase Latin letters (A-Z),
 *  the 26 lowercase Latin letters (a-z), the 10 Hindu-Arabic digits (0-9),
 *  the plus symbol (+) and the forward slash (/). When the input length is
 *  *not* divisible by 3, and therefore less than 4 output characters would be
 *  generated, the output is padded with one or two equal signs (=).
 */

#ifndef COMMON_BASE64_H
#define COMMON_BASE64_H

#include <list>

#include "src/common/types.h"

namespace Common {

class UString;
class ReadStream;
class SeekableReadStream;

/** Encode the binary stream data into a Base64 string. */
void encodeBase64(ReadStream &data, UString &base64);
/** Encode the binary stream data into a list of Base64 strings of at max lineLength characters. */
void encodeBase64(ReadStream &data, std::list<UString> &base64, size_t lineLength);

/** Decode the Base64 string into binary data, returning a newly allocated stream. */
SeekableReadStream *decodeBase64(const UString &base64);
/** Decode the list of Base64 strings into binary data, returning a newly allocated stream. */
SeekableReadStream *decodeBase64(const std::list<UString> &base64);

} // End of namespace Common

#endif // COMMON_BASE64_H
