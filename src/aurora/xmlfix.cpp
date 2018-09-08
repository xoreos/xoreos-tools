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
* This class converts NWN2 xml code to proper XML code.
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
#include "src/common/ustring.h"
#include "src/aurora/xmlfix.h"

using std::cout;
using std::string;

int comCount = 0; //Track number of open/closed comments.
bool inUIButton = false; //Used to fix </UIButton> tags that are never opened. 

static Common::SeekableReadStream *fixXML(Common::SeekableReadStream *xml) {
	//TODO
	return nullptr;
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		cout << "Please specify an xml file to parse.\n"
			"a file name <fileName>Fixed will be created.\n";
			// TODO: update this when we no longer create new files.
			// Double-check, but I believe we want to modify the
			// original file, not create a new one. 
		return -1;
	}
	//First get our file names	
	Common::UString oldFileName = argv[1];
	Common::UString newFileName = oldFileName;
	Common::UString line; //Each line of XML we parse
	size_t perLoc = oldFileName.find("."); //Location of period in the file name.
										   //There is a period	
	if (perLoc != std::string::npos) {//Add Fixed before the extension
		newFileName.insert(perLoc, "Fixed");
	}
	else {//Add Fixed at the end of the file
		newFileName = newFileName + "Fixed";
	}
	//Then open the actual files. (converted to CStrings so fstream won't complain).
	ifstream readFile(oldFileName.c_str(), ios::in);
	if (!readFile.is_open()) {//We check twice so that we don't create files if garbage is passed in.
		cout << "Error opening files." << endl;
		return -1;
	}
	ofstream writeFile(newFileName.c_str(), ios::out | ios::trunc);//Create or overwrite
	if (!writeFile.is_open()) {
		cout << "Error opening files." << endl;
		return -1;
	}
	//Check for XML style
	if (getline(readFile, line)) {
		if (trim(line).find("<?xml") == 0) {//If we start with an XML formatter
											//Write this line first.
			line = fixXMLTag(line);
			writeFile << line << "\n";
		}
		else {
			cout << "Improper XML file.\n";
			return -1;
		}
	}
	else {
		cout << "Error reading file\n";
		return -1;
	}
	//Then insert the root element.
	writeFile << "<Root>\n";
	while (getline(readFile, line)) {
		countComments(line);
		writeFile << parseLine(line) << "\n";
	}
	writeFile << "</Root>\n";
	readFile.close();
	writeFile.close();
	return 0;
}

/**Read and fix any line of XML that is passed in,
* Returns that fixed line.
*/
Common::UString parseLine(Common::UString line) {
	line = fixUnclosedNodes(line);
	line = escapeSpacedStrings(line, false);//Fix problematic strings (with spaces or special characters)
	line = fixMismatchedParen(line);
	line = fixOpenQuotes(line);//It's imperative that this run before the copyright line, or not on it at all. Could update to ignore comments.
	line = escapeInnerQuotes(line);
	line = fixCopyright(line); //Does this need to run EVERY line? //Worst case improvement, we could have a global variable for whether or not we've found it, and just run it once per file, on the assumption that it will only appear once per file.
	line = doubleDashFix(line);
	line = quotedCloseFix(line);
	line = tripleQuoteFix(line);
	line = escapeSpacedStrings(line, true); //Restore the problematic strings to their proper values.
	return line;
}

/**
* Removes copyright sign, as it is invalid
* Unicode that xmllint doesn't like.
* This probably doesn't need to run on every
* Line.
*/
Common::UString fixCopyright(Common::UString line) {
	//If this is the copyright line, remove the unicode.
	if (line.find("Copyright") != std::string::npos) {
		if (!comCount) {
			return "<!-- Copyright 2006 Obsidian Entertainment, Inc. -->";
		}
		else {//If we're in a comment, don't add a new one.
			return "Copyright 2006 Obsidian Entertainment, Inc.";
		}//This may not be a perfect match (in the else case), but it gets the point across.
	}
	return line;
}

/**Corrects improper opening XML tags.
* An improper XML tag has <xml instead
* Of <?xml.
* Also changes references to NWN2UI to
* XML so xml-lint reads it properly.
* Returns the unmodified line with the
* Proper opening XML tag.
*/
Common::UString fixXMLTag(Common::UString line) {
	//Let's ensure we close this properly.
	if (line.find("<?xml") != string::npos) {
		line = trim(line);
		if (line.at(line.length() - 2) != '?') {
			line.insert(line.length() - 1, "?");
		}//NWN2UI is not a supported format. changing it to xml appears to work.
		if (line.find("encoding=\"NWN2UI\"") != std::string::npos) {
			return "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
		}
	}
	return line;
}

/**
* If there is a close node without an open node
* This will delete it. Right now it only works
* If there is a closed UIButton without an open
* UIButton.
*/
Common::UString fixUnclosedNodes(Common::UString line) {
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
* In the context open("FooBar"), so that's the only
* Case we look for right now.
*/
Common::UString escapeInnerQuotes(Common::UString line) {
	if (countOccurances(line, '"') > 2) {//We have more than 2 quotes in one line
		size_t firstQuotPos = line.find("\""); //The first quotation mark
		size_t lastQuotPos = line.find_last_of("\""); //The last quotation mark
		bool inPar = false;
		for (size_t i = firstQuotPos + 1; i < lastQuotPos - 1; i++) {
			//We're in a parenthetical, all quotes need to be replaced
			//This is not covered by our previous cases if there are 
			//Multiple quoted entries in one set of parens.
			if (line.at(i) == '(') {
				inPar = true;
			}
			else if (line.at(i) == ')') {
				inPar = false;
			}
			else if (inPar && line.at(i) == '"') {
				line.replace(i, 1, "&quot;");
				lastQuotPos = line.find_last_of("\""); //Update string length
			}if (line.at(i) == '(' && line.at(i + 1) == '"') {//Opening paren, encode the quote
				line.replace(i + 1, 1, "&quot;");
				lastQuotPos = line.find_last_of("\""); //Our string changed, last quote should too.
			}
			//If we have a close paren or a comma [as in foo=("elem1",bar)]
			//Closed paren, encode the quote
			else if ((line.at(i) == '"' && line.at(i + 1) == ')') || (line.at(i) == '"' && line.at(i + 1) == ',')) {
				line.replace(i, 1, "&quot;");
				lastQuotPos = line.find_last_of("\""); //Update string length
			}
		}
	}
	return line;
}

/**
* Counts the number of times a character, find,
* Appears in a string, line, and returns that
* Number. //TODO: can we replace this with std::count?
*/
int countOccurances(Common::UString, char find) {
	int count = 0;
	for (size_t i = 0; i < line.length(); i++) {
		if (line[i] == find) {
			count++;
		}
	}
	return count;
}

//Adds a closing paren if a line is missing such a thing.
Common::UString fixMismatchedParen(Common::UString line) {
	bool inParen = false;
	size_t end = line.length();
	for (size_t i = 0; i < end; i++) {
		if (!inParen) {
			if (line[i] == '(') {
				inParen = true;
			}
		}
		else {
			size_t pos = line.find("/>");
			if (line[i] == ')') {
				inParen = false;
			}
			else if (i == pos - 1) {//We're at the end of the string and haven't closed a paren.
				if (line.at(pos - 1) != ')') {
					line.insert(pos, ")");
					break;
				}
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
Common::UString fixOpenQuotes(Common::UString line) {
	//We have an equal with no open quote
	size_t end = line.length() - 1;
	for (size_t i = 0; i < end; i++) {
		//Equal sign should be followed by a quote
		if (line.at(i) == '='&& line.at(i + 1) != '"') {
			line.insert(i + 1, "\"");
			i++;//Our string got longer.
			end++;
		}

		//Open paren should be followed by a &quot; (or an immediate close paren)
		//But if we replace it directly here, it will be doubly escaped
		//Because we run escapeInnerQuotes() next.
		if (line.at(i) == '(' && line.at(i + 1) != '"' && line.at(i + 1) != ')') {
			line.insert(i + 1, "\"");
			end++;
		}

		//A closed quote should be preceeded by &quot; See above.
		//There are some exceptions to this, like when we have one quoted element
		//In a 2 element parenthesis set. This is always a number. ("elem="foo",local=5)
		//Or when we have () empty.
		if (i > 0 && line.at(i) == ')' && line.at(i - 1) != '"' && line.at(i - 1) != '(') {
			line.insert(i, "\"");
			end++;
		}

		//No quote before , add it in.
		if (i > 0 && line.at(i) == ',' && line.at(i - 1) != '"') {
			line.insert(i, "\"");
			end++;
		}

		//No quote after a comma
		if (line.at(i) == ',' && line.at(i + 1) != '"') {
			line.insert(i + 1, "\"");
			end++;

		}

		//A close paren should be followed by a " or a space and a \>
		if (i < end - 1 && line.at(i) == ')'&& line.at(i + 2) != '\\') {
			line.insert(i + 1, "\"");
			i++;//Our string got longer.
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
Common::UString fixUnevenQuotes(Common::UString line) {
	size_t closeBrace = line.find("/>");
	//We don't have a close quote before our close brace
	//Sometimes there is a space after a quote
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
std::string fixUnclosedQuote(std::string line) {
	bool inQuote = false; //Tracks if we are inside a quote
	size_t end = line.length();
	for (size_t i = 0; i < end; i++) {
		if (!inQuote) {
			if (line[i] == '"') {
				inQuote = true;
			}
		}
		else {
			if (line[i] == '"') {//Inquote is true, we're in a quoted part.
				inQuote = false; //This is a close quote
								 //A close quote should be followed by a space.
				if (line.at(i + 1) != ' ' && line.at(i + 1) != '/' && line.at(i + 1) != '"') {
					line.insert(i + 1, " ");
					i++;
					end++;
				}
			}
			else if (isspace(line[i])) {//We can't check for just a space, 
										//because files sometimes also contain newlines.
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
std::string fixCloseBraceQuote(std::string line) {
	bool inQuote = false;
	size_t end = line.length();
	for (size_t i = 0; i < end; i++) {
		if (!inQuote) {
			if (line[i] == '"') {
				inQuote = true;
			}
		}
		else {
			size_t pos = line.find("/>");
			if (line[i] == '"') {//Inquote is true, we're in a quoted part.
				inQuote = false;
			}
			else if (pos != std::string::npos) {
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
* If there are any -- inside of a comment,
* This will remove them and replace it with
* A single dash. Otherwise this breaks
* Compatibility.
*/
std::string doubleDashFix(std::string line) {
	size_t pos = line.find("--");
	//It's not a comment
	if (pos < line.length() - 1 && line.at(pos + 2) != '>' && (pos > 0 && line.at(pos - 1) != '!')) {
		line.erase(pos, 1);//Remove one dash.
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
std::string tripleQuoteFix(std::string line) {
	size_t pos = line.find("\"\"\"");
	if (pos != std::string::npos) {
		line.erase(pos, 2);//Remove two quotes.
	}

	//Might as well escape "" as well, while we're at it.
	if (line.find("name=\"\"") == std::string::npos) {//If the line doesn't contain name=""	
		pos = line.find("\"\"");
		if (pos != std::string::npos) {
			line.erase(pos, 2);//Remove one quote.
		}
	}
	return line;
}

/**
* Some lines contain problematic phrases. (phrases that include
* String literals with spaces or the > or / characters).
* If left untouched, other functions will destroy these strings
* Instead of fixing them.
* Returns a safe string (devoid of problematic phrases) if undo is false, or
* The original string, with problematic phrases restored if undo is true.
*/
std::string escapeSpacedStrings(std::string line, bool undo) {
	//Just used as containers.
	//Might be an easier/cleaner way to do this, perhaps with some sort of map instead of two arrays.
	string switchWordsFrom[] = { "portrait frame" , "0 / 0 MB", "->", ">>",
		"capturemouseevents=false", "Speaker Name", " = ", "Player Chat" };
	string switchWordsTo[] = { "portrait_frame" , "0_/_0_MB", "ReplaceMe1",
		"ReplaceMe2","capturemouseevents=false ", "Speaker_Name", "=", "Player_Chat" };

	//The arrays we actually reference
	string * fromTemp = switchWordsFrom;
	string * toTemp = switchWordsTo;
	//Swap
	//No need to switch the first time, but The second time we
	//Call this, we want to switch from safe to original strings
	if (undo) {
		//Native array swap wasn't introduced until c++ 2011
		//So we do this with pointers.	
		string * swapTemp = fromTemp;
		fromTemp = toTemp;
		toTemp = swapTemp;
	}
	//Number of elements in the array.
	int length = sizeof(switchWordsFrom) / sizeof(switchWordsFrom[0]);
	//Do the actual replacement inline.	
	for (int i = 0; i < length; i++) {
		size_t pos = line.find(*(fromTemp + i));
		if (pos != std::string::npos) {
			line.replace(pos, (fromTemp + i)->length(), *(toTemp + i));
		}
	}
	return line;
}

/**
* Track number of open and closed comments
* Used for tracking copyright.
*/
void countComments(std::string line) {
	if (line.find("<!--") != std::string::npos)
	{
		comCount++;
	}
	if (line.find("-->") != std::string::npos)
	{
		comCount--;
	}
}

//TODO replace this with Common::UString.trim()
/**
* Remove leading and trailing whitespace.
* Returns line without leading and trailing
* Spaces.
*/
std::string trim(std::string line) {
	string whitespace = " \t\n\v\f\r";
	size_t lineBegin = line.find_first_not_of(whitespace);
	if (lineBegin == std::string::npos) {
		return ""; //empty string
	}
	int lineEnd = line.find_last_not_of(whitespace);
	int lineRange = lineEnd - lineBegin + 1;
	return line.substr(lineBegin, lineRange);
}

/**
* If we have a "/>", replace it with a />
* If we have a />", raplce it with a />
* If we have a >", replace it with a ">
* This function is another instance of
* Cleaning up after ourselves.
*/
std::string quotedCloseFix(std::string line) {
	size_t pos = line.find("\"/>\"");
	if (pos != std::string::npos) {
		line.erase(pos, 1);
		line.erase(pos + 2, 1);
	}
	pos = line.find("/>\"");
	if (pos != std::string::npos) {
		line.erase(pos + 2, 1);
	}
	pos = line.find(">\"");
	if (pos != std::string::npos) {
		line.erase(pos + 1, 1);
		line.insert(pos, "\"");
	}
	return line;
}
