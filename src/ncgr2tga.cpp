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

#include "src/common/ustring.h"
#include "src/common/stream.h"
#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/file.h"

#include "src/aurora/types.h"
#include "src/aurora/util.h"

#include "src/images/ncgr.h"

void printUsage(FILE *stream, const char *name);
bool parseCommandLine(int argc, char **argv, int &returnValue, uint32 &width, uint32 &height,
                      std::vector<Common::UString> &ncgrFiles, Common::UString &nclrFile,
                      Common::UString &outFile);

void convert(std::vector<Common::UString> &ncgrFiles, Common::UString &nclrFile,
             Common::UString &outFile, uint32 width, uint32 height);

int main(int argc, char **argv) {
	int returnValue;
	uint32 width, height;
	std::vector<Common::UString> ncgrFiles;
	Common::UString nclrFile, outFile;
	if (!parseCommandLine(argc, argv, returnValue, width, height, ncgrFiles, nclrFile, outFile))
		return returnValue;

	try {
		convert(ncgrFiles, nclrFile, outFile, width, height);
	} catch (Common::Exception &e) {
		Common::printException(e);
		return -1;
	} catch (std::exception &e) {
		error("%s", e.what());
	}

	return 0;
}

bool parseCommandLine(int argc, char **argv, int &returnValue, uint32 &width, uint32 &height,
                      std::vector<Common::UString> &ncgrFiles, Common::UString &nclrFile,
                      Common::UString &outFile) {

	if (argc < 4) {
		printUsage(stderr, argv[0]);
		returnValue = -1;

		return false;
	}

	std::vector<Common::UString> args;

	bool optionsEnd = false;
	for (int i = 1; i < argc; i++) {
		bool isOption = false;

		// A "--" marks an end to all options
		if (!strcmp(argv[i], "--")) {
			optionsEnd = true;
			continue;
		}

		// We're still handling options
		if (!optionsEnd) {
			// Help text
			if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
				printUsage(stdout, argv[0]);
				returnValue = 0;

				return false;
			}

			if (!strncmp(argv[i], "-", 1) || !strncmp(argv[i], "--", 2)) {
			  // An options, but we already checked for all known ones

				printUsage(stderr, argv[0]);
				returnValue = -1;

				return false;
			}
		}

		// Was this a valid option? If so, don't try to use it as a file
		if (isOption)
			continue;

		args.push_back(argv[i]);
	}

	if (args.size() < 5) {
		printUsage(stderr, argv[0]);
		returnValue = -1;

		return false;
	}

	width  = (args.size() >= 2) ? atoi(args[0].c_str()) : 0;
	height = (args.size() >= 2) ? atoi(args[1].c_str()) : 0;

	if ((width == 0) || (height == 0) || (args.size() != (width * height + 4))) {
		printUsage(stderr, argv[0]);
		returnValue = -1;

		return false;
	}

	ncgrFiles.resize(width * height);
	for (uint32 i = 0; i < (width * height); i++)
		ncgrFiles[i] = args[i + 2];

	nclrFile = args[2 + width * height];
	outFile  = args[3 + width * height];

	return true;
}

void printUsage(FILE *stream, const char *name) {
	std::fprintf(stream, "Nintendo NCGR image to TGA converter\n");
	std::fprintf(stream, "Usage: %s <width> <height> <ncgr> [<ngr> [...]] <nclr> <out file>\n", name);
	std::fprintf(stream, "  -h      --help              This help text\n");
}

void convert(std::vector<Common::UString> &ncgrFiles, Common::UString &nclrFile,
             Common::UString &outFile, uint32 width, uint32 height) {

	Common::File nclr(nclrFile);

	std::vector<Common::SeekableReadStream *> ncgrs;
	ncgrs.resize(ncgrFiles.size(), 0);

	try {
		for (uint32 i = 0; i < ncgrFiles.size(); i++)
			if (!ncgrFiles[i].empty())
				ncgrs[i] = new Common::File(ncgrFiles[i]);

		Images::NCGR image(ncgrs, width, height, nclr);

		image.flipVertically();

		image.dumpTGA(outFile);

	} catch (...) {
		for (uint32 i = 0; i < ncgrs.size(); i++)
			delete ncgrs[i];

		throw;
	}

	for (uint32 i = 0; i < ncgrs.size(); i++)
		delete ncgrs[i];
}
