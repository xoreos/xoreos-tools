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
 *  Tool to convert SSF files into XML.
 */

#include <cstring>
#include <cstdio>

#include <memory>

#include "src/version/version.h"

#include "src/common/ustring.h"
#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/readfile.h"
#include "src/common/writefile.h"
#include "src/common/stdoutstream.h"
#include "src/common/cli.h"

#include "src/xml/ssfdumper.h"

#include "src/util.h"

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile);

void dumpSSF(const Common::UString &inFile, const Common::UString &outFile);

int main(int argc, char **argv) {
	initPlatform();

	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		int returnValue = 1;
		Common::UString inFile, outFile;

		if (!parseCommandLine(args, returnValue, inFile, outFile))
			return returnValue;

		dumpSSF(inFile, outFile);
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
	NoOption inFileOpt(false, new ValGetter<Common::UString &>(inFile, "input file"));
	NoOption outFileOpt(true, new ValGetter<Common::UString &>(outFile, "output file"));
	Parser parser(argv[0], "BioWare SSF to XML converter",
	              "\nIf no output file is given, the output is written to stdout.",
	              returnValue, makeEndArgs(&inFileOpt, &outFileOpt));

	return parser.process(argv);
}

void dumpSSF(const Common::UString &inFile, const Common::UString &outFile) {
	Common::ReadFile ssf(inFile);
	std::unique_ptr<Common::WriteStream> out(openFileOrStdOut(outFile));

	XML::SSFDumper::dump(*out, ssf);

	out->flush();

	if (!outFile.empty())
		status("Converted \"%s\" to \"%s\"", inFile.c_str(), outFile.c_str());
}
