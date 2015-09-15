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
