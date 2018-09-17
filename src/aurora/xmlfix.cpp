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

/**
* This class converts NWN2 xml code to standard XML code.
* It fixes unescaped special characters, root elements,
* mismatched nodes, unclosed parentheses, and unclosed
* quotes.
*/

#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <stdio.h>
#include <ctype.h>
#include "src/aurora/xmlfix.h"
#include "src/common/ustring.h"
#include "src/common/encoding.h"
#include "src/common/memreadstream.h"
#include "src/common/memwritestream.h"
#include "src/common/scopedptr.h"
#include "src/common/error.h"
#include "src/common/util.h"
using namespace Aurora;

const uint32 quote_mark = '\"';

/**
 * This filter converts the contents of an NWN2 XML data
 * stream into standardized XML.
 */
Common::SeekableReadStream *XMLFix::fixXMLStream(Common::SeekableReadStream &xml, bool hideComments) {
	Common::UString line, buffer;
	
	// Initialize the internal tracking variables
	_hideComments = hideComments;
	_comCount = 0;
	_openTag = false;
	_fixedCopyright = false;
	_inUIButton = false;
	
	// Loop until a non-blank line is found
	xml.seek(0);
	do {
		line = Common::readStringLine(xml, Common::kEncodingLatin9);
		line.trim();
	} while (line.size() == 0);

	/*
	 * Check for a standard header line.
	 * The input encoding is set to Common::kEncodingLatin9
	 * so it doesn't throw an error on the copyright symbol.
	 */
	Common::UString::iterator pos = line.findFirst("<?xml");
	if (pos == line.end()) 
		throw Common::Exception("Input stream does not have an XML header");

	// Create the output stream
	Common::MemoryWriteStreamDynamic out;
	out.reserve(xml.size());

	try {
		xml.seek(0);
		// Loop until a non-blank line is found
		do {
			line = Common::readStringLine(xml, Common::kEncodingLatin9);
			line.trim();
		} while (line.size() == 0);
		
		// Fix the header
		line = fixXMLTag(line) + "\n";
		out.write(line.c_str(), line.size());

		// Insert root element
		const Common::UString startRoot = "<Root>\n";
		out.write(startRoot.c_str(), startRoot.size());

		// Cycle through the remaining input stream
		while (!xml.eos()) {
			// Track the previous state
			bool priorTag = _openTag;

			// Read a line of text
			line = Common::readStringLine(xml, Common::kEncodingLatin9);

			// Trim now for maximum performance benefit
			if (priorTag || _hideComments) {
				// Compacting, or already in a wrap, so trim both ends
				line.trim();
			} else {
				// Preseve the indent by only trimming the right end
				line.trimRight();
			}

			// Fix the XML format
			if (line.size() > 0)
				line = XMLFix::parseLine(line);
			
			if (_openTag) {
				// This is a multi-line wrap
				if (!priorTag) {
					// Starting a new buffer
					buffer = line;
				} else if (line.size() > 0) {
					// Append line to the buffer with a space
					buffer += " " + line;
				}
			} else {
				// Check for a multi-line wrap
				if (buffer.size() > 0) {
					// Finish wrapping the lines
					line = buffer + " " + line;
					buffer = "";
				}
				
				// Unless we are both hiding comments and the line length is zero
				if (!(_hideComments && line.size() == 0)) {
					// Write line to the output stream
					line += "\n"; // <CR> stripped by readStringLine
					out.write(line.c_str(), line.size());
				}

				// Initialize for the next line
				priorTag = false;
			}
		}

		if (_openTag) {
			// Print the final line and buffer
			line = buffer + " " + line;
			line += "\n"; // Stripped by readStringLine
			out.write(line.c_str(), line.size());
		}

		// Insert end root element
		const Common::UString endRoot = "</Root>\n";
		out.write(endRoot.c_str(), endRoot.size());

	} catch (Common::Exception &e) {
		e.add("Failed to fix XML stream");
		throw e;
	}
	
	out.setDisposable(false);
	return new Common::MemoryReadStream(out.getData(), out.size(), true);
}

/**
* Read and fix any line of XML that is passed in.
* Returns that fixed line.
*/
Common::UString XMLFix::parseLine(Common::UString line) {
	// Call isCommentLine() to track comment lines and blocks
	if (isCommentLine(line)) {
		if (_hideComments) {
			// Comments are not wanted, so blank out the line
			line = "";
		} else {
			// These calls are only needed for a comment
			if (!_fixedCopyright)
				line = fixCopyright(line);
			line = commentFix(line);
		}	
	} else {
		// Split off an appended comment (per fontfamily.xml)
		Common::UString comment = "";
		Common::UString::iterator it = line.findFirst("<!--");
		if (it != line.end()) {
			size_t pos = line.getPosition(it);
			comment = line.substr(it, line.end());
			if (pos > 0) {
				// Remove the comment
				it = line.getPosition(pos - 1);
				line = line.substr(line.begin(), it);
			} else {
				// This should never happen
				throw Common::Exception("Comment found where none expected: %s", line.c_str());
			}
		}

		// Fix a non-comment line
		line = fixKnownIssues(line);
		line = fixUnclosedNodes(line);
		line = fixMismatchedParen(line);
		line = fixOpenQuotes(line);
		line = escapeInnerQuotes(line);

		if (!_hideComments && comment.size() > 0) {
			// Restore appended comment
			line += " " + comment;
		}
	} 
	
	return line;
}

/**
* Removes copyright sign, as it is invalid Unicode that xmllint
* doesn't like. This probably doesn't need to run on every line.
*/
Common::UString XMLFix::fixCopyright(Common::UString line) {
	//If this is the copyright line, remove the unicode.
	Common::UString::iterator startPos = line.findFirst("Copyright");
	if (startPos != line.end()) {
		Common::UString::iterator endPos = line.findFirst("Obsidian");
		size_t pos = line.getPosition(startPos);
		
		// Remove the copyright character
		line.erase(startPos, endPos);
		
		// Regenerate iterator to avoid throwing an exception
		startPos = line.getPosition(pos);
		
		// Insert the replacement
		line.insert(startPos, "Copyright &copy; 2006 ");

		// Flag as fixed
		_fixedCopyright = true;
	}
	return line;
}

/**
* Corrects improper opening XML tags.
* An improper XML tag has '<xml' instead of '<?xml'.
* Also changes references to NWN2UI encoding to utf-8
* so xml-lint reads it properly.
*/
Common::UString XMLFix::fixXMLTag(Common::UString line) {
	// Check for an xml tag
	Common::UString::iterator pos = line.findFirst("<?xml");
	if (pos != line.end()) {
		line.trim();

		// Fix for SlimGUI fontFamily.xml
		pos = line.findFirst("<?xml"); // Regenerate with changed string
		if (line.getPosition(pos) > 0) {
			line.erase(line.begin(), pos);
		}

		// Ensure we close this properly.
		int n = (int)line.size() - 2;
		if (n > -1 && line.at(n) != '?') {
			pos = line.getPosition(n+1);
			line.insert(pos, '?');
		}

		// Check for unsupported encoding format NWN2UI
		const Common::UString badEncoding = "encoding=\"NWN2UI\"";
		const Common::UString goodEncoding = "encoding=\"utf-8\"";
		line = replaceText( line, badEncoding, goodEncoding );
	}
	return line;
}

/**
* If there is a close node without an open node
* This will delete it. Right now it only works
* if there is a close UIButton without a prior
* open UIButton.
*/
Common::UString XMLFix::fixUnclosedNodes(Common::UString line) {
	const Common::UString startButton = "<UIButton";
	const Common::UString endButton   = "</UIButton>";
	
	// Open node	
	Common::UString::iterator pos = line.findFirst(startButton);
	if (pos != line.end()) {
		_inUIButton = true;
	}
	
	// Close node	
	pos = line.findFirst(endButton);
	if (pos != line.end()) {
		// If we aren't in a node, delete the close node.
		if (!_inUIButton) {
			line = replaceText(line, endButton, "");
		}
		_inUIButton = false;
	}
	return line;
}

/**
* Finds and escapes quotes in an element by changing
* them to an HTML tag. Returns a fixed line.
* The only time we're seeing faulty quotes is
* in the context open("FooBar"), so that's the only
* case we look for right now.
*/
Common::UString XMLFix::escapeInnerQuotes(Common::UString line) {
	Common::UString::iterator pos;

	if (countOccurances(line, quote_mark) > 2) {
		// We have more than 2 quotes in one line
		size_t firstQuotPos = line.getPosition(line.findFirst(quote_mark)); // First quote mark
		size_t lastQuotPos = line.getPosition(line.findLast(quote_mark));   // Last quote mark
		bool inPar = false;
		for (size_t i = firstQuotPos + 1; i < lastQuotPos - 1; i++) {
			// For a parenthetical, all quotes need to be replaced
			// This is not covered by our previous cases if there are 
			// multiple quoted entries in one set of parens.
			uint32 c = line.at(i);
			if (c == '(') {
				inPar = true;
			} else if (c == ')') {
				inPar = false;
			} else if (inPar && c == quote_mark) {
				pos = line.getPosition(i);
				line.erase(pos);
				pos = line.getPosition(i); // Avoid error throw
				line.insert(pos, "&quot;");
				lastQuotPos = line.getPosition(line.findLast(quote_mark)); // Updated string length
			}
			
			c = line.at(i); // May have changed
			uint32 d = line.at(i + 1);
			if (c == '(' && d == quote_mark) {
				// Opening paren, encode the quote
				pos = line.getPosition(i + 1);
				line.erase(pos);
				pos = line.getPosition(i + 1); // Avoid error throw
				line.insert(pos, "&quot;");
				lastQuotPos = line.getPosition(line.findLast(quote_mark)); // Updated string length
			} else if ((c == quote_mark && d == ')') || (c == quote_mark && d == ',')) {
				// Found a close paren or a comma [as in foo=("elem1",bar)], so encode the quote
				pos = line.getPosition(i);
				line.erase(pos);
				pos = line.getPosition(i); // Avoid error throw
				line.insert(pos, "&quot;");
				lastQuotPos = line.getPosition(line.findLast(quote_mark)); // Updated string length
			}
		}
	}
	return line;
}

/**
* Counts the number of times a character 'find'
* appears in a USstring 'line' and returns that number.
*/
int XMLFix::countOccurances(Common::UString line, uint32 find) {
	int count = 0;
	for (size_t i = 0; i < line.size(); i++) {
		if (line.at(i) == find) {
			count++;
		}
	}
	return count;
}

/*
 * Adds a closing paren if a line is missing such a thing.
 */
Common::UString XMLFix::fixMismatchedParen(Common::UString line) {
	bool inParen = false;
	size_t end = line.size();
	Common::UString::iterator it = line.findFirst("/>");
	size_t pos = line.getPosition(it);
	for (size_t i = 0; i < end; i++) {
		uint32 c = line.at(i);
		if (!inParen) {
			if (c == '(') {
				inParen = true;
			}
		} else if (c == ')') {
			inParen = false;
		} else if (i == pos - 1) {
			// We're at the end of the string and haven't closed a paren.
			if (c != ')') {
				if (c == ' ' || c == '>') {
					// Stick it before the space
					it = line.getPosition(i);
				} else {
					// Stick it before the closing tag
					it = line.getPosition(i + 1);
				}
				line.insert(it, ')');
				break;
			}
		}
	}
	return line;
}

/*
* Find any element that has an equal sign not followed
* By a quotation mark. Insert that quotation mark,
* and return the fixed line.
*/
Common::UString XMLFix::fixOpenQuotes(Common::UString line) {
	Common::UString::iterator pos;
	int quoteCount = 0; // Count quote marks
	uint32 c;

	// We have an equal with no open quote
	for (size_t i = 0; i < line.size(); i++) {
		if (line.at(i) == quote_mark)
			quoteCount++;

		if (line.at(i) == ')') {
			// A close paren should be followed by: "
			if (i < line.size() - 1 && line.at(i + 1) != quote_mark) {
				pos = line.getPosition(i + 1);
				line.insert(pos, quote_mark);
			}	

			/* 
			 * A closed paren should usually be preceeded by: "
			 *
			 * There are some exceptions to this:
			 *  - when we have one quoted element in a 2 element
			 *    parenthesis set. This is always a number.
			 *    Example: ("elem="foo",local=5)
			 *  - when we have () empty.
			 *  - when the paren is preceded by a bare comma (1 case)
			 */
			if (i > 0) {
				c = line.at(i - 1);
				if (c != quote_mark && c != '(' && c != ',') {
					pos = line.getPosition(i);
					line.insert(pos, quote_mark);
					quoteCount++; // Add quote

					// Skip forward to avoid extra quotes
					i++;
				}
			}
			
		}

		// Close the quotes for an equals
		if (quoteCount % 2 && i > 0) {
			c = line.at(i);
			bool needQuote = false;
			if (c == ' ' || c == '\t' || c == '/') {
				/*
				 * Look ahead for a non-whitespace character
				 * then look for an equals, comma, or '>'.
				 */
				for (size_t j = i + 1; j < line.size(); j++) {
					uint32 d = line.at(j);
					if (needQuote) {
						// Check following non-space character
						if (d == '=' || d == ',' || d == '>') {
							// Past prior assignment
							break;
						} else if (d == ' ' || d == '\t' || d == quote_mark) {
							// Another space or closing quotes
							needQuote = false;
							break;
						}
					} else if (d != ' ' && d != '\t') {
						// Found a non-space character following the space
						needQuote = true;
					}
				}
			} else if (c == '>') {
				needQuote = true;
			}

			// Check if a quote was added earlier
			if (needQuote && line.at(i - 1) != quote_mark) {
				pos = line.getPosition(i);
				line.insert(pos, quote_mark);
				quoteCount++; // Add quote
			}
		}

		// Equal sign should be followed by a quote
		if (line.at(i) == '=' && i < line.size() - 1 && line.at(i + 1) != quote_mark) {
			pos = line.getPosition(i + 1);
			line.insert(pos, quote_mark);
		}

		/*
		 * Open paren should be followed by a &quot; (or an immediate close paren)
		 * But if we replace it directly here, it will be doubly escaped
		 * because we run escapeInnerQuotes() next.
		 */
		if (line.at(i) == '(' && i < line.size() - 1 && line.at(i + 1) != quote_mark && line.at(i + 1) != ')') {
			pos = line.getPosition(i + 1);
			line.insert(pos, quote_mark);
		}

		// No quote before ',', so add it in.
		if (line.at(i) == ',' && i > 0 && line.at(i - 1) != quote_mark) {
			pos = line.getPosition(i);
			line.insert(pos, quote_mark);
			quoteCount++; // Add quote
		}

		// No quote after a comma, so add it in unless there's a paren (1 case)
		if (line.at(i) == ',' && i < line.size() - 1 && line.at(i + 1) != quote_mark && line.at(i + 1) != ')') {
			pos = line.getPosition(i + 1);
			line.insert(pos, quote_mark);
		}

		// Check for a space or slash inside quotes
		c = line.at(i);
		if (quoteCount % 2 && (line.isSpace(c) || c == '/')) {
			if (i < line.size() - 1) {
				// Read forward to look for an equals
				for (size_t j = i + 1; j < line.size(); j++) {
					uint32 d = line.at(j);
					if (d == '=' || c == ',' || c == '>') {
						break;
					} else if (d == quote_mark) {
						// Replace space character with an HTML ASCII tag
						Common::UString::iterator it = line.getPosition(i);
						line.erase(it);
						it = line.getPosition(i); // Avoid error throw
						Common::UString s = line.format("&#%02d;", (int)c);
						line.insert(it, s);
						break;
					}
				}
			}
		}
		
		// Track open and closed tags
		if ((quoteCount % 2) == 0) {
			// We're not in quotes
			c = line.at(i);
			if (c == '<') {
				_openTag = true;
			} else if (c == '>') {
				_openTag = false;
			}
		}
	}

	// Check for an open equals at the end of the line
	if ((quoteCount > 0) && (quoteCount % 2 != 0)) {
		line.insert(line.end(), quote_mark);
	}

	line = fixCloseBraceQuote(line);
	line = fixUnevenQuotes(line);

	return line;
}

/**
* If a close brace exists (not a comment),
* there isn't a close quote, AND we have an
* odd number of quotes.
*/
Common::UString XMLFix::fixUnevenQuotes(Common::UString line) {
	Common::UString::iterator pos = line.findFirst("/>");
	size_t closeBrace = line.getPosition(pos);

	// We don't have a close quote before our close brace
	// Sometimes there is a space after a quote
	if (pos != line.end() && pos != line.begin() &&
		(line.at(closeBrace - 1) != quote_mark || line.at(closeBrace - 2) != quote_mark) &&
		countOccurances(line, quote_mark) % 2) {
		line.insert(pos, quote_mark);
	}
	return line;
}

/**
* After all of this, if we can iterate through a string
* and find a quote followed by a whitespace character, insert a quote.
* Preconditions are such that this should never occur naturally at this
* point in the code, and if we do end up adding too many, it will be
* removed in a later function (such as fixTripleQuote())
*/
Common::UString XMLFix::fixUnclosedQuote(Common::UString line) {
	Common::UString::iterator pos;
	bool inQuote = false; // Tracks if we are inside a quote
	size_t end = line.size();

	for (size_t i = 0; i < end; i++) {
		uint32 c = line.at(i);
		if (!inQuote) {
			if (c == quote_mark) {
				inQuote = true;
			}
		} else {
			if (c == quote_mark) {   // Inquote is true, we're in a quoted part.
				inQuote = false; // This is a close quote
						 // A close quote should be followed by a space.
				uint32 d = line.at(i + 1);
				if (d != ' ' && d != '/' && d != quote_mark) {
					pos = line.getPosition(i + 1);
					line.insert(pos, ' ');
					i++;
					end++;
				}
			} else if (line.isSpace(c)) { // We can't check for just a space, 
						      // because files sometimes also contain newlines.
				pos = line.getPosition(i);
				line.insert(pos, quote_mark);
				i++;
				end++;
				inQuote = false;
			}
		}
	}
	return line;
}

/**
* Another close brace fix. If we're in a quote and we don't
* have a close quote and we see a />, we add a close quote.
*/
Common::UString XMLFix::fixCloseBraceQuote(Common::UString line) {
	// Look for a tag close
	Common::UString::iterator pos = line.findFirst("/>");
	if (pos != line.end()) {
		size_t end = line.getPosition(pos);
		bool inQuote = false;

		// Track the open/close state of the quotes
		for (size_t i = 0; i < end; i++) {
			uint32 c = line.at(i);
			if (c == quote_mark) {
				// Flip the quote state
				inQuote = !inQuote;
			}
		}

		// Check for an open quote at the end
		if (inQuote) {
			// Look for a space before the "/>"
			if (line.at(end - 1) == ' ') {
				pos = line.getPosition(end - 1);
			}
			
			// Insert a close quote
			line.insert(pos, quote_mark);
		}
	}
	return line;
}

/**
* If there are any '--' inside of a comment,
* this will remove them and replace it with
* a single dash. Otherwise this breaks
* compatibility.
*/
Common::UString XMLFix::commentFix(Common::UString line) {
	Common::UString::iterator first = line.findFirst("--");

	// Does the line have a '--'?
	if (first != line.end()) {
		size_t pos = line.getPosition(first);

		// It's not a comment
		if (pos < line.size() - 1 && line.at(pos + 2) != '>' && (pos > 0 && line.at(pos - 1) != '!')) {
			// Remove one dash
			Common::UString::iterator it = line.getPosition(pos);
			line.erase(it);
		}
	}

	// Look for a non utf-8 character
	for (size_t i = 0; i < line.size(); i++) {
		uint32 c = line.at(i);
		
		// Not a full utf-8 check, but it works for the stock files
		if (c & 0x8000) {
			// Discard this character and recheck position
			Common::UString::iterator it = line.getPosition(i--);
			line.erase(it);
		}
	}
	return line;
}

/**
 * Change any instances of badStr to goodStr in line.
 * If undo is true, change instances of badStr to goodStr.
 */
Common::UString XMLFix::replaceText(Common::UString line,
		const Common::UString badStr, const Common::UString goodStr) {
	int limit = (int)line.size(); // Failsafe countdown
	
	// Look for an instance of badStr
	Common::UString::iterator startPos = line.findFirst(badStr);
	while (startPos != line.end()) {
		size_t pos = line.getPosition(startPos);
		size_t width = badStr.size();
		Common::UString::iterator endPos = line.getPosition(pos + width);
		
		// Replace badStr with goodStr
		line.erase(startPos, endPos);
		startPos = line.getPosition(pos); // Avoid throwing error
		line.insert(startPos, goodStr);

		// Look for another instance
		startPos = line.findFirst(badStr);

		// Check failsafe
		if (--limit < 0)
			throw Common::Exception("Stuck in a loop searching for '%s'", badStr.c_str());
	}
	return line;
}

/**
* Fix known issues with the stock XML files.
*/
Common::UString XMLFix::fixKnownIssues(Common::UString line) {
	// Initialize the array of token/substr pairs
	const int rows = 6;
	const Common::UString pair[rows][2] = {			// Example file:
		{ "=true fontFamily=", "=truefontfamily=" },	//  examine.xml
		{ "=181357", "=\"181357\"\"" },			//  gfx_options.xml
		{ ",ALIGN_LEFT)", ",ALIGN_LEFT\")" },		//  ig_chargen_abilities.xml
		{ "cter\" fontfamily=", "cter\"fontfamily=" },	//  multiplayer_downloadsx2.xml
		{ "=\"-&#62;\"", "=\"->\"" },                   //  gamespydetails.xml
		{ "\"&#62;&#62;\"", "\">>\"" },                 //  internetbrowser.xml
	};

	// Loop through the array
	for (int i = 0; i < rows; i++) {
		// Fix the substr
		line = replaceText(line, pair[i][1], pair[i][0] );
	}

	return line;
}

/**
* Track number of open and closed HTML comments, one per line
*/
bool XMLFix::isCommentLine(Common::UString line) {
	Common::UString::iterator pos, tag;
	bool isComment = false;

	// Check for a line with no tag
	tag = line.findFirst("<");
	if (_comCount < 1 && tag == line.end())
		return false;

	// Start of a comment
	pos = line.findFirst("<!--");
	if (pos != line.end()) {
		// Check for an appended inline comment, per fontfamily.xml
		tag = line.findFirst(">");
		if (tag != line.end() && line.getPosition(tag) < line.getPosition(pos)) {
			/*
			 * Found a close tag '>' before the start of a comment.
			 * Appended comments aren't counted.
			 */
			return false;
		} else {
			_comCount++;
		}
	}
	if (_comCount > 0)
		isComment = true;

	// End of a comment
	pos = line.findFirst("-->");
	if (pos != line.end()) {
		_comCount--;

		// Should never be negative
		if (_comCount < 0)
			throw Common::Exception("Invalid closing comment tag in XML");
	}
	return isComment;
}
