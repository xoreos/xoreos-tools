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
 *  Encoding <-> Encoding name mappings.
 */

#ifndef COMMON_ENCODING_STRINGS_H
#define COMMON_ENCODING_STRINGS_H

namespace Common {

struct EncodingStrings {
	Encoding encoding;

	const char *strings[10];
};

static const EncodingStrings kEncodingStrings[] = {
	{ kEncodingASCII,   { "ascii", "ansi", 0 } },
	{ kEncodingUTF8,    { "utf-8", "utf8", 0 } },
	{ kEncodingUTF16LE, { "utf-16le", "utf16le", "utf-16", "utf16", 0 } },
	{ kEncodingUTF16BE, { "utf-16be", "utf16be", 0 } },
	{ kEncodingLatin9,  { "latin9", "latin-9", "8859-15", "iso-8859-15", "iso8859-15", "iso885915", 0 } },
	{ kEncodingCP1250,  { "cp-1250", "cp1250", "windows-1250", "windows1250", 0 } },
	{ kEncodingCP1251,  { "cp-1251", "cp1251", "windows-1251", "windows1251", 0 } },
	{ kEncodingCP1252,  { "cp-1252", "cp1252", "windows-1252", "windows1252", 0 } },
	{ kEncodingCP932,   { "cp-932", "cp932", "windows-932", "windows932",
	                      "shift-jis", "shift_jis", "shiftjis", 0 } },
	{ kEncodingCP936,   {  "cp-936", "cp936", "windows-936", "windows936", 0 } },
	{ kEncodingCP949,   { "cp-949", "cp949", "windows-949", "windows949", 0 } },
	{ kEncodingCP950,   { "cp-950", "cp950", "windows-950", "windows950", "cp-1370", "cp1370", "big5", 0 } }
};

} // End of namespace COMMON

#endif // COMMON_ENCODING_STRINGS_H
