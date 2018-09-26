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
 *  Fix broken, non-standard NWN2 XML files.
 */

#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <stdio.h>
#include <ctype.h>

#include "src/common/memreadstream.h"
#include "src/common/memwritestream.h"
#include "src/common/error.h"
#include "src/common/util.h"
#include "src/aurora/xmlfixer.h"

namespace Aurora {

/**
 * This filter converts the contents of an NWN2 XML data stream 'in'
 * into standardized XML and returned the result as a new data stream.
 */
Common::SeekableReadStream *XMLFixer::fixXMLStream(Common::SeekableReadStream &in) {
	Common::MemoryWriteStreamDynamic out(true, in.size());

	try {
	} catch (Common::Exception &e) {
		e.add("Failed to fix XML stream");
		throw e;
	}

	// Return the converted stream
	out.setDisposable(false);
	return new Common::MemoryReadStream(out.getData(), out.size(), true);
}

}

