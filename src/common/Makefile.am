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

# Common support code used all over the codebase.

include $(top_srcdir)/Makefile.common

noinst_LTLIBRARIES = libcommon.la

noinst_HEADERS = \
                 system.h \
                 types.h \
                 endianness.h \
                 maths.h \
                 noncopyable.h \
                 singleton.h \
                 ustring.h \
                 hash.h \
                 md5.h \
                 blowfish.h \
                 deflate.h \
                 base64.h \
                 error.h \
                 util.h \
                 strutil.h \
                 encoding.h \
                 encoding_strings.h \
                 platform.h \
                 readstream.h \
                 memreadstream.h \
                 writestream.h \
                 memwritestream.h \
                 stdinstream.h \
                 stdoutstream.h \
                 streamtokenizer.h \
                 readfile.h \
                 writefile.h \
                 filepath.h \
                 binsearch.h \
                 $(EMPTY)

libcommon_la_SOURCES = \
                       maths.cpp \
                       ustring.cpp \
                       md5.cpp \
                       blowfish.cpp \
                       deflate.cpp \
                       base64.cpp \
                       error.cpp \
                       util.cpp \
                       strutil.cpp \
                       encoding.cpp \
                       platform.cpp \
                       readstream.cpp \
                       memreadstream.cpp \
                       writestream.cpp \
                       memwritestream.cpp \
                       stdinstream.cpp \
                       stdoutstream.cpp \
                       streamtokenizer.cpp \
                       readfile.cpp \
                       writefile.cpp \
                       filepath.cpp \
                       $(EMPTY)
