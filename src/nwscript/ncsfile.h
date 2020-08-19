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
 *  Handling BioWare's NCS, compiled NWScript bytecode.
 */

#ifndef NWSCRIPT_NCSFILE_H
#define NWSCRIPT_NCSFILE_H

#include <boost/noncopyable.hpp>

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

/** Parse an NCS file, compiled NWScript bytecode, into a structure of
 *  instructions.
 *
 *  This structure is then automatically analyzed on a block and subroutine
 *  level, giving access to a control flow graph of interconnected blocks of
 *  uninterrupted instructions, grouped into subroutines that call each other.
 *
 *  Additionally, a deeper stack analysis can be performed by calling the
 *  analyzeStack() method. Since this depends on game-dependant information,
 *  the game this script is from needs to be passed into the constructor for
 *  this to work.
 *
 *  Likewise, a deeper analysis of the control flow can be performed by calling
 *  the analyzeControlFlow() method. This also requires a GameID.
 */
class NCSFile : boost::noncopyable, public Aurora::AuroraFile {
public:
	NCSFile(Common::SeekableReadStream &ncs, Aurora::GameID game = Aurora::kGameIDUnknown);
	~NCSFile();

	/** Return the game this allegedly script is from.
	 *  This is the information that has been supplied in the constructor. */
	Aurora::GameID getGame() const;

	/** Perform a deep analysis of the script stack.
	 *  For this to work, a game value has to have been supplied in the constructor. */
	void analyzeStack();

	/** Perform a deep analysis of the control flow.
	 *  For this to work, a game value has to have been supplied in the constructor. */
	void analyzeControlFlow();

	/** Return the size of the script bytecode in bytes.
	 *  Should be equal to the size of the containing stream. */
	size_t size() const;

	/** Did we successfully analyze the script stack? */
	bool hasStackAnalysis() const;

	/** Did we successfully analyze the control flow? */
	bool hasControlFlowAnalysis() const;

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
	const Instruction *findInstruction(uint32_t address) const;

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

	SpecialSubRoutines _specialSubRoutines;

	bool _hasStackAnalysis;
	bool _hasControlFlowAnalysis;

	VariableSpace _variables;
	Stack _globals;


	void load(Common::SeekableReadStream &ncs);
	void parse(Common::SeekableReadStream &ncs);

	bool parseStep(Common::SeekableReadStream &ncs);

	void analyzeBlocks();
	void analyzeSubRoutines();
};

} // End of namespace NWScript

#endif // NWSCRIPT_NCSFILE_H
