/* xoreos - A reimplementation of BioWare's Aurora engine
 *
 * xoreos is the legal property of its developers, whose names
 * can be found in the AUTHORS file distributed with this source
 * distribution.
 *
 * xoreos is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * xoreos is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xoreos. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file
 *  A variable used in BioWare's NWScript.
 */

#include <algorithm>

#include "src/nwscript/variable.h"

namespace NWScript {

std::vector<size_t> Variable::getSiblingGroup() const {
	std::vector<size_t> sib;

	sib.reserve(siblings.size() + 1);

	for (std::set<const Variable *>::const_iterator s = siblings.begin(); s != siblings.end(); ++s)
			sib.push_back((*s)->id);
	sib.push_back(id);

	std::sort(sib.begin(), sib.end());

	return sib;
}

size_t Variable::getLowestSibling() const {
	return getSiblingGroup().front();
}

} // End of namespace NWScript
