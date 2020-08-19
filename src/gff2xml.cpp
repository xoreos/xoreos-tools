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

#include <memory>

#include "src/version/version.h"

#include "src/common/ustring.h"
#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/readfile.h"
#include "src/common/writefile.h"
#include "src/common/stdoutstream.h"
#include "src/common/encoding.h"
#include "src/common/cli.h"

#include "src/aurora/types.h"
#include "src/aurora/language.h"

#include "src/xml/gffdumper.h"

#include "src/util.h"

typedef std::map<uint32_t, Common::Encoding> EncodingOverrides;

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile,
                      Common::Encoding &encoding, Aurora::GameID &game,
                      EncodingOverrides &encOverrides, bool &nwnPremium, bool &sacFile);

bool parseEncodingOverride(const Common::UString &arg, EncodingOverrides &encOverrides);

void dumpGFF(const Common::UString &inFile, const Common::UString &outFile, Common::Encoding encoding, bool nwnPremium,
             bool sacFile);

int main(int argc, char **argv) {
	initPlatform();

	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		Common::Encoding encoding = Common::kEncodingInvalid;
		Aurora::GameID   game     = Aurora::kGameIDUnknown;

		EncodingOverrides encOverrides;

		bool nwnPremium = false;
		bool sacFile = false;

		int returnValue = 1;
		Common::UString inFile, outFile;

		if (!parseCommandLine(args, returnValue, inFile, outFile, encoding, game, encOverrides, nwnPremium, sacFile))
			return returnValue;

		LangMan.declareLanguages(game);

		for (EncodingOverrides::const_iterator e = encOverrides.begin(); e != encOverrides.end(); ++e)
			LangMan.overrideEncoding(e->first, e->second);

		dumpGFF(inFile, outFile, encoding, nwnPremium, sacFile);
	} catch (...) {
		Common::exceptionDispatcherError();
	}

	return 0;
}

bool parseEncodingOverride(const Common::UString &arg, EncodingOverrides &encOverrides) {
	Common::UString::iterator sep = arg.findFirst('=');
	if (sep == arg.end())
		return false;

	uint32_t id = 0xFFFFFFFF;
	try {
		Common::parseString(arg.substr(arg.begin(), sep), id);
	} catch (...) {
	}

	if (id == 0xFFFFFFFF)
		return false;

	Common::Encoding encoding = Common::parseEncoding(arg.substr(++sep, arg.end()));
	if (encoding == Common::kEncodingInvalid) {
		status("Unknown encoding \"%s\"", arg.substr(sep, arg.end()).c_str());
		return false;
	}

	encOverrides[id] = encoding;
	return true;
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile,
                      Common::Encoding &encoding, Aurora::GameID &game,
                      EncodingOverrides &encOverrides, bool &nwnPremium, bool &sacFile) {
	using Common::CLI::NoOption;
	using Common::CLI::kContinueParsing;
	using Common::CLI::Parser;
	using Common::CLI::ValGetter;
	using Common::CLI::Callback;
	using Common::CLI::ValAssigner;
	using Common::CLI::makeEndArgs;
	using Common::CLI::makeAssigners;
	using Aurora::GameID;

	NoOption inFileOpt(false, new ValGetter<Common::UString &>(inFile, "input file"));
	NoOption outFileOpt(true, new ValGetter<Common::UString &>(outFile, "output file"));
	Parser parser(argv[0], "BioWare GFF to XML converter",
	              "If no output file is given, the output is written to stdout.\n\n"
	              "Depending on the game, LocStrings in GFF files might be encoded in various\n"
	              "ways and there's no way to autodetect how. If a game is specified, the\n"
	              "encoding tables for this game are used. Otherwise, gff2xml tries some\n"
	              "heuristics that might fail for certain strings.\n\n"
	              "Additionally, the --encoding parameter can be used to override the encoding\n"
	              "for a specific language ID. The string has to be of the form n=encoding,\n"
	              "for example 0=cp-1252 to override the encoding of the (ungendered) language\n"
	              "ID 0 to be Windows codepage 1252. To override several encodings, specify\n"
	              "the --encoding parameter multiple times.\n",
	              returnValue,
	              makeEndArgs(&inFileOpt, &outFileOpt));


	parser.addSpace();
	parser.addOption("cp1252", "Read GFF4 strings as Windows CP-1252", kContinueParsing,
	                 makeAssigners(new ValAssigner<Common::Encoding>(Common::kEncodingCP1252,
	                 encoding)));
	parser.addSpace();
	parser.addOption("nwnpremium", "This is a broken GFF from a Neverwinter Nights premium module",
	                 kContinueParsing,
	                 makeAssigners(new ValAssigner<bool>(true, nwnPremium)));
	parser.addSpace();
	parser.addOption("nwn", "Use Neverwinter Nights encodings", kContinueParsing,
	                 makeAssigners(new ValAssigner<GameID>(Aurora::kGameIDNWN, game)));
	parser.addOption("nwn2", "Use Neverwinter Nights 2 encodings", kContinueParsing,
	                 makeAssigners(new ValAssigner<GameID>(Aurora::kGameIDNWN2, game)));
	parser.addOption("kotor", "Use Knights of the Old Republic encodings", kContinueParsing,
	                 makeAssigners(new ValAssigner<GameID>(Aurora::kGameIDKotOR, game)));
	parser.addOption("kotor2", "Use Knights of the Old Republic II encodings", kContinueParsing,
	                 makeAssigners(new ValAssigner<GameID>(Aurora::kGameIDKotOR2, game)));
	parser.addOption("jade", "Use Jade Empire encodings", kContinueParsing,
	                 makeAssigners(new ValAssigner<GameID>(Aurora::kGameIDJade, game)));
	parser.addOption("witcher", "Use The Witcher encodings", kContinueParsing,
	                 makeAssigners(new ValAssigner<GameID>(Aurora::kGameIDWitcher, game)));
	parser.addOption("dragonage", "Use Dragon Age encodings", kContinueParsing,
	                 makeAssigners(new ValAssigner<GameID>(Aurora::kGameIDDragonAge, game)));
	parser.addOption("dragonage2", "Use Dragon Age II encodings", kContinueParsing,
	                 makeAssigners(new ValAssigner<GameID>(Aurora::kGameIDDragonAge2, game)));
	parser.addSpace();
	parser.addOption("encoding", "Override an encoding", kContinueParsing,
	                 new Callback<EncodingOverrides &>("str", parseEncodingOverride, encOverrides));
	parser.addOption("sac", "Read the extra sac file header", kContinueParsing,
	                 makeAssigners(new ValAssigner<bool>(true, sacFile)));

	return parser.process(argv);
}


void dumpGFF(const Common::UString &inFile, const Common::UString &outFile, Common::Encoding encoding, bool nwnPremium,
             bool sacFile) {

	std::unique_ptr<Common::SeekableReadStream> gff = std::make_unique<Common::ReadFile>(inFile);

	std::unique_ptr<XML::GFFDumper> dumper(XML::GFFDumper::identify(*gff, nwnPremium, sacFile));

	std::unique_ptr<Common::WriteStream> out(openFileOrStdOut(outFile));

	dumper->dump(*out, gff.release(), encoding, nwnPremium);

	out->flush();

	if (!outFile.empty())
		status("Converted \"%s\" to \"%s\"", inFile.c_str(), outFile.c_str());
}
