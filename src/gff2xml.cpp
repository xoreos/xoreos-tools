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
 *  Tool to convert GFF files into XML.
 */

#include <cstring>
#include <cstdio>

#include "src/common/ustring.h"
#include "src/common/stream.h"
#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/file.h"

#include "src/xml/gffdumper.h"

void printUsage(FILE *stream, const char *name);
bool parseCommandLine(int argc, char **argv, int &returnValue, Common::UString &file);

void dumpGFF(const Common::UString &file);

int main(int argc, char **argv) {
	int returnValue;
	Common::UString file;
	if (!parseCommandLine(argc, argv, returnValue, file))
		return returnValue;

	try {
		dumpGFF(file);
	} catch (Common::Exception &e) {
		Common::printException(e);
		return -1;
	}

	return 0;
}

bool parseCommandLine(int argc, char **argv, int &returnValue, Common::UString &file) {
	file.clear();

	if (argc < 2) {
		printUsage(stderr, argv[0]);
		returnValue = -1;

		return false;
	}

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

			// An options, but we already checked for all known ones
			if (!strncmp(argv[i], "-", 1) || !strncmp(argv[i], "--", 2)) {
				printUsage(stderr, argv[0]);
				returnValue = -1;

				return false;
			}
		}

		// We already have a file => error
		if (!file.empty()) {
			printUsage(stderr, argv[0]);
			returnValue = -1;

			return false;
		}

		// This is a file to use
		file = argv[i];
	}

	// No file? Error.
	if (file.empty()) {
		printUsage(stderr, argv[0]);
		returnValue = -1;

		return false;
	}

	return true;
}

void printUsage(FILE *stream, const char *name) {
	std::fprintf(stream, "BioWare GFF to XML converter\n\n");
	std::fprintf(stream, "Usage: %s [options] <file>\n", name);
	std::fprintf(stream, "  -h      --help              This help text\n");
}

void dumpGFF(const Common::UString &file) {
	Common::File gff(file);

	XML::GFFDumper *dumper = XML::GFFDumper::identify(gff);

	Common::StdOutStream xml;
	dumper->dump(xml, gff);

	delete dumper;
}
