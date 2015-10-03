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
 *  NWScript utility functions.
 */

#include "src/common/util.h"

#include "src/nwscript/util.h"

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
	/* 0x34 */ "??",            "??",          "??",        "GETARAY",
	/* 0x38 */ "??",            "GETARRAYREF", "??",        "??",
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

static const OpcodeArgument kOpcodeArguments[kOpcodeMAX][kOpcodeMaxArgumentCount] = {
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
	for (size_t i = 0; i < kOpcodeMaxArgumentCount; i++, n++)
		if (args[i] == kOpcodeArgNone)
			break;

	return n;
}

Common::UString formatBytes(const Instruction &instr) {
	Common::UString str = Common::UString::format("%02X %02X", (uint8)instr.opcode, (uint8)instr.type);

	for (size_t i = 0; i < instr.argCount; i++) {
		switch (instr.argTypes[i]) {
			case kOpcodeArgUint8:
				str += Common::UString::format(" %02X", (uint8)instr.args[i]);
				break;

			case kOpcodeArgUint16:
				str += Common::UString::format(" %04X", (uint16)instr.args[i]);
				break;

			case kOpcodeArgSint16:
				str += Common::UString::format(" %04X", (uint16)(int16)instr.args[i]);
				break;

			case kOpcodeArgSint32:
			case kOpcodeArgUint32:
				str += Common::UString::format(" %08X", (uint32)instr.args[i]);
				break;

			case kOpcodeArgVariable:
				switch (instr.type) {
					case kInstTypeInt:
						str += Common::UString::format(" %08X", (uint32)instr.constValueInt);
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

Common::UString formatInstruction(const Instruction &instr) {
	Common::UString str = Common::UString::format("%s%s", getOpcodeName(instr.opcode).c_str(),
	                                                      getInstTypeName(instr.type).c_str());

	for (size_t i = 0; i < instr.argCount; i++) {
		switch (instr.argTypes[i]) {
			case kOpcodeArgUint8:
			case kOpcodeArgUint16:
			case kOpcodeArgSint16:
			case kOpcodeArgSint32:
				str += Common::UString::format(" %d", instr.args[i]);
				break;

			case kOpcodeArgUint32:
				str += Common::UString::format(" %u", (uint32)instr.args[i]);
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

} // End of namespace NWScript