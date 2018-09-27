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

#ifndef AURORA_XMLFIX_H
#define AURORA_XMLFIX_H

#include <iostream>
#include <string>

#include "src/common/ustring.h"

namespace Common {
	class UString;
	class SeekableReadStream;
	class MemoryWriteStreamDynamic;
}

namespace Aurora {

class XMLFixer {

public:
	static Common::SeekableReadStream *fixXMLStream(Common::SeekableReadStream &in);

private:
	typedef std::vector<Common::UString>	ElementList;
	typedef std::vector<Common::UString>	SegmentList;

	bool isTagClose(const Common::UString value);
	bool isValidXMLHeader(Common::SeekableReadStream &in);
	bool isFixSpecialCase(Common::UString *value);
	void readXMLStream(Common::SeekableReadStream &in, ElementList *elements);
	Common::UString fixXMLElement(const Common::UString element);
	Common::UString fixXMLValue(const Common::UString value);
	Common::UString stripEndQuotes(const Common::UString value);
	Common::UString fixFunction(const Common::UString function, const Common::UString::iterator it);
};

} // End of namespace Aurora

#endif // AURORA_XMLFIX_H
