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
 *  Higher-level control flow analysis on NWScript bytecode.
 */

#ifndef NWSCRIPT_CONTROLFLOW_H
#define NWSCRIPT_CONTROLFLOW_H

namespace NWScript {

struct Block;
typedef std::deque<Block> Blocks;

/** Given a whole set of script blocks, perform a deeper control flow analysis.
 *
 *  Control structures such as loops and conditionals will be identified, and
 *  the blocks' controls field will be updated with this new information.
 */
void analyzeControlFlow(Blocks &blocks);

} // End of namespace NWScript

#endif // NWSCRIPT_CONTROLFLOW_H
