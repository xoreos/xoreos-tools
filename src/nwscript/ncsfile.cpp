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

#include <cassert>

#include <algorithm>

#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"
#include "src/common/encoding.h"
#include "src/common/readstream.h"

#include "src/nwscript/ncsfile.h"
#include "src/nwscript/util.h"

static const uint32 kNCSID     = MKTAG('N', 'C', 'S', ' ');
static const uint32 kVersion10 = MKTAG('V', '1', '.', '0');

namespace NWScript {

static void parseOpcodeConst  (Instruction &instr, Common::SeekableReadStream &ncs);
static void parseOpcodeEq     (Instruction &instr, Common::SeekableReadStream &ncs);
static void parseOpcodeNEq    (Instruction &instr, Common::SeekableReadStream &ncs);
static void parseOpcodeStore  (Instruction &instr, Common::SeekableReadStream &ncs);
static void parseOpcodeDefault(Instruction &instr, Common::SeekableReadStream &ncs);

typedef void (*ParseFunc)(Instruction &instr, Common::SeekableReadStream &ncs);

struct OpcodeProc {
	ParseFunc parseFunc;
};

static const OpcodeProc kOpcodeProc[kOpcodeMAX] = {
	// 0x00
	/*               */ { 0                  },
	/* CPDOWNSP      */ { parseOpcodeDefault },
	/* RSADD         */ { parseOpcodeDefault },
	/* CPTOPSP       */ { parseOpcodeDefault },
	// 0x04
	/* CONST         */ { parseOpcodeConst   },
	/* ACTION        */ { parseOpcodeDefault },
	/* LOGAND        */ { parseOpcodeDefault },
	/* LOGAR         */ { parseOpcodeDefault },
	// 0x08
	/* INCOR         */ { parseOpcodeDefault },
	/* EXCOR         */ { parseOpcodeDefault },
	/* BOOLAND       */ { parseOpcodeDefault },
	/* EQ            */ { parseOpcodeEq      },
	// 0x0C
	/* NEQ           */ { parseOpcodeNEq     },
	/* GEQ           */ { parseOpcodeDefault },
	/* GT            */ { parseOpcodeDefault },
	/* LT            */ { parseOpcodeDefault },
	// 0x10
	/* LEQ           */ { parseOpcodeDefault },
	/* SHLEFT        */ { parseOpcodeDefault },
	/* SHRIGHT       */ { parseOpcodeDefault },
	/* USHRIGHT      */ { parseOpcodeDefault },
	// 0x14
	/* ADD           */ { parseOpcodeDefault },
	/* SUB           */ { parseOpcodeDefault },
	/* MUL           */ { parseOpcodeDefault },
	/* DIV           */ { parseOpcodeDefault },
	// 0x18
	/* MOD           */ { parseOpcodeDefault },
	/* NEG           */ { parseOpcodeDefault },
	/* COMP          */ { parseOpcodeDefault },
	/* MOVSP         */ { parseOpcodeDefault },
	// 0x1C
	/* STORESTATEALL */ { parseOpcodeDefault },
	/* JMP           */ { parseOpcodeDefault },
	/* JSR           */ { parseOpcodeDefault },
	/* JZ            */ { parseOpcodeDefault },
	// 0x20
	/* RETN          */ { parseOpcodeDefault },
	/* DESTRUCT      */ { parseOpcodeDefault },
	/* NOT           */ { parseOpcodeDefault },
	/* DECSP         */ { parseOpcodeDefault },
	// 0x24
	/* INCSP         */ { parseOpcodeDefault },
	/* JNZ           */ { parseOpcodeDefault },
	/* CPDOWNBP      */ { parseOpcodeDefault },
	/* CPTOPBP       */ { parseOpcodeDefault },
	// 0x28
	/* DECBP         */ { parseOpcodeDefault },
	/* INCBP         */ { parseOpcodeDefault },
	/* SAVEBP        */ { parseOpcodeDefault },
	/* RESTOREBP     */ { parseOpcodeDefault },
	// 0x2C
	/* STORESTATE    */ { parseOpcodeStore   },
	/* NOP           */ { parseOpcodeDefault },
	/*               */ { 0                  },
	/*               */ { 0                  },
	// 0x30
	/* WRITEARRAY    */ { parseOpcodeDefault },
	/*               */ { 0                  },
	/* READARRAY     */ { parseOpcodeDefault },
	/*               */ { 0                  },
	// 0x34
	/*               */ { 0                  },
	/*               */ { 0                  },
	/*               */ { 0                  },
	/* GETREF        */ { parseOpcodeDefault },
	// 0x38
	/*               */ { 0                  },
	/* GETREFARRAY   */ { parseOpcodeDefault },
	/*               */ { 0                  },
	/*               */ { 0                  },
	// 0x3C
	/*               */ { 0                  },
	/*               */ { 0                  },
	/*               */ { 0                  },
	/*               */ { 0                  },
	// 0x40
	/*               */ { 0                  },
	/*               */ { 0                  },
	/* SCRIPTSIZE    */ { parseOpcodeDefault }
};


NCSFile::NCSFile(Common::SeekableReadStream &ncs) : _size(0) {
	load(ncs);
}

NCSFile::~NCSFile() {
}

size_t NCSFile::size() const {
	return _size;
}

const NCSFile::Instructions &NCSFile::getInstructions() const {
	return _instructions;
}

const Instruction *NCSFile::findInstruction(uint32 address) const {
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

	} catch (Common::Exception &e) {
		e.add("Failed to load NCS file");

		throw e;
	}
}

NCSFile::Instructions::iterator NCSFile::findInstruction(uint32 address) {
	Instructions::iterator it = std::lower_bound(_instructions.begin(), _instructions.end(), address);
	if ((it == _instructions.end()) || (it->address != address))
		return _instructions.end();

	return it;
}

void NCSFile::parse(Common::SeekableReadStream &ncs) {
	while (parseStep(ncs))
		;

	// Go through all instructions and link them according to the flow graph
	for (Instructions::iterator i = _instructions.begin(); i != _instructions.end(); ++i) {
		// If this is an instruction that has a natural follower, link it
		if ((i->opcode != kOpcodeJMP) && (i->opcode != kOpcodeRETN)) {
			Instructions::iterator follower = i + 1;

			i->follower = (follower != _instructions.end()) ? &*follower : 0;
		}

		// Link destinations of unconditional branches
		if ((i->opcode == kOpcodeJMP) || (i->opcode == kOpcodeJSR)) {
			assert(i->argCount == 1);

			Instructions::iterator branch = findInstruction(i->address + i->args[0]);
			if (branch == _instructions.end())
				throw Common::Exception("Can't find destination of unconditional branch");

			i->branches.push_back(&*branch);

			if (i->opcode == kOpcodeJSR)
				// If this is a JSR opcode, the destination starts a subroutine
				branch->isSubRoutine = true;
			else
				// Otherwise, it's at least a jump destination still
				branch->isJumpDestination = true;
		}

		// Link destinations of conditional branches
		if ((i->opcode == kOpcodeJZ) || (i->opcode == kOpcodeJNZ)) {
			assert(i->argCount == 1);

			if (!i->follower)
				throw Common::Exception("Conditional branch has no false destination");

			Instructions::iterator branch = findInstruction(i->address + i->args[0]);
			if (branch == _instructions.end())
				throw Common::Exception("Can't find destination of conditional branch");

			branch->isJumpDestination = true;

			i->branches.push_back(&*branch);    // True branch
			i->branches.push_back(i->follower); // False branch
		}
	}
}

bool NCSFile::parseStep(Common::SeekableReadStream &ncs) {
	Instruction instr((uint32) ncs.pos());

	try {
		instr.opcode = (Opcode)          ncs.readByte();
		instr.type   = (InstructionType) ncs.readByte();
	} catch (...) {
		if (ncs.eos())
			return false;

		throw;
	}

	if (((size_t)instr.opcode >= ARRAYSIZE(kOpcodeProc)) || !kOpcodeProc[(size_t)instr.opcode].parseFunc)
		throw Common::Exception("Invalid opcode 0x%02X", (uint8)instr.opcode);

	const OpcodeProc &info = kOpcodeProc[(size_t)instr.opcode];
	(*info.parseFunc)(instr, ncs);

	_instructions.push_back(instr);

	return true;
}

void parseOpcodeConst(Instruction &instr, Common::SeekableReadStream &ncs) {
	switch (instr.type) {
		case kInstTypeInt:
			instr.constValueInt = ncs.readSint32BE();
			break;

		case kInstTypeFloat:
			instr.constValueFloat = ncs.readIEEEFloatBE();
			break;

		case kInstTypeString:
		case kInstTypeResource:
			try {
				instr.constValueString = Common::readStringFixed(ncs, Common::kEncodingASCII, ncs.readUint16BE());
			} catch (...) {
				instr.constValueString = "[INVALID STRING]";
			}
			break;

		case kInstTypeObject:
			instr.constValueObject = ncs.readUint32BE();
			break;

		default:
			throw Common::Exception("Illegal type for opcode CONST: 0x%02X", (uint8)instr.type);
	}

	instr.argTypes[0] = kOpcodeArgVariable;

	instr.argCount = 1;
}

void parseOpcodeEq(Instruction &instr, Common::SeekableReadStream &ncs) {
	if (instr.type != kInstTypeStructStruct)
		return;

	instr.args    [0] = ncs.readUint16BE();
	instr.argTypes[0] = kOpcodeArgSint16;

	instr.argCount = 1;
}

void parseOpcodeNEq(Instruction &instr, Common::SeekableReadStream &ncs) {
	if (instr.type != kInstTypeStructStruct)
		return;

	instr.args    [0] = ncs.readUint16BE();
	instr.argTypes[0] = kOpcodeArgSint16;

	instr.argCount = 1;
}

void parseOpcodeStore(Instruction &instr, Common::SeekableReadStream &ncs) {
	instr.args[0] = (uint8) instr.type;
	instr.args[1] = ncs.readUint32BE();
	instr.args[2] = ncs.readUint32BE();

	instr.argTypes[0] = kOpcodeArgUint8;
	instr.argTypes[1] = kOpcodeArgUint32;
	instr.argTypes[2] = kOpcodeArgUint32;

	instr.argCount = 3;

	instr.type = kInstTypeDirect;
}

void parseOpcodeDefault(Instruction &instr, Common::SeekableReadStream &ncs) {
	instr.argCount = getDirectArgumentCount(instr.opcode);

	const OpcodeArgument * const args = getDirectArguments(instr.opcode);
	for (size_t i = 0; i < instr.argCount; i++) {
		instr.argTypes[i] = args[i];

		switch (instr.argTypes[i]) {
			case kOpcodeArgUint8:
				instr.args[i] = ncs.readByte();
				break;

			case kOpcodeArgUint16:
				instr.args[i] = ncs.readUint16BE();
				break;

			case kOpcodeArgSint16:
				instr.args[i] = ncs.readSint16BE();
				break;

			case kOpcodeArgSint32:
				instr.args[i] = ncs.readSint32BE();
				break;

			case kOpcodeArgUint32:
				instr.args[i] = (int32)ncs.readUint32BE();
				break;

			default:
				break;
		}
	}
}

} // End of namespace NWScript
