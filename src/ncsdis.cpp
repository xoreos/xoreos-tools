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
                      Aurora::GameID &game, Command &command, bool &printStack);

void disNCS(const Common::UString &inFile, const Common::UString &outFile,
            Aurora::GameID &game, Command &command, bool printStack);

void createList    (NWScript::NCSFile &ncs, Common::WriteStream &out, Aurora::GameID &game,
                    bool printStack);
void createAssembly(NWScript::NCSFile &ncs, Common::WriteStream &out, Aurora::GameID &game,
                    bool printStack);
void createDot     (NWScript::NCSFile &ncs, Common::WriteStream &out, Aurora::GameID &game);

int main(int argc, char **argv) {
	std::vector<Common::UString> args;
	Common::Platform::getParameters(argc, argv, args);

	Aurora::GameID game = Aurora::kGameIDUnknown;

	int returnValue = 1;
	Command command = kCommandNone;
	bool printStack = false;
	Common::UString inFile, outFile;

	try {
		if (!parseCommandLine(args, returnValue, inFile, outFile, game, command, printStack))
			return returnValue;

		disNCS(inFile, outFile, game, command, printStack);
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
                      Aurora::GameID &game, Command &command, bool &printStack) {

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
	std::fprintf(stream, "          --stack             Print the stack frame for each instruction \n\n");
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

static void writeInfo(Common::WriteStream &out, NWScript::NCSFile &ncs) {
	out.writeString(Common::UString::format("; %u bytes, %u instructions\n\n",
	                (uint)ncs.size(), (uint)ncs.getInstructions().size()));
}

static void writeEngineTypes(Common::WriteStream &out, Aurora::GameID &game) {
	size_t engineTypeCount = NWScript::getEngineTypeCount(game);
	if (engineTypeCount > 0) {
		out.writeString("; Engine types:\n");

		for (size_t i = 0; i < engineTypeCount; i++) {
			const Common::UString name = NWScript::getEngineTypeName(game, i);
			if (name.empty())
				continue;

			const Common::UString gName = NWScript::getGenericEngineTypeName(i);

			out.writeString(Common::UString::format("; %s: %s\n", gName.c_str(), name.c_str()));
		}

		out.writeString("\n");
	}
}

static void writeStack(Common::WriteStream &out, size_t indent,
                       const NWScript::Instruction &instr, Aurora::GameID &game) {

	out.writeString(Common::UString(' ', indent));
	out.writeString(Common::UString::format("; .--- Stack: %3u ---\n", (uint)instr.stack.size()));

	for (size_t s = 0; s < instr.stack.size(); s++) {
		out.writeString(Common::UString(' ', indent));
		out.writeString(Common::UString::format("; | %04u - %06u: %s (%08X)\n",
		    (uint)s, (uint)instr.stack[s].variable->id,
		    NWScript::getVariableTypeName(instr.stack[s].variable->type, game).toLower().c_str(),
		    instr.stack[s].variable->creator ? instr.stack[s].variable->creator->address : 0));
	}

	out.writeString(Common::UString(' ', indent));
	out.writeString("; '--- ---------- ---\n");
}

static Common::UString getSignature(NWScript::NCSFile &ncs, const NWScript::SubRoutine &sub,
                                    Aurora::GameID &game) {
	if (!ncs.hasStackAnalysis())
		return "";

	if ((sub.type == NWScript::kSubRoutineTypeStart) || (sub.type == NWScript::kSubRoutineTypeGlobal) ||
	    (sub.type == NWScript::kSubRoutineTypeStoreState))
		return "";

	if (sub.stackAnalyzeState != NWScript::kStackAnalyzeStateFinished)
		return "";

	return NWScript::formatSignature(sub, game);
}

static Common::UString getSignature(NWScript::NCSFile &ncs, const NWScript::Instruction &instr,
                                    Aurora::GameID &game) {
	if (!ncs.hasStackAnalysis())
		return "";

	if ((instr.addressType != NWScript::kAddressTypeSubRoutine) || !instr.block || !instr.block->subRoutine)
		return "";

	return getSignature(ncs, *instr.block->subRoutine, game);
}

static void writeJumpLabel(Common::WriteStream &out, NWScript::NCSFile &ncs,
                           const NWScript::Instruction &instr, Aurora::GameID &game) {

	Common::UString jumpLabel = NWScript::formatJumpLabelName(instr);
	if (!jumpLabel.empty()) {
		jumpLabel += ":";

		const Common::UString signature = getSignature(ncs, instr, game);
		if (!signature.empty())
			jumpLabel += " ; " + signature;
	}

	if (!jumpLabel.empty())
		out.writeString(jumpLabel + "\n");
}

void createList(NWScript::NCSFile &ncs, Common::WriteStream &out, Aurora::GameID &game,
                bool printStack) {

	writeInfo(out, ncs);
	writeEngineTypes(out, game);

	const NWScript::NCSFile::Instructions &instr = ncs.getInstructions();

	for (NWScript::NCSFile::Instructions::const_iterator i = instr.begin(); i != instr.end(); ++i) {
		writeJumpLabel(out, ncs, *i, game);

		if (printStack)
			writeStack(out, 36, *i, game);

		// Print the actual disassembly line
		out.writeString(Common::UString::format("  %08X %-26s %s\n", i->address,
			              NWScript::formatBytes(*i).c_str(), NWScript::formatInstruction(*i, game).c_str()));

		// If this instruction has no natural follower, print a separator
		if (!i->follower)
			out.writeString("  -------- -------------------------- ---\n");
	}
}

void createAssembly(NWScript::NCSFile &ncs, Common::WriteStream &out, Aurora::GameID &game,
                    bool printStack) {

	writeInfo(out, ncs);
	writeEngineTypes(out, game);

	const NWScript::NCSFile::Instructions &instr = ncs.getInstructions();

	for (NWScript::NCSFile::Instructions::const_iterator i = instr.begin(); i != instr.end(); ++i) {
		writeJumpLabel(out, ncs, *i, game);

		if (printStack)
			writeStack(out, 0, *i, game);

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

		Common::UString clusterLabel = getSignature(ncs, *s, game);
		if (clusterLabel.empty())
			clusterLabel = NWScript::formatJumpLabelName(*s);
		if (clusterLabel.empty())
			clusterLabel = NWScript::formatJumpDestination(s->address);

		out.writeString(Common::UString::format("    label=\"%s\"\n\n", clusterLabel.c_str()));

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

			labels[0] = NWScript::formatJumpLabelName(**b);
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

				case NWScript::kBlockEdgeTypeDead:
					attr = "color=gray40";
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
            Aurora::GameID &game, Command &command, bool printStack) {

	Common::SeekableReadStream *ncsFile = new Common::ReadFile(inFile);

	Common::WriteStream *out = 0;
	try {
		if (!outFile.empty())
			out = new Common::WriteFile(outFile);
		else
			out = new Common::StdOutStream;

		status("Disassembling script...");
		NWScript::NCSFile ncs(*ncsFile);

		if (game != Aurora::kGameIDUnknown) {
			try {
				status("Analyzing script stack...");
				ncs.analyzeStack(game);
			} catch (Common::Exception &e) {
				printStack = false;

				e.add("Script analysis failed");
				Common::printException(e, "WARNING: ");
			}
		}

		switch (command) {
			case kCommandListing:
				createList(ncs, *out, game, printStack);
				break;

			case kCommandAssembly:
				createAssembly(ncs, *out, game, printStack);
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
