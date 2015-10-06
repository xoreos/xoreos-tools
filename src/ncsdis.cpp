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
#include <map>

#include "src/common/version.h"
#include "src/common/ustring.h"
#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/maths.h"
#include "src/common/platform.h"
#include "src/common/readfile.h"
#include "src/common/writefile.h"
#include "src/common/stdoutstream.h"

#include "src/aurora/types.h"

#include "src/nwscript/ncsfile.h"
#include "src/nwscript/util.h"
#include "src/nwscript/game.h"

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
                      Aurora::GameID &game, Command &command);

void disNCS(const Common::UString &inFile, const Common::UString &outFile,
            Aurora::GameID &game, Command &command);
void createList(NWScript::NCSFile &ncs, Common::WriteStream &out, Aurora::GameID &game);
void createAssembly(NWScript::NCSFile &ncs, Common::WriteStream &out, Aurora::GameID &game);

int main(int argc, char **argv) {
	std::vector<Common::UString> args;
	Common::Platform::getParameters(argc, argv, args);

	Aurora::GameID game = Aurora::kGameIDUnknown;

	int returnValue = 1;
	Command command = kCommandNone;
	Common::UString inFile, outFile;

	try {
		if (!parseCommandLine(args, returnValue, inFile, outFile, game, command))
			return returnValue;

		disNCS(inFile, outFile, game, command);
	} catch (Common::Exception &e) {
		Common::printException(e);
		return 1;
	} catch (std::exception &e) {
		error("%s", e.what());
	}

	return 0;
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &inFile, Common::UString &outFile,
                      Aurora::GameID &game, Command &command) {

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
	std::fprintf(stream, "          --assembly          Only create disassembly mnemonics\n\n");
	std::fprintf(stream, "          --dot               Create a graphviz dot file\n\n");
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

void createList(NWScript::NCSFile &ncs, Common::WriteStream &out, Aurora::GameID &game) {
	const NWScript::NCSFile::Instructions &instr = ncs.getInstructions();

	out.writeString(Common::UString::format("%u bytes, %u instructions\n\n",
	                (uint)ncs.size(), (uint)instr.size()));

	size_t engineTypeCount = NWScript::getEngineTypeCount(game);
	if (engineTypeCount > 0) {
		out.writeString("Engine types:\n");

		for (size_t i = 0; i < engineTypeCount; i++) {
			const Common::UString name = NWScript::getEngineTypeName(game, i);
			if (name.empty())
				continue;

			const Common::UString gName = NWScript::getGenericEngineTypeName(i);

			out.writeString(Common::UString::format("%s: %s\n", gName.c_str(), name.c_str()));
		}

		out.writeString("\n");
	}

	for (NWScript::NCSFile::Instructions::const_iterator i = instr.begin(); i != instr.end(); ++i) {
		// Print jump label
		const Common::UString jumpLabel = NWScript::formatJumpLabel(*i);
		if (!jumpLabel.empty())
			out.writeString(jumpLabel + ":\n");

		// Print the actual disassembly line
		out.writeString(Common::UString::format("  %08X %-26s %s\n", i->address,
			              NWScript::formatBytes(*i).c_str(), NWScript::formatInstruction(*i, game).c_str()));

		// If this instruction has no natural follower, print a separator
		if (!i->follower)
			out.writeString("  -------- -------------------------- ---\n");
	}
}

void createAssembly(NWScript::NCSFile &ncs, Common::WriteStream &out, Aurora::GameID &game) {
	const NWScript::NCSFile::Instructions &instr = ncs.getInstructions();

	for (NWScript::NCSFile::Instructions::const_iterator i = instr.begin(); i != instr.end(); ++i) {
		// Print jump label
		const Common::UString jumpLabel = NWScript::formatJumpLabel(*i);
		if (!jumpLabel.empty())
			out.writeString(jumpLabel + ":\n");

		// Print the actual disassembly line
		out.writeString(Common::UString::format("  %s\n", NWScript::formatInstruction(*i, game).c_str()));

		// If this instruction has no natural follower, print an empty line as separator
		if (!i->follower)
			out.writeString("\n");
	}
}

static Common::UString quoteString(const Common::UString &str) {
	Common::UString out;

	for (Common::UString::iterator c = str.begin(); c != str.end(); ++c) {
		if      (*c == '\\')
			out += "\\\\";
		else if (*c == '"')
			out += "\\\"";
		else
			out += *c;
	}

	return out;
}

void createDot(NWScript::NCSFile &ncs, Common::WriteStream &out, Aurora::GameID &game) {
	/* This creates a GraphViz dot file, which can be drawn into a graph image
	 * with graphviz's dot tool.
	 *
	 * Each block of NWScript instructions is drawn into one (or several, for large blocks)
	 * node, clustered by subroutine. Edges are drawn between the nodes to show the control
	 * flow.
	 */

	// Max number of instructions per node
	static const size_t kMaxNodeSize = 10;

	const NWScript::NCSFile::SubRoutines &subs   = ncs.getSubRoutines();
	const NWScript::NCSFile::Blocks      &blocks = ncs.getBlocks();

	out.writeString("digraph {\n");
	out.writeString("  overlap=false\n");
	out.writeString("  concentrate=true\n");
	out.writeString("  splines=ortho\n\n");

	std::map<uint32, size_t> blockNodeCount;

	// Block nodes grouped into subroutines clusters
	for (NWScript::NCSFile::SubRoutines::const_iterator s = subs.begin(); s != subs.end(); ++s) {
		if (s->blocks.empty() || s->blocks.front()->instructions.empty())
			continue;

		out.writeString(Common::UString::format(
		                "  subgraph cluster_s%08X {\n"
		                "    style=filled\n"
		                "    color=lightgrey\n", s->address));

		if (s->blocks.front()->instructions.front()->addressType == NWScript::kAddressTypeStateStore)
			out.writeString(Common::UString::format("    label=\"%s\"\n\n",
			                NWScript::formatStateStore(s->address).c_str()));
		else
			out.writeString(Common::UString::format("    label=\"%s\"\n\n",
			                NWScript::formatSubRoutine(s->address).c_str()));

		// Blocks
		for (std::vector<const NWScript::Block *>::const_iterator b = s->blocks.begin();
		     b != s->blocks.end(); ++b) {

			/* To keep large nodes from messing up the layout, we divide blocks with
			 * a huge amount of instructions into several, equal-sized nodes. */

			const size_t nodeCount = ceil((*b)->instructions.size() / (double)kMaxNodeSize);

			std::vector<Common::UString> labels;
			labels.resize(nodeCount);

			blockNodeCount[(*b)->address] = nodeCount;

			const size_t linesPerNode = ceil((*b)->instructions.size() / (double)labels.size());

			labels[0] = NWScript::formatJumpLabel(*(*b)->instructions.front());
			if (labels[0].empty())
				labels[0] = NWScript::formatJumpDestination((*b)->instructions.front()->address);

			labels[0] += ":\\l";

			// Instructions
			for (size_t i = 0; i < (*b)->instructions.size(); i++) {
				const NWScript::Instruction &instr = *(*b)->instructions[i];

				labels[i / linesPerNode] += "  " + quoteString(NWScript::formatInstruction(instr, game)) + "\\l";
			}

			// Nodes
			for (size_t i = 0; i < labels.size(); i++) {
				const Common::UString name = Common::UString::format("b%08X_%u", (*b)->address, (uint)i);

				out.writeString(Common::UString::format("    \"%s\" ", name.c_str()));
				out.writeString("[ shape=\"box\" label=\"" + labels[i] + "\" ]\n");
			}

			// Edges between the divided block nodes
			if (labels.size() > 1) {
				for (size_t i = 0; i < labels.size(); i++) {
					out.writeString((i == 0) ? "    " : " -> ");
					out.writeString(Common::UString::format("b%08X_%u", (*b)->address, (uint)i));
				}
				out.writeString(" [ style=dotted ]\n");
			}

			if (b != --s->blocks.end())
				out.writeString("\n");
		}

		out.writeString("  }\n\n");
	}

	// Edges
	for (NWScript::NCSFile::Blocks::const_iterator b = blocks.begin(); b != blocks.end(); ++b) {
		assert(b->children.size() == b->childrenTypes.size());

		for (size_t i = 0; i < b->children.size(); i++) {

			out.writeString(Common::UString::format("  b%08X_%u -> b%08X_0", b->address,
			                (uint)(blockNodeCount[b->address] - 1), b->children[i]->address));

			Common::UString attr;

			// Color the edge specific to the flow type
			switch (b->childrenTypes[i]) {
				default:
				case NWScript::kBlockEdgeTypeUnconditional:
					attr = "color=blue";
					break;

				case NWScript::kBlockEdgeTypeConditionalTrue:
					attr = "color=green";
					break;

				case NWScript::kBlockEdgeTypeConditionalFalse:
					attr = "color=red";
					break;

				case NWScript::kBlockEdgeTypeFunctionCall:
					attr = "color=cyan";
					break;

				case NWScript::kBlockEdgeTypeFunctionReturn:
					attr = "color=orange";
					break;

				case NWScript::kBlockEdgeTypeStoreState:
					attr = "color=purple";
					break;
			}

			// If this is a jump back, make the edge bold
			if (b->children[i]->address < b->address)
				attr += " style=bold";

			// If this edge goes between subroutines, don't let the edge influence the node rank
			if (b->subRoutine != b->children[i]->subRoutine)
				attr += " constraint=false";

			out.writeString(" [ " + attr + " ]\n");
		}
	}

	out.writeString("}\n");
}

void disNCS(const Common::UString &inFile, const Common::UString &outFile,
            Aurora::GameID &game, Command &command) {

	Common::SeekableReadStream *ncsFile = new Common::ReadFile(inFile);

	Common::WriteStream *out = 0;
	try {
		if (!outFile.empty())
			out = new Common::WriteFile(outFile);
		else
			out = new Common::StdOutStream;

		NWScript::NCSFile ncs(*ncsFile);

		switch (command) {
			case kCommandListing:
				createList(ncs, *out, game);
				break;

			case kCommandAssembly:
				createAssembly(ncs, *out, game);
				break;

			case kCommandDot:
				createDot(ncs, *out, game);
				break;

			default:
				throw Common::Exception("Invalid command %u", (uint)command);
		}

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
