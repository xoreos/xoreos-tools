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
 *  Implementing the reading stream interface for stdin.
 */

#include <cstdio>

#include "src/common/stdinstream.h"

namespace Common {

StdInStream::StdInStream() {
}

StdInStream::~StdInStream() {
}

bool StdInStream::eos() const {
	return std::feof(stdin);
}

size_t StdInStream::read(void *dataPtr, size_t dataSize) {
	return std::fread(dataPtr, 1, dataSize, stdin);
}

} // End of namespace Common
