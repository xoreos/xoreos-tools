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

# Top-level automake build recipe.

# Extra files to help with the sources
EXTRA_DIST += \
    BUILDDEP.md \
    .uncrustifyrc \
    $(EMPTY)

# Extra autotools files
EXTRA_DIST += \
    autogen.sh \
    $(EMPTY)

# Files for CMake, our alternative build system
EXTRA_DIST += \
    CMakeLists.txt \
    cmake/CMakeAM.cmake \
    cmake/CMakeUninstall.cmake \
    cmake/FindLibLZMA.cmake \
    cmake/SetCheckCompilerFlag.cmake \
    cmake/toolchain/i686-windows-mingw.cmake \
    cmake/toolchain/x86_64-windows-mingw.cmake \
    $(EMPTY)

# Subdirectories

include dists/rules.mk
include man/rules.mk
include doc/rules.mk

include external/rules.mk

include src/rules.mk

include tests/rules.mk
