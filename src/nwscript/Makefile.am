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

include $(top_srcdir)/Makefile.common

noinst_LTLIBRARIES = libnwscript.la

noinst_HEADERS = \
                 variable.h \
                 stack.h \
                 instruction.h \
                 block.h \
                 subroutine.h \
                 util.h \
                 ncsfile.h \
                 game.h \
                 game_nwn.h \
                 game_nwn2.h \
                 game_kotor.h \
                 game_kotor2.h \
                 game_jade.h \
                 game_witcher.h \
                 game_dragonage.h \
                 game_dragonage2.h \
                 controlflow.h \
                 disassembler.h \
                 $(EMPTY)

libnwscript_la_SOURCES = \
                         variable.cpp \
                         stack.cpp \
                         instruction.cpp \
                         block.cpp \
                         subroutine.cpp \
                         util.cpp \
                         ncsfile.cpp \
                         game.cpp \
                         controlflow.cpp \
                         disassembler.cpp \
                         $(EMPTY)
