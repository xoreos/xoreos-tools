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
 *  Tool to convert XML files back into GFF.
 */

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

#include "src/xml/gffcreator.h"

#include "src/util.h"

typedef std::map<uint32_t, Common::Encoding> EncodingOverrides;

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile,
                      Aurora::GameID &game, EncodingOverrides &encOverrides,
                      XML::GFFCreator::GFF3Version &gff3Version);

bool parseEncodingOverride(const Common::UString &arg, EncodingOverrides &encOverrides);

void createGFF(const Common::UString &inFile, const Common::UString &outFile, XML::GFFCreator::GFF3Version gff3Version);

int main(int argc, char **argv) {
	initPlatform();

	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		Aurora::GameID game = Aurora::kGameIDUnknown;
		EncodingOverrides encOverrides;
		XML::GFFCreator::GFF3Version gff3Version = XML::GFFCreator::GFF3Version::Unknown;

		int returnValue = 1;
		Common::UString inFile, outFile;

		if (!parseCommandLine(args, returnValue, inFile, outFile, game, encOverrides, gff3Version))
			return returnValue;

		LangMan.declareLanguages(game);

		for (EncodingOverrides::const_iterator e = encOverrides.begin(); e != encOverrides.end(); ++e)
			LangMan.overrideEncoding(e->first, e->second);

		if (gff3Version == XML::GFFCreator::GFF3Version::Unknown) {
			if (game == Aurora::kGameIDWitcher)
				gff3Version = XML::GFFCreator::GFF3Version::V3_3;
			else
				gff3Version = XML::GFFCreator::GFF3Version::V3_2;
		}

		createGFF(inFile, outFile, gff3Version);
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
                      Aurora::GameID &game, EncodingOverrides &encOverrides,
                      XML::GFFCreator::GFF3Version &gff3Version) {

	using Common::CLI::NoOption;
	using Common::CLI::kContinueParsing;
	using Common::CLI::Parser;
	using Common::CLI::ValGetter;
	using Common::CLI::Callback;
	using Common::CLI::ValAssigner;
	using Common::CLI::makeEndArgs;
	using Common::CLI::makeAssigners;
	using Aurora::GameID;
	using XML::GFFCreator;

	std::vector<Common::UString> args;
	NoOption inFileOpt(false, new ValGetter<Common::UString &>(inFile, "input file"));
	NoOption outFileOpt(false, new ValGetter<Common::UString &>(outFile, "output file"));
	Parser parser(argv[0], "XML to BioWare GFF converter",
	              "If the input file is -, the input is read from stdin.\n\n"
	              "The XML root tag determines, if a GFF3 or GFF4 file will be written\n"
	              "and the type property determines which GFF ID will be written. GFF IDs\n"
	              "can be at most 4 characters long.\n\n"
	              "Depending on the game, LocStrings in GFF files might be encoded in various\n"
	              "ways and there's no way to autodetect how. If a game is specified, the\n"
	              "encoding tables for this game are used. Otherwise, xml2gff writes plain\n"
	              "ASCII strings, removing other characters.\n\n"
	              "Additionally, the --encoding parameter can be used to override the encoding\n"
	              "for a specific language ID. The string has to be of the form n=encoding,\n"
	              "for example 0=cp-1252 to override the encoding of the (ungendered) language\n"
	              "ID 0 to be Windows codepage 1252. To override several encodings, specify\n"
	              "the --encoding parameter multiple times.\n",
	              returnValue,
	              makeEndArgs(&inFileOpt, &outFileOpt));

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
	parser.addSpace();
	parser.addOption("v32", "Create GFF3 V3.2 file (default)", kContinueParsing,
	                 makeAssigners(new ValAssigner<GFFCreator::GFF3Version>(GFFCreator::GFF3Version::V3_2, gff3Version)));
	parser.addOption("v33", "Create GFF3 V3.3 file (default for The Witcher)", kContinueParsing,
	                 makeAssigners(new ValAssigner<GFFCreator::GFF3Version>(GFFCreator::GFF3Version::V3_3, gff3Version)));

	return parser.process(argv);
}

void createGFF(const Common::UString &inFile, const Common::UString &outFile, XML::GFFCreator::GFF3Version gff3Version) {
	std::unique_ptr<Common::ReadStream> xml(openFileOrStdIn(inFile));
	std::unique_ptr<Common::WriteStream> gff(openFileOrStdOut(outFile));

	XML::GFFCreator::create(*gff, *xml, inFile, gff3Version);

	gff->flush();
}
