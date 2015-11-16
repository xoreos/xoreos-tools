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
 *  Tool to convert Nintendo NCGR images into TGA.
 */

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include "src/common/version.h"
#include "src/common/ustring.h"
#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/readstream.h"
#include "src/common/readfile.h"

#include "src/aurora/types.h"
#include "src/aurora/util.h"

#include "src/images/ncgr.h"

void printUsage(FILE *stream, const Common::UString &name);
bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      uint32 &width, uint32 &height, std::vector<Common::UString> &ncgrFiles,
                      Common::UString &nclrFile, Common::UString &outFile);

void convert(std::vector<Common::UString> &ncgrFiles, Common::UString &nclrFile,
             Common::UString &outFile, uint32 width, uint32 height);

int main(int argc, char **argv) {
	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		int returnValue = 1;
		uint32 width, height;
		std::vector<Common::UString> ncgrFiles;
		Common::UString nclrFile, outFile;

		if (!parseCommandLine(args, returnValue, width, height, ncgrFiles, nclrFile, outFile))
			return returnValue;

		convert(ncgrFiles, nclrFile, outFile, width, height);
	} catch (...) {
		Common::exceptionDispatcherError();
	}

	return 0;
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      uint32 &width, uint32 &height, std::vector<Common::UString> &ncgrFiles,
                      Common::UString &nclrFile, Common::UString &outFile) {

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

	if (args.size() < 5) {
		printUsage(stderr, argv[0]);
		returnValue = 1;

		return false;
	}

	width  = (args.size() >= 2) ? atoi(args[0].c_str()) : 0;
	height = (args.size() >= 2) ? atoi(args[1].c_str()) : 0;

	if ((width == 0) || (height == 0) || (args.size() != (width * height + 4))) {
		printUsage(stderr, argv[0]);
		returnValue = 1;

		return false;
	}

	ncgrFiles.resize(width * height);
	for (uint32 i = 0; i < (width * height); i++)
		ncgrFiles[i] = args[i + 2];

	nclrFile = args[2 + width * height];
	outFile  = args[3 + width * height];

	return true;
}

void printUsage(FILE *stream, const Common::UString &name) {
	std::fprintf(stream, "Nintendo NCGR image to TGA converter\n");
	std::fprintf(stream, "Usage: %s [<options>] <width> <height> <ncgr> [<ngr> [...]] <nclr> <tga>\n", name.c_str());
	std::fprintf(stream, "  -h      --help              This help text\n");
	std::fprintf(stream, "          --version           Display version information\n");
}

void convert(std::vector<Common::UString> &ncgrFiles, Common::UString &nclrFile,
             Common::UString &outFile, uint32 width, uint32 height) {

	Common::ReadFile nclr(nclrFile);

	std::vector<Common::SeekableReadStream *> ncgrs;
	ncgrs.resize(ncgrFiles.size(), 0);

	try {
		for (size_t i = 0; i < ncgrFiles.size(); i++)
			if (!ncgrFiles[i].empty() && (ncgrFiles[i] != "\"\"") && (ncgrFiles[i] != "\'\'"))
				ncgrs[i] = new Common::ReadFile(ncgrFiles[i]);

		Images::NCGR image(ncgrs, width, height, nclr);

		image.flipVertically();

		image.dumpTGA(outFile);

	} catch (...) {
		for (size_t i = 0; i < ncgrs.size(); i++)
			delete ncgrs[i];

		throw;
	}

	for (size_t i = 0; i < ncgrs.size(); i++)
		delete ncgrs[i];
}
