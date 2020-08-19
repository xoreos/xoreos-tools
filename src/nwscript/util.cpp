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
 *  NWScript utility functions.
 */

#include "src/common/util.h"
#include "src/common/strutil.h"
#include "src/common/error.h"

#include "src/nwscript/util.h"
#include "src/nwscript/block.h"
#include "src/nwscript/subroutine.h"
#include "src/nwscript/game.h"

namespace NWScript {

static const char * const kOpcodeName[kOpcodeMAX] = {
	/* 0x00 */ "??",            "CPDOWNSP",    "RSADD",     "CPTOPSP",
	/* 0x04 */ "CONST",         "ACTION",      "LOGAND",    "LOGOR",
	/* 0x08 */ "INCOR",         "EXCOR",       "BOOLAND",   "EQ",
	/* 0x0C */ "NEQ",           "GEQ",         "GT",        "LT",
	/* 0x10 */ "LEQ",           "SHLEFT",      "SHRIGHT",   "USHRIGHT",
	/* 0x14 */ "ADD",           "SUB",         "MUL",       "DIV",
	/* 0x18 */ "MOD",           "NEG",         "COMP",      "MOVSP",
	/* 0x1C */ "STORESTATEALL", "JMP",         "JSR",       "JZ",
	/* 0x20 */ "RETN",          "DESTRUCT",    "NOT",       "DECSP",
	/* 0x24 */ "INCSP",         "JNZ",         "CPDOWNBP",  "CPTOPBP",
	/* 0x28 */ "DECBP",         "INCBP",       "SAVEBP",    "RESTOREBP",
	/* 0x2C */ "STORESTATE",    "NOP",         "??",        "??",
	/* 0x30 */ "WRITEARRAY",    "??",          "READARRAY", "??",
	/* 0x34 */ "??",            "??",          "??",        "GETREF",
	/* 0x38 */ "??",            "GETREFARRAY", "??",        "??",
	/* 0x3C */ "??",            "??",          "??",        "??",
	/* 0x40 */ "??",            "??",          "SCRIPTSIZE"
};

static const char * const kInstTypeName[kInstTypeInstTypeMAX] = {
	/*  0 */ "",     "",     "?",    "I",    "F",    "S",    "O",  "?",
	/*  8 */ "?",    "?",    "?",    "?",    "?",    "?",    "?",  "?",
	/* 16 */ "E0",   "E1",   "E2",   "E3",   "E4",   "E5",   "?",  "?",
	/* 24 */ "?",    "?",    "?",    "?",    "?",    "?",    "?",  "?",
	/* 32 */ "II",   "FF",   "OO",   "SS",   "TT",   "IF",   "FI", "?",
	/* 40 */ "?",    "?",    "?",    "?",    "?",    "?",    "?",  "?",
	/* 48 */ "E0E0", "E1E1", "E2E2", "E3E3", "E4E4", "E5E5", "?",  "?",
	/* 56 */ "?",    "?",    "VV",   "VF",   "FV",   "?",    "?",  "?",
	/* 64 */ "I[]",  "F[]",  "S[]",  "O[]",  "R[]",  "?",    "?",  "?",
	/* 72 */ "?",    "?",    "?",    "?",    "?",    "?",    "?",  "?",
	/* 80 */ "E0[]", "E1[]", "E2[]", "E3[]", "E4[]", "E5[]", "?",  "?",
	/* 88 */ "?",    "?",    "?",    "?",    "?",    "?",    "?",  "?",
	/* 96 */ "R"
};

static const char * const kVarTypeName[] = {
	"void"  , "int"   ,  "float"    , "string"    , "resource"     , "object"    ,
	"vector", "struct" ,
	"E0"    , "E1"     , "E2"       , "E3"        , "E4"           , "E5"        ,
	"action", "int[]"  , "float[]"  , "string[]"  , "resource[]"   , "object[]"  ,
	"E0[]"  , "E1[]"   , "E2[]"     , "E3[]"      , "E4[]"         , "E5[]"      ,
	"any"   , "ref int", "ref float", "ref string", "ref resource" , "ref object",
	"ref E0", "ref E1" , "ref E2"   , "ref E3"    , "ref E4"       , "ref E5"
};

static const OpcodeArgument kOpcodeArguments[kOpcodeMAX][Instruction::kOpcodeMaxArgumentCount] = {
	// 0x00
	/*               */ { },
	/* CPDOWNSP      */ { kOpcodeArgSint32, kOpcodeArgSint16 },
	/* RSADD         */ { },
	/* CPTOPSP       */ { kOpcodeArgSint32, kOpcodeArgSint16 },
	// 0x04
	/* CONST         */ { },
	/* ACTION        */ { kOpcodeArgUint16, kOpcodeArgUint8 },
	/* LOGAND        */ { },
	/* LOGAR         */ { },
	// 0x08
	/* INCOR         */ { },
	/* EXCOR         */ { },
	/* BOOLAND       */ { },
	/* EQ            */ { },
	// 0x0C
	/* NEQ           */ { },
	/* GEQ           */ { },
	/* GT            */ { },
	/* LT            */ { },
	// 0x10
	/* LEQ           */ { },
	/* SHLEFT        */ { },
	/* SHRIGHT       */ { },
	/* USHRIGHT      */ { },
	// 0x14
	/* ADD           */ { },
	/* SUB           */ { },
	/* MUL           */ { },
	/* DIV           */ { },
	// 0x18
	/* MOD           */ { },
	/* NEG           */ { },
	/* COMP          */ { },
	/* MOVSP         */ { kOpcodeArgSint32 },
	// 0x1C
	/* STORESTATEALL */ { },
	/* JMP           */ { kOpcodeArgSint32 },
	/* JSR           */ { kOpcodeArgSint32 },
	/* JZ            */ { kOpcodeArgSint32 },
	// 0x20
	/* RETN          */ { },
	/* DESTRUCT      */ { kOpcodeArgSint16, kOpcodeArgSint16, kOpcodeArgSint16 },
	/* NOT           */ { },
	/* DECSP         */ { kOpcodeArgSint32 },
	// 0x24
	/* INCSP         */ { kOpcodeArgSint32 },
	/* JNZ           */ { kOpcodeArgSint32 },
	/* CPDOWNBP      */ { kOpcodeArgSint32, kOpcodeArgSint16 },
	/* CPTOPBP       */ { kOpcodeArgSint32, kOpcodeArgSint16 },
	// 0x28
	/* DECBP         */ { kOpcodeArgSint32 },
	/* INCBP         */ { kOpcodeArgSint32 },
	/* SAVEBP        */ { },
	/* RESTOREBP     */ { },
	// 0x2C
	/* STORESTATE    */ { },
	/* NOP           */ { },
	/*               */ { },
	/*               */ { },
	// 0x30
	/* WRITEARRAY    */ { kOpcodeArgSint32, kOpcodeArgSint16 },
	/*               */ { },
	/* READARRAY     */ { kOpcodeArgSint32, kOpcodeArgSint16 },
	/*               */ { },
	// 0x34
	/*               */ { },
	/*               */ { },
	/*               */ { },
	/* GETREF        */ { kOpcodeArgSint32, kOpcodeArgSint16 },
	// 0x38
	/*               */ { },
	/* GETREFARRAY   */ { kOpcodeArgSint32, kOpcodeArgSint16 },
	/*               */ { },
	/*               */ { },
	// 0x3C
	/*               */ { },
	/*               */ { },
	/*               */ { },
	/*               */ { },
	// 0x40
	/*               */ { },
	/*               */ { },
	/* SCRIPTSIZE    */ { }
};

Common::UString getOpcodeName(Opcode op) {
	if ((size_t)op >= ARRAYSIZE(kOpcodeName))
		return "??";

	return kOpcodeName[(size_t)op];
}

Common::UString getInstTypeName(InstructionType type) {
	if ((size_t)type >= ARRAYSIZE(kInstTypeName))
		return "?";

	return kInstTypeName[(size_t)type];
}

VariableType instructionTypeToVariableType(InstructionType type) {
	switch (type) {
		case kInstTypeInt:
		case kInstTypeIntInt:
			return kTypeInt;

		case kInstTypeFloat:
		case kInstTypeFloatFloat:
		case kInstTypeIntFloat:
		case kInstTypeFloatInt:
			return kTypeFloat;

		case kInstTypeString:
		case kInstTypeStringString:
			return kTypeString;

		case kInstTypeObject:
		case kInstTypeObjectObject:
			return kTypeObject;

		case kInstTypeResource:
			return kTypeResource;

		case kInstTypeVectorVector:
		case kInstTypeVectorFloat:
		case kInstTypeFloatVector:
			return kTypeVector;

		case kInstTypeEngineType0:
		case kInstTypeEngineType0EngineType0:
			return kTypeEngineType0;

		case kInstTypeEngineType1:
		case kInstTypeEngineType1EngineType1:
			return kTypeEngineType1;

		case kInstTypeEngineType2:
		case kInstTypeEngineType2EngineType2:
			return kTypeEngineType2;

		case kInstTypeEngineType3:
		case kInstTypeEngineType3EngineType3:
			return kTypeEngineType3;

		case kInstTypeEngineType4:
		case kInstTypeEngineType4EngineType4:
			return kTypeEngineType4;

		case kInstTypeEngineType5:
		case kInstTypeEngineType5EngineType5:
			return kTypeEngineType5;

		case kInstTypeIntArray:
			return kTypeIntArray;

		case kInstTypeFloatArray:
			return kTypeFloatArray;

		case kInstTypeStringArray:
			return kTypeStringArray;

		case kInstTypeObjectArray:
			return kTypeObjectArray;

		case kInstTypeResourceArray:
			return kTypeResourceArray;

		case kInstTypeEngineType0Array:
			return kTypeEngineType0Array;

		case kInstTypeEngineType1Array:
			return kTypeEngineType1Array;

		case kInstTypeEngineType2Array:
			return kTypeEngineType2Array;

		case kInstTypeEngineType3Array:
			return kTypeEngineType3Array;

		case kInstTypeEngineType4Array:
			return kTypeEngineType4Array;

		case kInstTypeEngineType5Array:
			return kTypeEngineType5Array;

		default:
			break;
	}

	return kTypeVoid;
}

Common::UString getVariableTypeName(VariableType type, Aurora::GameID game) {
	if ((size_t)type >= ARRAYSIZE(kVarTypeName))
		return "";

	if ((type >= kTypeEngineType0) && (type <= kTypeEngineType5))
		return getEngineTypeName(game, (size_t)type - (size_t)kTypeEngineType0);

	if ((type >= kTypeEngineType0Array) && (type <= kTypeEngineType5Array))
		return getEngineTypeName(game, (size_t)type - (size_t)kTypeEngineType0Array) + "[]";

	if ((type >= kTypeEngineType0Ref) && (type <= kTypeEngineType5Ref))
		return "ref " + getEngineTypeName(game, (size_t)type - (size_t)kTypeEngineType0Ref);

	return kVarTypeName[(size_t)type];
}

VariableType typeToArrayType(VariableType type) {
	switch (type) {
		case kTypeInt:
			return kTypeIntArray;
		case kTypeFloat:
			return kTypeFloatArray;
		case kTypeString:
			return kTypeStringArray;
		case kTypeResource:
			return kTypeResourceArray;
		case kTypeObject:
			return kTypeObjectArray;
		case kTypeEngineType0:
			return kTypeEngineType0Array;
		case kTypeEngineType1:
			return kTypeEngineType1Array;
		case kTypeEngineType2:
			return kTypeEngineType2Array;
		case kTypeEngineType3:
			return kTypeEngineType3Array;
		case kTypeEngineType4:
			return kTypeEngineType4Array;
		case kTypeEngineType5:
			return kTypeEngineType5Array;

		default:
			break;
	}

	return kTypeAny;
}

VariableType arrayTypeToType(VariableType type) {
	switch (type) {
		case kTypeIntArray:
			return kTypeInt;
		case kTypeFloatArray:
			return kTypeFloat;
		case kTypeStringArray:
			return kTypeString;
		case kTypeResourceArray:
			return kTypeResource;
		case kTypeObjectArray:
			return kTypeObject;
		case kTypeEngineType0Array:
			return kTypeEngineType0;
		case kTypeEngineType1Array:
			return kTypeEngineType1;
		case kTypeEngineType2Array:
			return kTypeEngineType2;
		case kTypeEngineType3Array:
			return kTypeEngineType3;
		case kTypeEngineType4Array:
			return kTypeEngineType4;
		case kTypeEngineType5Array:
			return kTypeEngineType5;

		default:
			break;
	}

	return kTypeAny;
}

VariableType typeToRefType(VariableType type) {
	switch (type) {
		case kTypeInt:
			return kTypeIntRef;
		case kTypeFloat:
			return kTypeFloatRef;
		case kTypeString:
			return kTypeStringRef;
		case kTypeResource:
			return kTypeResourceRef;
		case kTypeObject:
			return kTypeObjectRef;
		case kTypeEngineType0:
			return kTypeEngineType0Ref;
		case kTypeEngineType1:
			return kTypeEngineType1Ref;
		case kTypeEngineType2:
			return kTypeEngineType2Ref;
		case kTypeEngineType3:
			return kTypeEngineType3Ref;
		case kTypeEngineType4:
			return kTypeEngineType4Ref;
		case kTypeEngineType5:
			return kTypeEngineType5Ref;

		default:
			break;
	}

	return kTypeAny;
}

VariableType refTypeToType(VariableType type) {
	switch (type) {
		case kTypeIntRef:
			return kTypeInt;
		case kTypeFloatRef:
			return kTypeFloat;
		case kTypeStringRef:
			return kTypeString;
		case kTypeResourceRef:
			return kTypeResource;
		case kTypeObjectRef:
			return kTypeObject;
		case kTypeEngineType0Ref:
			return kTypeEngineType0;
		case kTypeEngineType1Ref:
			return kTypeEngineType1;
		case kTypeEngineType2Ref:
			return kTypeEngineType2;
		case kTypeEngineType3Ref:
			return kTypeEngineType3;
		case kTypeEngineType4Ref:
			return kTypeEngineType4;
		case kTypeEngineType5Ref:
			return kTypeEngineType5;

		default:
			break;
	}

	return kTypeAny;
}

const OpcodeArgument *getDirectArguments(Opcode op) {
	if ((size_t)op >= ARRAYSIZE(kOpcodeArguments))
		return kOpcodeArguments[0];

	return kOpcodeArguments[(size_t)op];
}

size_t getDirectArgumentCount(Opcode op) {
	if ((size_t)op >= ARRAYSIZE(kOpcodeArguments))
		return 0;

	const OpcodeArgument * const args = kOpcodeArguments[(size_t)op];

	size_t n = 0;
	for (size_t i = 0; i < Instruction::kOpcodeMaxArgumentCount; i++, n++)
		if (args[i] == kOpcodeArgNone)
			break;

	return n;
}

Common::UString formatBytes(const Instruction &instr) {
	Common::UString str = Common::UString::format("%02X %02X", (uint8_t)instr.opcode, (uint8_t)instr.type);

	for (size_t i = 0; i < instr.argCount; i++) {
		switch (instr.argTypes[i]) {
			case kOpcodeArgUint8:
				str += Common::UString::format(" %02X", (uint8_t)instr.args[i]);
				break;

			case kOpcodeArgUint16:
				str += Common::UString::format(" %04X", (uint16_t)instr.args[i]);
				break;

			case kOpcodeArgSint16:
				str += Common::UString::format(" %04X", (uint16_t)(int16_t)instr.args[i]);
				break;

			case kOpcodeArgSint32:
			case kOpcodeArgUint32:
				str += Common::UString::format(" %08X", (uint32_t)instr.args[i]);
				break;

			case kOpcodeArgVariable:
				switch (instr.type) {
					case kInstTypeInt:
						str += Common::UString::format(" %08X", (uint32_t)instr.constValueInt);
						break;

					case kInstTypeFloat:
						str += Common::UString::format(" %08X", convertIEEEFloat(instr.constValueFloat));
						break;

					case kInstTypeString:
					case kInstTypeResource:
						str += Common::UString::format(" str");
						break;

					case kInstTypeObject:
						str += Common::UString::format(" %08X", instr.constValueObject);
						break;

					default:
						break;
				}
				break;

			default:
				break;
		}
	}

	return str;
}

Common::UString formatInstruction(const Instruction &instr, Aurora::GameID game) {
	Common::UString str = Common::UString::format("%s%s", getOpcodeName(instr.opcode).c_str(),
	                                                      getInstTypeName(instr.type).c_str());

	/* If this is a jump instruction, print the address of the destination
	 * instead of the relative offset. */
	if (((instr.opcode == kOpcodeJMP) || (instr.opcode == kOpcodeJSR) ||
	     (instr.opcode == kOpcodeJZ ) || (instr.opcode == kOpcodeJNZ) ||
	     (instr.opcode == kOpcodeSTORESTATE)) &&
	    (!instr.branches.empty() && instr.branches[0])) {

		const Common::UString jumpLabel = formatJumpLabelName(*instr.branches[0]);
		if (jumpLabel.empty())
			throw Common::Exception("Branch destination is not a jump destination?!?");

		Common::UString parameters;
		if ((instr.opcode == kOpcodeSTORESTATE) && (instr.argCount == 3))
			parameters = Common::UString::format(" %d %d", instr.args[1], instr.args[2]);

		return str + " " + jumpLabel + parameters;
	}

	if ((instr.opcode == kOpcodeACTION) && (instr.argCount == 2)) {
		Common::UString functionName = getFunctionName(game, instr.args[0]);
		if (functionName.empty())
			functionName = Common::UString::format("InvalidFunction%d", instr.args[0]);

		return str + " " + functionName + Common::UString::format(" %d", instr.args[1]);
	}

	for (size_t i = 0; i < instr.argCount; i++) {
		switch (instr.argTypes[i]) {
			case kOpcodeArgUint8:
			case kOpcodeArgUint16:
			case kOpcodeArgSint16:
			case kOpcodeArgSint32:
				str += Common::UString::format(" %d", instr.args[i]);
				break;

			case kOpcodeArgUint32:
				str += Common::UString::format(" %u", (uint32_t)instr.args[i]);
				break;

			case kOpcodeArgVariable:
				switch (instr.type) {
					case kInstTypeInt:
						str += Common::UString::format(" %d", instr.constValueInt);
						break;

					case kInstTypeFloat:
						str += Common::UString::format(" %f", instr.constValueFloat);
						break;

					case kInstTypeString:
					case kInstTypeResource:
						str += Common::UString::format(" \"%s\"", instr.constValueString.c_str());
						break;

					case kInstTypeObject:
						str += Common::UString::format(" %d", instr.constValueObject);
						break;

					default:
						break;
				}
				break;

			default:
				break;
		}
	}

	return str;
}

Common::UString formatSubRoutine(uint32_t address) {
	return Common::UString::format("sub_%08X", address);
}

Common::UString formatStoreState(uint32_t address) {
	return Common::UString::format("sta_%08X", address);
}

Common::UString formatJumpDestination(uint32_t address) {
	return Common::UString::format("loc_%08X", address);
}

Common::UString formatJumpLabel(const Instruction &instr) {
	if (instr.addressType == kAddressTypeSubRoutine)
		return formatSubRoutine(instr.address);
	if (instr.addressType == kAddressTypeStoreState)
		return formatStoreState(instr.address);
	if (instr.addressType == kAddressTypeJumpLabel)
		return formatJumpDestination(instr.address);

	return "";
}

Common::UString formatJumpLabel(const Block &block) {
	if (block.instructions.empty() || !block.instructions.front())
		return "";

	return formatJumpLabel(*block.instructions.front());
}

Common::UString formatJumpLabel(const SubRoutine &sub) {
	if (sub.blocks.empty() || !sub.blocks.front())
		return "";

	return formatJumpLabel(*sub.blocks.front());
}

Common::UString formatJumpLabelName(const Instruction &instr) {
	if ((instr.addressType == kAddressTypeSubRoutine) &&
	    instr.block && instr.block->subRoutine && !instr.block->subRoutine->name.empty())
		return instr.block->subRoutine->name;

	return formatJumpLabel(instr);
}

Common::UString formatJumpLabelName(const Block &block) {
	if (block.instructions.empty() || !block.instructions.front())
		return "";

	return formatJumpLabelName(*block.instructions.front());
}

Common::UString formatJumpLabelName(const SubRoutine &sub) {
	if (sub.blocks.empty() || !sub.blocks.front())
		return "";

	return formatJumpLabelName(*sub.blocks.front());
}

Common::UString formatParameters(const std::vector<const Variable *> &params,
                                 Aurora::GameID game, bool names) {

	Common::UString paramTypes;
	for (std::vector<const Variable *>::const_iterator p = params.begin(); p != params.end(); ++p) {
		if (p != params.begin())
			paramTypes += ", ";

		paramTypes += getVariableTypeName(*p ? (*p)->type : kTypeAny, game).toLower();
		if (names && *p)
			paramTypes += " arg_" + Common::composeString((*p)->id);
	}

	return paramTypes;
}

Common::UString formatReturn(const std::vector<const Variable *> &returns, Aurora::GameID game) {
	if (returns.size() > 1)
		return "struct";

	if (returns.empty())
		return "void";

	return getVariableTypeName(returns[0] ? returns[0]->type : kTypeAny, game).toLower();
}

Common::UString formatSignature(const SubRoutine &sub, Aurora::GameID game, bool names) {
	return formatReturn(sub.returns, game) + " " + formatJumpLabelName(sub) +
	       "(" + formatParameters(sub.params, game, names) + ")";
}

Common::UString formatVariableName(const Variable *variable) {
	Common::UString v;

	switch (variable->use) {
		case VariableUse::kVariableUseGlobal:
			v += "global_";
			break;
		case VariableUse::kVariableUseLocal:
			v += "local_";
			break;
		case VariableUse::kVariableUseParameter:
			v += "arg_";
			break;
		case VariableUse::kVariableUseReturn:
			v += "return_";
			break;
		default:
			v += "unknown_";
			break;
	}

	v += Common::composeString(variable->id);

	return v;
}

Common::UString formatInstructionData(const Instruction &instruction) {
	Common::UString str;
	switch (instruction.type) {
		case kInstTypeInt:
			str += Common::UString::format("%d", instruction.constValueInt);
			break;

		case kInstTypeFloat:
			str += Common::UString::format("%f", instruction.constValueFloat);
			break;
		case kInstTypeString:
		case kInstTypeResource:
			str += Common::UString::format("\"%s\"", instruction.constValueString.c_str());
			break;
		case kInstTypeObject:
			str += Common::UString::format("%d", instruction.constValueObject);
			break;
		default:
			break;
	}
	return str;
}

} // End of namespace NWScript
