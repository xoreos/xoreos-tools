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
 *  Disassembling NWScript bytecode.
 */

#ifndef NWSCRIPT_DISASSEMBLER_H
#define NWSCRIPT_DISASSEMBLER_H

#include <vector>

#include <boost/noncopyable.hpp>

#include "src/common/types.h"
#include "src/common/scopedptr.h"
#include "src/common/ustring.h"

#include "src/aurora/types.h"

namespace Common {
	class SeekableReadStream;
	class WriteStream;
}

namespace NWScript {

class NCSFile;

struct Instruction;
struct SubRoutine;
struct Block;

class Disassembler : boost::noncopyable {
public:
	Disassembler(Common::SeekableReadStream &ncs, Aurora::GameID game = Aurora::kGameIDUnknown);
	Disassembler(NCSFile *ncs);
	~Disassembler();

	/** Perform a deep analysis of the script stack, so that more information is available. */
	void analyzeStack();
	/** Perform a deep analysis of the control flow, so that more information is available. */
	void analyzeControlFlow();

	/** Create a full disassembly listing, with addresses and raw bytes. */
	void createListing (Common::WriteStream &out, bool printStack = false);
	/** Create bare disassembly output, potentially capable of being re-assembled. */
	void createAssembly(Common::WriteStream &out, bool printStack = false);
	/** Create a graphviz dot file that can be plotted into a control flow graph. */
	void createDot     (Common::WriteStream &out, bool printControlTypes = false);
	/** Create a decompiled nss file. */
	void createNSS     (Common::WriteStream &out);


private:
	Common::ScopedPtr<NCSFile> _ncs;


	void writeInfo       (Common::WriteStream &out);
	void writeEngineTypes(Common::WriteStream &out);
	void writeJumpLabel  (Common::WriteStream &out, const Instruction &instr);
	void writeStack      (Common::WriteStream &out, const Instruction &instr, size_t indent);

	Common::UString getSignature(const SubRoutine  &sub);
	Common::UString getSignature(const Instruction &instr);

	void writeDotClusteredBlocks(Common::WriteStream &out, bool printControlTypes);
	void writeDotBlocks         (Common::WriteStream &out, bool printControlTypes,
	                             const std::vector<const Block *> &blocks);
	void writeDotBlockEdges     (Common::WriteStream &out);

	void writeNSSSubRoutine(Common::WriteStream &out, const SubRoutine &subRoutine);
	void writeNSSBlock(Common::WriteStream &out, const Block *block, size_t indent);
	void writeNSSIfBlock(Common::WriteStream &out, const NWScript::ControlStructure &control, size_t indent);
	void writeNSSInstruction(Common::WriteStream &out, const Instruction *instruction, size_t indent);
	void writeNSSIndent(Common::WriteStream &out, size_t indent);
};

} // End of namespace NWScript

#endif // NWSCRIPT_DISASSEMBLER_H
