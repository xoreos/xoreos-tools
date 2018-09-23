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

# Main tools entry points.

# Headers

noinst_HEADERS += \
    src/util.h \
    $(EMPTY)

# The individual tools

bin_PROGRAMS += src/gff2xml
src_gff2xml_SOURCES = \
    src/gff2xml.cpp \
    src/util.cpp \
    $(EMPTY)
src_gff2xml_LDADD = \
    src/xml/libxml.la \
    src/aurora/libaurora.la \
    src/common/libcommon.la \
    src/version/libversion.la \
    $(LDADD) \
    $(EMPTY)

bin_PROGRAMS += src/tlk2xml
src_tlk2xml_SOURCES = \
    src/tlk2xml.cpp \
    src/util.cpp \
    $(EMPTY)
src_tlk2xml_LDADD = \
    src/xml/libxml.la \
    src/aurora/libaurora.la \
    src/common/libcommon.la \
    src/version/libversion.la \
    $(LDADD) \
    $(EMPTY)

bin_PROGRAMS += src/ssf2xml
src_ssf2xml_SOURCES = \
    src/ssf2xml.cpp \
    src/util.cpp \
    $(EMPTY)
src_ssf2xml_LDADD = \
    src/xml/libxml.la \
    src/aurora/libaurora.la \
    src/common/libcommon.la \
    src/version/libversion.la \
    $(LDADD) \
    $(EMPTY)

bin_PROGRAMS += src/xml2tlk
src_xml2tlk_SOURCES = \
    src/xml2tlk.cpp \
    src/util.cpp \
    $(EMPTY)
src_xml2tlk_LDADD = \
    src/xml/libxml.la \
    src/aurora/libaurora.la \
    src/common/libcommon.la \
    src/version/libversion.la \
    $(LDADD) \
    $(EMPTY)

bin_PROGRAMS += src/xml2ssf
src_xml2ssf_SOURCES = \
    src/xml2ssf.cpp \
    src/util.cpp \
    $(EMPTY)
src_xml2ssf_LDADD = \
    src/xml/libxml.la \
    src/aurora/libaurora.la \
    src/common/libcommon.la \
    src/version/libversion.la \
    $(LDADD) \
    $(EMPTY)

bin_PROGRAMS += src/convert2da
src_convert2da_SOURCES = \
    src/convert2da.cpp \
    src/util.cpp \
    $(EMPTY)
src_convert2da_LDADD = \
    src/aurora/libaurora.la \
    src/common/libcommon.la \
    src/version/libversion.la \
    $(LDADD) \
    $(EMPTY)

bin_PROGRAMS += src/fixpremiumgff
src_fixpremiumgff_SOURCES = \
    src/fixpremiumgff.cpp \
    src/util.cpp \
    $(EMPTY)
src_fixpremiumgff_LDADD = \
    src/aurora/libaurora.la \
    src/common/libcommon.la \
    src/version/libversion.la \
    $(LDADD) \
    $(EMPTY)

bin_PROGRAMS += src/unerf
src_unerf_SOURCES = \
    src/unerf.cpp \
    src/util.cpp \
    $(EMPTY)
src_unerf_LDADD = \
    src/archives/libarchives.la \
    src/aurora/libaurora.la \
    src/common/libcommon.la \
    src/version/libversion.la \
    $(LDADD) \
    $(EMPTY)

bin_PROGRAMS += src/unherf
src_unherf_SOURCES = \
    src/unherf.cpp \
    src/util.cpp \
    $(EMPTY)
src_unherf_LDADD = \
    src/archives/libarchives.la \
    src/aurora/libaurora.la \
    src/common/libcommon.la \
    src/version/libversion.la \
    $(LDADD) \
    $(EMPTY)

bin_PROGRAMS += src/unrim
src_unrim_SOURCES = \
    src/unrim.cpp \
    src/util.cpp \
    $(EMPTY)
src_unrim_LDADD = \
    src/archives/libarchives.la \
    src/aurora/libaurora.la \
    src/common/libcommon.la \
    src/version/libversion.la \
    $(LDADD) \
    $(EMPTY)

bin_PROGRAMS += src/unkeybif
src_unkeybif_SOURCES = \
    src/unkeybif.cpp \
    src/util.cpp \
    $(EMPTY)
src_unkeybif_LDADD = \
    src/archives/libarchives.la \
    src/aurora/libaurora.la \
    src/common/libcommon.la \
    src/version/libversion.la \
    $(LDADD) \
    $(EMPTY)

bin_PROGRAMS += src/unnds
src_unnds_SOURCES = \
    src/unnds.cpp \
    src/util.cpp \
    $(EMPTY)
src_unnds_LDADD = \
    src/archives/libarchives.la \
    src/aurora/libaurora.la \
    src/common/libcommon.la \
    src/version/libversion.la \
    $(LDADD) \
    $(EMPTY)

bin_PROGRAMS += src/unobb
src_unobb_SOURCES = \
    src/unobb.cpp \
    src/util.cpp \
    $(EMPTY)
src_unobb_LDADD = \
    src/archives/libarchives.la \
    src/aurora/libaurora.la \
    src/common/libcommon.la \
    src/version/libversion.la \
    $(LDADD) \
    $(EMPTY)

bin_PROGRAMS += src/unnsbtx
src_unnsbtx_SOURCES = \
    src/unnsbtx.cpp \
    src/util.cpp \
    $(EMPTY)
src_unnsbtx_LDADD = \
    src/archives/libarchives.la \
    src/images/libimages.la \
    src/aurora/libaurora.la \
    src/common/libcommon.la \
    src/version/libversion.la \
    $(LDADD) \
    $(EMPTY)

bin_PROGRAMS += src/desmall
src_desmall_SOURCES = \
    src/desmall.cpp \
    src/util.cpp \
    $(EMPTY)
src_desmall_LDADD = \
    src/aurora/libaurora.la \
    src/common/libcommon.la \
    src/version/libversion.la \
    $(LDADD) \
    $(EMPTY)

bin_PROGRAMS += src/xoreostex2tga
src_xoreostex2tga_SOURCES = \
    src/xoreostex2tga.cpp \
    src/util.cpp \
    $(EMPTY)
src_xoreostex2tga_LDADD = \
    src/images/libimages.la \
    src/aurora/libaurora.la \
    src/common/libcommon.la \
    src/version/libversion.la \
    $(LDADD) \
    $(EMPTY)

bin_PROGRAMS += src/nbfs2tga
src_nbfs2tga_SOURCES = \
    src/nbfs2tga.cpp \
    src/util.cpp \
    $(EMPTY)
src_nbfs2tga_LDADD = \
    src/images/libimages.la \
    src/aurora/libaurora.la \
    src/common/libcommon.la \
    src/version/libversion.la \
    $(LDADD) \
    $(EMPTY)

bin_PROGRAMS += src/ncgr2tga
src_ncgr2tga_SOURCES = \
    src/ncgr2tga.cpp \
    src/util.cpp \
    $(EMPTY)
src_ncgr2tga_LDADD = \
    src/images/libimages.la \
    src/aurora/libaurora.la \
    src/common/libcommon.la \
    src/version/libversion.la \
    $(LDADD) \
    $(EMPTY)

bin_PROGRAMS += src/cbgt2tga
src_cbgt2tga_SOURCES = \
    src/cbgt2tga.cpp \
    src/util.cpp \
    $(EMPTY)
src_cbgt2tga_LDADD = \
    src/images/libimages.la \
    src/aurora/libaurora.la \
    src/common/libcommon.la \
    src/version/libversion.la \
    $(LDADD) \
    $(EMPTY)

bin_PROGRAMS += src/cdpth2tga
src_cdpth2tga_SOURCES = \
    src/cdpth2tga.cpp \
    src/util.cpp \
    $(EMPTY)
src_cdpth2tga_LDADD = \
    src/images/libimages.la \
    src/aurora/libaurora.la \
    src/common/libcommon.la \
    src/version/libversion.la \
    $(LDADD) \
    $(EMPTY)

bin_PROGRAMS += src/ncsdis
src_ncsdis_SOURCES = \
    src/ncsdis.cpp \
    src/util.cpp \
    $(EMPTY)
src_ncsdis_LDADD = \
    src/nwscript/libnwscript.la \
    src/aurora/libaurora.la \
    src/common/libcommon.la \
    src/version/libversion.la \
    $(LDADD) \
    $(EMPTY)

bin_PROGRAMS += src/erf
src_erf_SOURCES = \
    src/erf.cpp \
    src/util.cpp \
    $(EMPTY)
src_erf_LDADD = \
    src/aurora/libaurora.la \
    src/common/libcommon.la \
    src/version/libversion.la \
    $(LDADD) \
    $(EMPTY)

bin_PROGRAMS += src/fix2xml
src_fix2xml_SOURCES = \
    src/fix2xml.cpp \
    src/util.cpp \
    $(EMPTY)
src_fix2xml_LDADD = \
    src/aurora/libaurora.la \
    src/common/libcommon.la \
    src/version/libversion.la \
    $(LDADD) \
    $(EMPTY)

# Subdirectories

include src/version/rules.mk
include src/common/rules.mk
include src/aurora/rules.mk
include src/archives/rules.mk
include src/nwscript/rules.mk
include src/images/rules.mk
include src/xml/rules.mk
