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

# Our distribution files.

# Arch Linux
EXTRA_DIST += \
    dists/arch/PKGBUILD \
    $(EMPTY)

# Debian
EXTRA_DIST += \
    dists/debian/changelog \
    dists/debian/compat \
    dists/debian/control \
    dists/debian/copyright \
    dists/debian/rules \
    dists/debian/xoreos-tools.lintian-overrides \
    dists/debian/source/format \
    $(EMPTY)

# Fedora
EXTRA_DIST += \
    dists/fedora/build-from-git.sh \
    dists/fedora/xoreos-tools.spec \
    $(EMPTY)
