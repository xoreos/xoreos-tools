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

#include <algorithm>

#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/encoding.h"
#include "src/common/readstream.h"

#include "src/nwscript/ncsfile.h"
#include "src/nwscript/util.h"
#include "src/nwscript/controlflow.h"

static const uint32_t kNCSID     = MKTAG('N', 'C', 'S', ' ');
static const uint32_t kVersion10 = MKTAG('V', '1', '.', '0');

namespace NWScript {

NCSFile::NCSFile(Common::SeekableReadStream &ncs, Aurora::GameID game) :
	_game(game), _size(0), _hasStackAnalysis(false), _hasControlFlowAnalysis(false) {

	load(ncs);
}

NCSFile::~NCSFile() {
}

Aurora::GameID NCSFile::getGame() const {
	return _game;
}

size_t NCSFile::size() const {
	return _size;
}

bool NCSFile::hasStackAnalysis() const {
	return _hasStackAnalysis;
}

bool NCSFile::hasControlFlowAnalysis() const {
	return _hasControlFlowAnalysis;
}

const Instructions &NCSFile::getInstructions() const {
	return _instructions;
}

const Blocks &NCSFile::getBlocks() const {
	return _blocks;
}

const Block &NCSFile::getRootBlock() const {
	if (_blocks.empty())
		throw Common::Exception("This NCS file is empty!");

	return _blocks.front();
}

const SubRoutines &NCSFile::getSubRoutines() const {
	return _subRoutines;
}

const SubRoutine *NCSFile::getStartSubRoutine() const {
	return _specialSubRoutines.startSub;
}

const SubRoutine *NCSFile::getGlobalSubRoutine() const {
	return _specialSubRoutines.globalSub;
}

const SubRoutine *NCSFile::getMainSubRoutine() const {
	return _specialSubRoutines.mainSub;
}

const Instruction *NCSFile::findInstruction(uint32_t address) const {
	Instructions::const_iterator it = std::lower_bound(_instructions.begin(), _instructions.end(), address);
	if ((it == _instructions.end()) || (it->address != address))
		return 0;

	return &*it;
}

void NCSFile::load(Common::SeekableReadStream &ncs) {
	try {
		readHeader(ncs);

		if (_id != kNCSID)
			throw Common::Exception("Not an NCS file (%s)", Common::debugTag(_id).c_str());

		if (_version != kVersion10)
			throw Common::Exception("Unsupported NCS file version %s", Common::debugTag(_version).c_str());

		const byte sizeOpcode = ncs.readByte();
		if (sizeOpcode != kOpcodeSCRIPTSIZE)
			throw Common::Exception("Script size opcode != 0x42 (0x%02X)", sizeOpcode);

		_size = ncs.readUint32BE();
		if (_size > ncs.size())
			throw Common::Exception("Script size %u > stream size %u", (uint)_size, (uint)ncs.size());

		if (_size < ncs.size())
			warning("Script size %u < stream size %u", (uint)_size, (uint)ncs.size());

		parse(ncs);

		analyzeBlocks();
		analyzeSubRoutines();

	} catch (Common::Exception &e) {
		e.add("Failed to load NCS file");

		throw e;
	}
}

const VariableSpace &NCSFile::getVariables() const {
	return _variables;
}

const Stack &NCSFile::getGlobals() const {
	return _globals;
}

void NCSFile::parse(Common::SeekableReadStream &ncs) {
	while (parseStep(ncs))
		;
}

bool NCSFile::parseStep(Common::SeekableReadStream &ncs) {
	Instruction instr;

	if (!parseInstruction(ncs, instr))
		return false;

	_instructions.push_back(instr);

	return true;
}

void NCSFile::analyzeBlocks() {
	/* Analyze the instructions on a block level. */

	// Link branching instructions to their destination instructions
	linkInstructionBranches(_instructions);
	// Construct a block graph by following the code flow
	constructBlocks(_blocks, _instructions);
	// Mark logically dead block edges
	findDeadBlockEdges(_blocks);
}

void NCSFile::analyzeSubRoutines() {
	/* Analyze the instructions and blocks on a subroutine level. */

	// Construct a set of subroutines over our blocks
	constructSubRoutines(_subRoutines, _blocks);
	// Interlink subroutine callers and callees
	linkSubRoutineCallers(_subRoutines);
	// Find entry and exist points of our subroutines
	findSubRoutineEntryAndExits(_subRoutines);

	// Now analyze the subroutine types and see if we can identify a few special ones
	try {
		_specialSubRoutines = analyzeSubRoutineTypes(_subRoutines);
	} catch (...) {
		Common::exceptionDispatcherWarnAndIgnore();
	}
}

void NCSFile::analyzeStack() {
	if ((_game == Aurora::kGameIDUnknown) || _hasStackAnalysis)
		return;

	if (!_specialSubRoutines.mainSub)
		throw Common::Exception("Failed to identify the main subroutine");

	_variables.clear();
	_globals.clear();

	if (_specialSubRoutines.globalSub)
		analyzeStackGlobals(*_specialSubRoutines.globalSub, _variables, _game, _globals);

	analyzeStackSubRoutine(*_specialSubRoutines.mainSub, _variables, _game, &_globals);

	_hasStackAnalysis = true;
}

void NCSFile::analyzeControlFlow() {
	if ((_game == Aurora::kGameIDUnknown) || _hasControlFlowAnalysis)
		return;

	NWScript::analyzeControlFlow(_blocks);

	_hasControlFlowAnalysis = true;
}

} // End of namespace NWScript
