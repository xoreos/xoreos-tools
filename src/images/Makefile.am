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

# Image loaders.

include $(top_srcdir)/Makefile.common

SUBDIRS = \
          $(EMPTY)

noinst_LTLIBRARIES = libimages.la

noinst_HEADERS = \
                 types.h \
                 util.h \
                 s3tc.h \
                 decoder.h \
                 dumptga.h \
                 winiconimage.h \
                 tga.h \
                 dds.h \
                 tpc.h \
                 txb.h \
                 sbm.h \
                 xoreositex.h \
                 nbfs.h \
                 nclr.h \
                 ncgr.h \
                 cbgt.h \
                 cdpth.h \
                 $(EMPTY)

libimages_la_SOURCES = \
                       s3tc.cpp \
                       decoder.cpp \
                       dumptga.cpp \
                       winiconimage.cpp \
                       tga.cpp \
                       dds.cpp \
                       tpc.cpp \
                       txb.cpp \
                       sbm.cpp \
                       xoreositex.cpp \
                       nbfs.cpp \
                       nclr.cpp \
                       ncgr.cpp \
                       cbgt.cpp \
                       cdpth.cpp \
                       $(EMPTY)

libimages_la_LIBADD = \
                      $(EMPTY)
