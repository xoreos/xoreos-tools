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
 *  Implementing the stream writing interfaces for files.
 */

#include "src/common/writefile.h"
#include "src/common/error.h"
#include "src/common/ustring.h"
#include "src/common/platform.h"

namespace Common {

WriteFile::WriteFile() : _handle(0) {
}

WriteFile::WriteFile(const UString &fileName) : _handle(0) {
	if (!open(fileName))
		throw Exception("Can't open file \"%s\" for writing", fileName.c_str());
}

WriteFile::~WriteFile() {
	try {
		close();
	} catch (...) {
	}
}

bool WriteFile::open(const UString &fileName) {
	close();

	if (fileName.empty())
		return false;

	if (!(_handle = Platform::openFile(fileName, Platform::kFileModeWrite)))
		return false;

	return true;
}

void WriteFile::close() {
	flush();

	if (_handle)
		std::fclose(_handle);

	_handle = 0;
}

bool WriteFile::isOpen() const {
	return _handle != 0;
}

void WriteFile::flush() {
	if (!_handle)
		return;

	if (std::fflush(_handle) != 0)
		throw Exception(kWriteError);
}

size_t WriteFile::write(const void *dataPtr, size_t dataSize) {
	if (!_handle)
		return 0;

	return std::fwrite(dataPtr, 1, dataSize, _handle);
}

} // End of namespace Common
