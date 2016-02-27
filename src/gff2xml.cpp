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

#include "src/aurora/types.h"
#include "src/aurora/language.h"

#include "src/xml/gffdumper.h"

void printUsage(FILE *stream, const Common::UString &name);
bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile,
                      Common::Encoding &encoding, Aurora::GameID &game, bool &nwnPremium);

void dumpGFF(const Common::UString &inFile, const Common::UString &outFile,
             Common::Encoding encoding, bool nwnPremium);

int main(int argc, char **argv) {
	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		Common::Encoding encoding = Common::kEncodingUTF16LE;
		Aurora::GameID   game     = Aurora::kGameIDUnknown;

		bool nwnPremium = false;

		int returnValue = 1;
		Common::UString inFile, outFile;

		if (!parseCommandLine(args, returnValue, inFile, outFile, encoding, game, nwnPremium))
			return returnValue;

		LangMan.declareLanguages(game);

		dumpGFF(inFile, outFile, encoding, nwnPremium);
	} catch (...) {
		Common::exceptionDispatcherError();
	}

	return 0;
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile,
                      Common::Encoding &encoding, Aurora::GameID &game, bool &nwnPremium) {

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

			if        (argv[i] == "--cp1252") {
				// Set the GFF4 string encoding to CP1252

				isOption = true;
				encoding = Common::kEncodingCP1252;

			} else if (argv[i] == "--nwnpremium") {

				isOption   = true;
				nwnPremium = true;

			} else if (argv[i] == "--nwn") {
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
			} else if (argv[i] == "--jade") {
				isOption = true;
				game     = Aurora::kGameIDJade;
			} else if (argv[i] == "--witcher") {
				isOption = true;
				game     = Aurora::kGameIDWitcher;
			} else if (argv[i] == "--dragonage") {
				isOption = true;
				game     = Aurora::kGameIDDragonAge;
			} else if (argv[i] == "--dragonage2") {
				isOption = true;
				game     = Aurora::kGameIDDragonAge2;
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

	if ((args.size() < 1) || (args.size() > 2)) {
		printUsage(stderr, argv[0]);
		returnValue = 1;

		return false;
	}

	inFile = args[0];

	if (args.size() > 1)
		outFile = args[1];

	return true;
}

void printUsage(FILE *stream, const Common::UString &name) {
	std::fprintf(stream, "BioWare GFF to XML converter\n\n");
	std::fprintf(stream, "Usage: %s [<options>] <input file> [<output file>]\n", name.c_str());
	std::fprintf(stream, "  -h      --help              This help text\n");
	std::fprintf(stream, "          --version           Display version information\n");
	std::fprintf(stream, "          --cp1252            Read GFF4 strings as Windows CP-1252\n");
	std::fprintf(stream, "          --nwnpremium        This is a broken GFF from a Neverwinter\n");
	std::fprintf(stream, "                              Nights premium module\n\n");
	std::fprintf(stream, "          --nwn               Use Neverwinter Nights encodings\n");
	std::fprintf(stream, "          --nwn2              Use Neverwinter Nights 2 encodings\n");
	std::fprintf(stream, "          --kotor             Use Knights of the Old Republic encodings\n");
	std::fprintf(stream, "          --kotor2            Use Knights of the Old Republic II encodings\n");
	std::fprintf(stream, "          --jade              Use Jade Empire encodings\n");
	std::fprintf(stream, "          --witcher           Use The Witcher encodings\n");
	std::fprintf(stream, "          --dragonage         Use Dragon Age encodings\n");
	std::fprintf(stream, "          --dragonage2        Use Dragon Age II encodings\n\n");
	std::fprintf(stream, "If no output file is given, the output is written to stdout.\n\n");
	std::fprintf(stream, "Depending on the game, LocStrings in GFF files might be encoded in various\n");
	std::fprintf(stream, "ways and there's no way to autodetect how. If a game is specified, the\n");
	std::fprintf(stream, "encoding tables for this game are used. Otherwise, gff2xml tries some\n");
	std::fprintf(stream, "heuristics that might fail for certain strings.\n");
}

void dumpGFF(const Common::UString &inFile, const Common::UString &outFile,
             Common::Encoding encoding, bool nwnPremium) {

	Common::SeekableReadStream *gff = new Common::ReadFile(inFile);

	XML::GFFDumper *dumper = 0;
	try {
		dumper = XML::GFFDumper::identify(*gff, nwnPremium);
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

		dumper->dump(*out, gff, encoding, nwnPremium);

	} catch (...) {
		delete dumper;
		delete out;
		throw;
	}

	out->flush();

	if (!outFile.empty())
		status("Converted \"%s\" to \"%s\"", inFile.c_str(), outFile.c_str());

	delete dumper;
	delete out;
}
