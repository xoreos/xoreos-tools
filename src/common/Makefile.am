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

# Pass the git version information to the compiler

VER_REVDESC = $(shell cat $(top_srcdir)/gitstamp/gitstamp)
VERSION_FLAGS = $(shell if test x$(VER_REVDESC) != "x"; then echo "-DXOREOS_REVDESC=\\\"$(VER_REVDESC)\\\""; fi)

VER_REV = $(shell cat $(top_srcdir)/gitstamp/gitstamp | cut -d '+' -f 2-)
VERSION_FLAGS += $(shell if test x$(VER_REV) != "x"; then echo "-DXOREOS_REV=\\\"$(VER_REV)\\\""; fi)

VER_DATE = $(shell date -u '+%Y-%m-%dT%H:%M:%SZ')
VERSION_FLAGS += $(shell if test x$(VER_DATE) != "x"; then echo "-DXOREOS_BUILDDATE=\\\"$(VER_DATE)\\\""; fi)

AM_CXXFLAGS += $(VERSION_FLAGS)
AM_CFLAGS   += $(VERSION_FLAGS)

# Rebuild the version.lo when the git version changed
version.lo: $(top_srcdir)/gitstamp/gitstamp

noinst_LTLIBRARIES = libcommon.la

noinst_HEADERS = \
                 system.h \
                 types.h \
                 endianness.h \
                 version.h \
                 maths.h \
                 noncopyable.h \
                 singleton.h \
                 ustring.h \
                 hash.h \
                 md5.h \
                 blowfish.h \
                 base64.h \
                 error.h \
                 util.h \
                 strutil.h \
                 encoding.h \
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
                       version.cpp \
                       maths.cpp \
                       ustring.cpp \
                       md5.cpp \
                       blowfish.cpp \
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
