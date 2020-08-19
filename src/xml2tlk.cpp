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

#include <memory>

#include "src/version/version.h"

#include "src/common/ustring.h"
#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/readfile.h"
#include "src/common/writefile.h"
#include "src/common/stdinstream.h"
#include "src/common/encoding.h"
#include "src/common/cli.h"

#include "src/aurora/types.h"
#include "src/aurora/language.h"

#include "src/xml/tlkcreator.h"

#include "src/util.h"

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile,
                      Common::Encoding &encoding, Aurora::GameID &game,
                      XML::TLKCreator::Version &version, uint32_t &language);

void createTLK(const Common::UString &inFile, const Common::UString &outFile, Common::Encoding encoding,
               XML::TLKCreator::Version &version, uint32_t &language);

int main(int argc, char **argv) {
	initPlatform();

	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		Common::Encoding encoding = Common::kEncodingInvalid;
		Aurora::GameID   game     = Aurora::kGameIDUnknown;

		XML::TLKCreator::Version version = XML::TLKCreator::kVersionInvalid;
		uint32_t language = Aurora::kLanguageInvalid;

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
                      XML::TLKCreator::Version &version, uint32_t &language) {

	using Common::CLI::NoOption;
	using Common::CLI::kContinueParsing;
	using Common::CLI::Parser;
	using Common::CLI::ValGetter;
	using Common::CLI::ValAssigner;
	using Common::CLI::makeEndArgs;
	using Common::CLI::makeAssigners;
	using Common::Encoding;
	using Aurora::GameID;
	std::vector<Common::UString> args;
	NoOption inFileOpt(false, new ValGetter<Common::UString &>(inFile, "input files"));
	NoOption outFileOpt(true, new ValGetter<Common::UString &>(outFile, "output files"));
	Parser parser(argv[0], "XML to BioWare TLK converter",
	              "If no input file is given, the input is read from stdin.\n\n"
	              "One of the flags to identify the game this TLK is to be used is mandatory,\n"
	              "as is a language ID unless the XML file specifies one (in which case, the\n"
	              "--language parameter can be used to override it).\n\n"
	              "By default, the version of TLK fitting for the specified game and the\n"
	              "encoding fitting for the game and language is used. These choice can be\n"
	              "overwritten by using the appropriate encoding and --version flags.\n",
	              returnValue, makeEndArgs(&inFileOpt, &outFileOpt));

	encoding = Common::kEncodingInvalid;
	game     = Aurora::kGameIDUnknown;
	version  = XML::TLKCreator::kVersionInvalid;
	language = Aurora::kLanguageInvalid;

	parser.addSpace();
	parser.addOption("version30", '3', "Write a V3.0 TLK file", kContinueParsing,
	                 makeAssigners(new ValAssigner<XML::TLKCreator::Version>
	                 (XML::TLKCreator::kVersion30, version)));
	parser.addOption("version40", '4', "Write a V4.0 TLK file", kContinueParsing,
	                 makeAssigners(new ValAssigner<XML::TLKCreator::Version>
	                 (XML::TLKCreator::kVersion40, version)));

	parser.addOption("language", 'l', "Override the TLK language ID", kContinueParsing,
	                 new ValGetter<uint32_t &>(language, "id"));

	parser.addSpace();
	parser.addOption("cp1250", "Write TLK strings as Windows CP-1250", kContinueParsing,
	                 makeAssigners(new ValAssigner<Encoding>(Common::kEncodingCP1250, encoding),
	                 new ValAssigner<GameID>(Aurora::kGameIDUnknown, game)));
	parser.addOption("cp1251", "Write TLK strings as Windows CP-1251", kContinueParsing,
	                 makeAssigners(new ValAssigner<Encoding>(Common::kEncodingCP1251, encoding),
	                 new ValAssigner<GameID>(Aurora::kGameIDUnknown, game)));
	parser.addOption("cp1252", "Write TLK strings as Windows CP-1252", kContinueParsing,
	                 makeAssigners(new ValAssigner<Encoding>(Common::kEncodingCP1252, encoding),
	                 new ValAssigner<GameID>(Aurora::kGameIDUnknown, game)));
	parser.addOption("cp932", "Write TLK strings as Windows CP-932", kContinueParsing,
	                 makeAssigners(new ValAssigner<Encoding>(Common::kEncodingCP932, encoding),
	                 new ValAssigner<GameID>(Aurora::kGameIDUnknown, game)));
	parser.addOption("cp936", "Write TLK strings as Windows CP-936", kContinueParsing,
	                 makeAssigners(new ValAssigner<Encoding>(Common::kEncodingCP936, encoding),
	                 new ValAssigner<GameID>(Aurora::kGameIDUnknown, game)));
	parser.addOption("cp949", "Write TLK strings as Windows CP-949", kContinueParsing,
	                 makeAssigners(new ValAssigner<Encoding>(Common::kEncodingCP949, encoding),
	                 new ValAssigner<GameID>(Aurora::kGameIDUnknown, game)));
	parser.addOption("cp950", "Write TLK strings as Windows CP-950", kContinueParsing,
	                 makeAssigners(new ValAssigner<Encoding>(Common::kEncodingCP950, encoding),
	                 new ValAssigner<GameID>(Aurora::kGameIDUnknown, game)));
	parser.addOption("utf8", "Write TLK strings as UTF-8", kContinueParsing,
	                 makeAssigners(new ValAssigner<Encoding>(Common::kEncodingUTF8, encoding),
	                 new ValAssigner<GameID>(Aurora::kGameIDUnknown, game)));
	parser.addOption("utf16le", "Write TLK strings as little-endian UTF-16", kContinueParsing,
	                 makeAssigners(new ValAssigner<Encoding>(Common::kEncodingUTF16LE, encoding),
	                 new ValAssigner<GameID>(Aurora::kGameIDUnknown, game)));
	parser.addOption("utf16be", "Write TLK strings as big-endian UTF-16", kContinueParsing,
	                 makeAssigners(new ValAssigner<Encoding>(Common::kEncodingUTF16BE, encoding),
	                 new ValAssigner<GameID>(Aurora::kGameIDUnknown, game)));
	parser.addSpace();
	parser.addOption("nwn", "Use Neverwinter Nights encodings", kContinueParsing,
	                 makeAssigners(new ValAssigner<Encoding>(Common::kEncodingInvalid, encoding),
	                 new ValAssigner<GameID>(Aurora::kGameIDNWN, game)));
	parser.addOption("nwn2", "Use Neverwinter Nights 2 encodings", kContinueParsing,
	                 makeAssigners(new ValAssigner<Encoding>(Common::kEncodingInvalid, encoding),
	                 new ValAssigner<GameID>(Aurora::kGameIDNWN2, game)));
	parser.addOption("kotor", "Use Knights of the Old Republic encodings", kContinueParsing,
	                 makeAssigners(new ValAssigner<Encoding>(Common::kEncodingInvalid, encoding),
	                 new ValAssigner<GameID>(Aurora::kGameIDKotOR, game)));
	parser.addOption("kotor2", "Use Knights of the Old Republic II encodings", kContinueParsing,
	                 makeAssigners(new ValAssigner<Encoding>(Common::kEncodingInvalid, encoding),
	                 new ValAssigner<GameID>(Aurora::kGameIDKotOR2, game)));
	parser.addOption("jade", "Use Jade Empire encodings", kContinueParsing,
	                 makeAssigners(new ValAssigner<Encoding>(Common::kEncodingInvalid, encoding),
	                 new ValAssigner<GameID>(Aurora::kGameIDJade, game)));
	parser.addOption("witcher", "Use The Witcher encodings", kContinueParsing,
	                 makeAssigners(new ValAssigner<Encoding>(Common::kEncodingInvalid, encoding),
	                 new ValAssigner<GameID>(Aurora::kGameIDWitcher, game)));
	parser.addOption("dragonage", "Use Dragon Age encodings", kContinueParsing,
	                 makeAssigners(new ValAssigner<Encoding>(Common::kEncodingInvalid, encoding),
	                 new ValAssigner<GameID>(Aurora::kGameIDDragonAge, game)));
	parser.addOption("dragonage2", "Use Dragon Age II encodings", kContinueParsing,
	                 makeAssigners(new ValAssigner<Encoding>(Common::kEncodingInvalid, encoding),
	                 new ValAssigner<GameID>(Aurora::kGameIDDragonAge2, game)));

	if (!parser.process(argv))
		return false;

	if (game == Aurora::kGameIDUnknown) {
		parser.usage();
		returnValue = Common::CLI::kEndFail;
		return false;
	}

	if (version == XML::TLKCreator::kVersionInvalid) {
		switch (game) {
			case Aurora::kGameIDNWN:     XOREOS_FALLTHROUGH;
			case Aurora::kGameIDNWN2:    XOREOS_FALLTHROUGH;
			case Aurora::kGameIDKotOR:   XOREOS_FALLTHROUGH;
			case Aurora::kGameIDKotOR2:  XOREOS_FALLTHROUGH;
			case Aurora::kGameIDWitcher:
				version = XML::TLKCreator::kVersion30;
				break;
			case Aurora::kGameIDJade:
				version = XML::TLKCreator::kVersion40;
				break;
			case Aurora::kGameIDSonic:
			case Aurora::kGameIDDragonAge:
			case Aurora::kGameIDDragonAge2:
				std::printf("Specified game uses unsupported TLK version.\n");
				returnValue = Common::CLI::kEndFail;
				return false;

			default:
				break;
		}
	}

	return true;
}

void createTLK(const Common::UString &inFile, const Common::UString &outFile, Common::Encoding encoding,
               XML::TLKCreator::Version &version, uint32_t &language) {

	Common::WriteFile tlk(outFile);
	std::unique_ptr<Common::ReadStream> xml(openFileOrStdIn(inFile));

	XML::TLKCreator::create(tlk, *xml, version, encoding, inFile, language);

	tlk.flush();
	tlk.close();
}
