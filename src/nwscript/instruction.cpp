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
 *  An instruction in BioWare's NWScript bytecode.
 */

#include <cassert>

#include "src/common/util.h"
#include "src/common/error.h"
#include "src/common/readstream.h"

#include "src/nwscript/instruction.h"
#include "src/nwscript/util.h"

namespace NWScript {

static void setAddressType(Instruction *instr, AddressType type) {
	/* Only update the address type of an instruction with a higher-priority one. */

	if (!instr || ((uint)instr->addressType >= (uint)type))
		return;

	instr->addressType = type;
}

static Instruction *findInstruction(Instructions &instructions, uint32_t address) {
	Instructions::iterator it = std::lower_bound(instructions.begin(), instructions.end(), address);
	if ((it == instructions.end()) || (it->address != address))
		return 0;

	return &*it;
}


typedef void (*ParseFunc)(Instruction &instr, Common::SeekableReadStream &ncs);

static void parseOpcodeConst  (Instruction &instr, Common::SeekableReadStream &ncs);
static void parseOpcodeEq     (Instruction &instr, Common::SeekableReadStream &ncs);
static void parseOpcodeNEq    (Instruction &instr, Common::SeekableReadStream &ncs);
static void parseOpcodeStore  (Instruction &instr, Common::SeekableReadStream &ncs);
static void parseOpcodeDefault(Instruction &instr, Common::SeekableReadStream &ncs);

static const ParseFunc kParseFunc[kOpcodeMAX] = {
	// 0x00
	/*               */ 0                 ,
	/* CPDOWNSP      */ parseOpcodeDefault,
	/* RSADD         */ parseOpcodeDefault,
	/* CPTOPSP       */ parseOpcodeDefault,
	// 0x04
	/* CONST         */ parseOpcodeConst  ,
	/* ACTION        */ parseOpcodeDefault,
	/* LOGAND        */ parseOpcodeDefault,
	/* LOGAR         */ parseOpcodeDefault,
	// 0x08
	/* INCOR         */ parseOpcodeDefault,
	/* EXCOR         */ parseOpcodeDefault,
	/* BOOLAND       */ parseOpcodeDefault,
	/* EQ            */ parseOpcodeEq     ,
	// 0x0C
	/* NEQ           */ parseOpcodeNEq    ,
	/* GEQ           */ parseOpcodeDefault,
	/* GT            */ parseOpcodeDefault,
	/* LT            */ parseOpcodeDefault,
	// 0x10
	/* LEQ           */ parseOpcodeDefault,
	/* SHLEFT        */ parseOpcodeDefault,
	/* SHRIGHT       */ parseOpcodeDefault,
	/* USHRIGHT      */ parseOpcodeDefault,
	// 0x14
	/* ADD           */ parseOpcodeDefault,
	/* SUB           */ parseOpcodeDefault,
	/* MUL           */ parseOpcodeDefault,
	/* DIV           */ parseOpcodeDefault,
	// 0x18
	/* MOD           */ parseOpcodeDefault,
	/* NEG           */ parseOpcodeDefault,
	/* COMP          */ parseOpcodeDefault,
	/* MOVSP         */ parseOpcodeDefault,
	// 0x1C
	/* STORESTATEALL */ parseOpcodeDefault,
	/* JMP           */ parseOpcodeDefault,
	/* JSR           */ parseOpcodeDefault,
	/* JZ            */ parseOpcodeDefault,
	// 0x20
	/* RETN          */ parseOpcodeDefault,
	/* DESTRUCT      */ parseOpcodeDefault,
	/* NOT           */ parseOpcodeDefault,
	/* DECSP         */ parseOpcodeDefault,
	// 0x24
	/* INCSP         */ parseOpcodeDefault,
	/* JNZ           */ parseOpcodeDefault,
	/* CPDOWNBP      */ parseOpcodeDefault,
	/* CPTOPBP       */ parseOpcodeDefault,
	// 0x28
	/* DECBP         */ parseOpcodeDefault,
	/* INCBP         */ parseOpcodeDefault,
	/* SAVEBP        */ parseOpcodeDefault,
	/* RESTOREBP     */ parseOpcodeDefault,
	// 0x2C
	/* STORESTATE    */ parseOpcodeStore  ,
	/* NOP           */ parseOpcodeDefault,
	/*               */ 0                 ,
	/*               */ 0                 ,
	// 0x30
	/* WRITEARRAY    */ parseOpcodeDefault,
	/*               */ 0                 ,
	/* READARRAY     */ parseOpcodeDefault,
	/*               */ 0                 ,
	// 0x34
	/*               */ 0                 ,
	/*               */ 0                 ,
	/*               */ 0                 ,
	/* GETREF        */ parseOpcodeDefault,
	// 0x38
	/*               */ 0                 ,
	/* GETREFARRAY   */ parseOpcodeDefault,
	/*               */ 0                 ,
	/*               */ 0                 ,
	// 0x3C
	/*               */ 0                 ,
	/*               */ 0                 ,
	/*               */ 0                 ,
	/*               */ 0                 ,
	// 0x40
	/*               */ 0                 ,
	/*               */ 0                 ,
	/* SCRIPTSIZE    */ parseOpcodeDefault
};

static Common::UString readStringQuoting(Common::SeekableReadStream &ncs, size_t length) {
	Common::UString str;

	while (length-- > 0) {
		byte c = ncs.readByte();
		if (!c)
			break;

		if      (c == '\n')
			str += "\\n";
		else if (c == '\r')
			str += "\\r";
		else if (c == '\t')
			str += "\\t";
		else if (c == '\"')
			str += "\\\"";
		else if (c == '\\')
			str += "\\\\";
		else if (c < 32 || c > 126)
			str += Common::UString::format("\\x%02X", c);
		else
			str += (uint32_t) c;
	}

	if (length != SIZE_MAX)
		ncs.skip(length);

	return str;
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
			instr.constValueString = readStringQuoting(ncs, ncs.readUint16BE());
			break;

		case kInstTypeObject:
			instr.constValueObject = ncs.readUint32BE();
			break;

		default:
			throw Common::Exception("Illegal type for opcode CONST: 0x%02X", (uint8_t)instr.type);
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
	instr.args[0] = (uint8_t) instr.type;
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
				instr.args[i] = (int32_t)ncs.readUint32BE();
				break;

			default:
				break;
		}
	}
}


bool parseInstruction(Common::SeekableReadStream &ncs, Instruction &instr) {
	instr.address = ncs.pos();

	try {
		instr.opcode = (Opcode)          ncs.readByte();
		instr.type   = (InstructionType) ncs.readByte();
	} catch (...) {
		if (ncs.eos())
			return false;

		throw;
	}

	if (((size_t)instr.opcode >= ARRAYSIZE(kParseFunc)) || !kParseFunc[(size_t)instr.opcode])
		throw Common::Exception("Invalid opcode 0x%02X", (uint8_t)instr.opcode);

	const ParseFunc func = kParseFunc[(size_t)instr.opcode];
	(*func)(instr, ncs);

	return true;
}

void linkInstructionBranches(Instructions &instructions) {
	/* Go through all instructions and link them according to the flow graph.
	 *
	 * In specifics, link each instruction's follower, the instruction that
	 * naturally follows if no branches are taken. Also fill in the branches
	 * array, which contains all branches an instruction can take. This
	 * directly creates an address type for each instruction: does it start
	 * a subroutine, is it a jump destination, is it a tail of a jump or none
	 * of these?
	 */

	for (Instructions::iterator i = instructions.begin(); i != instructions.end(); ++i) {
		// If this is an instruction that has a natural follower, link it
		if ((i->opcode != kOpcodeJMP) && (i->opcode != kOpcodeRETN)) {
			Instructions::iterator follower = i + 1;

			i->follower = (follower != instructions.end()) ? &*follower : 0;

			if (follower != instructions.end())
				follower->predecessors.push_back(&*i);
		}

		// Link destinations of unconditional branches
		if ((i->opcode == kOpcodeJMP) || (i->opcode == kOpcodeJSR) || (i->opcode == kOpcodeSTORESTATE)) {
			assert(((i->opcode == kOpcodeSTORESTATE) && (i->argCount == 3)) || (i->argCount == 1));

			Instruction *branch = findInstruction(instructions, i->address + i->args[0]);
			if (!branch)
				throw Common::Exception("Can't find destination of unconditional branch");

			i->branches.push_back(branch);

			if      (i->opcode == kOpcodeJSR)
				setAddressType(branch, kAddressTypeSubRoutine);
			else if (i->opcode == kOpcodeSTORESTATE)
				setAddressType(branch, kAddressTypeStoreState);
			else {
				setAddressType(branch, kAddressTypeJumpLabel);
				branch->predecessors.push_back(&*i);
			}

			setAddressType(const_cast<Instruction *>(i->follower), kAddressTypeTail);
		}

		// Link destinations of conditional branches
		if ((i->opcode == kOpcodeJZ) || (i->opcode == kOpcodeJNZ)) {
			assert(i->argCount == 1);

			if (!i->follower)
				throw Common::Exception("Conditional branch has no false destination");

			Instruction *branch = findInstruction(instructions, i->address + i->args[0]);
			if (!branch)
				throw Common::Exception("Can't find destination of conditional branch");

			setAddressType(branch, kAddressTypeJumpLabel);

			setAddressType(const_cast<Instruction *>(i->follower), kAddressTypeTail);

			i->branches.push_back(branch);      // True branch
			i->branches.push_back(i->follower); // False branch

			branch->predecessors.push_back(&*i);
		}
	}
}

} // End of namespace NWScript
