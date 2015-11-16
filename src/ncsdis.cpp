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

#include "src/common/version.h"
#include "src/common/ustring.h"
#include "src/common/util.h"
#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/readfile.h"
#include "src/common/writefile.h"
#include "src/common/stdoutstream.h"

#include "src/aurora/types.h"

#include "src/nwscript/disassembler.h"

enum Command {
	kCommandNone     = -1,
	kCommandListing  =  0,
	kCommandAssembly =  1,
	kCommandDot      =  2,
	kCommandMAX
};

void printUsage(FILE *stream, const Common::UString &name);
bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile,
                      Aurora::GameID &game, Command &command,
                      bool &printStack, bool &printControlTypes);

void disNCS(const Common::UString &inFile, const Common::UString &outFile,
            Aurora::GameID &game, Command &command, bool printStack, bool printControlTypes);

int main(int argc, char **argv) {
	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		Aurora::GameID game = Aurora::kGameIDUnknown;

		int returnValue = 1;
		Command command = kCommandNone;
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

	inFile.clear();
	outFile.clear();
	std::vector<Common::UString> args;

	command = kCommandListing;

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

			if        (argv[i] == "--list") {
				isOption = true;
				command  = kCommandListing;
			} else if (argv[i] == "--assembly") {
				isOption = true;
				command  = kCommandAssembly;
			} else if (argv[i] == "--dot") {
				isOption = true;
				command  = kCommandDot;
			} else if (argv[i] == "--stack") {
				isOption   = true;
				printStack = true;
			} else if (argv[i] == "--control") {
				isOption          = true;
				printControlTypes = true;
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

	assert(command != kCommandNone);

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
	std::fprintf(stream, "BioWare NWScript bytecode disassembler\n\n");
	std::fprintf(stream, "Usage: %s [<options>] <input file> [<output file>]\n", name.c_str());
	std::fprintf(stream, "  -h      --help              This help text\n");
	std::fprintf(stream, "          --version           Display version information\n\n");
	std::fprintf(stream, "          --list              Create full disassembly listing (default)\n");
	std::fprintf(stream, "          --assembly          Only create disassembly mnemonics\n");
	std::fprintf(stream, "          --dot               Create a graphviz dot file\n\n");
	std::fprintf(stream, "          --stack             Print the stack frame for each instruction\n");
	std::fprintf(stream, "                              (Only available in list or assembly mode)\n");
	std::fprintf(stream, "          --control           Print the control types for each block\n");
	std::fprintf(stream, "                              (Only available in dot mode)\n\n");
	std::fprintf(stream, "          --nwn               This is a Neverwinter Nights script\n");
	std::fprintf(stream, "          --nwn2              This is a Neverwinter Nights 2 script\n");
	std::fprintf(stream, "          --kotor             This is a Knights of the Old Republic script\n");
	std::fprintf(stream, "          --kotor2            This is a Knights of the Old Republic II script\n");
	std::fprintf(stream, "          --jade              This is a Jade Empire script\n");
	std::fprintf(stream, "          --witcher           This is a The Witcher script\n");
	std::fprintf(stream, "          --dragonage         This is a Dragon Age script\n");
	std::fprintf(stream, "          --dragonage2        This is a Dragon Age II script\n\n");
	std::fprintf(stream, "If no output file is given, the output is written to stdout.\n");
}

void disNCS(const Common::UString &inFile, const Common::UString &outFile,
            Aurora::GameID &game, Command &command, bool printStack, bool printControlTypes) {

	Common::SeekableReadStream *ncs = new Common::ReadFile(inFile);

	Common::WriteStream *out = 0;
	try {
		if (!outFile.empty())
			out = new Common::WriteFile(outFile);
		else
			out = new Common::StdOutStream;

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

	} catch (...) {
		delete ncs;
		delete out;
		throw;
	}

	out->flush();

	if (!outFile.empty())
		status("Disassembled \"%s\" into \"%s\"", inFile.c_str(), outFile.c_str());

	delete ncs;
	delete out;
}
