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

#include "src/common/ustring.h"
#include "src/common/encoding.h"
#include "src/common/memreadstream.h"
#include "src/common/memwritestream.h"
#include "src/common/scopedptr.h"
#include "src/common/error.h"
#include "src/common/util.h"
#include "src/aurora/xmlfixer.h"

const Common::Encoding encoding = Common::kEncodingLatin9; // Encoding format for reading NWN2 XML

namespace Aurora {

/**
 * This filter converts the contents of an NWN2 XML data stream 'in'
 * into standardized XML and returned the result as a new data stream.
 */
Common::SeekableReadStream *XMLFixer::fixXMLStream(Common::SeekableReadStream &in) {
	Common::MemoryWriteStreamDynamic out(true, in.size());
	XMLFixer fixer;

	try {
		// Read in the input stream
		fixer.readXMLStream(in);

	} catch (Common::Exception &e) {
		e.add("Failed to fix XML stream");
		throw e;
	}

	// Return the converted stream
	out.setDisposable(false);
	return new Common::MemoryReadStream(out.getData(), out.size(), true);
}

/**
 * Convert the input stream to a vector of elements.
 */
void XMLFixer::readXMLStream(Common::SeekableReadStream &in) {
	// Read in the header
	readXMLHeader(in);
}

/**
 * Read in the header and check the format.
 */
void XMLFixer::readXMLHeader(Common::SeekableReadStream &in) {
	Common::UString line;
	Common::UString header;

	// Set to the stream start
	in.seek(0);

	// Loop until a non-blank line is found
	do {
		// Read a line
		line = Common::readStringLine(in, encoding);

		// Trim white space off the ends
		line.trim();
	} while (line.size() == 0);

	// Check for an XML header line
	Common::UString::iterator it = line.findFirst("<?xml");
	if (it == line.end())
		throw Common::Exception("Input stream does not have an XML header");

	// Extract header string
	header = line.substr(it, line.end());
}

} // End of namespace AURORA
