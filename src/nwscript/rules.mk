# xoreos - A reimplementation of BioWare's Aurora engine
#
# xoreos is the legal property of its developers, whose names
# can be found in the AUTHORS file distributed with this source
# distribution.
#
# xoreos is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 3
# of the License, or (at your option) any later version.
#
# xoreos is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with xoreos. If not, see <http://www.gnu.org/licenses/>.

# NWScript analysis.

noinst_LTLIBRARIES += src/nwscript/libnwscript.la
src_nwscript_libnwscript_la_SOURCES =

src_nwscript_libnwscript_la_SOURCES += \
    src/nwscript/variable.h \
    src/nwscript/stack.h \
    src/nwscript/instruction.h \
    src/nwscript/block.h \
    src/nwscript/subroutine.h \
    src/nwscript/util.h \
    src/nwscript/ncsfile.h \
    src/nwscript/game.h \
    src/nwscript/game_nwn.h \
    src/nwscript/game_nwn2.h \
    src/nwscript/game_kotor.h \
    src/nwscript/game_kotor2.h \
    src/nwscript/game_jade.h \
    src/nwscript/game_witcher.h \
    src/nwscript/game_dragonage.h \
    src/nwscript/game_dragonage2.h \
    src/nwscript/controlflow.h \
    src/nwscript/disassembler.h \
    $(EMPTY)

src_nwscript_libnwscript_la_SOURCES += \
    src/nwscript/variable.cpp \
    src/nwscript/stack.cpp \
    src/nwscript/instruction.cpp \
    src/nwscript/block.cpp \
    src/nwscript/subroutine.cpp \
    src/nwscript/util.cpp \
    src/nwscript/ncsfile.cpp \
    src/nwscript/game.cpp \
    src/nwscript/controlflow.cpp \
    src/nwscript/disassembler.cpp \
    $(EMPTY)
