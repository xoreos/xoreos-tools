include $(top_srcdir)/Makefile.common

SUBDIRS = \
          common \
          aurora \
          xml \
          $(EMPTY)

noinst_HEADERS = \
                 util.h \
                 $(EMPTY)

bin_PROGRAMS = \
               gff2xml \
               unerf \
               unrim \
               unkeybif \
               unnds \
               $(EMPTY)

gff2xml_SOURCES = \
                  gff2xml.cpp \
                  $(EMPTY)
gff2xml_LDADD   = \
                  xml/libxml.la \
                  aurora/libaurora.la \
                  common/libcommon.la \
                  $(EMPTY)

unerf_SOURCES = \
                unerf.cpp \
                util.cpp \
                $(EMPTY)
unerf_LDADD   = \
                aurora/libaurora.la \
                common/libcommon.la \
                $(EMPTY)

unrim_SOURCES = \
                unrim.cpp \
                util.cpp \
                $(EMPTY)
unrim_LDADD   = \
                aurora/libaurora.la \
                common/libcommon.la \
                $(EMPTY)

unkeybif_SOURCES = \
                   unkeybif.cpp \
                   util.cpp \
                   $(EMPTY)
unkeybif_LDADD   = \
                   aurora/libaurora.la \
                   common/libcommon.la \
                   $(EMPTY)

unnds_SOURCES = \
                unnds.cpp \
                util.cpp \
                $(EMPTY)
unnds_LDADD   = \
                aurora/libaurora.la \
                common/libcommon.la \
                $(EMPTY)
