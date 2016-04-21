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

# Top-level automake build recipe.

include $(top_srcdir)/Makefile.common

ACLOCAL_AMFLAGS = -I m4 --install

EXTRA_DIST = \
             autogen.sh \
             BUILDDEP.md \
             dists/arch/PKGBUILD \
             dists/debian/changelog \
             dists/debian/compat \
             dists/debian/control \
             dists/debian/copyright \
             dists/debian/rules \
             dists/debian/xoreos-tools.lintian-overrides \
             dists/debian/source/format \
             dists/fedora/build-from-git.sh \
             dists/fedora/xoreos-tools.spec \
             .uncrustifyrc \
             CMakeLists.txt \
             cmake/CMakeAM.cmake \
             cmake/FindIconv.cmake \
             cmake/SetCheckCompilerFlag.cmake \
             cmake/toolchain/i686-windows-mingw.cmake \
             cmake/toolchain/x86_64-windows-mingw.cmake \
             $(EMPTY)

dist_doc_DATA = \
                COPYING \
                COPYING.GPLv2 \
                AUTHORS \
                ChangeLog \
                NEWS.md \
                TODO \
                README.md \
                CONTRIBUTING.md \
                CODE_OF_CONDUCT.md \
                $(EMPTY)

dist_man1_MANS = \
                 man/cbgt2tga.1 \
                 man/cdpth2tga.1 \
                 man/convert2da.1 \
                 man/fixpremiumgff.1 \
                 man/desmall.1 \
                 man/gff2xml.1 \
                 man/nbfs2tga.1 \
                 man/ncgr2tga.1 \
                 man/tlk2xml.1 \
                 man/ssf2xml.1 \
                 man/xml2tlk.1 \
                 man/xml2ssf.1 \
                 man/unerf.1 \
                 man/unherf.1 \
                 man/unkeybif.1 \
                 man/unnds.1 \
                 man/unnsbtx.1 \
                 man/unrim.1 \
                 man/xoreostex2tga.1 \
                 man/ncsdis.1 \
                 $(EMPTY)

SUBDIRS = \
          utf8cpp \
          gitstamp \
          src \
          $(EMPTY)
