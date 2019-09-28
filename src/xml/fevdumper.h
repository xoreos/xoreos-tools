/* xoreos-tools - Tools to help with xoreos development
 *
 * xoreos-tools is the legal property of its developers, whose names
 * can be found in the AUTHORS file distributed with this source
 * distribution.
 *
 * xoreos-tools is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * xoreos-tools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xoreos-tools. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file
 *  Dump FEV into XML files.
 */

#ifndef XML_FEVDUMPER_H
#define XML_FEVDUMPER_H

#include <boost/core/noncopyable.hpp>

#include "src/sound/fmodeventfile.h"

#include "src/common/writestream.h"
#include "src/common/readstream.h"
#include "src/xml/xmlwriter.h"

namespace XML {

class FEVDumper : boost::noncopyable {
public:
	/** Dump the FEV into XML. */
	static void dump(Common::WriteStream &output, Common::SeekableReadStream &input);

private:
	static void dumpWavebanks(XML::XMLWriter &xml, const std::vector<Sound::FMODEventFile::WaveBank> &waveBanks);
	static void dumpEvents(XML::XMLWriter &xml, const std::vector<Sound::FMODEventFile::Event> &events);
	static void dumpReverbs(XML::XMLWriter &xml, const std::vector<Sound::FMODEventFile::ReverbDefinition> &reverbs);
	static void dumpUserProperties(XML::XMLWriter &xml, const std::map<Common::UString, Sound::FMODEventFile::Property> &properties);
};

}

#endif //XOREOS_TOOLS_FEVDUMPER_H
