include $(top_srcdir)/Makefile.common

ACLOCAL_AMFLAGS = -I m4 --install

EXTRA_DIST = \
             autogen.sh \
             BUILDDEP \
             README.md \
             CONTRIBUTING.md \
             CODE_OF_CONDUCT.md \
             .uncrustifyrc \
             CMakeLists.txt \
             cmake/CMakeAM.cmake \
             cmake/toolchain/i686-windows-mingw.cmake \
             cmake/toolchain/x86_64-windows-mingw.cmake \
             $(EMPTY)

dist_doc_DATA = \
                COPYING \
                AUTHORS \
                README.md \
                CONTRIBUTING.md \
                CODE_OF_CONDUCT.md \
                $(EMPTY)

SUBDIRS = \
          utf8cpp \
          gitstamp \
          src \
          $(EMPTY)
