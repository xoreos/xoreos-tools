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
 *  Tool to decompiling NWScript bytecode.
 */

#include <memory>

#include "src/version/version.h"

#include "src/common/ustring.h"
#include "src/common/util.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/readfile.h"
#include "src/common/writefile.h"
#include "src/common/stdoutstream.h"
#include "src/common/cli.h"

#include "src/aurora/types.h"

#include "src/nwscript/decompiler.h"

#include "src/util.h"

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile,
                      Aurora::GameID &game);

void decNCS(const Common::UString &inFile, const Common::UString &outFile,
            Aurora::GameID &game);

int main(int argc, char **argv) {
	initPlatform();

	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		Aurora::GameID game = Aurora::kGameIDUnknown;

		int returnValue = 1;
		Common::UString inFile, outFile;

		if (!parseCommandLine(args, returnValue, inFile, outFile, game))
			return returnValue;

		if (game == Aurora::kGameIDUnknown)
			throw Common::Exception("No game id specified");

		decNCS(inFile, outFile, game);
	} catch (...) {
		Common::exceptionDispatcherError();
	}

	return 0;
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile,
                      Aurora::GameID &game) {
	using Common::CLI::NoOption;
	using Common::CLI::kContinueParsing;
	using Common::CLI::Parser;
	using Common::CLI::ValGetter;
	using Common::CLI::Callback;
	using Common::CLI::ValAssigner;
	using Common::CLI::makeEndArgs;
	using Common::CLI::makeAssigners;
	using Aurora::GameID;

	Common::UString encodingStr;
	NoOption inFileOpt(false, new ValGetter<Common::UString &>(inFile, "input files"));
	NoOption outFileOpt(true, new ValGetter<Common::UString &>(outFile, "output files"));
	Parser parser(argv[0], "BioWare NWScript bytecode decompiler",
	              "\nIf no output file is given, the output is written to stdout.",
	              returnValue,
	              makeEndArgs(&inFileOpt, &outFileOpt));

	parser.addSpace();
	parser.addOption("nwn", "This is a Neverwinter Nights script", kContinueParsing,
	                 makeAssigners(new ValAssigner<GameID>(Aurora::kGameIDNWN, game)));
	parser.addOption("nwn2", "This is a Neverwinter Nights 2 script", kContinueParsing,
	                 makeAssigners(new ValAssigner<GameID>(Aurora::kGameIDNWN2, game)));
	parser.addOption("kotor", "This is a Knights of the Old Republic script", kContinueParsing,
	                 makeAssigners(new ValAssigner<GameID>(Aurora::kGameIDKotOR, game)));
	parser.addOption("kotor2", "This is a Knights of the Old Republic II script", kContinueParsing,
	                 makeAssigners(new ValAssigner<GameID>(Aurora::kGameIDKotOR2, game)));
	parser.addOption("jade", "This is a Jade Empire script", kContinueParsing,
	                 makeAssigners(new ValAssigner<GameID>(Aurora::kGameIDJade, game)));
	parser.addOption("witcher", "This is a The Witcher script", kContinueParsing,
	                 makeAssigners(new ValAssigner<GameID>(Aurora::kGameIDWitcher, game)));
	parser.addOption("dragonage", "This is a Dragon Age script", kContinueParsing,
	                 makeAssigners(new ValAssigner<GameID>(Aurora::kGameIDDragonAge, game)));
	parser.addOption("dragonage2", "This is a Dragon Age II script", kContinueParsing,
	                 makeAssigners(new ValAssigner<GameID>(Aurora::kGameIDDragonAge2, game)));

	return parser.process(argv);
}

void decNCS(const Common::UString &inFile, const Common::UString &outFile, Aurora::GameID &game) {
	std::unique_ptr<Common::SeekableReadStream> ncs = std::make_unique<Common::ReadFile>(inFile);
	std::unique_ptr<Common::WriteStream> out(openFileOrStdOut(outFile));

	status("Decompiling script...");
	NWScript::Decompiler decompiler(*ncs, game);
	decompiler.createNSS(*out);

	out->flush();

	if (!outFile.empty())
		status("Decompiled \"%s\" into \"%s\"", inFile.c_str(), outFile.c_str());
}
