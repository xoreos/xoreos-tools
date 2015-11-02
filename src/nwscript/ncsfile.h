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
 *  Handling BioWare's NCS, compiled NWScript bytecode.
 */

#ifndef NWSCRIPT_NCSFILE_H
#define NWSCRIPT_NCSFILE_H

#include <deque>

#include "src/common/types.h"
#include "src/common/ustring.h"

#include "src/aurora/types.h"
#include "src/aurora/aurorafile.h"

#include "src/nwscript/variable.h"
#include "src/nwscript/stack.h"
#include "src/nwscript/instruction.h"
#include "src/nwscript/block.h"
#include "src/nwscript/subroutine.h"

namespace Common {
	class SeekableReadStream;
}

namespace NWScript {

/** Parse an NCS file, compiled NWScript bytecode, into a structure of instructions. */
class NCSFile : public Aurora::AuroraBase {
public:
	typedef std::deque<Instruction> Instructions;
	typedef std::deque<Block>       Blocks;
	typedef std::deque<SubRoutine>  SubRoutines;


	/** Read a script out of the NCS bytecode stream.
	 *
	 *  The game this NCS is from can't be autodetected and has to be
	 *  manually specified. If this information is not supplied, the
	 *  script can still be read, but no deeper stack analysis can be
	 *  performed, and the meaning of which engine function (opcode
	 *  ACTION) is which is lost.
	 */
	NCSFile(Common::SeekableReadStream &ncs, Aurora::GameID game = Aurora::kGameIDUnknown);
	~NCSFile();

	/** Return the game this allegedly script is from.
	 *  This is the information that has been supplied in the constructor. */
	Aurora::GameID getGame() const;

	/** Perform a deep analysis of the script stack.
	 *  For this to work, a game value has to have been supplied in the constructor. */
	void analyzeStack();

	/** Return the size of the script bytecode in bytes.
	 *  Should be equal to the size of the containing stream. */
	size_t size() const;

	/** Did we successfully analyze the script stack? */
	bool hasStackAnalysis() const;

	/** Return all the instructions within this NCS file. */
	const Instructions &getInstructions() const;

	/** Return all blocks in this NCS file. */
	const Blocks &getBlocks() const;

	/** Return the root block of this NCS file. */
	const Block &getRootBlock() const;

	/** Return all subroutines in this NCS file. */
	const SubRoutines &getSubRoutines() const;

	/** Return the _start() subroutine where execution starts.
	 *  If there are no subroutines in this script at all, return 0. */
	const SubRoutine *getStartSubRoutine() const;

	/** Return the _global() subroutine that sets up global variables.
	 *  If there is no such subroutine in this script, return 0. */
	const SubRoutine *getGlobalSubRoutine() const;

	/** Return the main() subroutine.
	 *  If we failed to identify the main subroutine, return 0. */
	const SubRoutine *getMainSubRoutine() const;

	/** Find an instruction by address. */
	const Instruction *findInstruction(uint32 address) const;

	/** Return all the variables we found while analyzing this script. */
	const VariableSpace &getVariables() const;

	/** Return the global variables we found while analyzing this script. */
	const Stack &getGlobals() const;


private:
	Aurora::GameID _game;

	size_t _size;

	Instructions _instructions;
	Blocks       _blocks;
	SubRoutines  _subRoutines;

	bool _multipleGlobal;

	SubRoutine *_startSubRoutine;
	SubRoutine *_globalSubRoutine;
	SubRoutine *_mainSubRoutine;

	bool _hasStackAnalysis;

	VariableSpace _variables;
	Stack _globals;


	void load(Common::SeekableReadStream &ncs);
	void parse(Common::SeekableReadStream &ncs);

	bool parseStep(Common::SeekableReadStream &ncs);

	void linkBranches();
	void findBlocks();
	void findDeadEdges();

	void identifySubRoutineTypes();

	void constructBlocks  (SubRoutine  &sub, Block &block, const Instruction &instr);
	void followBranchBlock(SubRoutine  &sub, Block &block, const Instruction &instr);
	bool addBranchBlock   (SubRoutine *&sub, Block &block, const Instruction &branchDestination,
	                       Block *&branchBlock, BlockEdgeType type);


	Instructions::iterator findInstruction(uint32 address);
};

} // End of namespace NWScript

#endif // NWSCRIPT_NCSFILE_H
