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

# Unit tests for the Aurora namespace.

aurora_LIBS = \
    $(test_LIBS) \
    src/aurora/libaurora.la \
    src/common/libcommon.la \
    tests/version/libversion.la \
    $(LDADD)

check_PROGRAMS                 += tests/aurora/test_util
tests_aurora_test_util_SOURCES  = tests/aurora/util.cpp
tests_aurora_test_util_LDADD    = $(aurora_LIBS)
tests_aurora_test_util_CXXFLAGS = $(test_CXXFLAGS)

check_PROGRAMS                     += tests/aurora/test_language
tests_aurora_test_language_SOURCES  = tests/aurora/language.cpp
tests_aurora_test_language_LDADD    = $(aurora_LIBS)
tests_aurora_test_language_CXXFLAGS = $(test_CXXFLAGS)

check_PROGRAMS                      += tests/aurora/test_locstring
tests_aurora_test_locstring_SOURCES  = tests/aurora/locstring.cpp
tests_aurora_test_locstring_LDADD    = $(aurora_LIBS)
tests_aurora_test_locstring_CXXFLAGS = $(test_CXXFLAGS)

check_PROGRAMS                    += tests/aurora/test_zipfile
tests_aurora_test_zipfile_SOURCES  = tests/aurora/zipfile.cpp
tests_aurora_test_zipfile_LDADD    = $(aurora_LIBS)
tests_aurora_test_zipfile_CXXFLAGS = $(test_CXXFLAGS)

check_PROGRAMS                    += tests/aurora/test_ssffile
tests_aurora_test_ssffile_SOURCES  = tests/aurora/ssffile.cpp
tests_aurora_test_ssffile_LDADD    = $(aurora_LIBS)
tests_aurora_test_ssffile_CXXFLAGS = $(test_CXXFLAGS)

check_PROGRAMS                      += tests/aurora/test_smallfile
tests_aurora_test_smallfile_SOURCES  = tests/aurora/smallfile.cpp
tests_aurora_test_smallfile_LDADD    = $(aurora_LIBS)
tests_aurora_test_smallfile_CXXFLAGS = $(test_CXXFLAGS)

check_PROGRAMS                    += tests/aurora/test_rimfile
tests_aurora_test_rimfile_SOURCES  = tests/aurora/rimfile.cpp
tests_aurora_test_rimfile_LDADD    = $(aurora_LIBS)
tests_aurora_test_rimfile_CXXFLAGS = $(test_CXXFLAGS)

check_PROGRAMS                     += tests/aurora/test_herffile
tests_aurora_test_herffile_SOURCES  = tests/aurora/herffile.cpp
tests_aurora_test_herffile_LDADD    = $(aurora_LIBS)
tests_aurora_test_herffile_CXXFLAGS = $(test_CXXFLAGS)

check_PROGRAMS                   += tests/aurora/test_ndsrom
tests_aurora_test_ndsrom_SOURCES  = tests/aurora/ndsrom.cpp
tests_aurora_test_ndsrom_LDADD    = $(aurora_LIBS)
tests_aurora_test_ndsrom_CXXFLAGS = $(test_CXXFLAGS)

check_PROGRAMS                    += tests/aurora/test_keyfile
tests_aurora_test_keyfile_SOURCES  = tests/aurora/keyfile.cpp
tests_aurora_test_keyfile_LDADD    = $(aurora_LIBS)
tests_aurora_test_keyfile_CXXFLAGS = $(test_CXXFLAGS)

check_PROGRAMS                    += tests/aurora/test_biffile
tests_aurora_test_biffile_SOURCES  = tests/aurora/biffile.cpp
tests_aurora_test_biffile_LDADD    = $(aurora_LIBS)
tests_aurora_test_biffile_CXXFLAGS = $(test_CXXFLAGS)

check_PROGRAMS                    += tests/aurora/test_bzffile
tests_aurora_test_bzffile_SOURCES  = tests/aurora/bzffile.cpp
tests_aurora_test_bzffile_LDADD    = $(aurora_LIBS)
tests_aurora_test_bzffile_CXXFLAGS = $(test_CXXFLAGS)

check_PROGRAMS                    += tests/aurora/test_erffile
tests_aurora_test_erffile_SOURCES  = tests/aurora/erffile.cpp
tests_aurora_test_erffile_LDADD    = $(aurora_LIBS)
tests_aurora_test_erffile_CXXFLAGS = $(test_CXXFLAGS)

check_PROGRAMS                     += tests/aurora/test_gff3file
tests_aurora_test_gff3file_SOURCES  = tests/aurora/gff3file.cpp
tests_aurora_test_gff3file_LDADD    = $(aurora_LIBS)
tests_aurora_test_gff3file_CXXFLAGS = $(test_CXXFLAGS)

check_PROGRAMS                     += tests/aurora/test_gff4file
tests_aurora_test_gff4file_SOURCES  = tests/aurora/gff4file.cpp
tests_aurora_test_gff4file_LDADD    = $(aurora_LIBS)
tests_aurora_test_gff4file_CXXFLAGS = $(test_CXXFLAGS)

check_PROGRAMS                    += tests/aurora/test_2dafile
tests_aurora_test_2dafile_SOURCES  = tests/aurora/2dafile.cpp
tests_aurora_test_2dafile_LDADD    = $(aurora_LIBS)
tests_aurora_test_2dafile_CXXFLAGS = $(test_CXXFLAGS)

check_PROGRAMS                    += tests/aurora/test_gdafile
tests_aurora_test_gdafile_SOURCES  = tests/aurora/gdafile.cpp
tests_aurora_test_gdafile_LDADD    = $(aurora_LIBS)
tests_aurora_test_gdafile_CXXFLAGS = $(test_CXXFLAGS)

check_PROGRAMS                      += tests/aurora/test_erfwriter
tests_aurora_test_erfwriter_SOURCES  = tests/aurora/erfwriter.cpp
tests_aurora_test_erfwriter_LDADD    = $(aurora_LIBS)
tests_aurora_test_erfwriter_CXXFLAGS = $(test_CXXFLAGS)

check_PROGRAMS                      += tests/aurora/test_xmlfix
tests_aurora_test_xmlfix_SOURCES     = tests/aurora/xmlfix.cpp
tests_aurora_test_xmlfix_LDADD       = $(aurora_LIBS)
tests_aurora_test_xmlfix_CXXFLAGS    = $(test_CXXFLAGS)

