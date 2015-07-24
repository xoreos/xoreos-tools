include $(top_srcdir)/Makefile.common

ACLOCAL_AMFLAGS = -I m4 --install

EXTRA_DIST = \
             autogen.sh \
             BUILDDEP \
             README.md \
             CONTRIBUTING.md \
             CODE_OF_CONDUCT.md \
             dists/arch/PKGBUILD \
             dists/debian/changelog \
             dists/debian/compat \
             dists/debian/control \
             dists/debian/copyright \
             dists/debian/rules \
             dists/debian/xoreos-tools.lintian-overrides \
             dists/debian/source/format \
             .uncrustifyrc \
             CMakeLists.txt \
             cmake/CMakeAM.cmake \
             cmake/toolchain/i686-windows-mingw.cmake \
             cmake/toolchain/x86_64-windows-mingw.cmake \
             $(EMPTY)

dist_doc_DATA = \
                COPYING \
                AUTHORS \
                ChangeLog \
                NEWS \
                README.md \
                CONTRIBUTING.md \
                CODE_OF_CONDUCT.md \
                $(EMPTY)

dist_man1_MANS = \
                 man/cbgt2tga.1 \
                 man/cdpth2tga.1 \
                 man/convert2da.1 \
                 man/desmall.1 \
                 man/gff2xml.1 \
                 man/nbfs2tga.1 \
                 man/ncgr2tga.1 \
                 man/tlk2xml.1 \
                 man/unerf.1 \
                 man/unherf.1 \
                 man/unkeybif.1 \
                 man/unnds.1 \
                 man/unnsbtx.1 \
                 man/unrim.1 \
                 man/xoreostex2tga.1 \
                 $(EMPTY)

SUBDIRS = \
          utf8cpp \
          gitstamp \
          src \
          $(EMPTY)
