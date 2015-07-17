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

#include "src/common/version.h"
#include "src/common/ustring.h"
#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/readfile.h"
#include "src/common/writefile.h"
#include "src/common/stdoutstream.h"
#include "src/common/encoding.h"

#include "src/xml/gffdumper.h"

void printUsage(FILE *stream, const Common::UString &name);
bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile, Common::Encoding &encoding);

void dumpGFF(const Common::UString &inFile, const Common::UString &outFile, Common::Encoding encoding);

int main(int argc, char **argv) {
	std::vector<Common::UString> args;
	Common::Platform::getParameters(argc, argv, args);

	Common::Encoding encoding = Common::kEncodingUTF16LE;

	int returnValue;
	Common::UString inFile, outFile;
	if (!parseCommandLine(args, returnValue, inFile, outFile, encoding))
		return returnValue;

	try {
		dumpGFF(inFile, outFile, encoding);
	} catch (Common::Exception &e) {
		Common::printException(e);
		return -1;
	} catch (std::exception &e) {
		error("%s", e.what());
	}

	return 0;
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile, Common::Encoding &encoding) {

	inFile.clear();
	outFile.clear();
	std::vector<Common::UString> args;

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

			if (argv[i] == "--cp1252") {
				// Set the GFF4 string encoding to CP1252

				isOption = true;
				encoding = Common::kEncodingCP1252;

			} else if (argv[i].beginsWith("-") || argv[i].beginsWith("--")) {
			  // An options, but we already checked for all known ones

				printUsage(stderr, argv[0]);
				returnValue = -1;

				return false;
			}
		}

		// Was this a valid option? If so, don't try to use it as a file
		if (isOption)
			continue;

		// This is a file to use
		args.push_back(argv[i]);
	}

	if (args.size() < 1) {
		printUsage(stderr, argv[0]);
		returnValue = -1;

		return false;
	}

	inFile = args[0];

	if (args.size() > 1)
		outFile = args[1];

	return true;
}

void printUsage(FILE *stream, const Common::UString &name) {
	std::fprintf(stream, "BioWare GFF to XML converter\n\n");
	std::fprintf(stream, "Usage: %s [options] <input file> [<output file>]\n", name.c_str());
	std::fprintf(stream, "  -h      --help              This help text\n");
	std::fprintf(stream, "          --version           Display version information\n");
	std::fprintf(stream, "          --cp1252            Read GFF4 strings as Windows CP-1252\n\n");
	std::fprintf(stream, "If no output file is given, the output is written to stdout.\n");
}

void dumpGFF(const Common::UString &inFile, const Common::UString &outFile, Common::Encoding encoding) {
	Common::SeekableReadStream *gff = new Common::ReadFile(inFile);

	XML::GFFDumper *dumper = 0;
	try {
		dumper = XML::GFFDumper::identify(*gff);
	} catch (...) {
		delete gff;
		throw;
	}

	Common::WriteStream *out = 0;
	try {

		if (!outFile.empty())
			out = new Common::WriteFile(outFile);
		else
			out = new Common::StdOutStream;

		dumper->dump(*out, gff, encoding);

	} catch (...) {
		delete dumper;
		delete out;
		throw;
	}

	out->flush();

	delete dumper;
	delete out;
}
