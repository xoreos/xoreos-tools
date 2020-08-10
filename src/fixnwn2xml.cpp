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

#include <cstdio>
#include <cctype>

#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <memory>

#include "src/util.h"

#include "src/common/ustring.h"
#include "src/common/util.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/cli.h"
#include "src/common/readfile.h"
#include "src/common/writefile.h"
#include "src/common/filepath.h"
#include "src/common/memreadstream.h"
#include "src/common/stdoutstream.h"

#include "src/aurora/xmlfixer.h"

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile);

void convert(Common::UString &inFile, Common::UString &outFile);


int main(int argc, char **argv) {
	initPlatform();

	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		int returnValue = 1;
		Common::UString inFile, outFile;

		if (!parseCommandLine(args, returnValue, inFile, outFile))
			return returnValue;

		convert(inFile, outFile);
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
	// Read the input file into memory
	std::unique_ptr<Common::SeekableReadStream> in(Common::ReadFile::readIntoMemory(inFile));
	std::unique_ptr<Common::WriteStream> out(openFileOrStdOut(outFile));

	// Filter the input
	std::unique_ptr<Common::SeekableReadStream> fixed(Aurora::XMLFixer::fixXMLStream(*in));

	// Write to output
	out->writeStream(*fixed);
	out->flush();

	if (!outFile.empty())
		status("Converted \"%s\" to \"%s\"", inFile.c_str(), outFile.c_str());
}
