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
 *  Tool to convert XML files back into TLK.
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
#include "src/aurora/language.h"

#include "src/xml/tlkcreator.h"

void printUsage(FILE *stream, const Common::UString &name);
bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile,
                      Common::Encoding &encoding, Aurora::GameID &game,
                      XML::TLKCreator::Version &version, uint32 &language);

void createTLK(const Common::UString &inFile, const Common::UString &outFile, Common::Encoding encoding,
               XML::TLKCreator::Version &version, uint32 &language);

int main(int argc, char **argv) {
	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		Common::Encoding encoding = Common::kEncodingInvalid;
		Aurora::GameID   game     = Aurora::kGameIDUnknown;

		XML::TLKCreator::Version version = XML::TLKCreator::kVersionInvalid;
		uint32 language = Aurora::kLanguageInvalid;

		int returnValue = 1;
		Common::UString inFile, outFile;

		if (!parseCommandLine(args, returnValue, inFile, outFile, encoding, game, version, language))
			return returnValue;

		LangMan.declareLanguages(game);

		createTLK(inFile, outFile, encoding, version, language);
	} catch (...) {
		Common::exceptionDispatcherError();
	}

	return 0;
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile,
                      Common::Encoding &encoding, Aurora::GameID &game,
                      XML::TLKCreator::Version &version, uint32 &language) {

	inFile.clear();
	outFile.clear();
	std::vector<Common::UString> args;

	version  = XML::TLKCreator::kVersionInvalid;
	language = Aurora::kLanguageInvalid;

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

			if        (argv[i] == "--cp1250") {
				isOption = true;
				encoding = Common::kEncodingCP1250;
				game     = Aurora::kGameIDUnknown;
			} else if (argv[i] == "--cp1251") {
				isOption = true;
				encoding = Common::kEncodingCP1251;
				game     = Aurora::kGameIDUnknown;
			} else if (argv[i] == "--cp1252") {
				isOption = true;
				encoding = Common::kEncodingCP1252;
				game     = Aurora::kGameIDUnknown;
			} else if (argv[i] == "--cp932") {
				isOption = true;
				encoding = Common::kEncodingCP932;
				game     = Aurora::kGameIDUnknown;
			} else if (argv[i] == "--cp936") {
				isOption = true;
				encoding = Common::kEncodingCP949;
				game     = Aurora::kGameIDUnknown;
			} else if (argv[i] == "--cp949") {
				isOption = true;
				encoding = Common::kEncodingCP949;
				game     = Aurora::kGameIDUnknown;
			} else if (argv[i] == "--cp950") {
				isOption = true;
				encoding = Common::kEncodingCP950;
				game     = Aurora::kGameIDUnknown;
			} else if (argv[i] == "--utf8") {
				isOption = true;
				encoding = Common::kEncodingUTF8;
				game     = Aurora::kGameIDUnknown;
			} else if (argv[i] == "--utf16le") {
				isOption = true;
				encoding = Common::kEncodingUTF16LE;
				game     = Aurora::kGameIDUnknown;
			} else if (argv[i] == "--utf16be") {
				isOption = true;
				encoding = Common::kEncodingUTF16BE;
				game     = Aurora::kGameIDUnknown;
			} else if (argv[i] == "--nwn") {
				isOption = true;
				encoding = Common::kEncodingInvalid;
				game     = Aurora::kGameIDNWN;
			} else if (argv[i] == "--nwn2") {
				isOption = true;
				encoding = Common::kEncodingInvalid;
				game     = Aurora::kGameIDNWN2;
			} else if (argv[i] == "--kotor") {
				isOption = true;
				encoding = Common::kEncodingInvalid;
				game     = Aurora::kGameIDKotOR;
			} else if (argv[i] == "--kotor2") {
				isOption = true;
				encoding = Common::kEncodingInvalid;
				game     = Aurora::kGameIDKotOR2;
			} else if (argv[i] == "--jade") {
				isOption = true;
				encoding = Common::kEncodingInvalid;
				game     = Aurora::kGameIDJade;
			} else if (argv[i] == "--witcher") {
				isOption = true;
				encoding = Common::kEncodingInvalid;
				game     = Aurora::kGameIDWitcher;
			} else if (argv[i] == "--dragonage") {
				isOption = true;
				encoding = Common::kEncodingInvalid;
				game     = Aurora::kGameIDDragonAge;
			} else if (argv[i] == "--dragonage2") {
				isOption = true;
				encoding = Common::kEncodingInvalid;
				game     = Aurora::kGameIDDragonAge2;
			} else if ((argv[i] == "-3") || (argv[i] == "--version30")) {
				isOption = true;
				version  = XML::TLKCreator::kVersion30;
			} else if ((argv[i] == "-4") || (argv[i] == "--version40")) {
				isOption = true;
				version  = XML::TLKCreator::kVersion40;
			} else if ((argv[i] == "-l") || (argv[i] == "--language")) {
				isOption = true;

				try {
					// Needs a language ID as the next parameter
					if (i++ == (argv.size() - 1))
						throw 0;

					Common::parseString(argv[i], language);

				} catch (...) {
					printUsage(stderr, argv[0]);
					returnValue = 1;

					return false;
				}

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

	if ((args.size() < 1) || (args.size() > 2) || (version == XML::TLKCreator::kVersionInvalid)) {
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
	std::fprintf(stream, "XML to BioWare TLK converter\n\n");
	std::fprintf(stream, "Usage: %s [<options>] [<input file>] <output file>\n", name.c_str());
	std::fprintf(stream, "  -h      --help              This help text\n");
	std::fprintf(stream, "          --version           Display version information\n");
	std::fprintf(stream, "  -3      --version30         Write a V3.0 TLK file\n");
	std::fprintf(stream, "  -4      --version40         Write a V4.0 TLK file\n");
	std::fprintf(stream, "  -l <id> --language <id>     Override the TLK language ID\n\n");
	std::fprintf(stream, "          --cp1250            Write TLK strings as Windows CP-1250\n");
	std::fprintf(stream, "          --cp1251            Write TLK strings as Windows CP-1251\n");
	std::fprintf(stream, "          --cp1252            Write TLK strings as Windows CP-1252\n");
	std::fprintf(stream, "          --cp932             Write TLK strings as Windows CP-932\n");
	std::fprintf(stream, "          --cp936             Write TLK strings as Windows CP-936\n");
	std::fprintf(stream, "          --cp949             Write TLK strings as Windows CP-949\n");
	std::fprintf(stream, "          --cp950             Write TLK strings as Windows CP-950\n");
	std::fprintf(stream, "          --utf8              Write TLK strings as UTF-8\n");
	std::fprintf(stream, "          --utf16le           Write TLK strings as little-endian UTF-16\n");
	std::fprintf(stream, "          --utf16be           Write TLK strings as big-endian UTF-16\n\n");
	std::fprintf(stream, "          --nwn               Use Neverwinter Nights encodings\n");
	std::fprintf(stream, "          --nwn2              Use Neverwinter Nights 2 encodings\n");
	std::fprintf(stream, "          --kotor             Use Knights of the Old Republic encodings\n");
	std::fprintf(stream, "          --kotor2            Use Knights of the Old Republic II encodings\n");
	std::fprintf(stream, "          --jade              Use Jade Empire encodings\n");
	std::fprintf(stream, "          --witcher           Use The Witcher encodings\n");
	std::fprintf(stream, "          --dragonage         Use Dragon Age encodings\n");
	std::fprintf(stream, "          --dragonage2        Use Dragon Age II encodings\n\n");
	std::fprintf(stream, "If no input file is given, the input is read from stdin.\n\n");
	std::fprintf(stream, "One of --version* to specify the version of TLK to write is mandatory,\n");
	std::fprintf(stream, "as is one of the flags for the encoding. If the XML file provides a\n");
	std::fprintf(stream, "language ID, the --language flag is optional.\n\n");
	std::fprintf(stream, "There is no way to autodetect the encoding of strings in TLK files,\n");
	std::fprintf(stream, "so an encoding must be specified. Alternatively, the game this TLK\n");
	std::fprintf(stream, "is from can be given, and an appropriate encoding according to that\n");
	std::fprintf(stream, "game and the language ID is used.\n");
}

void createTLK(const Common::UString &inFile, const Common::UString &outFile, Common::Encoding encoding,
               XML::TLKCreator::Version &version, uint32 &language) {

	Common::WriteFile tlk(outFile);

	Common::ReadStream *xml = 0;
	if (!inFile.empty())
		xml = new Common::ReadFile(inFile);
	else
		xml = new Common::StdInStream;

	try {
		XML::TLKCreator::create(tlk, *xml, version, encoding, language);
	} catch (...) {
		delete xml;

		throw;
	}

	delete xml;

	tlk.flush();
	tlk.close();
}
