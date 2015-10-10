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
 *  Types found in BioWare's NWScript.
 */

#ifndef NWSCRIPT_TYPES_H
#define NWSCRIPT_TYPES_H

#include <vector>

#include "src/common/types.h"
#include "src/common/ustring.h"

namespace NWScript {

/** The type of an NWScript variable. */
enum VariableType {
	kTypeVoid            = 0,
	kTypeInt                ,
	kTypeFloat              ,
	kTypeString             ,
	kTypeResource           ,
	kTypeObject             ,
	kTypeVector             ,
	kTypeStruct             ,
	kTypeEngineType         , ///< "effect", "event", "location", "talent"...
	kTypeScriptState        , ///< "action".
	kTypeIntArray           ,
	kTypeFloatArray         ,
	kTypeStringArray        ,
	kTypeResourceArray      ,
	kTypeObjectArray        ,
	kTypeEngineTypeArray    ,
	kTypeReference          , ///< A reference/pointer to another variable.
	kTypeAny                  ///< Any other type.
};

/** An instruction opcode, defining what it does. */
enum Opcode {
	kOpcodeCPDOWNSP      = 0x01,
	kOpcodeRSADD         = 0x02,
	kOpcodeCPTOPSP       = 0x03,
	kOpcodeCONST         = 0x04,
	kOpcodeACTION        = 0x05,
	kOpcodeLOGAND        = 0x06,
	kOpcodeLOGOR         = 0x07,
	kOpcodeINCOR         = 0x08,
	kOpcodeEXCOR         = 0x09,
	kOpcodeBOOLAND       = 0x0A,
	kOpcodeEQ            = 0x0B,
	kOpcodeNEQ           = 0x0C,
	kOpcodeGEQ           = 0x0D,
	kOpcodeGT            = 0x0E,
	kOpcodeLT            = 0x0F,
	kOpcodeLEQ           = 0x10,
	kOpcodeSHLEFT        = 0x11,
	kOpcodeSHRIGHT       = 0x12,
	kOpcodeUSHRIGHT      = 0x13,
	kOpcodeADD           = 0x14,
	kOpcodeSUB           = 0x15,
	kOpcodeMUL           = 0x16,
	kOpcodeDIV           = 0x17,
	kOpcodeMOD           = 0x18,
	kOpcodeNEG           = 0x19,
	kOpcodeCOMP          = 0x1A,
	kOpcodeMOVSP         = 0x1B,
	kOpcodeSTORESTATEALL = 0x1C,
	kOpcodeJMP           = 0x1D,
	kOpcodeJSR           = 0x1E,
	kOpcodeJZ            = 0x1F,
	kOpcodeRETN          = 0x20,
	kOpcodeDESTRUCT      = 0x21,
	kOpcodeNOT           = 0x22,
	kOpcodeDECSP         = 0x23,
	kOpcodeINCSP         = 0x24,
	kOpcodeJNZ           = 0x25,
	kOpcodeCPDOWNBP      = 0x26,
	kOpcodeCPTOPBP       = 0x27,
	kOpcodeDECBP         = 0x28,
	kOpcodeINCBP         = 0x29,
	kOpcodeSAVEBP        = 0x2A,
	kOpcodeRESTOREBP     = 0x2B,
	kOpcodeSTORESTATE    = 0x2C,
	kOpcodeNOP           = 0x2D,
	kOpcodeWRITEARRAY    = 0x30,
	kOpcodeREADARRAY     = 0x32,
	kOpcodeGETREF        = 0x37,
	kOpcodeGETREFARRAY   = 0x39,
	kOpcodeSCRIPTSIZE    = 0x42,

	kOpcodeMAX
};

/** An instruction type, defining on what arguments it operates. */
enum InstructionType {
	// Unary
	kInstTypeNone        =  0,
	kInstTypeDirect      =  1,
	kInstTypeInt         =  3,
	kInstTypeFloat       =  4,
	kInstTypeString      =  5,
	kInstTypeObject      =  6,
	kInstTypeResource    = 96,
	kInstTypeEngineType0 = 16, // NWN:     effect        DA: event
	kInstTypeEngineType1 = 17, // NWN:     event         DA: location
	kInstTypeEngineType2 = 18, // NWN:     location      DA: command
	kInstTypeEngineType3 = 19, // NWN:     talent        DA: effect
	kInstTypeEngineType4 = 20, // NWN:     itemproperty  DA: itemproperty
	kInstTypeEngineType5 = 21, // Witcher: mod           DA: player

	// Arrays
	kInstTypeIntArray          = 64,
	kInstTypeFloatArray        = 65,
	kInstTypeStringArray       = 66,
	kInstTypeObjectArray       = 67,
	kInstTypeResourceArray     = 68,
	kInstTypeEngineType0Array  = 80,
	kInstTypeEngineType1Array  = 81,
	kInstTypeEngineType2Array  = 82,
	kInstTypeEngineType3Array  = 83,
	kInstTypeEngineType4Array  = 84,
	kInstTypeEngineType5Array  = 85,

	// Binary
	kInstTypeIntInt                 = 32,
	kInstTypeFloatFloat             = 33,
	kInstTypeObjectObject           = 34,
	kInstTypeStringString           = 35,
	kInstTypeStructStruct           = 36,
	kInstTypeIntFloat               = 37,
	kInstTypeFloatInt               = 38,
	kInstTypeEngineType0EngineType0 = 48,
	kInstTypeEngineType1EngineType1 = 49,
	kInstTypeEngineType2EngineType2 = 50,
	kInstTypeEngineType3EngineType3 = 51,
	kInstTypeEngineType4EngineType4 = 52,
	kInstTypeEngineType5EngineType5 = 53,
	kInstTypeVectorVector           = 58,
	kInstTypeVectorFloat            = 59,
	kInstTypeFloatVector            = 60,

	kInstTypeInstTypeMAX = 97
};

/** The type of a direct instruction argument. */
enum OpcodeArgument {
	kOpcodeArgNone,
	kOpcodeArgUint8,
	kOpcodeArgUint16,
	kOpcodeArgSint16,
	kOpcodeArgSint32,
	kOpcodeArgUint32,
	kOpcodeArgVariable
};

/** The type of an instruction address. */
enum AddressType {
	kAddressTypeNone,       ///< No special address type.
	kAddressTypeTail,       ///< The tail (or false branch) of a jump instruction.
	kAddressTypeJumpLabel,  ///< Address that's the destination of a jump label.
	kAddressTypeStateStore, ///< Address that starts a subroutine with STATESTORE.
	kAddressTypeSubRoutine  ///< Address that starts a subroutine.
};

static const size_t kOpcodeMaxArgumentCount = 3;

struct Block;
struct SubRoutine;

/** An NWScript bytecode instruction. */
struct Instruction {
	uint32 address; ///< The address of this intruction with the NCS file

	Opcode opcode;        ///< The opcode of this instruction.
	InstructionType type; ///< The type of this instruction.

	/** The number of direct arguments this instruction has (0-3). */
	size_t argCount;
	/** The direct arguments of this instruction. */
	int32 args[kOpcodeMaxArgumentCount];
	/** The types of the direct arguments of this instruction. */
	OpcodeArgument argTypes[kOpcodeMaxArgumentCount];

	/** Parameter for kOpcodeCONST + kInstTypeInt. */
	int32 constValueInt;
	/** Parameter for kOpcodeCONST + kInstTypeFloat. */
	float constValueFloat;
	/** Parameter for kOpcodeCONST + kInstTypeObject. */
	uint32 constValueObject;
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


	Instruction(uint32 addr) : address(addr),
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

	bool operator<(uint32 right) const {
		return address < right;
	}
};

/** The types of an edge between blocks. */
enum BlockEdgeType {
	kBlockEdgeTypeUnconditional,    ///< This block follows unconditionally.
	kBlockEdgeTypeConditionalTrue,  ///< This block is a true branch of a conditional.
	kBlockEdgeTypeConditionalFalse, ///< This block is a false branch of a conditional.
	kBlockEdgeTypeFunctionCall,     ///< This block is a function call.
	kBlockEdgeTypeFunctionReturn,   ///< This block is a function return.
	kBlockEdgeTypeStoreState        ///< This block is a subroutine create by STORESTATE.
};

/** A block of NWScript instructions. */
struct Block {
	/** The address that starts this block. */
	uint32 address;

	/** The instructions making up this block. */
	std::vector<const Instruction *> instructions;

	std::vector<const Block *> parents;  ///< The blocks leading into this block.
	std::vector<const Block *> children; ///< The blocks following this block.

	/** How this block leads into its children. */
	std::vector<BlockEdgeType> childrenTypes;

	/** The subroutine this block belongs to. */
	const SubRoutine *subRoutine;


	Block(uint32 addr, const SubRoutine &sub) : address(addr), subRoutine(&sub) {
	}
};

/** A subroutine of NWScript blocks. */
struct SubRoutine {
	/** The address that starts this subroutine. */
	uint32 address;

	/** The blocks that are inside this subroutine. */
	std::vector<const Block *> blocks;


	SubRoutine(uint32 addr) : address(addr) {
	}
};

} // End of namespace NWScript

#endif // NWSCRIPT_TYPES_H
