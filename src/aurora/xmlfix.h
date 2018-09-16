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
	Common::SeekableReadStream *fixXMLStream(Common::SeekableReadStream &xml, bool hideComments = false);

private:
	int  _comCount = 0;        	// Track the number of open/closed comments
	bool _openTag = false;          // True if a non-comment tag has not been closed on current line
	bool _hideComments = false;     // If true, blank out any comment lines
	bool _fixedCopyright = false;	// Track if the copyright character is fixed
	bool _inUIButton = false; 	// Used to fix </UIButton> tags that were never opened

	int countOccurances(Common::UString line, uint32 find);
	bool isCommentLine(Common::UString line);

	// Line filters
	Common::UString parseLine(Common::UString line);
	Common::UString fixCopyright(Common::UString line);
	Common::UString fixXMLTag(Common::UString line);
	Common::UString fixUnclosedNodes(Common::UString line);
	Common::UString escapeInnerQuotes(Common::UString line);
	Common::UString fixMismatchedParen(Common::UString line);
	Common::UString fixOpenQuotes(Common::UString line);
	Common::UString fixUnevenQuotes(Common::UString line);
	Common::UString fixUnclosedQuote(Common::UString line);
	Common::UString fixCloseBraceQuote(Common::UString line);
	Common::UString commentFix(Common::UString line);
	Common::UString replaceText(Common::UString line, const Common::UString badStr, const Common::UString goodStr);
	Common::UString fixKnownIssues(Common::UString line);
};

} // End of namespace Aurora

#endif // AURORA_XMLFIX_H
