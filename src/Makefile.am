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

# Main tools entry points.

include $(top_srcdir)/Makefile.common

SUBDIRS = \
          common \
          aurora \
          nwscript \
          xml \
          images \
          $(EMPTY)

noinst_HEADERS = \
                 util.h \
                 files_sonic.h \
                 files_dragonage.h \
                 $(EMPTY)

bin_PROGRAMS =

bin_PROGRAMS   += gff2xml
gff2xml_SOURCES = \
                  gff2xml.cpp \
                  $(EMPTY)
gff2xml_LDADD   = \
                  xml/libxml.la \
                  aurora/libaurora.la \
                  common/libcommon.la \
                  $(LDADD) \
                  $(EMPTY)

bin_PROGRAMS   += tlk2xml
tlk2xml_SOURCES = \
                  tlk2xml.cpp \
                  $(EMPTY)
tlk2xml_LDADD   = \
                  xml/libxml.la \
                  aurora/libaurora.la \
                  common/libcommon.la \
                  $(LDADD) \
                  $(EMPTY)

bin_PROGRAMS   += ssf2xml
ssf2xml_SOURCES = \
                  ssf2xml.cpp \
                  $(EMPTY)
ssf2xml_LDADD   = \
                  xml/libxml.la \
                  aurora/libaurora.la \
                  common/libcommon.la \
                  $(LDADD) \
                  $(EMPTY)

bin_PROGRAMS   += xml2tlk
xml2tlk_SOURCES = \
                  xml2tlk.cpp \
                  $(EMPTY)
xml2tlk_LDADD   = \
                  xml/libxml.la \
                  aurora/libaurora.la \
                  common/libcommon.la \
                  $(LDADD) \
                  $(EMPTY)

bin_PROGRAMS   += xml2ssf
xml2ssf_SOURCES = \
                  xml2ssf.cpp \
                  $(EMPTY)
xml2ssf_LDADD   = \
                  xml/libxml.la \
                  aurora/libaurora.la \
                  common/libcommon.la \
                  $(LDADD) \
                  $(EMPTY)

bin_PROGRAMS      += convert2da
convert2da_SOURCES = \
                     convert2da.cpp \
                     $(EMPTY)
convert2da_LDADD   = \
                     aurora/libaurora.la \
                     common/libcommon.la \
                     $(LDADD) \
                     $(EMPTY)

bin_PROGRAMS         += fixpremiumgff
fixpremiumgff_SOURCES = \
                        fixpremiumgff.cpp \
                        $(EMPTY)
fixpremiumgff_LDADD   = \
                        aurora/libaurora.la \
                        common/libcommon.la \
                        $(LDADD) \
                        $(EMPTY)

bin_PROGRAMS += unerf
unerf_SOURCES = \
                unerf.cpp \
                util.cpp \
                files_dragonage.cpp \
                $(EMPTY)
unerf_LDADD   = \
                aurora/libaurora.la \
                common/libcommon.la \
                $(LDADD) \
                $(EMPTY)

bin_PROGRAMS  += unherf
unherf_SOURCES = \
                 unherf.cpp \
                 util.cpp \
                 files_sonic.cpp \
                 $(EMPTY)
unherf_LDADD   = \
                 aurora/libaurora.la \
                 common/libcommon.la \
                 $(LDADD) \
                 $(EMPTY)

bin_PROGRAMS += unrim
unrim_SOURCES = \
                unrim.cpp \
                util.cpp \
                $(EMPTY)
unrim_LDADD   = \
                aurora/libaurora.la \
                common/libcommon.la \
                $(LDADD) \
                $(EMPTY)

bin_PROGRAMS    += unkeybif
unkeybif_SOURCES = \
                   unkeybif.cpp \
                   util.cpp \
                   $(EMPTY)
unkeybif_LDADD   = \
                   aurora/libaurora.la \
                   common/libcommon.la \
                   $(LDADD) \
                   $(EMPTY)

bin_PROGRAMS += unnds
unnds_SOURCES = \
                unnds.cpp \
                util.cpp \
                $(EMPTY)
unnds_LDADD   = \
                aurora/libaurora.la \
                common/libcommon.la \
                $(LDADD) \
                $(EMPTY)

bin_PROGRAMS   += unnsbtx
unnsbtx_SOURCES = \
                  unnsbtx.cpp \
                  util.cpp \
                  $(EMPTY)
unnsbtx_LDADD   = \
                  images/libimages.la \
                  aurora/libaurora.la \
                  common/libcommon.la \
                  $(LDADD) \
                  $(EMPTY)

bin_PROGRAMS   += desmall
desmall_SOURCES = \
                  desmall.cpp \
                  $(EMPTY)
desmall_LDADD   = \
                  aurora/libaurora.la \
                  common/libcommon.la \
                  $(LDADD) \
                  $(EMPTY)

bin_PROGRAMS         += xoreostex2tga
xoreostex2tga_SOURCES = \
                        xoreostex2tga.cpp \
                        $(EMPTY)
xoreostex2tga_LDADD   = \
                        images/libimages.la \
                        aurora/libaurora.la \
                        common/libcommon.la \
                        $(LDADD) \
                        $(EMPTY)

bin_PROGRAMS    += nbfs2tga
nbfs2tga_SOURCES = \
                   nbfs2tga.cpp \
                   $(EMPTY)
nbfs2tga_LDADD   = \
                   images/libimages.la \
                   aurora/libaurora.la \
                   common/libcommon.la \
                   $(LDADD) \
                   $(EMPTY)

bin_PROGRAMS    += ncgr2tga
ncgr2tga_SOURCES = \
                   ncgr2tga.cpp \
                   $(EMPTY)
ncgr2tga_LDADD   = \
                   images/libimages.la \
                   aurora/libaurora.la \
                   common/libcommon.la \
                   $(LDADD) \
                   $(EMPTY)

bin_PROGRAMS    += cbgt2tga
cbgt2tga_SOURCES = \
                   cbgt2tga.cpp \
                   $(EMPTY)
cbgt2tga_LDADD   = \
                   images/libimages.la \
                   aurora/libaurora.la \
                   common/libcommon.la \
                   $(LDADD) \
                   $(EMPTY)

bin_PROGRAMS     += cdpth2tga
cdpth2tga_SOURCES = \
                    cdpth2tga.cpp \
                    $(EMPTY)
cdpth2tga_LDADD   = \
                    images/libimages.la \
                    aurora/libaurora.la \
                    common/libcommon.la \
                    $(LDADD) \
                    $(EMPTY)

bin_PROGRAMS  += ncsdis
ncsdis_SOURCES = \
                 ncsdis.cpp \
                 $(EMPTY)
ncsdis_LDADD   = \
                 nwscript/libnwscript.la \
                 aurora/libaurora.la \
                 common/libcommon.la \
                 $(LDADD) \
                 $(EMPTY)
