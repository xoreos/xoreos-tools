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
 *  A variable used in BioWare's NWScript.
 */

#ifndef NWSCRIPT_VARIABLE_H
#define NWSCRIPT_VARIABLE_H

#include <vector>
#include <deque>
#include <set>

#include "src/common/types.h"

namespace NWScript {

struct Instruction;

/** The type of an NWScript variable. */
enum VariableType {
	kTypeVoid             = 0,
	kTypeInt                 ,
	kTypeFloat               ,
	kTypeString              ,
	kTypeResource            ,
	kTypeObject              ,
	kTypeVector              ,
	kTypeStruct              ,
	kTypeEngineType0         ,
	kTypeEngineType1         ,
	kTypeEngineType2         ,
	kTypeEngineType3         ,
	kTypeEngineType4         ,
	kTypeEngineType5         ,
	kTypeScriptState         , ///< "action".
	kTypeIntArray            ,
	kTypeFloatArray          ,
	kTypeStringArray         ,
	kTypeResourceArray       ,
	kTypeObjectArray         ,
	kTypeEngineType0Array    ,
	kTypeEngineType1Array    ,
	kTypeEngineType2Array    ,
	kTypeEngineType3Array    ,
	kTypeEngineType4Array    ,
	kTypeEngineType5Array    ,
	kTypeAny                 , ///< Any other type.
	kTypeIntRef              ,
	kTypeFloatRef            ,
	kTypeStringRef           ,
	kTypeResourceRef         ,
	kTypeObjectRef           ,
	kTypeEngineType0Ref      ,
	kTypeEngineType1Ref      ,
	kTypeEngineType2Ref      ,
	kTypeEngineType3Ref      ,
	kTypeEngineType4Ref      ,
	kTypeEngineType5Ref
};

/** What a variable is used for. */
enum VariableUse {
	kVariableUseUnknown,   ///< We don't know anything about this variable.
	kVariableUseGlobal,    ///< This is a global variable.
	kVariableUseLocal,     ///< This is a subroutine-local variable.
	kVariableUseParameter, ///< This is a subroutine parameter.
	kVariableUseReturn     ///< This is a subroutine return value.
};

/** A struct describing how the type of a variable was inferred. */
struct TypeInference {
	/** The type we inferred. */
	VariableType type;

	/** The instruction where we inferred this type. */
	const Instruction *instruction;


	TypeInference(VariableType t, const Instruction *i) : type(t), instruction(i) {
	}
};

/** A unique variable defined and used by a script. */
struct Variable {
	size_t id;         ///< The unique ID of this variable.
	VariableType type; ///< The type of this variable.
	VariableUse  use;  ///< What this variable is used for.

	const Instruction *creator; ///< The instruction that created this variable.

	/** Instructions that read this variable. */
	std::vector<const Instruction *> readers;
	/** Instructions that write this variable. */
	std::vector<const Instruction *> writers;

	/** Variables that were created by duplicating this variable. */
	std::set<const Variable *> duplicates;

	/** Variables that are logically the very same variable as this one.
	 *
	 *  When control flow merges branching forks back together, these are
	 *  variables that occupy the same stack space. They are logically the
	 *  same variable, only created through a different potential path.
	 */
	std::set<const Variable *> siblings;

	/** Instructions that helped to infer the type of this variable. */
	std::deque<TypeInference> typeInference;


	Variable(size_t i, VariableType t, VariableUse u = kVariableUseUnknown) :
		id(i), type(t), use(u), creator(0) {

	}

	/** Return all variable IDs belonging to the same sibling group as this variable. */
	std::vector<size_t> getSiblingGroup() const;

	/** Return the lowest variable ID in this variable's sibling group. */
	size_t getLowestSibling() const;
};

/** The whole variable space used in one script. */
typedef std::deque<Variable> VariableSpace;

} // End of namespace NWScript

#endif // NWSCRIPT_VARIABLE_H
