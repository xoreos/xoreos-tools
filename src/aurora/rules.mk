# xoreos-tools - Tools to help with xoreos development
#
# xoreos-tools is the legal property of its developers, whose names
# can be found in the AUTHORS file distributed with this source
# distribution.
#
# xoreos-tools is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 3
# of the License, or (at your option) any later version.
#
# xoreos-tools is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with xoreos-tools. If not, see <http://www.gnu.org/licenses/>.

# Aurora file loaders and related support code.

noinst_LTLIBRARIES += src/aurora/libaurora.la
src_aurora_libaurora_la_SOURCES =

src_aurora_libaurora_la_SOURCES += \
    src/aurora/types.h \
    src/aurora/util.h \
    src/aurora/language.h \
    src/aurora/language_strings.h \
    src/aurora/archive.h \
    src/aurora/aurorafile.h \
    src/aurora/erffile.h \
    src/aurora/rimfile.h \
    src/aurora/keyfile.h \
    src/aurora/biffile.h \
    src/aurora/ndsrom.h \
    src/aurora/herffile.h \
    src/aurora/obbfile.h \
    src/aurora/locstring.h \
    src/aurora/gff3file.h \
    src/aurora/gff4file.h \
    src/aurora/gff4fields.h \
    src/aurora/talktable.h \
    src/aurora/talktable_tlk.h \
    src/aurora/talktable_gff.h \
    src/aurora/ssffile.h \
    src/aurora/2dafile.h \
    src/aurora/gdafile.h \
    src/aurora/gdaheaders.h \
    src/aurora/smallfile.h \
    src/aurora/nitrofile.h \
    src/aurora/nsbtxfile.h \
    src/aurora/erfwriter.h \
    $(EMPTY)

src_aurora_libaurora_la_SOURCES += \
    src/aurora/util.cpp \
    src/aurora/language.cpp \
    src/aurora/archive.cpp \
    src/aurora/aurorafile.cpp \
    src/aurora/erffile.cpp \
    src/aurora/rimfile.cpp \
    src/aurora/keyfile.cpp \
    src/aurora/biffile.cpp \
    src/aurora/ndsrom.cpp \
    src/aurora/herffile.cpp \
    src/aurora/obbfile.cpp \
    src/aurora/locstring.cpp \
    src/aurora/gff3file.cpp \
    src/aurora/gff4file.cpp \
    src/aurora/talktable.cpp \
    src/aurora/talktable_tlk.cpp \
    src/aurora/talktable_gff.cpp \
    src/aurora/ssffile.cpp \
    src/aurora/2dafile.cpp \
    src/aurora/gdafile.cpp \
    src/aurora/gdaheaders.cpp \
    src/aurora/smallfile.cpp \
    src/aurora/nitrofile.cpp \
    src/aurora/nsbtxfile.cpp \
    src/aurora/erfwriter.cpp \
    $(EMPTY)
