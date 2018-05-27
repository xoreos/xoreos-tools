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
 *  Tool to disassemble NWScript bytecode.
 */

#include <cassert>
#include <cstring>
#include <cstdio>

#include <vector>

#include "src/version/version.h"

#include "src/common/scopedptr.h"
#include "src/common/ustring.h"
#include "src/common/util.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/readfile.h"
#include "src/common/writefile.h"
#include "src/common/stdoutstream.h"
#include "src/common/cli.h"

#include "src/aurora/types.h"

#include "src/nwscript/disassembler.h"

#include "src/util.h"

enum Command {
	kCommandNone     = -1,
	kCommandListing  =  0,
	kCommandAssembly =  1,
	kCommandDot      =  2,
	kCommandMAX
};

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile,
                      Aurora::GameID &game, Command &command,
                      bool &printStack, bool &printControlTypes);

void disNCS(const Common::UString &inFile, const Common::UString &outFile,
            Aurora::GameID &game, Command &command, bool printStack, bool printControlTypes);

int main(int argc, char **argv) {
	initPlatform();

	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		Aurora::GameID game = Aurora::kGameIDUnknown;

		int returnValue = 1;
		Command command = kCommandListing;
		bool printStack = false;
		bool printControlTypes = false;
		Common::UString inFile, outFile;

		if (!parseCommandLine(args, returnValue, inFile, outFile, game, command, printStack, printControlTypes))
			return returnValue;

		disNCS(inFile, outFile, game, command, printStack, printControlTypes);
	} catch (...) {
		Common::exceptionDispatcherError();
	}

	return 0;
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile,
                      Aurora::GameID &game, Command &command,
                      bool &printStack, bool &printControlTypes) {
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
	Parser parser(argv[0], "BioWare NWScript bytecode disassembler",
	              "\nIf no output file is given, the output is written to stdout.",
	              returnValue,
	              makeEndArgs(&inFileOpt, &outFileOpt));

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
	parser.addSpace();
	parser.addOption("list", "Create full disassembly listing (default)", kContinueParsing,
	                 makeAssigners(new ValAssigner<Command>(kCommandListing, command)));
	parser.addOption("assembly", "Only create disassembly mnemonics", kContinueParsing,
	                 makeAssigners(new ValAssigner<Command>(kCommandAssembly, command)));
	parser.addOption("dot", "Create a graphviz dot file", kContinueParsing,
	                 makeAssigners(new ValAssigner<Command>(kCommandDot, command)));
	parser.addOption("stack", "Print the stack frame for each instruction"
	                 " (Only available in list or assembly mode)",
	                 kContinueParsing,
	                 makeAssigners(new ValAssigner<bool>(true, printStack)));
	parser.addOption("control", "Print the control types for each block"
	                 " (Only available in list or assembly mode)",
	                 kContinueParsing,
	                 makeAssigners(new ValAssigner<bool>(true, printControlTypes)));
	return parser.process(argv);
}

void disNCS(const Common::UString &inFile, const Common::UString &outFile,
            Aurora::GameID &game, Command &command, bool printStack, bool printControlTypes) {

	Common::ScopedPtr<Common::SeekableReadStream> ncs(new Common::ReadFile(inFile));
	Common::ScopedPtr<Common::WriteStream> out(openFileOrStdOut(outFile));

	status("Disassembling script...");
	NWScript::Disassembler disassembler(*ncs, game);

	if (game != Aurora::kGameIDUnknown) {
		try {
			status("Analyzing script stack...");
			disassembler.analyzeStack();
		} catch (...) {
			Common::exceptionDispatcherWarnAndIgnore("Script analysis failed");
		}

		try {
			status("Analyzing control flow...");
			disassembler.analyzeControlFlow();
		} catch (...) {
			Common::exceptionDispatcherWarnAndIgnore("Control flow analysis failed");
		}
	}

	switch (command) {
		case kCommandListing:
			disassembler.createListing(*out, printStack);
			break;

		case kCommandAssembly:
			disassembler.createAssembly(*out, printStack);
			break;

		case kCommandDot:
			disassembler.createDot(*out, printControlTypes);
			break;

		default:
			throw Common::Exception("Invalid command %u", (uint)command);
	}

	out->flush();

	if (!outFile.empty())
		status("Disassembled \"%s\" into \"%s\"", inFile.c_str(), outFile.c_str());
}
