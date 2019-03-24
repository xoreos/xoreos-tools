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

#ifndef NWSCRIPT_DECOMPILER_H
#define NWSCRIPT_DECOMPILER_H

#include "src/common/scopedptr.h"
#include "src/common/writestream.h"

#include "src/nwscript/ncsfile.h"

namespace NWScript {

class Decompiler {
public:
	Decompiler(Common::SeekableReadStream &ncs, Aurora::GameID game = Aurora::kGameIDUnknown);

	/** Decompile the NCS file into a NSS file. */
	void createNSS(Common::WriteStream &out);

private:
	void writeSubRoutine(Common::WriteStream &out, NWScript::SubRoutine subRoutine);
	void writeBlock(Common::WriteStream &out, const Block *block, size_t indent);
	void writeIfControl(Common::WriteStream &out, const ControlStructure &control, size_t indent);
	void writeInstruction(Common::WriteStream &out, const Instruction *instruction, size_t indent);
	void writeIndent(Common::WriteStream &out, size_t indent);

	Common::ScopedPtr<NCSFile> _ncs;
};

} // End of namespace NWScript

#endif // NWSCRIPT_DECOMPILER_H
