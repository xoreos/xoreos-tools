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
 *  Tool to convert XML files back into SSF.
 */

#include <cstring>
#include <cstdio>

#include "src/common/version.h"
#include "src/common/ustring.h"
#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/readfile.h"
#include "src/common/writefile.h"
#include "src/common/stdinstream.h"
#include "src/common/encoding.h"

#include "src/aurora/types.h"

#include "src/xml/ssfcreator.h"

void printUsage(FILE *stream, const Common::UString &name);
bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile, Aurora::GameID &game);

void createSSF(const Common::UString &inFile, const Common::UString &outFile, Aurora::GameID game);

int main(int argc, char **argv) {
	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		Aurora::GameID game = Aurora::kGameIDUnknown;

		int returnValue = 1;
		Common::UString inFile, outFile;

		if (!parseCommandLine(args, returnValue, inFile, outFile, game))
			return returnValue;

		createSSF(inFile, outFile, game);
	} catch (...) {
		Common::exceptionDispatcherError();
	}

	return 0;
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile, Aurora::GameID &game) {

	inFile.clear();
	outFile.clear();
	std::vector<Common::UString> args;

	game = Aurora::kGameIDUnknown;

	bool optionsEnd = false;
	for (size_t i = 1; i < argv.size(); i++) {
		bool isOption = false;

		// A "--" marks an end to all options
		if (argv[i] == "--") {
			optionsEnd = true;
			continue;
		}

		// We're still handling options
		if (!optionsEnd) {
			// Help text
			if ((argv[i] == "-h") || (argv[i] == "--help")) {
				printUsage(stdout, argv[0]);
				returnValue = 0;

				return false;
			}

			if (argv[i] == "--version") {
				printVersion();
				returnValue = 0;

				return false;
			}

			if        (argv[i] == "--nwn") {
				isOption = true;
				game     = Aurora::kGameIDNWN;
			} else if (argv[i] == "--nwn2") {
				isOption = true;
				game     = Aurora::kGameIDNWN2;
			} else if (argv[i] == "--kotor") {
				isOption = true;
				game     = Aurora::kGameIDKotOR;
			} else if (argv[i] == "--kotor2") {
				isOption = true;
				game     = Aurora::kGameIDKotOR2;

			} else if (argv[i].beginsWith("-") || argv[i].beginsWith("--")) {
			  // An options, but we already checked for all known ones

				printUsage(stderr, argv[0]);
				returnValue = 1;

				return false;
			}
		}

		// Was this a valid option? If so, don't try to use it as a file
		if (isOption)
			continue;

		// This is a file to use
		args.push_back(argv[i]);
	}

	if ((args.size() < 1) || (args.size() > 2) || (game == Aurora::kGameIDUnknown)) {
		printUsage(stderr, argv[0]);
		returnValue = 1;

		return false;
	}

	if (args.size() == 2) {
		inFile  = args[0];
		outFile = args[1];
	} else
		outFile = args[0];

	return true;
}

void printUsage(FILE *stream, const Common::UString &name) {
	std::fprintf(stream, "XML to BioWare SSF converter\n\n");
	std::fprintf(stream, "Usage: %s [<options>] [<input file>] <output file>\n", name.c_str());
	std::fprintf(stream, "  -h      --help              This help text\n");
	std::fprintf(stream, "          --version           Display version information\n\n");
	std::fprintf(stream, "          --nwn               Create an SSF for Neverwinter Nights\n");
	std::fprintf(stream, "          --nwn2              Create an SSF for Neverwinter Nights 2\n");
	std::fprintf(stream, "          --kotor             Create an SSF for Knights of the Old Republic\n");
	std::fprintf(stream, "          --kotor2            Create an SSF for Knights of the Old Republic II\n\n");
	std::fprintf(stream, "If no input file is given, the input is read from stdin.\n\n");
	std::fprintf(stream, "Since different games use different SSF file version, specifying the\n");
	std::fprintf(stream, "game for which to create the SSF file is necessary.\n");
}

void createSSF(const Common::UString &inFile, const Common::UString &outFile, Aurora::GameID game) {
	Common::WriteFile ssf(outFile);

	Common::ReadStream *xml = 0;
	if (!inFile.empty())
		xml = new Common::ReadFile(inFile);
	else
		xml = new Common::StdInStream;

	try {
		XML::SSFCreator::create(ssf, *xml, game);
	} catch (...) {
		delete xml;

		throw;
	}

	delete xml;

	ssf.flush();
	ssf.close();
}
