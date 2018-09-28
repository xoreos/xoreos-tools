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

# XML readers and writers.

noinst_LTLIBRARIES += src/xml/libxml.la
src_xml_libxml_la_SOURCES =

src_xml_libxml_la_SOURCES += \
    src/xml/xmlwriter.h \
    src/xml/xmlparser.h \
    src/xml/gffdumper.h \
    src/xml/gff3dumper.h \
    src/xml/gff4dumper.h \
    src/xml/gff4fields.h \
    src/xml/tlkdumper.h \
    src/xml/tlkcreator.h \
    src/xml/ssfdumper.h \
    src/xml/ssfcreator.h \
    src/xml/gffcreator.h \
    src/xml/gff3creator.h \
    $(EMPTY)

src_xml_libxml_la_SOURCES += \
    src/xml/xmlwriter.cpp \
    src/xml/xmlparser.cpp \
    src/xml/gffdumper.cpp \
    src/xml/gff3dumper.cpp \
    src/xml/gff4dumper.cpp \
    src/xml/tlkdumper.cpp \
    src/xml/tlkcreator.cpp \
    src/xml/ssfdumper.cpp \
    src/xml/ssfcreator.cpp \
    src/xml/gffcreator.cpp \
    src/xml/gff3creator.cpp \
    $(EMPTY)
