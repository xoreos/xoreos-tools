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

#ifndef NWSCRIPT_INSTRUCTION_H
#define NWSCRIPT_INSTRUCTION_H

#include <vector>
#include <deque>

#include "src/common/types.h"
#include "src/common/ustring.h"

#include "src/nwscript/stack.h"

namespace Common {
	class SeekableReadStream;
}

namespace NWScript {

struct Variable;

struct Block;
struct SubRoutine;

/** An instruction opcode, defining what it does. */
enum Opcode {
	kOpcodeCPDOWNSP      = 0x01, ///< CPDOWNSP. Copy a value into an existing stack element.
	kOpcodeRSADD         = 0x02, ///< RSADD. Push an empty element onto the stack.
	kOpcodeCPTOPSP       = 0x03, ///< CPTOPSP. Push a copy of a stack element on top of the stack.
	kOpcodeCONST         = 0x04, ///< CONST. Push a predetermined value onto the stack.
	kOpcodeACTION        = 0x05, ///< ACTION. Call a game-specific engine function.
	kOpcodeLOGAND        = 0x06, ///< LOGAND. Perform a logical boolean AND (&&).
	kOpcodeLOGOR         = 0x07, ///< LOGOR. Perform a logical boolean OR (||).
	kOpcodeINCOR         = 0x08, ///< INCOR. Perform a bit-wise inclusive OR (|).
	kOpcodeEXCOR         = 0x09, ///< EXCOR. Perform a bit-wise exclusive OR (^).
	kOpcodeBOOLAND       = 0x0A, ///< BOOLAND. Perform a bit-wise AND (&).
	kOpcodeEQ            = 0x0B, ///< EQ. Compare the top-most stack elements for equality (==).
	kOpcodeNEQ           = 0x0C, ///< NEQ. Compare the top-most stack elements for inequality (!=).
	kOpcodeGEQ           = 0x0D, ///< GEQ. Compare the top-most stack elements, greater-or-equal (>=).
	kOpcodeGT            = 0x0E, ///< GT. Compare the top-most stack elements, greater (>).
	kOpcodeLT            = 0x0F, ///< LT. Compare the top-most stack elements, less (<).
	kOpcodeLEQ           = 0x10, ///< LEQ. Compare the top-most stack elements, less-or-equal (<=).
	kOpcodeSHLEFT        = 0x11, ///< SHLEFT. Shift the top-most stack element to the left (<<).
	kOpcodeSHRIGHT       = 0x12, ///< SHRIGHT. Signed-shift the top-most stack element to the right (>>>).
	kOpcodeUSHRIGHT      = 0x13, ///< USHRIGHT. Shift the top-most stack element to the right (>>).
	kOpcodeADD           = 0x14, ///< ADD. Add the top-most stack elements (+).
	kOpcodeSUB           = 0x15, ///< SUB. Subtract the top-most stack elements (-).
	kOpcodeMUL           = 0x16, ///< MUL. Multiply the top-most stack elements (*).
	kOpcodeDIV           = 0x17, ///< DIV. Divide the top-most stack elements (/).
	kOpcodeMOD           = 0x18, ///< MOD. Calculate the remainder of an integer division (%).
	kOpcodeNEG           = 0x19, ///< NEG. Negate the top-most stack element (unary -).
	kOpcodeCOMP          = 0x1A, ///< COMP. Calculate the 1-complement of the top-most stack element (~).
	kOpcodeMOVSP         = 0x1B, ///< MOVSP. Pop elements off the stack.
	kOpcodeSTORESTATEALL = 0x1C, ///< STORESTATEALL. Unused, obsolete opcode.
	kOpcodeJMP           = 0x1D, ///< JMP. Jump directly to a different script offset.
	kOpcodeJSR           = 0x1E, ///< JSR. Call a subroutine.
	kOpcodeJZ            = 0x1F, ///< JZ. Jump if the top-most stack element is 0.
	kOpcodeRETN          = 0x20, ///< RETN. Return from a subroutine call.
	kOpcodeDESTRUCT      = 0x21, ///< DESTRUCT. Remove elements from the stack.
	kOpcodeNOT           = 0x22, ///< NOT. Boolean-negate the top-most stack element (!).
	kOpcodeDECSP         = 0x23, ///< DECSP. Decrement the value of a stack element (--).
	kOpcodeINCSP         = 0x24, ///< INCSP. Increment the value of a stack element (++).
	kOpcodeJNZ           = 0x25, ///< JNZ. Jump if the top-most stack element is not 0.
	kOpcodeCPDOWNBP      = 0x26, ///< CPDOWNBP. Copy a value into an existing base-pointer stack element.
	kOpcodeCPTOPBP       = 0x27, ///< CPTOPBP. Push a copy of a base-pointer stack element on top of the stack.
	kOpcodeDECBP         = 0x28, ///< DECBP. Decrement the value of a base-pointer stack element (--).
	kOpcodeINCBP         = 0x29, ///< INCBP. Increment the value of a base-pointer stack element (++).
	kOpcodeSAVEBP        = 0x2A, ///< SAVEBP. Set the value of the base-pointer.
	kOpcodeRESTOREBP     = 0x2B, ///< RESTOREBP. Restore the value of the base-pointer to a prior value.
	kOpcodeSTORESTATE    = 0x2C, ///< STORESTATE. Create a functor of a subroutine with the current stack.
	kOpcodeNOP           = 0x2D, ///< NOP. No operation.
	kOpcodeWRITEARRAY    = 0x30, ///< WRITEARRAY. Write the value of an array element on the stack.
	kOpcodeREADARRAY     = 0x32, ///< READARRAY. Push the value of an array element onto of the stack.
	kOpcodeGETREF        = 0x37, ///< GETREF. Push the reference to a stack element onto the stack.
	kOpcodeGETREFARRAY   = 0x39, ///< GETREFARRAY. Push the reference to an array element onto the stack.
	kOpcodeSCRIPTSIZE    = 0x42, ///< SCRIPTSIZE. Specify the size of the following script bytecode in bytes.

	kOpcodeMAX
};

/** An instruction type, defining on what arguments it operates. */
enum InstructionType {
	// Unary
	kInstTypeNone        =  0, ///< This instruction takes no arguments at all.
	kInstTypeDirect      =  1, ///< This instruction takes only direct arguments.
	kInstTypeInt         =  3, ///< This instruction takes an integer variable from the stack.
	kInstTypeFloat       =  4, ///< This instruction takes a floating point variable from the stack.
	kInstTypeString      =  5, ///< This instruction takes a string variable from the stack.
	kInstTypeObject      =  6, ///< This instruction takes an object variable from the stack.
	kInstTypeResource    = 96, ///< This instruction takes a resource variable from the stack.
	kInstTypeEngineType0 = 16, ///< This instruction takes an engine type (0) variable from the stack.
	kInstTypeEngineType1 = 17, ///< This instruction takes an engine type (1) variable from the stack.
	kInstTypeEngineType2 = 18, ///< This instruction takes an engine type (2) variable from the stack.
	kInstTypeEngineType3 = 19, ///< This instruction takes an engine type (3) variable from the stack.
	kInstTypeEngineType4 = 20, ///< This instruction takes an engine type (4) variable from the stack.
	kInstTypeEngineType5 = 21, ///< This instruction takes an engine type (5) variable from the stack.

	// Arrays
	kInstTypeIntArray          = 64, ///< This instruction operates on an array of integers.
	kInstTypeFloatArray        = 65, ///< This instruction operates on an array of floating pointer numbers.
	kInstTypeStringArray       = 66, ///< This instruction operates on an array of strings.
	kInstTypeObjectArray       = 67, ///< This instruction operates on an array of objects.
	kInstTypeResourceArray     = 68, ///< This instruction operates on an array of resources.
	kInstTypeEngineType0Array  = 80, ///< This instruction operates on an array of engine types (0).
	kInstTypeEngineType1Array  = 81, ///< This instruction operates on an array of engine types (1).
	kInstTypeEngineType2Array  = 82, ///< This instruction operates on an array of engine types (2).
	kInstTypeEngineType3Array  = 83, ///< This instruction operates on an array of engine types (3).
	kInstTypeEngineType4Array  = 84, ///< This instruction operates on an array of engine types (4).
	kInstTypeEngineType5Array  = 85, ///< This instruction operates on an array of engine types (5).

	// Binary
	kInstTypeIntInt                 = 32, ///< This instruction operates on two integers.
	kInstTypeFloatFloat             = 33, ///< This instruction operates on two floating point numbers.
	kInstTypeObjectObject           = 34, ///< This instruction operates on two objects.
	kInstTypeStringString           = 35, ///< This instruction operates on two strings.
	kInstTypeStructStruct           = 36, ///< This instruction operates on two structs.
	kInstTypeIntFloat               = 37, ///< This instruction operates on an integer and a float.
	kInstTypeFloatInt               = 38, ///< This instruction operates on a float and an integer.
	kInstTypeEngineType0EngineType0 = 48, ///< This instruction operates on two engine types (0).
	kInstTypeEngineType1EngineType1 = 49, ///< This instruction operates on two engine types (1).
	kInstTypeEngineType2EngineType2 = 50, ///< This instruction operates on two engine types (2).
	kInstTypeEngineType3EngineType3 = 51, ///< This instruction operates on two engine types (3).
	kInstTypeEngineType4EngineType4 = 52, ///< This instruction operates on two engine types (4).
	kInstTypeEngineType5EngineType5 = 53, ///< This instruction operates on two engine types (5).
	kInstTypeVectorVector           = 58, ///< This instruction operates on two vectors.
	kInstTypeVectorFloat            = 59, ///< This instruction operates on a vector and a float.
	kInstTypeFloatVector            = 60, ///< This instruction operates on a float and a vector.

	kInstTypeInstTypeMAX = 97
};

/** The type of a direct instruction argument. */
enum OpcodeArgument {
	kOpcodeArgNone,    ///< Empty/Unused direct argument.
	kOpcodeArgUint8,   ///< Unsigned 8-bit integer.
	kOpcodeArgUint16,  ///< Unsigned 16-bit integer.
	kOpcodeArgSint16,  ///< Signed 16-bit integer.
	kOpcodeArgSint32,  ///< Signed 32-bit integer.
	kOpcodeArgUint32,  ///< Unsigned 32-bit integer.
	kOpcodeArgVariable ///< A variable value, as supplied by kOpcodeCONST.
};

/** The type of an instruction address. */
enum AddressType {
	kAddressTypeNone,       ///< No special address type.
	kAddressTypeTail,       ///< The tail (or false branch) of a jump instruction.
	kAddressTypeJumpLabel,  ///< Address that's the destination of a jump label.
	kAddressTypeStoreState, ///< Address that starts a subroutine with STORESTATE.
	kAddressTypeSubRoutine  ///< Address that starts a subroutine.
};

/** An NWScript bytecode instruction. */
struct Instruction {
	static const size_t kOpcodeMaxArgumentCount = 3;

	uint32_t address; ///< The address of this instruction with the NCS file.

	Opcode opcode;        ///< The opcode of this instruction.
	InstructionType type; ///< The type of this instruction.

	/** The number of direct arguments this instruction has (0-3). */
	size_t argCount;
	/** The direct arguments of this instruction. */
	int32_t args[kOpcodeMaxArgumentCount];
	/** The types of the direct arguments of this instruction. */
	OpcodeArgument argTypes[kOpcodeMaxArgumentCount];

	/** Parameter for kOpcodeCONST + kInstTypeInt. */
	int32_t constValueInt;
	/** Parameter for kOpcodeCONST + kInstTypeFloat. */
	float constValueFloat;
	/** Parameter for kOpcodeCONST + kInstTypeObject. */
	uint32_t constValueObject;
	/** Parameter for kOpcodeCONST + kInstTypeString or kInstTypeResource. */
	Common::UString constValueString;

	/** The type of this instruction address. */
	AddressType addressType;

	/** The instruction directly, naturally following this instruction.
	 *
	 *  The instruction that is taken when the code flows without taking
	 *  any branches. If the instruction has no natural follower (which
	 *  is the case for RETN and JMP), this value is 0.
	 */
	const Instruction *follower;

	/** The instructions that lead into this instruction, either naturally
	 *  or by a jump, as long as it's not across subroutine boundaries.
	 */
	std::vector<const Instruction *> predecessors;

	/** The destinations of the branches this instruction takes.
	 *
	 *  If this vector has:
	 *  - no elements, the instruction doesn't branch
	 *  - one element, the instruction branches unconditionally
	 *  - two elements, the first is the true branch, the second the false branch
	 *  - three or more elements, something went horribly, horribly wrong and the
	 *    universe might be on fire
	 */
	std::vector<const Instruction *> branches;

	/** The block this instruction belongs to. */
	const Block *block;

	/** The NWScript stack before this instruction is executed. */
	Stack stack;

	/** The variables this instruction manipulates (creates, writes, reads). */
	std::vector<const Variable *> variables;


	Instruction(uint32_t addr = 0) : address(addr),
		opcode(kOpcodeMAX), type(kInstTypeInstTypeMAX), argCount(0),
		constValueInt(0), constValueFloat(0.0f), constValueObject(0),
		addressType(kAddressTypeNone), follower(0), block(0) {

		for (size_t i = 0; i < kOpcodeMaxArgumentCount; i++) {
			args    [i] = 0;
			argTypes[i] = kOpcodeArgNone;
		}
	}

	/** Order Instructions by address. */
	bool operator<(const Instruction &right) const {
		return address < right.address;
	}

	bool operator<(uint32_t right) const {
		return address < right;
	}
};

/** The whole set of instructions found in a script. */
typedef std::deque<Instruction> Instructions;

/** Parse an instruction out of the NCS stream. */
bool parseInstruction(Common::SeekableReadStream &ncs, Instruction &instr);

/** Given a whole set of script instructions, interlink branching instructions. */
void linkInstructionBranches(Instructions &instructions);

} // End of namespace NWScript

#endif // NWSCRIPT_INSTRUCTION_H
