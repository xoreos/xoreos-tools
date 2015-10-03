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

#ifndef NWSCRIPT_NCSFILE_H
#define NWSCRIPT_NCSFILE_H

#include <deque>

#include "src/common/types.h"
#include "src/common/ustring.h"

#include "src/aurora/aurorafile.h"

#include "src/nwscript/types.h"

namespace Common {
	class SeekableReadStream;
}

namespace NWScript {

/** Parse an NCS file, compiled NWScript bytecode, into a structure of instructions. */
class NCSFile : public Aurora::AuroraBase {
public:
	typedef std::deque<Instruction> Instructions;


	NCSFile(Common::SeekableReadStream &ncs);
	~NCSFile();

	/** Return the size of the script bytecode in bytes.
	 *  Should be equal to the size of the containing stream. */
	size_t size() const;

	/** Return all the instructions within this NCS file. */
	const Instructions &getInstructions() const;

	/** Find an instruction by address. */
	const Instruction *findInstruction(uint32 address) const;


private:
	size_t _size;

	Instructions _instructions;


	void load(Common::SeekableReadStream &ncs);
	void parse(Common::SeekableReadStream &ncs);

	bool parseStep(Common::SeekableReadStream &ncs);

	Instructions::iterator findInstruction(uint32 address);
};

} // End of namespace NWScript

#endif // NWSCRIPT_NCSFILE_H
