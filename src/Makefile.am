include $(top_srcdir)/Makefile.common

SUBDIRS = \
          common \
          aurora \
          xml \
          $(EMPTY)

noinst_HEADERS = \
                 $(EMPTY)

bin_PROGRAMS = \
               gff2xml \
               $(EMPTY)

gff2xml_SOURCES = \
                  gff2xml.cpp \
                  $(EMPTY)
gff2xml_LDADD   = \
                  xml/libxml.la \
                  aurora/libaurora.la \
                  common/libcommon.la \
                  $(EMPTY)
