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
 *  Tool to convert TLK files into XML.
 */

#include <cstring>
#include <cstdio>

#include "src/common/version.h"
#include "src/common/ustring.h"
#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/readfile.h"
#include "src/common/writefile.h"
#include "src/common/stdoutstream.h"
#include "src/common/encoding.h"

#include "src/xml/tlkdumper.h"

void printUsage(FILE *stream, const char *name);
bool parseCommandLine(int argc, char **argv, int &returnValue, Common::UString &inFile,
                      Common::UString &outFile, Common::Encoding &encoding);

void dumpTLK(const Common::UString &inFile, const Common::UString &outFile, Common::Encoding encoding);

int main(int argc, char **argv) {
	Common::Encoding encoding = Common::kEncodingInvalid;

	int returnValue;
	Common::UString inFile, outFile;
	if (!parseCommandLine(argc, argv, returnValue, inFile, outFile, encoding))
		return returnValue;

	try {
		dumpTLK(inFile, outFile, encoding);
	} catch (Common::Exception &e) {
		Common::printException(e);
		return -1;
	} catch (std::exception &e) {
		error("%s", e.what());
	}

	return 0;
}

bool parseCommandLine(int argc, char **argv, int &returnValue, Common::UString &inFile,
                      Common::UString &outFile, Common::Encoding &encoding) {

	inFile.clear();
	outFile.clear();
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

			if (!strcmp(argv[i], "--version")) {
				printVersion();
				returnValue = 0;

				return false;
			}

			if        (!strcmp(argv[i], "--cp1250")) {
				isOption = true;
				encoding = Common::kEncodingCP1250;
			} else if (!strcmp(argv[i], "--cp1252")) {
				isOption = true;
				encoding = Common::kEncodingCP1252;
			} else if (!strcmp(argv[i], "--cp932")) {
				isOption = true;
				encoding = Common::kEncodingCP932;
			} else if (!strcmp(argv[i], "--cp936")) {
				isOption = true;
				encoding = Common::kEncodingCP949;
			} else if (!strcmp(argv[i], "--cp949")) {
				isOption = true;
				encoding = Common::kEncodingCP949;
			} else if (!strcmp(argv[i], "--cp950")) {
				isOption = true;
				encoding = Common::kEncodingCP950;
			} else if (!strcmp(argv[i], "--utf8")) {
				isOption = true;
				encoding = Common::kEncodingUTF8;
			} else if (!strcmp(argv[i], "--utf16le")) {
				isOption = true;
				encoding = Common::kEncodingUTF16LE;
			} else if (!strcmp(argv[i], "--utf16be")) {
				isOption = true;
				encoding = Common::kEncodingUTF16BE;
			} else if (!strncmp(argv[i], "-", 1) || !strncmp(argv[i], "--", 2)) {
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

void printUsage(FILE *stream, const char *name) {
	std::fprintf(stream, "BioWare TLK to XML converter\n\n");
	std::fprintf(stream, "Usage: %s [options] <input file> [<output file>]\n", name);
	std::fprintf(stream, "  -h      --help              This help text\n");
	std::fprintf(stream, "          --version           Display version information\n");
	std::fprintf(stream, "          --cp1250            Read TLK strings as Windows CP-1250\n");
	std::fprintf(stream, "          --cp1252            Read TLK strings as Windows CP-1252\n");
	std::fprintf(stream, "          --cp932             Read TLK strings as Windows CP-932\n");
	std::fprintf(stream, "          --cp936             Read TLK strings as Windows CP-936\n");
	std::fprintf(stream, "          --cp949             Read TLK strings as Windows CP-949\n");
	std::fprintf(stream, "          --cp950             Read TLK strings as Windows CP-950\n");
	std::fprintf(stream, "          --utf8              Read TLK strings as UTF-8\n");
	std::fprintf(stream, "          --utf16le           Read TLK strings as little-endian UTF-16\n");
	std::fprintf(stream, "          --utf16be           Read TLK strings as big-endian UTF-16\n\n");
	std::fprintf(stream, "If no output file is given, the output is written to stdout.\n");
}

void dumpTLK(const Common::UString &inFile, const Common::UString &outFile, Common::Encoding encoding) {
	Common::SeekableReadStream *tlk = new Common::ReadFile(inFile);

	Common::WriteStream *out = 0;
	try {
		if (!outFile.empty())
			out = new Common::WriteFile(outFile);
		else
			out = new Common::StdOutStream;

	} catch (...) {
		delete tlk;
		throw;
	}

	try {
		XML::TLKDumper dumper;
		dumper.dump(*out, tlk, encoding);

	} catch (...) {
		delete out;
		throw;
	}

	out->flush();

	delete out;
}
