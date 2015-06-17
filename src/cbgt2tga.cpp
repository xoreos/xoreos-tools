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
 *  Tool to convert CBGT images to TGA. */

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include "src/common/ustring.h"
#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/readfile.h"

#include "src/aurora/types.h"
#include "src/aurora/util.h"

#include "src/images/cbgt.h"

void printUsage(FILE *stream, const char *name);
bool parseCommandLine(int argc, char **argv, int &returnValue,
                      Common::UString &cbgtFile , Common::UString &palFile,
                      Common::UString &twoDAFile, Common::UString &outFile);

void convert(const Common::UString &cbgtFile , const Common::UString &palFile,
             const Common::UString &twoDAFile, const Common::UString &outFile);

int main(int argc, char **argv) {
	int returnValue;
	Common::UString cbgtFile, palFile, twoDAFile, outFile;
	if (!parseCommandLine(argc, argv, returnValue, cbgtFile, palFile, twoDAFile, outFile))
		return returnValue;

	try {
		convert(cbgtFile, palFile, twoDAFile, outFile);
	} catch (Common::Exception &e) {
		Common::printException(e);
		return -1;
	} catch (std::exception &e) {
		error("%s", e.what());
	}

	return 0;
}

bool parseCommandLine(int argc, char **argv, int &returnValue,
                      Common::UString &cbgtFile , Common::UString &palFile,
                      Common::UString &twoDAFile, Common::UString &outFile) {
	if (argc < 5) {
		printUsage(stderr, argv[0]);
		returnValue = -1;

		return false;
	}

	std::vector<Common::UString> args;

	bool optionsEnd = false;
	for (int i = 1; i < argc; i++) {
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

		args.push_back(argv[i]);
	}

	if (args.size() != 4) {
		printUsage(stderr, argv[0]);
		returnValue = -1;

		return false;
	}

	cbgtFile  = args[0];
	palFile   = args[1];
	twoDAFile = args[2];
	outFile   = args[3];

	return true;
}

void printUsage(FILE *stream, const char *name) {
	std::fprintf(stream, "CBGT image to TGA converter\n");
	std::fprintf(stream, "       %s <cbgt file> <pal file> <2da file> <out file>\n", name);
	std::fprintf(stream, "  -h      --help              This help text\n");
}

void convert(const Common::UString &cbgtFile , const Common::UString &palFile,
             const Common::UString &twoDAFile, const Common::UString &outFile) {

	Common::ReadFile cbgt(cbgtFile), pal(palFile), twoDA(twoDAFile);
	Images::CBGT image(cbgt, pal, twoDA);

	image.flipVertically();

	image.dumpTGA(outFile);
}
