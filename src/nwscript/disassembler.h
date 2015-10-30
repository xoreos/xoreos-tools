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
 *  Disassembling NWScript bytecode.
 */

#ifndef NWSCRIPT_DISASSEMBLER_H
#define NWSCRIPT_DISASSEMBLER_H

#include <map>
#include <vector>

#include "src/common/types.h"

#include "src/aurora/types.h"

namespace Common {
	class SeekableReadStream;
	class WriteStream;
};

namespace NWScript {

class NCSFile;
struct Block;

class Disassembler {
public:
	Disassembler(Common::SeekableReadStream &ncs);
	Disassembler(NCSFile *ncs);
	~Disassembler();

	/** Perform a deep analysis of the script stack, so that more information is available. */
	void analyzeStack(Aurora::GameID &game);

	/** Create a full disassembly listing, with addresses and raw bytes. */
	void createListing (Common::WriteStream &out, Aurora::GameID &game, bool printStack);
	/** Create bare disassembly output, potentially capable of being re-assembled. */
	void createAssembly(Common::WriteStream &out, Aurora::GameID &game, bool printStack);
	/** Create a graphviz dot file that can be plotted into a control flow graph. */
	void createDot     (Common::WriteStream &out, Aurora::GameID &game);


private:
	NCSFile *_ncs;


	void writeDotClusteredBlocks(Common::WriteStream &out, Aurora::GameID &game,
	                             std::map<uint32, size_t> &blockNodeCount);
	void writeDotBlocks         (Common::WriteStream &out, Aurora::GameID &game,
	                             const std::vector<const Block *> &blocks,
	                             std::map<uint32, size_t> &blockNodeCount);
	void writeDotBlockEdges     (Common::WriteStream &out, std::map<uint32, size_t> &blockNodeCount);
};

} // End of namespace NWScript

#endif // NWSCRIPT_DISASSEMBLER_H
