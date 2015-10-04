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

#include "src/nwscript/ncsfile.h"
#include "src/nwscript/util.h"

void printUsage(FILE *stream, const Common::UString &name);
bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile);

void disNCS(Common::SeekableReadStream &ncsFile, Common::WriteStream &out);
void disNCS(const Common::UString &inFile, const Common::UString &outFile);

int main(int argc, char **argv) {
	std::vector<Common::UString> args;
	Common::Platform::getParameters(argc, argv, args);

	int returnValue = 1;
	Common::UString inFile, outFile;

	try {
		if (!parseCommandLine(args, returnValue, inFile, outFile))
			return returnValue;

		disNCS(inFile, outFile);
	} catch (Common::Exception &e) {
		Common::printException(e);
		return 1;
	} catch (std::exception &e) {
		error("%s", e.what());
	}

	return 0;
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile) {

	inFile.clear();
	outFile.clear();
	std::vector<Common::UString> args;

	bool optionsEnd = false;
	for (size_t i = 1; i < argv.size(); i++) {
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

			if        (argv[i].beginsWith("-") || argv[i].beginsWith("--")) {
			  // An options, but we already checked for all known ones

				printUsage(stderr, argv[0]);
				returnValue = 1;

				return false;
			}
		}

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
	std::fprintf(stream, "BioWare NWScript bytecode disassembler\n\n");
	std::fprintf(stream, "Usage: %s [<options>] <input file> [<output file>]\n", name.c_str());
	std::fprintf(stream, "  -h      --help              This help text\n");
	std::fprintf(stream, "          --version           Display version information\n\n");
	std::fprintf(stream, "If no output file is given, the output is written to stdout.\n");
}

void disNCS(Common::SeekableReadStream &ncsFile, Common::WriteStream &out) {
	NWScript::NCSFile ncs(ncsFile);

	const NWScript::NCSFile::Instructions &instr = ncs.getInstructions();

	out.writeString(Common::UString::format("%u bytes, %u instructions\n\n",
	                (uint)ncs.size(), (uint)instr.size()));

	for (NWScript::NCSFile::Instructions::const_iterator i = instr.begin(); i != instr.end(); ++i) {
		// Print subroutine / jump labels
		if      (i->isSubRoutine)
			out.writeString(NWScript::formatSubRoutine(i->address) + ":\n");
		else if (i->isJumpDestination)
			out.writeString(NWScript::formatJumpDestination(i->address) + ":\n");

		// Print the actual disassembly line
		out.writeString(Common::UString::format("  %08X %-26s %s\n", i->address,
			              NWScript::formatBytes(*i).c_str(), NWScript::formatInstruction(*i).c_str()));

		// If this instruction has no natural follower, print a separator
		if (!i->follower)
			out.writeString("  -------- -------------------------- ---\n");
	}
}

void disNCS(const Common::UString &inFile, const Common::UString &outFile) {
	Common::SeekableReadStream *ncsFile = new Common::ReadFile(inFile);

	Common::WriteStream *out = 0;
	try {
		if (!outFile.empty())
			out = new Common::WriteFile(outFile);
		else
			out = new Common::StdOutStream;

		disNCS(*ncsFile, *out);

	} catch (...) {
		delete ncsFile;
		delete out;
		throw;
	}

	out->flush();

	if (!outFile.empty())
		status("Disassembled \"%s\" into \"%s\"", inFile.c_str(), outFile.c_str());

	delete ncsFile;
	delete out;
}
