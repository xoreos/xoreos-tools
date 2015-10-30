/* xoreos - A reimplementation of BioWare's Aurora engine
 *
 * xoreos is the legal property of its developers, whose names
 * can be found in the AUTHORS file distributed with this source
 * distribution.
 *
 * xoreos is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * xoreos is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xoreos. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file
 *  Disassembling NWScript bytecode.
 */

#include <cassert>

#include "src/common/ustring.h"
#include "src/common/strutil.h"
#include "src/common/maths.h"
#include "src/common/readstream.h"
#include "src/common/writestream.h"

#include "src/nwscript/disassembler.h"
#include "src/nwscript/ncsfile.h"
#include "src/nwscript/util.h"
#include "src/nwscript/game.h"

namespace NWScript {

static void writeInfo(Common::WriteStream &out, NCSFile &ncs) {
	out.writeString(Common::UString::format("; %u bytes, %u instructions\n\n",
	                (uint)ncs.size(), (uint)ncs.getInstructions().size()));
}

static void writeEngineTypes(Common::WriteStream &out, Aurora::GameID game) {
	size_t engineTypeCount = getEngineTypeCount(game);
	if (engineTypeCount > 0) {
		out.writeString("; Engine types:\n");

		for (size_t i = 0; i < engineTypeCount; i++) {
			const Common::UString name = getEngineTypeName(game, i);
			if (name.empty())
				continue;

			const Common::UString gName = getGenericEngineTypeName(i);

			out.writeString(Common::UString::format("; %s: %s\n", gName.c_str(), name.c_str()));
		}

		out.writeString("\n");
	}
}

static void writeStack(Common::WriteStream &out, size_t indent,
                       const Instruction &instr, Aurora::GameID game) {

	out.writeString(Common::UString(' ', indent));
	out.writeString(Common::UString::format("; .--- Stack: %3u ---\n", (uint)instr.stack.size()));

	for (size_t s = 0; s < instr.stack.size(); s++) {
		const Variable &var = *instr.stack[s].variable;

		Common::UString siblings;
		for (std::set<const Variable *>::const_iterator sib = var.siblings.begin();
		     sib != var.siblings.end(); ++sib) {

			if (!siblings.empty())
				siblings += ",";

			siblings += Common::composeString((*sib)->id);
		}

		if (!siblings.empty())
			siblings = " (" + siblings + ")";

		out.writeString(Common::UString(' ', indent));
		out.writeString(Common::UString::format("; | %04u - %06u: %s (%08X)%s\n",
		    (uint)s, (uint)var.id, getVariableTypeName(var.type, game).toLower().c_str(),
		    var.creator ? var.creator->address : 0, siblings.c_str()));
	}

	out.writeString(Common::UString(' ', indent));
	out.writeString("; '--- ---------- ---\n");
}

static Common::UString getSignature(NCSFile &ncs, const SubRoutine &sub, Aurora::GameID game) {
	if (!ncs.hasStackAnalysis())
		return "";

	if ((sub.type == kSubRoutineTypeStart) || (sub.type == kSubRoutineTypeGlobal) ||
	    (sub.type == kSubRoutineTypeStoreState))
		return "";

	if (sub.stackAnalyzeState != kStackAnalyzeStateFinished)
		return "";

	return formatSignature(sub, game);
}

static Common::UString getSignature(NCSFile &ncs, const Instruction &instr, Aurora::GameID game) {
	if (!ncs.hasStackAnalysis())
		return "";

	if ((instr.addressType != kAddressTypeSubRoutine) || !instr.block || !instr.block->subRoutine)
		return "";

	return getSignature(ncs, *instr.block->subRoutine, game);
}

static void writeJumpLabel(Common::WriteStream &out, NCSFile &ncs,
                           const Instruction &instr, Aurora::GameID game) {

	Common::UString jumpLabel = formatJumpLabelName(instr);
	if (!jumpLabel.empty()) {
		jumpLabel += ":";

		const Common::UString signature = getSignature(ncs, instr, game);
		if (!signature.empty())
			jumpLabel += " ; " + signature;
	}

	if (!jumpLabel.empty())
		out.writeString(jumpLabel + "\n");
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


Disassembler::Disassembler(Common::SeekableReadStream &ncs, Aurora::GameID game) : _ncs(0) {
	_ncs = new NCSFile(ncs, game);
}

Disassembler::Disassembler(NCSFile *ncs) : _ncs(ncs) {
}

Disassembler::~Disassembler() {
	delete _ncs;
}

void Disassembler::analyzeStack() {
	_ncs->analyzeStack();
}

void Disassembler::createListing(Common::WriteStream &out, bool printStack) {
	writeInfo(out, *_ncs);
	writeEngineTypes(out, _ncs->getGame());

	const NCSFile::Instructions &instr = _ncs->getInstructions();

	for (NCSFile::Instructions::const_iterator i = instr.begin(); i != instr.end(); ++i) {
		writeJumpLabel(out, *_ncs, *i, _ncs->getGame());

		if (_ncs->hasStackAnalysis() && printStack)
			writeStack(out, 36, *i, _ncs->getGame());

		// Print the actual disassembly line
		out.writeString(Common::UString::format("  %08X %-26s %s\n", i->address,
			              formatBytes(*i).c_str(), formatInstruction(*i, _ncs->getGame()).c_str()));

		// If this instruction has no natural follower, print a separator
		if (!i->follower)
			out.writeString("  -------- -------------------------- ---\n");
	}
}

void Disassembler::createAssembly(Common::WriteStream &out, bool printStack) {
	writeInfo(out, *_ncs);
	writeEngineTypes(out, _ncs->getGame());

	const NCSFile::Instructions &instr = _ncs->getInstructions();

	for (NCSFile::Instructions::const_iterator i = instr.begin(); i != instr.end(); ++i) {
		writeJumpLabel(out, *_ncs, *i, _ncs->getGame());

		if (_ncs->hasStackAnalysis() && printStack)
			writeStack(out, 0, *i, _ncs->getGame());

		// Print the actual disassembly line
		out.writeString(Common::UString::format("  %s\n", formatInstruction(*i, _ncs->getGame()).c_str()));

		// If this instruction has no natural follower, print an empty line as separator
		if (!i->follower)
			out.writeString("\n");
	}
}

void Disassembler::createDot(Common::WriteStream &out) {
	/* This creates a GraphViz dot file, which can be drawn into a graph image
	 * with graphviz's dot tool.
	 *
	 * Each block of NWScript instructions is drawn into one (or several, for large blocks)
	 * node, clustered by subroutine. Edges are drawn between the nodes to show the control
	 * flow.
	 */

	out.writeString("digraph {\n");
	out.writeString("  overlap=false\n");
	out.writeString("  concentrate=true\n");
	out.writeString("  splines=ortho\n\n");

	writeDotClusteredBlocks(out);
	writeDotBlockEdges     (out);

	out.writeString("}\n");
}

void Disassembler::writeDotClusteredBlocks(Common::WriteStream &out) {
	const NCSFile::SubRoutines &subs = _ncs->getSubRoutines();

	// Block nodes grouped into subroutines clusters
	for (NCSFile::SubRoutines::const_iterator s = subs.begin(); s != subs.end(); ++s) {
		if (s->blocks.empty() || s->blocks.front()->instructions.empty())
			continue;

		out.writeString(Common::UString::format(
		                "  subgraph cluster_s%08X {\n"
		                "    style=filled\n"
		                "    color=lightgrey\n", s->address));

		Common::UString clusterLabel = getSignature(*_ncs, *s, _ncs->getGame());
		if (clusterLabel.empty())
			clusterLabel = formatJumpLabelName(*s);
		if (clusterLabel.empty())
			clusterLabel = formatJumpDestination(s->address);

		out.writeString(Common::UString::format("    label=\"%s\"\n\n", clusterLabel.c_str()));

		writeDotBlocks(out, s->blocks);

		out.writeString("  }\n\n");
	}
}

static size_t calculateNodesPerBlock(size_t blockSize) {
	// Max number of instructions per node
	static const size_t kMaxNodeSize = 10;

	return ceil(blockSize / (double)kMaxNodeSize);
}

void Disassembler::writeDotBlocks(Common::WriteStream &out, const std::vector<const Block *> &blocks) {
	for (std::vector<const Block *>::const_iterator b = blocks.begin();
	     b != blocks.end(); ++b) {

		/* To keep large nodes from messing up the layout, we divide blocks with
		 * a huge amount of instructions into several, equal-sized nodes. */

		const size_t nodeCount = calculateNodesPerBlock((*b)->instructions.size());

		std::vector<Common::UString> labels;
		labels.resize(nodeCount);

		const size_t linesPerNode = ceil((*b)->instructions.size() / (double)labels.size());

		labels[0] = formatJumpLabelName(**b);
		if (labels[0].empty())
			labels[0] = formatJumpDestination((*b)->instructions.front()->address);

		labels[0] += ":\\l";

		// Instructions
		for (size_t i = 0; i < (*b)->instructions.size(); i++) {
			const Instruction &instr = *(*b)->instructions[i];

			labels[i / linesPerNode] += "  " + quoteString(formatInstruction(instr, _ncs->getGame())) + "\\l";
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

		if (b != --blocks.end())
			out.writeString("\n");
	}
}

void Disassembler::writeDotBlockEdges(Common::WriteStream &out) {
	const NCSFile::Blocks &blocks = _ncs->getBlocks();

	for (NCSFile::Blocks::const_iterator b = blocks.begin(); b != blocks.end(); ++b) {
		assert(b->children.size() == b->childrenTypes.size());

		for (size_t i = 0; i < b->children.size(); i++) {

			out.writeString(Common::UString::format("  b%08X_%u -> b%08X_0", b->address,
			                (uint)(calculateNodesPerBlock(b->instructions.size()) - 1),
			                b->children[i]->address));

			Common::UString attr;

			// Color the edge specific to the flow type
			switch (b->childrenTypes[i]) {
				default:
				case kBlockEdgeTypeUnconditional:
					attr = "color=blue";
					break;

				case kBlockEdgeTypeConditionalTrue:
					attr = "color=green";
					break;

				case kBlockEdgeTypeConditionalFalse:
					attr = "color=red";
					break;

				case kBlockEdgeTypeFunctionCall:
					attr = "color=cyan";
					break;

				case kBlockEdgeTypeFunctionReturn:
					attr = "color=orange";
					break;

				case kBlockEdgeTypeStoreState:
					attr = "color=purple";
					break;

				case kBlockEdgeTypeDead:
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
}

} // End of namespace NWScript
