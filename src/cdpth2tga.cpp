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
 *  Tool to convert CDPTH depth images to TGA. */

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include "src/common/version.h"
#include "src/common/ustring.h"
#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/readfile.h"

#include "src/aurora/types.h"
#include "src/aurora/util.h"
#include "src/aurora/2dafile.h"

#include "src/images/cdpth.h"

void printUsage(FILE *stream, const Common::UString &name);
bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &cdpthFile, Common::UString &twoDAFile,
                      Common::UString &outFile);

void convert(const Common::UString &cdpthFile, const Common::UString &twoDAFile,
             const Common::UString &outFile);

int main(int argc, char **argv) {
	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		int returnValue = 1;
		Common::UString cdpthFile, twoDAFile, outFile;

		if (!parseCommandLine(args, returnValue, cdpthFile, twoDAFile, outFile))
			return returnValue;

		convert(cdpthFile, twoDAFile, outFile);
	} catch (...) {
		Common::exceptionDispatcherError();
	}

	return 0;
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &cdpthFile, Common::UString &twoDAFile,
                      Common::UString &outFile) {

	std::vector<Common::UString> args;

	bool optionsEnd = false;
	for (size_t i = 1; i < argv.size(); i++) {
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

			if (argv[i].beginsWith("-") || argv[i].beginsWith("--")) {
			  // An options, but we already checked for all known ones

				printUsage(stderr, argv[0]);
				returnValue = 1;

				return false;
			}
		}

		args.push_back(argv[i]);
	}

	if (args.size() != 3) {
		printUsage(stderr, argv[0]);
		returnValue = 1;

		return false;
	}

	cdpthFile = args[0];
	twoDAFile = args[1];
	outFile   = args[2];

	return true;
}

void printUsage(FILE *stream, const Common::UString &name) {
	std::fprintf(stream, "CDPTH depth image to TGA converter\n");
	std::fprintf(stream, "Usage: %s [<options>] <cdpth> <2da> <tga>\n", name.c_str());
	std::fprintf(stream, "  -h      --help              This help text\n");
	std::fprintf(stream, "          --version           Display version information\n");
}

static void getDimensions(const Common::UString &twoDAFile, uint32 &width, uint32 &height) {

	Common::ReadFile twoDAStream(twoDAFile);
	Aurora::TwoDAFile twoDA(twoDAStream);

	width  = twoDA.getColumnCount() * 64;
	height = twoDA.getRowCount()    * 64;
}

void convert(const Common::UString &cdpthFile, const Common::UString &twoDAFile,
             const Common::UString &outFile) {

	uint32 width, height;
	getDimensions(twoDAFile, width, height);

	Common::ReadFile cdpth(cdpthFile);
	Images::CDPTH image(cdpth, width, height);

	image.flipVertically();

	image.dumpTGA(outFile);
}
