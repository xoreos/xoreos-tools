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

#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <stdio.h>
#include <ctype.h>
#include "src/util.h"
#include "src/common/scopedptr.h"
#include "src/common/ustring.h"
#include "src/common/util.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/cli.h"
#include "src/common/readfile.h"
#include "src/common/writefile.h"
#include "src/common/filepath.h"
#include "src/common/memreadstream.h"
#include "src/aurora/xmlfix.h"

const Common::UString OUTPUT_FILE_TAG = "_Fixed"; // Add tag if no output file specified

// Prototypes
bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile);

void convert(Common::UString &inFile, Common::UString &outFile);
void old_convert(const Common::UString &inFile, const Common::UString &outFile);

std::string strim(std::string line);

int main(int argc, char **argv) {
	initPlatform();

	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		int returnValue = 1;
		Common::UString xmlFile, outFile;

		if (!parseCommandLine(args, returnValue, xmlFile, outFile))
			return returnValue;

		convert(xmlFile, outFile);
	} catch (...) {
		Common::exceptionDispatcherError();
	}

	return 0;
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile) {
	using Common::CLI::Parser;
	using Common::CLI::ValGetter;
	using Common::CLI::NoOption;
	using Common::CLI::makeEndArgs;
	std::vector<Common::CLI::Getter *> getters;
	NoOption inFileOpt(false, new ValGetter<Common::UString &>(inFile, "input file"));
	NoOption outFileOpt(true, new ValGetter<Common::UString &>(outFile, "output file"));
	Parser parser(argv[0], "Convert NWN2 XML file to standard XML format", "", returnValue,
                      makeEndArgs(&inFileOpt, &outFileOpt));
	return parser.process(argv);
}

/*
 * Read in the input file, apply XML format corrections, then write to output file.
 */
void convert(Common::UString &inFile, Common::UString &outFile) {
	Common::UString outFileName = outFile;

	// If outFile is an empty string, use a default
	if (outFile == "") {
		// Default to input file name
		outFileName = inFile;

		// Find location of period in the file name.
		size_t perLoc = inFile.find(".");
		if (perLoc != std::string::npos) {
			// Found a period, so insert tag before the extension
			outFileName.insert(perLoc, OUTPUT_FILE_TAG);
		} else {
			// Add tag at the end of the file
			outFileName += OUTPUT_FILE_TAG;
		}
	}
	
	// Read the input file into memory
	Common::ScopedPtr<Common::SeekableReadStream> in(Common::ReadFile::readIntoMemory(inFile));

	// Filter the input
	Aurora::XMLFix converter;
	Common::SeekableReadStream *fixed = converter.fixXMLStream(*in);

	// Write converted XML to output file
	Common::WriteFile out(outFileName);
	out.writeStream(*fixed);
	out.flush();
	out.close();
}

