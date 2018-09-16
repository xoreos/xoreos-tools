
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
 * Command-line tool to fix broken, non-standard NWN2 XML files.
 */

/*
 * TODO:
 * Use XMLFix::fixXMLStream for XML corrections
 * Remove need for 'trim' call
 * Throw exceptions from file processing
 * Pass output file path as an (optional) argument
 * Use the common parser
 * Standardize per project practices
 */

#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <stdio.h>
#include <ctype.h>
#include "src/common/ustring.h"
#include "src/aurora/xmlfix.h"
// using namespace Aurora;

using std::cout;
using std::endl;
using std::string;

// Prototypes
int fixXMLFile(Common::UString &inFile);
std::string strim(std::string line);

int main(int argc, char** argv) {
	if (argc != 2) {
		cout << "Please specify an xml file to parse.\n"
			"a file name <fileName>Fixed will be created.\n";
		return -1;
	}

	Common::UString inFile = argv[1];

	int rCode = fixXMLFile( inFile );

	return rCode;
}

/*
 * Read in the input file, apply XML format corections, then write to output file.
 */
int fixXMLFile(Common::UString &inFile) {
	// Get the i/o file names	
	std::string oldFileName = inFile.c_str();
	std::string newFileName = oldFileName;
	std::string sline;    // Input line
	Common::UString line; // Conversion line for XMLFix class call
	Aurora::XMLFix fixer; // Need a copy of the XML filter class
	
	// Find location of period in the file name.
	size_t perLoc = oldFileName.find(".");
	if (perLoc != std::string::npos) {
		// Found a period, so add 'Fixed' before the extension
		newFileName.insert(perLoc, "Fixed");
	} else {
		// Add 'Fixed' at the end of the file
		newFileName = newFileName + "Fixed";
	}
	
	// Open the files
	std::ifstream readFile(oldFileName, std::ios::in);
	if (!readFile.is_open()) {
		// We check twice so that we don't create files if garbage is passed in.
		cout << "Error opening files." << endl;
		return -1;
	}
	std::ofstream writeFile(newFileName, std::ios::out | std::ios::trunc); // Create or overwrite
	if (!writeFile.is_open()) {
		cout << "Error opening files." << endl;
		return -1;
	}
	
	// Read in the first line
	if (std::getline(readFile, sline)) {
		// Convert to UString for XMLFix class function call
		line = sline;

		// Check for XML format
		if (strim(sline).find("<?xml") == 0) {
			// Filter the line then write to file
			line = fixer.fixXMLTag(line);
			writeFile << line.c_str() << "\n";
		} else {
			// Abort
			cout << "Improper XML file.\n";
			return -1;
		}
	} else {
		// Abort
		cout << "Error reading file\n";
		return -1;
	}

	// Insert the root element.
	writeFile << "<Root>\n";
	while (std::getline(readFile, sline)) {
		// Convert to UString for class function call
		line = sline;
		line = fixer.parseLine(line);
		writeFile << line.c_str() << "\n";
	}
	writeFile << "</Root>\n";
	
	// Close the files
	readFile.close();
	writeFile.close();
	return 0;
}


// Temporarily copied from xmlfix.cpp
/**
* Remove leading and trailing whitespace.
* Returns line without leading and trailing
* Spaces.
*/
std::string strim(std::string line) {
	string whitespace = " \t\n\v\f\r";
	size_t lineBegin = line.find_first_not_of(whitespace);
	if (lineBegin == std::string::npos) {
		return ""; // empty string
	}
	int lineEnd = line.find_last_not_of(whitespace);
	int lineRange = lineEnd - lineBegin + 1;
	return line.substr(lineBegin, lineRange);
}
