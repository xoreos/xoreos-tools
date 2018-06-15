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

# Unit tests for the XML namespace.

xml_LIBS = \
    $(test_LIBS) \
    src/xml/libxml.la \
    src/aurora/libaurora.la \
    src/common/libcommon.la \
    tests/version/libversion.la \
    $(LDADD)

check_PROGRAMS                   += tests/xml/test_xmlparser
tests_xml_test_xmlparser_SOURCES  = tests/xml/xmlparser.cpp
tests_xml_test_xmlparser_LDADD    = $(xml_LIBS)
tests_xml_test_xmlparser_CXXFLAGS = $(test_CXXFLAGS)
