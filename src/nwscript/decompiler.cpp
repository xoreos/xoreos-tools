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
 *  NWScript byte code to source code decompiler.
 */

#include "src/common/strutil.h"

#include "src/nwscript/decompiler.h"
#include "src/nwscript/util.h"
#include "src/nwscript/game.h"

namespace NWScript {

Decompiler::Decompiler(Common::SeekableReadStream &ncs, Aurora::GameID game) {
	_ncs.reset(new NCSFile(ncs, game));
}

void Decompiler::createNSS(Common::WriteStream &out) {
	_ncs->analyzeStack();
	_ncs->analyzeControlFlow();

	out.writeString("// Decompiled using ncsdecomp");

	const Stack &stack = _ncs->getGlobals();

	out.writeString("\n\n");
	for (const auto &global : stack) {
		out.writeString(getVariableTypeName(global.variable->type, _ncs->getGame()));
		out.writeString(" " + formatVariableName(global.variable));
		out.writeString(";\n");
	}

	const SubRoutines &subRoutines = _ncs->getSubRoutines();
	for (const auto &subRoutine : subRoutines) {
		writeSubRoutine(out, subRoutine);
	}
}

void Decompiler::writeSubRoutine(Common::WriteStream &out, const NWScript::SubRoutine &subRoutine) {
	out.writeString("\n");
	out.writeString(formatSignature(subRoutine, _ncs->getGame(), true));
	out.writeString(" {\n");

	// TODO: local sub routine variables

	if (!subRoutine.blocks.empty())
		writeBlock(out, subRoutine.blocks.front(), 1);

	out.writeString("}\n");
}

void Decompiler::writeBlock(Common::WriteStream &out, const Block *block, size_t indent) {
	for (const auto instruction : block->instructions) {
		writeInstruction(out, instruction, indent);
	}

	for (const auto &childType : block->childrenTypes) {
		if (isSubRoutineCall(childType)) {
			writeIndent(out, indent);
			const Instruction *instruction = block->instructions.back();


			out.writeString(formatJumpLabelName(*instruction->branches[0]));
			out.writeString("(");

			for (size_t i = 0; i < instruction->variables.size(); ++i) {
				out.writeString(formatVariableName(instruction->variables[i]));
				if (i < instruction->variables.size() - 1)
					out.writeString(", ");
			}

			out.writeString(");\n");

			writeBlock(out, block->children[1], indent);
		}
	}

	for (const auto &control : block->controls) {
		if (control.type == kControlTypeReturn) {
			writeIndent(out, indent);
			out.writeString("return;\n");
		} else if (control.type == kControlTypeIfCond) {
			writeIfControl(out, control, indent);
		}

		// TODO: while
		// TODO: break
		// TODO: continue
	}
}

void Decompiler::writeIfControl(Common::WriteStream &out, const ControlStructure &control, size_t indent) {
	writeIndent(out, indent);

	const Variable *cond = control.ifCond->instructions.back()->variables[0];
	out.writeString("if (");
	out.writeString(formatVariableName(cond));
	out.writeString(") {\n");

	if (control.ifTrue)
		writeBlock(out, control.ifTrue, indent + 1);

	writeIndent(out, indent);
	out.writeString("}");

	if (control.ifElse) {
		out.writeString(" else {\n");
		writeBlock(out, control.ifElse, indent + 1);

		writeIndent(out, indent);
		out.writeString("}");
	}
	out.writeString("\n");

	if (control.ifNext)
		writeBlock(out, control.ifNext, indent);
}

void Decompiler::writeInstruction(Common::WriteStream &out, const Instruction* instruction, size_t indent) {
	switch (instruction->opcode) {
		case kOpcodeCONST: {
			const Variable *v = instruction->variables[0];
			writeIndent(out, indent);
			out.writeString(getVariableTypeName(v->type) + " " + formatVariableName(v) + " = " + formatInstructionData(*instruction) + ";\n");

			break;
		}

		case kOpcodeACTION: {
			unsigned int paramCount = instruction->args[1];

			writeIndent(out, indent);

			if (instruction->variables.size() > paramCount) {
				const Variable *ret = instruction->variables.back();
				out.writeString(getVariableTypeName(ret->type, _ncs->getGame()) + " " + formatVariableName(ret) + " = ");
			}

			out.writeString(getFunctionName(_ncs->getGame(), instruction->args[0]));
			out.writeString("(");
			for (unsigned int i = 0; i < paramCount; ++i) {
				out.writeString(formatVariableName(instruction->variables[i]));
				if (i < paramCount - 1)
					out.writeString(", ");
			}
			out.writeString(");\n");

			break;
		}

		case kOpcodeCPDOWNBP:
		case kOpcodeCPDOWNSP:
		case kOpcodeCPTOPBP:
		case kOpcodeCPTOPSP: {
			const Variable *v1 = instruction->variables[0];
			const Variable *v2 = instruction->variables[1];

			writeIndent(out, indent);
			out.writeString(getVariableTypeName(v2->type, _ncs->getGame()) + " " + formatVariableName(v2) + " = " + formatVariableName(v1) + ";\n");

			break;
		}

		case kOpcodeLOGAND: {
			const Variable *v1 = instruction->variables[0];
			const Variable *v2 = instruction->variables[1];
			const Variable *result = instruction->variables[2];

			writeIndent(out, indent);
			out.writeString(
					getVariableTypeName(result->type, _ncs->getGame()) + " " +
					formatVariableName(result) + " = " +
					formatVariableName(v1) + " && " + formatVariableName(v2) + ";\n"
			);

			break;
		}

		case kOpcodeLOGOR: {
			const Variable *v1 = instruction->variables[0];
			const Variable *v2 = instruction->variables[1];
			const Variable *result = instruction->variables[2];

			writeIndent(out, indent);
			out.writeString(
					getVariableTypeName(result->type, _ncs->getGame()) + " " +
					formatVariableName(result) + " = " +
					formatVariableName(v1) + " || " + formatVariableName(v2) + ";\n"
			);

			break;
		}

		case kOpcodeEQ: {
			const Variable *v1 = instruction->variables[0];
			const Variable *v2 = instruction->variables[1];
			const Variable *result = instruction->variables[2];

			writeIndent(out, indent);
			out.writeString(
					getVariableTypeName(result->type, _ncs->getGame()) + " " +
					formatVariableName(result) + " = " +
					formatVariableName(v1) + " == " + formatVariableName(v2) + ";\n"
			);


			break;
		}

		case kOpcodeLEQ: {
			const Variable *v1 = instruction->variables[0];
			const Variable *v2 = instruction->variables[1];
			const Variable *result = instruction->variables[2];

			writeIndent(out, indent);
			out.writeString(
					getVariableTypeName(result->type, _ncs->getGame()) + " " +
					formatVariableName(result) + " = " +
					formatVariableName(v1) + " <= " + formatVariableName(v2) + ";\n"
			);

			break;
		}

		case kOpcodeLT: {
			const Variable *v1 = instruction->variables[0];
			const Variable *v2 = instruction->variables[1];
			const Variable *result = instruction->variables[2];

			writeIndent(out, indent);
			out.writeString(
					getVariableTypeName(result->type, _ncs->getGame()) + " " +
					formatVariableName(result) + " = " +
					formatVariableName(v1) + " < " + formatVariableName(v2) + ";\n"
			);

			break;
		}

		case kOpcodeGEQ: {
			const Variable *v1 = instruction->variables[0];
			const Variable *v2 = instruction->variables[1];
			const Variable *result = instruction->variables[2];

			writeIndent(out, indent);
			out.writeString(
					getVariableTypeName(result->type, _ncs->getGame()) + " " +
					formatVariableName(result) + " = " +
					formatVariableName(v1) + " >= " + formatVariableName(v2) + ";\n"
			);

			break;
		}

		case kOpcodeGT: {
			const Variable *v1 = instruction->variables[0];
			const Variable *v2 = instruction->variables[1];
			const Variable *result = instruction->variables[2];

			writeIndent(out, indent);
			out.writeString(
					getVariableTypeName(result->type, _ncs->getGame()) + " " +
					formatVariableName(result) + " = " +
					formatVariableName(v1) + " > " + formatVariableName(v2) + ";\n"
			);

			break;
		}

		case kOpcodeNOT: {
			const Variable *v = instruction->variables[0];
			const Variable *result = instruction->variables[1];

			writeIndent(out, indent);
			out.writeString(
					getVariableTypeName(result->type, _ncs->getGame()) + " " +
					formatVariableName(result) + " = " +
					"!" + formatVariableName(v) + ";\n"
			);

			break;
		}

		case kOpcodeRSADD: {
			if (instruction->variables.empty()) {
				writeIndent(out, indent);
				out.writeString("// RSADD not interpretable\n");
				break;
			}

			const Variable *v = instruction->variables[0];

			writeIndent(out, indent);
			out.writeString(
					getVariableTypeName(v->type, _ncs->getGame()) + " " +
					formatVariableName(v) + " = "
			);

			switch (v->type) {
				case kTypeString:
					out.writeString("\"\"");
					break;
				case kTypeInt:
					out.writeString("0");
					break;
				case kTypeFloat:
					out.writeString("0.0");
					break;

				default:
					// TODO: No idea how empty objects or engine types are intialized.
					out.writeString("0");
					break;
			}

			out.writeString(";\n");

			break;
		}

		// TODO: Not all necessary instruction are implemented here

		default:
			break;
	}
}

void Decompiler::writeIndent(Common::WriteStream &out, size_t indent) {
	for (size_t i = 0; i < indent; ++i)
		out.writeString("\t");
}

} // End of namespace NWScript
