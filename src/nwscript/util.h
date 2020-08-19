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

#ifndef NWSCRIPT_UTIL_H
#define NWSCRIPT_UTIL_H

#include "src/common/ustring.h"

#include "src/aurora/types.h"

#include "src/nwscript/variable.h"
#include "src/nwscript/instruction.h"

namespace NWScript {

struct Block;
struct SubRoutine;

/** Return the textual name of the opcode. */
Common::UString getOpcodeName(Opcode op);

/** Return the textual suffix of the opcode's type. */
Common::UString getInstTypeName(InstructionType type);

/** Return a stack variable type that results when this instruction type is applied. */
VariableType instructionTypeToVariableType(InstructionType type);

/** Return the textual name of the variable type. */
Common::UString getVariableTypeName(VariableType type, Aurora::GameID game = Aurora::kGameIDUnknown);

/** Convert a variable type to an array of this type.
 *
 *  Example: kTypeInt -> kTypeIntArray.
 */
VariableType typeToArrayType(VariableType type);

/** Convert an array variable type to an element of this array type.
 *
 *  Example: kTypeIntArray -> kTypeInt.
 */
VariableType arrayTypeToType(VariableType type);

/** Convert a variable type to a reference of this type.
 *
 *  Example: kTypeInt -> kTypeIntRef.
 */
VariableType typeToRefType(VariableType type);

/** Convert a reference type to a type of the variable it references.
 *
 *  Example: kTypeIntRef -> kTypeInt.
 */
VariableType refTypeToType(VariableType type);

/** Return the direct arguments this opcode takes.
 *
 *  Please note that there are 3 exceptions that require special handling:
 *  - kOpcodeCONST has one argument of a variable type
 *  - kOpcodeEQ has no direct arguments, except if type is kInstTypeStructStruct,
 *    then it has one of type kOpcodeArgUint16
 *  - kOpcodeNEQ has no direct arguments, except if type is kInstTypeStructStruct,
 *    then it has one of type kOpcodeArgUint16
 */
const OpcodeArgument *getDirectArguments(Opcode op);

/** Return the number of direct arguments this opcode takes.
 *
 *  Please note that there are 3 exceptions that require special handling:
 *  - kOpcodeCONST has one argument of a variable type
 *  - kOpcodeEQ has no direct arguments, except if type is kInstTypeStructStruct,
 *    then it has one of type kOpcodeArgUint16
 *  - kOpcodeNEQ has no direct arguments, except if type is kInstTypeStructStruct,
 *    then it has one of type kOpcodeArgUint16
 */
size_t getDirectArgumentCount(Opcode op);

/** Format the bytes compromising this instruction into a string.
 *
 *  This includes the opcode, the instruction type and the direct
 *  arguments. However, for the CONST instruction with a string
 *  direct argument, the literal text "str" is printed instead
 *  of the actual string.
 *
 *  Examples:
 *  01 01 FFFFFFFC 04
 *  04 05 str
 *
 *  The final formatted string will not exceed 26 characters.
 */
Common::UString formatBytes(const Instruction &instr);

/** Format the instruction into an assembly-like mnemonic string.
 *
 *  This includes the opcode, the instruction type and the direct
 *  arguments.
 *
 *  Examples:
 *  CPDOWNSP -4 4
 *  CONSTS "Foobar"
 */
Common::UString formatInstruction(const Instruction &instr, Aurora::GameID game = Aurora::kGameIDUnknown);

/** Format this address to be the name of a subroutine.
 *
 *  Example: "sub_000023FF".
 *
 *  Always exactly 12 characters long.
 */
Common::UString formatSubRoutine(uint32_t address);

/** Format this address to be the name of a subroutine started with STORESTATE.
 *
 *  Example: "sta_000023FF".
 *
 *  Always exactly 12 characters long.
 */
Common::UString formatStoreState(uint32_t address);

/** Format this address to be the name of a jump destination.
 *
 *  Example: "loc_000023FF".
 *
 *  Always exactly 12 characters long.
 */
Common::UString formatJumpDestination(uint32_t address);

/** Format a jump label for the address of this instruction.
 *
 *  - If the instruction starts a subroutine, format its address
 *    as a subroutine (see formatSubRoutine())
 *  - If the instruction starts a subroutine with STORESTATE,
 *    format its address as a subroutine (see formatStoreState())
 *  - If the instruction is a jump destination, format its address
 *    as a jump destination (see formatJumpDestination())
 *  - If the instruction is neither, return an empty string
 *
 *  Always either empty or exactly 12 characters long.
 */
Common::UString formatJumpLabel(const Instruction &instr);

/** Format a jump label for the address of this block.
 *
 *  See formatJumpLabel(const Instruction &instr).
 */
Common::UString formatJumpLabel(const Block &block);

/** Format a jump label for the address of this subroutine.
 *
 *  See formatJumpLabel(const Instruction &instr).
 */
Common::UString formatJumpLabel(const SubRoutine &sub);

/** Format a jump label for this instruction and substitute a name if we can.
 *
 *  Functions very similar to formatJumpLabel(), with one exception:
 *  If we have a human-readable name for this address, use the name instead.
 *
 *  This means the resulting string can be of any length.
 */
Common::UString formatJumpLabelName(const Instruction &instr);

/** Format a jump label for this block.
 *
 *  See formatJumpLabelName(const Instruction &instr).
 */
Common::UString formatJumpLabelName(const Block &block);

/** Format a jump label for this subroutine.
 *
 *  See formatJumpLabelName(const Instruction &instr).
 */
Common::UString formatJumpLabelName(const SubRoutine &sub);

/** Format a list of subroutine parameter types.
 *
 *  The resulting string will contain the textual name of each parameter type,
 *  separated by a comma.
 *
 *  Example: "int, float, string, string"
 *  Example: "int arg_32, float arg_124"
 */
Common::UString formatParameters(const std::vector<const Variable *> &params,
                                 Aurora::GameID game = Aurora::kGameIDUnknown,
                                 bool names = false);

/** Format a list of subroutine return types.
 *
 *  - If the list is empty, the resulting string will be "void"
 *  - If the list contains one element, the resulting string will be the
 *    textual name of this type
 *  - If the list contains more than one element, the resulting string will
 *    be "struct"
 */
Common::UString formatReturn(const std::vector<const Variable *> &returns,
                             Aurora::GameID game = Aurora::kGameIDUnknown);

/** Format the signature of a subroutine.
 *
 *  @param sub   The subroutine to format
 *  @param game  The game the script is from. Used to format the parameter types
 *  @param names Also print the parameter variable names?
 *
 *  Examples: "void main(int, string, object)"
 *            "void main(int arg_32, string arg_124)"
 */
Common::UString formatSignature(const SubRoutine &sub, Aurora::GameID game = Aurora::kGameIDUnknown,
                                bool names = false);

/** Generate a variable name containing the usage of the variable (argument, global, local, ...)
 *  and its number. The resulting string would be something like global_142.
 *
 *  @param  variable The variable to generate the name for.
 *  @return The generated name of the variable.
 */
Common::UString formatVariableName(const Variable *variable);

/** Generate a proper string for the data of an instruction.
 *
 *  @param  instruction The instruction to generate its data.
 *  @return The generated value string of the instruction.
 */
Common::UString formatInstructionData(const Instruction &instruction);

} // End of namespace NWScript

#endif // NWSCRIPT_UTIL_H
