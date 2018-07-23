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
 *  Archive tools utility functions.
 */

#ifndef ARCHIVES_UTIL_H
#define ARCHIVES_UTIL_H

#include <set>

#include "src/common/ustring.h"

#include "src/aurora/types.h"

namespace Aurora {
	class Archive;

	class KEYFile;
	class NSBTXFile;
}

namespace Archives {

/** List all files found in this archive on stdout.
 *
 *  @param archive The archive to list the contents of.
 *  @param game The game to alias types with.
 *  @param directories Print directories? If false, directories will be stripped.
 */
void listFiles(const Aurora::Archive &archive, Aurora::GameID game, bool directories);

/** List all files found in a KEY on stdout. */
void listFiles(const Aurora::KEYFile &key, const Common::UString &keyName, Aurora::GameID game);
/** List the images found in an NSBTX file on stdout. */
void listFiles(const Aurora::NSBTXFile &nsbtx);

/** Extract files from an archive.
 *
 *  @param archive The archive to extract from.
 *  @param game The game to alias types with.
 *  @param directories Create directories? If false, directories will be stripped and the file
 *         will be written directly into the current directory.
 *  @param files A list of files to extract. If empty, all files from the archive will be
 *         extracted.
 */
void extractFiles(const Aurora::Archive &archive, Aurora::GameID game, bool directories,
                  const std::set<Common::UString> &files);

/** Extract files from an NSBTX. */
void extractFiles(const Aurora::NSBTXFile &nsbtx, const std::set<Common::UString> &files,
                  void (*dumper)(Common::SeekableReadStream &stream, const Common::UString &fileName));

std::set<Common::UString> fixPathSeparator(const std::set<Common::UString> &files);

} // End of namespace Archives

#endif // ARCHIVES_UTIL_H
