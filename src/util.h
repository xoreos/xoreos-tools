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
 *  General tool utility functions.
 */

#ifndef UTIL_H
#define UTIL_H

namespace Common {
	class UString;
	class ReadStream;
	class SeekableReadStream;
	class WriteStream;
}

void initPlatform();

void dumpStream(Common::SeekableReadStream &stream, const Common::UString &fileName);

/** Does this filename represent a stdin/stdout stream? */
bool isFileStd(const Common::UString &file);

Common::WriteStream *openFileOrStdOut(const Common::UString &file);
Common::ReadStream  *openFileOrStdIn (const Common::UString &file);

#endif // UTIL_H
