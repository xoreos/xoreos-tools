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

# Common support code used all over the codebase.

noinst_LTLIBRARIES += src/common/libcommon.la
src_common_libcommon_la_SOURCES =

src_common_libcommon_la_SOURCES += \
    src/common/system.h \
    src/common/noreturn.h \
    src/common/fallthrough.h \
    src/common/types.h \
    src/common/endianness.h \
    src/common/deallocator.h \
    src/common/scopedptr.h \
    src/common/disposableptr.h \
    src/common/ptrlist.h \
    src/common/ptrvector.h \
    src/common/ptrmap.h \
    src/common/maths.h \
    src/common/singleton.h \
    src/common/ustring.h \
    src/common/hash.h \
    src/common/md5.h \
    src/common/blowfish.h \
    src/common/deflate.h \
    src/common/base64.h \
    src/common/error.h \
    src/common/util.h \
    src/common/strutil.h \
    src/common/encoding.h \
    src/common/encoding_strings.h \
    src/common/platform.h \
    src/common/readstream.h \
    src/common/memreadstream.h \
    src/common/writestream.h \
    src/common/memwritestream.h \
    src/common/stdinstream.h \
    src/common/stdoutstream.h \
    src/common/streamtokenizer.h \
    src/common/readfile.h \
    src/common/writefile.h \
    src/common/filepath.h \
    src/common/binsearch.h \
    src/common/cli.h \
    $(EMPTY)

src_common_libcommon_la_SOURCES += \
    src/common/maths.cpp \
    src/common/ustring.cpp \
    src/common/md5.cpp \
    src/common/blowfish.cpp \
    src/common/deflate.cpp \
    src/common/base64.cpp \
    src/common/error.cpp \
    src/common/util.cpp \
    src/common/strutil.cpp \
    src/common/encoding.cpp \
    src/common/platform.cpp \
    src/common/readstream.cpp \
    src/common/memreadstream.cpp \
    src/common/writestream.cpp \
    src/common/memwritestream.cpp \
    src/common/stdinstream.cpp \
    src/common/stdoutstream.cpp \
    src/common/streamtokenizer.cpp \
    src/common/readfile.cpp \
    src/common/writefile.cpp \
    src/common/filepath.cpp \
    src/common/cli.cpp \
    $(EMPTY)
