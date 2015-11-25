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

/** The type of an NWScript variable.
 *
 *  For the most part, this denotes a simple variable type, like
 *  an integer or a string. There are a few special cases, though.
 *
 *  A variable with a struct type does not actually exist. Instead,
 *  it is implemented by storing the individual components of the
 *  struct as individual variable on the stack. For example, a struct
 *  consisting of an integer and a float is represented by an integer
 *  and a float variable. To quickly isolate a single member of a
 *  struct, the opcode DESTRUCT is often used.
 *
 *  Likewise, a vector is in actuality three consecutive float
 *  variables.
 *
 *  Arrays, on the other hand, only occupy a single element on the
 *  stack. They are also dynamic, growing and shrinking in size as
 *  needed.
 *
 *  A resource type is internally handled quite like a string. In
 *  fact, string/string comparisons between a string and a resource
 *  are legal.
 *
 *  An object is an opaque pointer to the script. However, there are
 *  a few special values, used by the CONST opcode. For example,
 *  there's OBJECT_INVALID, which stands for an invalid object, and
 *  OBJECT_SELF, which is the object executing the current script.
 *
 *  The engine types are also handled opaquely, but even more so.
 *  There are no special values, and the game can redefine what each
 *  engine means. For example, in Neverwinter Nights, EngineType0 is
 *  "effect", while in Dragon Age: Origins this is "event".
 *
 *  The any type can represent a variable of any type. This is, for
 *  example, used to implement a generic GetSize() engine function
 *  that queries the cardinality of a variable of any array type.
 */
enum VariableType {
	kTypeVoid             = 0, ///< void. Unknown type or no variable.
	kTypeInt                 , ///< int. Signed 32-bit integer.
	kTypeFloat               , ///< float. 32-bit IEEE floating point.
	kTypeString              , ///< string. 0-terminated, ASCII.
	kTypeResource            , ///< resource. A game resource filename string.
	kTypeObject              , ///< object. An opaque pointer to an object in the game world.
	kTypeVector              , ///< vector. Three float variables.
	kTypeStruct              , ///< struct. An aggregation of several base types.
	kTypeEngineType0         , ///< For example: effect. Opaque pointer to a game-specific type.
	kTypeEngineType1         , ///< For example: event. Opaque pointer to a game-specific type.
	kTypeEngineType2         , ///< For example: location. Opaque pointer to a game-specific type.
	kTypeEngineType3         , ///< For example: talent. Opaque pointer to a game-specific type.
	kTypeEngineType4         , ///< For example: itemproperty. Opaque pointer to a game-specific type.
	kTypeEngineType5         , ///< For example: player. Opaque pointer to a game-specific type.
	kTypeScriptState         , ///< action. A functor, implemented as stackframe plus offset.
	kTypeIntArray            , ///< int[]. Dynamic array of integers.
	kTypeFloatArray          , ///< float[]. Dynamic array of floating point numbers.
	kTypeStringArray         , ///< string[]. Dynamic array of strings.
	kTypeResourceArray       , ///< resource[]. Dynamic array of game resource filenames.
	kTypeObjectArray         , ///< object[]. Dynamic array of opaque pointers to game world objects.
	kTypeEngineType0Array    , ///< For example: effect[]. Dynamic array of game-specific types.
	kTypeEngineType1Array    , ///< For example: event[]. Dynamic array of game-specific types.
	kTypeEngineType2Array    , ///< For example: location[]. Dynamic array of game-specific types.
	kTypeEngineType3Array    , ///< For example: talent[]. Dynamic array of game-specific types.
	kTypeEngineType4Array    , ///< For example: itemproperty[]. Dynamic array of game-specific types.
	kTypeEngineType5Array    , ///< For example: player[]. Dynamic array of game-specific types.
	kTypeAny                 , ///< any. Can hold any other type.
	kTypeIntRef              , ///< ref int. Reference to an integer.
	kTypeFloatRef            , ///< ref float. Reference to a floating point number.
	kTypeStringRef           , ///< ref string. Reference to a string.
	kTypeResourceRef         , ///< ref resource. Reference to a game resource filename.
	kTypeObjectRef           , ///< ref object. Reference to an opaque pointer to game world object.
	kTypeEngineType0Ref      , ///< For example: ref effect. Reference to game-specific type.
	kTypeEngineType1Ref      , ///< For example: ref event. Reference to game-specific type.
	kTypeEngineType2Ref      , ///< For example: ref location. Reference to game-specific type.
	kTypeEngineType3Ref      , ///< For example: ref talent. Reference to game-specific type.
	kTypeEngineType4Ref      , ///< For example: ref itemproperty. Reference to game-specific type.
	kTypeEngineType5Ref        ///< For example: ref player. Reference to game-specific type.
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

/** A unique variable defined and used by a script.
 *
 *  A Variable is usually created by the stack analysis of NWScript bytecode
 *  (see analyzeStackGlobals() and analyzeStackSubRoutine() in stack.h).
 */
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
