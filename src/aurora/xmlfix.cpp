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

/*
 * TODO:
 * - Merge in revisions to UString.h
 * - Only run fixCopyright once?
 * - Check for other NWN2 XML issues
 * - Convert quotedCloseFix to use Common::UString
 * - Replace countOccurances with count()?
 * - Update fixOpenQuotes to ignore comments?
 * - Optimize for performance
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
using namespace Aurora;

using std::string;

/**
 * This filter converts the contents of an NWN2 XML data
 * stream into standardized XML.
 */
Common::SeekableReadStream *XMLFix::fixXMLStream(Common::SeekableReadStream &xml) {
	Common::UString line;

	// Initialize the internal tracking variables
	comCount = 0;
	fixedXML = false;
	fixedCopyright = false;
	inUIButton = false;
	
	/*
	 * Check for a standard header line.
	 * The input encoding is set to Common::kEncodingLatin9
	 * so it doesn't throw an error on the copyright symbol.
	 */
	xml.seek(0);
	line = Common::readStringLine(xml, Common::kEncodingLatin9);
	Common::UString::iterator pos = line.findFirst("<?xml");
	if (pos == line.end()) 
		throw Common::Exception("Input stream does not have an XML header");

	// Create the output stream
	Common::MemoryWriteStreamDynamic out;
	out.reserve(xml.size());

	try {
		// Insert root element
		const Common::UString startRoot = "<Root>\n";
		out.write(startRoot.c_str(), startRoot.size());

		// Cycle through the input stream
		xml.seek(0);
		while (!xml.eos()) {
			// Read a line of text
			line = Common::readStringLine(xml, Common::kEncodingLatin9);

			// Fix the XML format
			if (line.size() > 0)
				line = XMLFix::parseLine(line);
			line += "\n"; // Stripped by readStringLine

			// Copy to the output stream
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
	if (!fixedXML)
		line = fixXMLTag(line);
	line = fixUnclosedNodes(line);
	line = escapeSpacedStrings(line, false); // Fix problematic strings (with spaces or special characters)
	line = fixMismatchedParen(line);
	line = fixOpenQuotes(line); // It's imperative that this run before the copyright line, or not on it at
	                            // all. Could update to ignore comments.
	line = escapeInnerQuotes(line);
	if (!fixedCopyright)
		line = fixCopyright(line);
	line = doubleDashFix(line);
	line = quotedCloseFix(line);
	line = tripleQuoteFix(line);
	line = escapeSpacedStrings(line, true); //Restore the problematic strings to their proper values.
	countComments(line);
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
		fixedCopyright = true;
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
		// Ensure we close this properly.
		line.trim();
		if (line.at(line.size() - 2) != '?') {
			pos = line.getPosition(line.size() - 1);
			line.insert(pos, '?');
		}

		// Check for unsupported encoding format NWN2UI
		const Common::UString badEncoding = "encoding=\"NWN2UI\"";
		const Common::UString goodEncoding = "encoding=\"utf-8\"";
		line = replaceText( line, badEncoding, goodEncoding );

		// Flag as fixed
		fixedXML = true;
	}
	return line;
}

/**
* If there is a close node without an open node
* This will delete it. Right now it only works
* If there is a closed UIButton without an open
* UIButton.
*/
Common::UString XMLFix::fixUnclosedNodes(Common::UString line) {
	size_t pos = line.find("<UIButton");
	//Open node	
	if (pos != string::npos) {
		inUIButton = true;
	}
	pos = line.find("</UIButton>");
	//Close node	
	if (pos != string::npos) {
		//If we aren't in a node, delete the close node.
		if (!inUIButton) {
			line.replace(pos, 11, "");
		}
		inUIButton = false;
	}
	return line;
}

/**
* Finds and escapes quotes in an element,
* Returns a fixed line.
* The only time we're seeing faulty quotes is
* in the context open("FooBar"), so that's the only
* case we look for right now.
*/
Common::UString XMLFix::escapeInnerQuotes(Common::UString line) {
	const uint32 quote_mark = '\"';
	if (countOccurances(line, quote_mark) > 2) {//We have more than 2 quotes in one line
		size_t firstQuotPos = line.find("\""); //The first quotation mark
		size_t lastQuotPos = line.find_last_of(quote_mark); //The last quotation mark
		bool inPar = false;
		for (size_t i = firstQuotPos + 1; i < lastQuotPos - 1; i++) {
			// For a parenthetical, all quotes need to be replaced
			// This is not covered by our previous cases if there are 
			// multiple quoted entries in one set of parens.
			uint32 c = line.at(1);
			if (c == '(') {
				inPar = true;
			} else if (c == ')') {
				inPar = false;
			} else if (inPar && c == '"') {
				line.replace(i, 1, "&quot;");
				lastQuotPos = line.find_last_of(quote_mark); // Update string length
			}
			
			c = line.at(i); // May have changed
			uint32 d = line.at(i + 1);
			if (c == '(' && d == '"') {
				// Opening paren, encode the quote
				line.replace(i + 1, 1, "&quot;");
				lastQuotPos = line.find_last_of(quote_mark); // Our string changed, last quote should too.
			} else if ((c == '"' && d == ')') || (c == '"' && d == ',')) {
				// Found a close paren or a comma [as in foo=("elem1",bar)], so encode the quote
				line.replace(i, 1, "&quot;");
				lastQuotPos = line.find_last_of(quote_mark); // Update string length
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
				line.insert(pos, ")");
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
	// We have an equal with no open quote
	size_t end = line.size() - 1;
	for (size_t i = 0; i < end; i++) {
		// Equal sign should be followed by a quote
		if (line.at(1) == '=' && line.at(i + 1) != '"') {
			line.insert(i + 1, "\"");
			i++; // Our string got longer.
			end++;
		}

		// Open paren should be followed by a &quot; (or an immediate close paren)
		// But if we replace it directly here, it will be doubly escaped
		// because we run escapeInnerQuotes() next.
		if (line.at(i) == '(' && line.at(i + 1) != '"' && line.at(i + 1) != ')') {
			line.insert(i + 1, "\"");
			end++;
		}

		// A closed quote should be preceeded by &quot; See above.
		// There are some exceptions to this, like when we have one quoted element
		// in a 2 element parenthesis set. This is always a number. ("elem="foo",local=5)
		// or when we have () empty.
		if (i > 0 && line.at(i) == ')' && line.at(i - 1) != '"' && line.at(i - 1) != '(') {
			line.insert(i, "\"");
			end++;
		}

		// No quote before , add it in.
		if (i > 0 && line.at(i) == ',' && line.at(i - 1) != '"') {
			line.insert(i, "\"");
			end++;
		}

		// No quote after a comma
		if (line.at(i) == ',' && line.at(i + 1) != '"') {
			line.insert(i + 1, "\"");
			end++;

		}

		// A close paren should be followed by a " or a space and a \>
		if (i < end - 1 && line.at(i) == ')' && line.at(i + 2) != '\\') {
			line.insert(i + 1, "\"");
			i++; // Our string got longer.
			end++;
		}
	}

	line = fixCloseBraceQuote(line);
	line = fixUnclosedQuote(line);
	line = fixUnevenQuotes(line);

	return line;
}

/**
* If a close brace exists (not a comment),
* there isn't a close quote, AND we have an
* odd number of quotes.
*/
Common::UString XMLFix::fixUnevenQuotes(Common::UString line) {
	size_t closeBrace = line.find("/>");
	// We don't have a close quote before our close brace
	// Sometimes there is a space after a quote
	if (closeBrace != string::npos && closeBrace > 0 &&
		(line.at(closeBrace - 1) != '\"' || line.at(closeBrace - 2) != '\"') &&
		countOccurances(line, '"') % 2) {
		line.insert(closeBrace, "\"");
	}
	return line;
}

/**
* After all of this, if we can iterate through a string
* And find a quote followed by a whitespace character, insert a quote.
* Preconditions are such that this should never occur naturally at this
* Point in the code, and if we do end up adding too many, it will be
* Removed in a later function (such as fixTripleQuote())
*/
Common::UString XMLFix::fixUnclosedQuote(Common::UString line) {
	bool inQuote = false; // Tracks if we are inside a quote
	size_t end = line.length();
	for (size_t i = 0; i < end; i++) {
		uint32 c = line.at(i);
		if (!inQuote) {
			if (c == '"') {
				inQuote = true;
			}
		} else {
			// Inquote is true, we're in a quoted part.
			if (c == '"') {
				// This is a close quote
				inQuote = false;

				// A close quote should be followed by a space.
				uint32 d = line.at(i + 1);
				if (d != ' ' && d != '/' && d != '"') {
					line.insert(i + 1, " ");
					i++;
					end++;
				}
			} else if (Common::UString::isSpace(c)) {
				// We can't check for just a space, because files
				// sometimes also contain newlines.
				line.insert(i, "\"");
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
	bool inQuote = false;
	size_t end = line.length();
	for (size_t i = 0; i < end; i++) {
		uint32 c = line.at(i);
		if (!inQuote) {
			if (c == '"') {
				inQuote = true;
			}
		} else {
			size_t pos = line.find("/>");
			if (c == '"') { // Inquote is true, we're in a quoted part.
				inQuote = false;
			} else if (pos != std::string::npos) {
				if (line.at(pos - 1) != '"') {
					line.insert(pos, "\"");
					break;
				}
			}
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
Common::UString XMLFix::doubleDashFix(Common::UString line) {
	size_t pos = line.find("--");

	// It's not a comment
	if (pos < line.length() - 1 && line.at(pos + 2) != '>' && (pos > 0 && line.at(pos - 1) != '!')) {
		// Remove one dash
		line.erase(pos, 1);
	}
	return line;
}

/**
* If there are three consecutive quotes,
* Replace with one quote.
* Let's be honest, this can only happen as a
* Result of other methods, and this is a kludgy fix.
* Note that this will only find one per line.
* This will also remove double quotes that are not
* Intended to be in the XML. name="" is the only
* Legitimate appearance of "" in the NWN xml code.
* Returns the modified line, without double or
* Triple quotes.
*/
Common::UString XMLFix::tripleQuoteFix(Common::UString line) {
	size_t pos = line.find("\"\"\"");
	if (pos != std::string::npos) {
		line.erase(pos, 2); //Remove two quotes.
	}

	// Might as well escape "" as well, while we're at it.
	if (line.find("name=\"\"") == std::string::npos) {
		// The line doesn't contain name=""	
		pos = line.find("\"\"");
		if (pos != std::string::npos) {
			line.erase(pos, 2);// Remove one quote.
		}
	} return line;
}

/*
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
		
		// Remove the badStr instance
		line.erase(startPos, endPos);
		
		// Regenerate iterator to avoid throwing an exception
		startPos = line.getPosition(pos);
		
		// Replace badStr with goodStr
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
* Some lines contain problematic phrases. (phrases that include
* string literals with spaces or the > or / or , characters).
* If left untouched, other functions will destroy these strings
* instead of fixing them.
*
* Returns a safe string (devoid of problematic phrases) if undo is false, or
* the original string, with problematic phrases restored if undo is true.
*/
Common::UString XMLFix::escapeSpacedStrings(Common::UString line, bool undo) {
	// Initialize the array of token/phrase pairs
	const int rows = 9;
	const Common::UString pair[rows][2] = {
		{ "$01$", "portrait frame" },
		{ "$02$", "0 / 0 MB" },
		{ "$03$", "->" },
		{ "$04$", ">>" },
		{ "$05$", "capturemouseevents=false" },
		{ "$06$", "Speaker Name" },
		{ "$07$", " = " },
		{ "$08$", "Player Chat" },
		{ "$09$", "Entertainment, Inc" }
	};
	
	// Loop through the array
	for (int i = 0; i < rows; i++) {
		if (undo) {
			// Change the token instances back to the key phrase
			line = replaceText(line, pair[i][0], pair[i][1] );
		} else {
			// Change the key phrase instances to the token
			line = replaceText(line, pair[i][1], pair[i][0] );
		}
	}
	return line;
}

/**
* Track number of open and closed comments
* Used for tracking copyright.
*/
void XMLFix::countComments(Common::UString line) {
	if (line.find("<!--") != std::string::npos) {
		comCount++;
	}
	if (line.find("-->") != std::string::npos) {
		comCount--;
	}
}

/**
* If we have a "/>", replace it with a />
* If we have a />", replace it with a />
* If we have a >", replace it with a ">
* This function is another instance of
* cleaning up after ourselves.
*/
Common::UString XMLFix::quotedCloseFix(Common::UString line) {
	// For now, just use std::string calls
	std::string sline = line.c_str();
	size_t pos = sline.find("\"/>\"");
	if (pos != std::string::npos) {
		sline.erase(pos, 1);
		sline.erase(pos + 2, 1);
	}
	pos = sline.find("/>\"");
	if (pos != std::string::npos) {
		sline.erase(pos + 2, 1);
	}
	pos = sline.find(">\"");
	if (pos != std::string::npos) {
		sline.erase(pos + 1, 1);
		sline.insert(pos, "\"");
	}
	return (Common::UString)sline;
}
