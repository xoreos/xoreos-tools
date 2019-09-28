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

#include "src/common/strutil.h"
#include "src/sound/fmodeventfile.h"

#include "src/xml/fevdumper.h"
#include "src/xml/xmlwriter.h"

namespace XML {

void FEVDumper::dump(Common::WriteStream &output, Common::SeekableReadStream &input) {
	Sound::FMODEventFile fev(input);

	XML::XMLWriter xml(output);

	// Write initial opening tag.
	xml.openTag("fev");

	// Write bank name.
	xml.addProperty("bankname", fev.getBankName());
	xml.breakLine();

	std::vector<Sound::FMODEventFile::WaveBank> waveBanks = fev.getWaveBanks();
	dumpWavebanks(xml, waveBanks);

	std::vector<Sound::FMODEventFile::Event> events = fev.getEvents();
	dumpEvents(xml, events);

	std::vector<Sound::FMODEventFile::ReverbDefinition> reverbs = fev.getReverbs();
	dumpReverbs(xml, reverbs);

	xml.closeTag();
}

void FEVDumper::dumpWavebanks(XML::XMLWriter &xml, const std::vector<Sound::FMODEventFile::WaveBank> &waveBanks) {
	// Write WaveBanks.
	xml.openTag("wavebanks");
	xml.breakLine();

	for (const auto &waveBank : waveBanks) {
		xml.openTag("wavebank");
		xml.breakLine();

		xml.openTag("name");
		xml.setContents(waveBank.name);
		xml.closeTag();
		xml.breakLine();

		xml.openTag("maxstreams");
		xml.setContents(Common::composeString(waveBank.maxStreams));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("banktype");
		switch (waveBank.streamingType) {
			case Sound::FMODEventFile::kDecompressIntoMemory:
				xml.setContents("DecompressIntoMemory");
				break;
			case Sound::FMODEventFile::kStreamFromDisk:
				xml.setContents("StreamFromDisk");
				break;
			case Sound::FMODEventFile::kLoadIntoMemory:
				xml.setContents("LoadIntoMemory");
				break;
		}
		xml.closeTag();
		xml.breakLine();

		xml.closeTag();
		xml.breakLine();
	}

	xml.closeTag();
	xml.breakLine();
}

void FEVDumper::dumpEvents(XML::XMLWriter &xml, const std::vector<Sound::FMODEventFile::Event> &events) {
	// Write Events.
	xml.openTag("events");
	xml.breakLine();

	for (const auto &event : events) {
		xml.openTag("event");
		xml.breakLine();

		xml.openTag("name");
		xml.setContents(event.name);
		xml.closeTag();
		xml.breakLine();

		Sound::FMODEventFile::EventMode mode = event.mode;
		xml.openTag("mode");
		if (mode == Sound::FMODEventFile::k2D)
			xml.setContents("2D");
		else if (mode == Sound::FMODEventFile::k3D)
			xml.setContents("3D");
		xml.closeTag();
		xml.breakLine();

		xml.openTag("category");
		xml.setContents(event.category);
		xml.closeTag();
		xml.breakLine();

		xml.openTag("volume");
		xml.addProperty("randomization", Common::composeString(event.volumeRandomization));
		xml.setContents(Common::composeString(event.volume));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("pitch");
		xml.addProperty("randomization", Common::composeString(event.pitchRandomization));
		xml.setContents(Common::composeString(event.pitch));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("priority");
		xml.setContents(Common::composeString(event.priority));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("nmaxplaybacks");
		xml.setContents(Common::composeString(event.maxPlaybacks));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("maxplaybacksbehaviour");
		xml.setContents(Common::composeString(event.maxPlaybacksBehavior));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("rolloff3d");
		switch (event.rollof3D) {
			case Sound::FMODEventFile::kLogarithmic:
				xml.setContents("Logarithmic");
				break;
			case Sound::FMODEventFile::kLinear:
				xml.setContents("Linear");
				break;
			case Sound::FMODEventFile::kCustom:
				xml.setContents("Custom");
				break;
			case Sound::FMODEventFile::kUnspecified:
				xml.setContents("Unspecified");
				break;
		}
		xml.closeTag();
		xml.breakLine();

		xml.openTag("mindistance3d");
		xml.setContents(Common::composeString(event.minDistance3D));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("maxdistance3d");
		xml.setContents(Common::composeString(event.maxDistance3D));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("position3d");
		xml.addProperty("randomization", Common::composeString(event.positionRandomization3D));
		switch (event.position3D) {
			case Sound::FMODEventFile::kWorldRelative:
				xml.setContents("WorldRelative");
				break;
			case Sound::FMODEventFile::kHeadRelative:
				xml.setContents("HeadRelative");
				break;
		}
		xml.closeTag();
		xml.breakLine();

		xml.openTag("coneinsideangle3d");
		xml.setContents(Common::composeString(event.coneInsideAngle3D));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("coneoutsideangle3d");
		xml.setContents(Common::composeString(event.coneOutsideAngle3D));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("outsidevolume3d");
		xml.setContents(Common::composeString(event.coneOutsideVolume3D));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("dopplerfactor3d");
		xml.setContents(Common::composeString(event.dopplerFactor3D));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("speakerspread3d");
		xml.setContents(Common::composeString(event.speakerSpread3D));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("panlevel3d");
		xml.setContents(Common::composeString(event.panLevel3D));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("speakerl2d");
		xml.setContents(Common::composeString(event.Speaker2DL));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("speakerc2d");
		xml.setContents(Common::composeString(event.Speaker2DC));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("speakerr2d");
		xml.setContents(Common::composeString(event.Speaker2DR));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("speakerlr2d");
		xml.setContents(Common::composeString(event.Speaker2DLR));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("speakerrr2d");
		xml.setContents(Common::composeString(event.Speaker2DRR));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("speakerls2d");
		xml.setContents(Common::composeString(event.Speaker2DLS));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("speakerrs2d");
		xml.setContents(Common::composeString(event.Speaker2DRS));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("speakerlfe");
		xml.setContents(Common::composeString(event.SpeakerLFE));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("reverbdrylevel");
		xml.setContents(Common::composeString(event.ReverbDryLevel));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("reverbwetlevel");
		xml.setContents(Common::composeString(event.ReverbWetLevel));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("fadeintime");
		xml.setContents(Common::composeString(event.fadeInTime));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("fadeouttime");
		xml.setContents(Common::composeString(event.fadeOutTime));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("spawnintensity");
		xml.addProperty("randomization", Common::composeString(event.spawnIntensityRandomization));
		xml.setContents(Common::composeString(event.spawnIntensity));
		xml.closeTag();
		xml.breakLine();

		dumpUserProperties(xml, event.userProperties);

		xml.closeTag();
		xml.breakLine();
	}

	xml.closeTag();
	xml.breakLine();
}

void FEVDumper::dumpReverbs(XML::XMLWriter &xml, const std::vector<Sound::FMODEventFile::ReverbDefinition> &reverbs) {
	xml.openTag("reverb");
	xml.breakLine();

	for (const auto &reverb : reverbs) {
		xml.openTag("room");
		xml.setContents(Common::composeString(reverb.room));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("roomhf");
		xml.setContents(Common::composeString(reverb.roomHF));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("decaytime");
		xml.setContents(Common::composeString(reverb.decayTime));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("decayhfratio");
		xml.setContents(Common::composeString(reverb.decayHFRatio));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("reflections");
		xml.setContents(Common::composeString(reverb.reflections));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("reflectdelay");
		xml.setContents(Common::composeString(reverb.reflectDelay));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("reverb");
		xml.setContents(Common::composeString(reverb.reverb));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("reverbdelay");
		xml.setContents(Common::composeString(reverb.reverbDelay));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("hfreference");
		xml.setContents(Common::composeString(reverb.hfReference));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("roomlf");
		xml.setContents(Common::composeString(reverb.roomLF));
		xml.closeTag();
		xml.breakLine();

		xml.openTag("lfreference");
		xml.setContents(Common::composeString(reverb.lfReference));
		xml.closeTag();
		xml.breakLine();
	}

	xml.closeTag();
	xml.breakLine();
}

void FEVDumper::dumpUserProperties(XML::XMLWriter &xml, const std::map<Common::UString, Sound::FMODEventFile::Property> &properties) {
	xml.openTag("userproperties");
	xml.breakLine();

	for (const auto &property : properties) {
		xml.openTag("property");
		xml.addProperty("name", property.first);

		switch (property.second.type) {
			case Sound::FMODEventFile::kPropertyInt:
				xml.addProperty("type", "int");
				xml.setContents(Common::composeString(boost::get<int>(property.second.value)));
				break;
			case Sound::FMODEventFile::kPropertyFloat:
				xml.addProperty("type", "float");
				xml.setContents(Common::composeString(boost::get<float>(property.second.value)));
				break;
			case Sound::FMODEventFile::kPropertyString:
				xml.addProperty("type", "string");
				xml.setContents(boost::get<Common::UString>(property.second.value));
				break;
		}
		xml.closeTag();
		xml.breakLine();
	}

	xml.closeTag();
	xml.breakLine();
}

}
