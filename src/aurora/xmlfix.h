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
 *  Fix broken, non-standard XML files.
 */

#ifndef AURORA_XMLFIX_H
#define AURORA_XMLFIX_H

#include <iostream>
#include <string>

#include "src/common/ustring.h"

namespace Common {
	class Ustring;
	class SeekableReadStream;
}

namespace Aurora {

class XMLFix { 

public:
	Common::SeekableReadStream *fixXMLStream(Common::SeekableReadStream *xml);

	// Temporarily public for testing because they're called from fix2xml
	Common::UString fixXMLTag(Common::UString line);
	Common::UString parseLine(Common::UString line);

private:
	int comCount = 0;        // Track the number of open/closed comments
	bool inUIButton = false; // Used to fix </UIButton> tags that were never opened

	void replaceAll(Common::UString& str, const Common::UString& from, const Common::UString& to);
	int countOccurances(Common::UString line, uint32 find);
	void countComments(Common::UString line);

	// Line filters
	Common::UString fixLine(Common::UString line);
	Common::UString trim(Common::UString line);
	Common::UString fixOpenQuotes(Common::UString line);
	Common::UString escapeInnerQuotes(Common::UString line);
	Common::UString replaceString(Common::UString& origStr, Common::UString& oldText, Common::UString newText);
	Common::UString fixCopyright(Common::UString line);
	Common::UString doubleDashFix(Common::UString line);
	Common::UString tripleQuoteFix(Common::UString line);
	Common::UString quotedCloseFix(Common::UString line);
	Common::UString escapeSpacedStrings(Common::UString line, bool undo);
	Common::UString fixMismatchedParen(Common::UString line);
	Common::UString fixCloseBraceQuote(Common::UString line);
	Common::UString fixUnclosedQuote(Common::UString line);
	Common::UString fixUnevenQuotes(Common::UString line);
	Common::UString fixUnclosedNodes(Common::UString line);
};

} // End of namespace Aurora

#endif // AURORA_XMLFIX_H