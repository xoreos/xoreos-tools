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
 *  Utility class for manipulating file paths.
 */

#ifndef COMMON_FILEPATH_H
#define COMMON_FILEPATH_H

#include "src/common/types.h"
#include "src/common/ustring.h"

namespace Common {

/** Utility class for manipulating file paths. */
class FilePath {
public:
	/** Return a file name's stem.
	 *
	 *  Example: "/path/to/file.ext" -> "file"
	 *
	 *  @param  p The path to manipulate.
	 *  @return The path's stem.
	 */
	static UString getStem(const UString &p);

	/** Return a file name's extension.
	 *
	 *  Example: "/path/to/file.ext" -> ".ext"
	 *
	 *  @param  p The path to manipulate.
	 *  @return The path's extension.
	 */
	static UString getExtension(const UString &p);

	/** Change a file name's extension.
	 *
	 *  Example: "/path/to/file.ext", ".bar" -> "/path/to/file.bar"
	 *
	 *  @param  p The path to manipulate.
	 *  @param  ext The path's new extension.
	 *  @return The new path.
	 */
	static UString changeExtension(const UString &p, const UString &ext = "");

	/** Return a file name from a path.
	 *
	 *  Example: "/path/to/file.ext" -> "file.ext"
	 *
	 *  @param  p The path to manipulate.
	 *  @return The path's file.
	 */
	static UString getFile(const UString &p);
};

} // End of namespace Common

#endif // COMMON_FILEPATH_H
