include $(top_srcdir)/Makefile.common

SUBDIRS = \
          common \
          aurora \
          xml \
          images \
          $(EMPTY)

noinst_HEADERS = \
                 util.h \
                 $(EMPTY)

bin_PROGRAMS = \
               gff2xml \
               tlk2xml \
               unerf \
               unherf \
               unrim \
               unkeybif \
               unnds \
               unnsbtx \
               desmall \
               texture2tga \
               nbfs2tga \
               ncgr2tga \
               cbgt2tga \
               cdpth2tga \
               $(EMPTY)

gff2xml_SOURCES = \
                  gff2xml.cpp \
                  $(EMPTY)
gff2xml_LDADD   = \
                  xml/libxml.la \
                  aurora/libaurora.la \
                  common/libcommon.la \
                  $(LDADD) \
                  $(EMPTY)

tlk2xml_SOURCES = \
                  tlk2xml.cpp \
                  $(EMPTY)
tlk2xml_LDADD   = \
                  xml/libxml.la \
                  aurora/libaurora.la \
                  common/libcommon.la \
                  $(LDADD) \
                  $(EMPTY)

unerf_SOURCES = \
                unerf.cpp \
                util.cpp \
                $(EMPTY)
unerf_LDADD   = \
                aurora/libaurora.la \
                common/libcommon.la \
                $(LDADD) \
                $(EMPTY)

unherf_SOURCES = \
                 unherf.cpp \
                 util.cpp \
                 $(EMPTY)
unherf_LDADD   = \
                 aurora/libaurora.la \
                 common/libcommon.la \
                 $(LDADD) \
                 $(EMPTY)

unrim_SOURCES = \
                unrim.cpp \
                util.cpp \
                $(EMPTY)
unrim_LDADD   = \
                aurora/libaurora.la \
                common/libcommon.la \
                $(LDADD) \
                $(EMPTY)

unkeybif_SOURCES = \
                   unkeybif.cpp \
                   util.cpp \
                   $(EMPTY)
unkeybif_LDADD   = \
                   aurora/libaurora.la \
                   common/libcommon.la \
                   $(LDADD) \
                   $(EMPTY)

unnds_SOURCES = \
                unnds.cpp \
                util.cpp \
                $(EMPTY)
unnds_LDADD   = \
                aurora/libaurora.la \
                common/libcommon.la \
                $(LDADD) \
                $(EMPTY)

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

desmall_SOURCES = \
                  desmall.cpp \
                  $(EMPTY)
desmall_LDADD   = \
                  aurora/libaurora.la \
                  common/libcommon.la \
                  $(LDADD) \
                  $(EMPTY)

texture2tga_SOURCES = \
                      texture2tga.cpp \
                      $(EMPTY)
texture2tga_LDADD   = \
                      images/libimages.la \
                      aurora/libaurora.la \
                      common/libcommon.la \
                      $(LDADD) \
                      $(EMPTY)

nbfs2tga_SOURCES = \
                   nbfs2tga.cpp \
                   $(EMPTY)
nbfs2tga_LDADD   = \
                   images/libimages.la \
                   aurora/libaurora.la \
                   common/libcommon.la \
                   $(LDADD) \
                   $(EMPTY)

ncgr2tga_SOURCES = \
                   ncgr2tga.cpp \
                   $(EMPTY)
ncgr2tga_LDADD   = \
                   images/libimages.la \
                   aurora/libaurora.la \
                   common/libcommon.la \
                   $(LDADD) \
                   $(EMPTY)

cbgt2tga_SOURCES = \
                   cbgt2tga.cpp \
                   $(EMPTY)
cbgt2tga_LDADD   = \
                   images/libimages.la \
                   aurora/libaurora.la \
                   common/libcommon.la \
                   $(LDADD) \
                   $(EMPTY)

cdpth2tga_SOURCES = \
                    cdpth2tga.cpp \
                    $(EMPTY)
cdpth2tga_LDADD   = \
                    images/libimages.la \
                    aurora/libaurora.la \
                    common/libcommon.la \
                    $(LDADD) \
                    $(EMPTY)
