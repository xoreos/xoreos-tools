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

#include "src/common/filepath.h"

namespace Common {

UString FilePath::getStem(const UString &p) {
	UString file = getFile(p);

	return UString(file.begin(), file.findLast('.'));
}

UString FilePath::getExtension(const UString &p) {
	return UString(p.findLast('.'), p.end());
}

UString FilePath::changeExtension(const UString &p, const UString &ext) {
	return UString(p.begin(), p.findLast('.')) + ext;
}

UString FilePath::getFile(const UString &p) {
	UString file = p;

	UString::iterator slash = file.findLast('/');
	if (slash != file.end())
		file = UString(++slash, file.end());

	UString::iterator backslash = file.findLast('\\');
	if (backslash != file.end())
		file = UString(++backslash, file.end());

	return file;
}

} // End of namespace Common
