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
 *  Utility functions to handle files used in BioWare's Aurora engine.
 */

#include "src/common/util.h"
#include "src/common/ustring.h"
#include "src/common/filepath.h"

#include "src/aurora/util.h"

namespace Aurora {

/** File type <-> extension mapping. */
struct FileExtension {
	FileType type;
	const char *extension;
};

static const FileExtension fileExtensions[] = {
	{kFileTypeNone,           ""    },
	{kFileTypeRES,            ".res"},
	{kFileTypeBMP,            ".bmp"},
	{kFileTypeMVE,            ".mve"},
	{kFileTypeTGA,            ".tga"},
	{kFileTypeWAV,            ".wav"},
	{kFileTypePLT,            ".plt"},
	{kFileTypeINI,            ".ini"},
	{kFileTypeBMU,            ".bmu"},
	{kFileTypeMPG,            ".mpg"},
	{kFileTypeTXT,            ".txt"},
	{kFileTypeWMA,            ".wma"},
	{kFileTypeWMV,            ".wmv"},
	{kFileTypeXMV,            ".xmv"},
	{kFileTypePLH,            ".plh"},
	{kFileTypeTEX,            ".tex"},
	{kFileTypeMDL,            ".mdl"},
	{kFileTypeTHG,            ".thg"},
	{kFileTypeFNT,            ".fnt"},
	{kFileTypeLUA,            ".lua"},
	{kFileTypeSLT,            ".slt"},
	{kFileTypeNSS,            ".nss"},
	{kFileTypeNCS,            ".ncs"},
	{kFileTypeMOD,            ".mod"},
	{kFileTypeARE,            ".are"},
	{kFileTypeSET,            ".set"},
	{kFileTypeIFO,            ".ifo"},
	{kFileTypeBIC,            ".bic"},
	{kFileTypeWOK,            ".wok"},
	{kFileType2DA,            ".2da"},
	{kFileTypeTLK,            ".tlk"},
	{kFileTypeTXI,            ".txi"},
	{kFileTypeGIT,            ".git"},
	{kFileTypeBTI,            ".bti"},
	{kFileTypeUTI,            ".uti"},
	{kFileTypeBTC,            ".btc"},
	{kFileTypeUTC,            ".utc"},
	{kFileTypeDLG,            ".dlg"},
	{kFileTypeITP,            ".itp"},
	{kFileTypeBTT,            ".btt"},
	{kFileTypeUTT,            ".utt"},
	{kFileTypeDDS,            ".dds"},
	{kFileTypeBTS,            ".bts"},
	{kFileTypeUTS,            ".uts"},
	{kFileTypeLTR,            ".ltr"},
	{kFileTypeGFF,            ".gff"},
	{kFileTypeFAC,            ".fac"},
	{kFileTypeBTE,            ".bte"},
	{kFileTypeUTE,            ".ute"},
	{kFileTypeBTD,            ".btd"},
	{kFileTypeUTD,            ".utd"},
	{kFileTypeBTP,            ".btp"},
	{kFileTypeUTP,            ".utp"},
	{kFileTypeDFT,            ".dft"},
	{kFileTypeDTF,            ".dtf"},
	{kFileTypeGIC,            ".gic"},
	{kFileTypeGUI,            ".gui"},
	{kFileTypeCSS,            ".css"},
	{kFileTypeCCS,            ".ccs"},
	{kFileTypeBTM,            ".btm"},
	{kFileTypeUTM,            ".utm"},
	{kFileTypeDWK,            ".dwk"},
	{kFileTypePWK,            ".pwk"},
	{kFileTypeBTG,            ".btg"},
	{kFileTypeUTG,            ".utg"},
	{kFileTypeJRL,            ".jrl"},
	{kFileTypeSAV,            ".sav"},
	{kFileTypeUTW,            ".utw"},
	{kFileType4PC,            ".4pc"},
	{kFileTypeSSF,            ".ssf"},
	{kFileTypeHAK,            ".hak"},
	{kFileTypeNWM,            ".nwm"},
	{kFileTypeBIK,            ".bik"},
	{kFileTypeNDB,            ".ndb"},
	{kFileTypePTM,            ".ptm"},
	{kFileTypePTT,            ".ptt"},
	{kFileTypeNCM,            ".ncm"},
	{kFileTypeMFX,            ".mfx"},
	{kFileTypeMAT,            ".mat"},
	{kFileTypeMDB,            ".mdb"},
	{kFileTypeSAY,            ".say"},
	{kFileTypeTTF,            ".ttf"},
	{kFileTypeTTC,            ".ttc"},
	{kFileTypeCUT,            ".cut"},
	{kFileTypeKA,             ".ka" },
	{kFileTypeJPG,            ".jpg"},
	{kFileTypeICO,            ".ico"},
	{kFileTypeOGG,            ".ogg"},
	{kFileTypeSPT,            ".spt"},
	{kFileTypeSPW,            ".spw"},
	{kFileTypeWFX,            ".wfx"},
	{kFileTypeUGM,            ".ugm"},
	{kFileTypeQDB,            ".qdb"},
	{kFileTypeQST,            ".qst"},
	{kFileTypeNPC,            ".npc"},
	{kFileTypeSPN,            ".spn"},
	{kFileTypeUTX,            ".utx"},
	{kFileTypeMMD,            ".mmd"},
	{kFileTypeSMM,            ".smm"},
	{kFileTypeUTA,            ".uta"},
	{kFileTypeMDE,            ".mde"},
	{kFileTypeMDV,            ".mdv"},
	{kFileTypeMDA,            ".mda"},
	{kFileTypeMBA,            ".mba"},
	{kFileTypeOCT,            ".oct"},
	{kFileTypeBFX,            ".bfx"},
	{kFileTypePDB,            ".pdb"},
	{kFileTypeTheWitcherSave, ".TheWitcherSave"},
	{kFileTypePVS,            ".pvs"},
	{kFileTypeCFX,            ".cfx"},
	{kFileTypeLUC,            ".luc"},
	{kFileTypePRB,            ".prb"},
	{kFileTypeCAM,            ".cam"},
	{kFileTypeVDS,            ".vds"},
	{kFileTypeBIN,            ".bin"},
	{kFileTypeWOB,            ".wob"},
	{kFileTypeAPI,            ".api"},
	{kFileTypeProperties,     ".properties"},
	{kFileTypePNG,            ".png"},
	{kFileTypeLYT,            ".lyt"},
	{kFileTypeVIS,            ".vis"},
	{kFileTypeRIM,            ".rim"},
	{kFileTypePTH,            ".pth"},
	{kFileTypeLIP,            ".lip"},
	{kFileTypeBWM,            ".bwm"},
	{kFileTypeTXB,            ".txb"},
	{kFileTypeTPC,            ".tpc"},
	{kFileTypeMDX,            ".mdx"},
	{kFileTypeRSV,            ".rsv"},
	{kFileTypeSIG,            ".sig"},
	{kFileTypeMAB,            ".mab"},
	{kFileTypeQST2,           ".qst"},
	{kFileTypeSTO,            ".sto"},
	{kFileTypeMDX2,           ".mdx"},
	{kFileTypeTXB2,           ".txb"},
	{kFileTypeFSM,            ".fsm"},
	{kFileTypeART,            ".art"},
	{kFileTypeBIP,            ".bip"},
	{kFileTypeMDB2,           ".mdb"},
	{kFileTypeMDA2,           ".mda"},
	{kFileTypeSPT2,           ".spt"},
	{kFileTypeGR2,            ".gr2"},
	{kFileTypeFXA,            ".fxa"},
	{kFileTypeFXE,            ".fxe"},
	{kFileTypeJPG2,           ".jpg"},
	{kFileTypePWC,            ".pwc"},
	{kFileType1DA,            ".1da"},
	{kFileTypeERF,            ".erf"},
	{kFileTypeBIF,            ".bif"},
	{kFileTypeKEY,            ".key"},

	{kFileTypeDBF,            ".dbf"},
	{kFileTypeCDX,            ".cdx"},
	{kFileTypeFPT,            ".fpt"},

	{kFileTypeZIP,            ".zip"},
	{kFileTypeFXM,            ".fxm"},
	{kFileTypeFXS,            ".fxs"},
	{kFileTypeXML,            ".xml"},
	{kFileTypeWLK,            ".wlk"},
	{kFileTypeUTR,            ".utr"},
	{kFileTypeSEF,            ".sef"},
	{kFileTypePFX,            ".pfx"},
	{kFileTypeTFX,            ".tfx"},
	{kFileTypeIFX,            ".ifx"},
	{kFileTypeLFX,            ".lfx"},
	{kFileTypeBBX,            ".bbx"},
	{kFileTypePFB,            ".pfb"},
	{kFileTypeUPE,            ".upe"},
	{kFileTypeUSC,            ".usc"},
	{kFileTypeULT,            ".ult"},
	{kFileTypeFX ,            ".fx" },
	{kFileTypeMAX,            ".max"},
	{kFileTypeDOC,            ".doc"},
	{kFileTypeSCC,            ".scc"},
	{kFileTypeWMP,            ".wmp"},
	{kFileTypeOSC,            ".osc"},
	{kFileTypeTRN,            ".trn"},
	{kFileTypeUEN,            ".uen"},
	{kFileTypeROS,            ".ros"},
	{kFileTypeRST,            ".rst"},
	{kFileTypePTX,            ".ptx"},
	{kFileTypeLTX,            ".ltx"},
	{kFileTypeTRX,            ".trx"},

	{kFileTypeNDS,            ".nds"},
	{kFileTypeHERF,           ".herf"},
	{kFileTypeDICT,           ".dict"},
	{kFileTypeSMALL,          ".small"},
	{kFileTypeCBGT,           ".cbgt"},
	{kFileTypeCDPTH,          ".cdpth"},
	{kFileTypeEMIT,           ".emit"},
	{kFileTypeITM,            ".itm"},
	{kFileTypeNANR,           ".nanr"},
	{kFileTypeNBFP,           ".nbfp"},
	{kFileTypeNBFS,           ".nbfs"},
	{kFileTypeNCER,           ".ncer"},
	{kFileTypeNCGR,           ".ncgr"},
	{kFileTypeNCLR,           ".nclr"},
	{kFileTypeNFTR,           ".nftr"},
	{kFileTypeNSBCA,          ".nsbca"},
	{kFileTypeNSBMD,          ".nsbmd"},
	{kFileTypeNSBTA,          ".nsbta"},
	{kFileTypeNSBTP,          ".nsbtp"},
	{kFileTypeNSBTX,          ".nsbtx"},
	{kFileTypePAL,            ".pal"},
	{kFileTypeRAW,            ".raw"},
	{kFileTypeSADL,           ".sadl"},
	{kFileTypeSDAT,           ".sdat"},
	{kFileTypeSMP,            ".smp"},
	{kFileTypeSPL,            ".spl"},
	{kFileTypeVX,             ".vx"},

	{kFileTypeANB,            ".anb"},
	{kFileTypeANI,            ".ani"},
	{kFileTypeCNS,            ".cns"},
	{kFileTypeCUR,            ".cur"},
	{kFileTypeEVT,            ".evt"},
	{kFileTypeFDL,            ".fdl"},
	{kFileTypeFXO,            ".fxo"},
	{kFileTypeGAD,            ".gad"},
	{kFileTypeGDA,            ".gda"},
	{kFileTypeGFX,            ".gfx"},
	{kFileTypeLDF,            ".ldf"},
	{kFileTypeLST,            ".lst"},
	{kFileTypeMAL,            ".mal"},
	{kFileTypeMAO,            ".mao"},
	{kFileTypeMMH,            ".mmh"},
	{kFileTypeMOP,            ".mop"},
	{kFileTypeMOR,            ".mor"},
	{kFileTypeMSH,            ".msh"},
	{kFileTypeMTX,            ".mtx"},
	{kFileTypeNCC,            ".ncc"},
	{kFileTypePHY,            ".phy"},
	{kFileTypePLO,            ".plo"},
	{kFileTypeSTG,            ".stg"},
	{kFileTypeTBI,            ".tbi"},
	{kFileTypeTNT,            ".tnt"},
	{kFileTypeARL,            ".arl"},
	{kFileTypeFEV,            ".fev"},
	{kFileTypeFSB,            ".fsb"},
	{kFileTypeOPF,            ".opf"},

	{kFileTypeMOV,            ".mov"},
	{kFileTypeCURS,           ".curs"},
	{kFileTypePICT,           ".pict"},
	{kFileTypeRSRC,           ".rsrc"},
	{kFileTypePLIST,          ".plist"},

	{kFileTypeCRE,            ".cre"},
	{kFileTypePSO,            ".pso"},
	{kFileTypeVSO,            ".vso"},
	{kFileTypeABC,            ".abc"},
	{kFileTypeSBM,            ".sbm"},
	{kFileTypePVD,            ".pvd"},
	{kFileTypeAMP,            ".amp"},
	{kFileTypePLA,            ".pla"},
	{kFileTypeTRG,            ".trg"},
	{kFileTypePK,             ".pk" },

	{kFileTypeALS,            ".als"},
	{kFileTypeAPL,            ".apl"},
	{kFileTypeAssembly,       ".assembly"},
	{kFileTypeBAK,            ".bak"},
	{kFileTypeBNK,            ".bnk"},
	{kFileTypeCL,             ".cl"},
	{kFileTypeCNV,            ".cnv"},
	{kFileTypeCON,            ".con"},
	{kFileTypeDAT,            ".dat"},
	{kFileTypeDX11,           ".dx11"},
	{kFileTypeIDS,            ".ids"},
	{kFileTypeLOG,            ".log"},
	{kFileTypeMAP,            ".map"},
	{kFileTypeMML,            ".mml"},
	{kFileTypeMP3,            ".mp3"},
	{kFileTypePCK,            ".pck"},
	{kFileTypeRML,            ".rml"},
	{kFileTypeS,              ".s"  },
	{kFileTypeSTA,            ".sta"},
	{kFileTypeSVR,            ".svr"},
	{kFileTypeVLM,            ".vlm"},
	{kFileTypeWBD,            ".wbd"},
	{kFileTypeXBX,            ".xbx"},
	{kFileTypeXLS,            ".xls"},

	{kFileTypeBZF,            ".bzf"},

	{kFileTypeADV,            ".adv"}
};

FileType getFileType(const Common::UString &path) {
	const Common::UString ext = Common::FilePath::getExtension(path);

	for (int i = 0; i < ARRAYSIZE(fileExtensions); i++)
		if (ext.equalsIgnoreCase(fileExtensions[i].extension))
			return fileExtensions[i].type;

	return kFileTypeNone;
}

FileType aliasFileType(FileType type, GameID game) {
	switch (game) {
		case kGameIDNWN2:
			switch ((int) type) {
				case 3000:
					return Aurora::kFileTypeOSC;
				case 3001:
					return Aurora::kFileTypeUSC;
				case 3002:
					return Aurora::kFileTypeTRN;
				case 3003:
					return Aurora::kFileTypeUTR;
				case 3004:
					return Aurora::kFileTypeUEN;
				case 3005:
					return Aurora::kFileTypeULT;
				case 3006:
					return Aurora::kFileTypeSEF;
				case 3007:
					return Aurora::kFileTypePFX;
				case 3008:
					return Aurora::kFileTypeCAM;
				case 3009:
					return Aurora::kFileTypeLFX;
				case 3010:
					return Aurora::kFileTypeBFX;
				case 3011:
					return Aurora::kFileTypeUPE;
				case 3012:
					return Aurora::kFileTypeROS;
				case 3013:
					return Aurora::kFileTypeRST;
				case 3014:
					return Aurora::kFileTypeIFX;
				case 3015:
					return Aurora::kFileTypePFB;
				case 3016:
					return Aurora::kFileTypeZIP;
				case 3017:
					return Aurora::kFileTypeWMP;
				case 3018:
					return Aurora::kFileTypeBBX;
				case 3019:
					return Aurora::kFileTypeTFX;
				case 3020:
					return Aurora::kFileTypeWLK;
				case 3021:
					return Aurora::kFileTypeXML;
				case 3022:
					return Aurora::kFileTypeSCC;
				case 3033:
					return Aurora::kFileTypePTX;
				case 3034:
					return Aurora::kFileTypeLTX;
				case 3035:
					return Aurora::kFileTypeTRX;
				default:
					break;
			}
			break;

		case kGameIDJade:
			switch (type) {
				case kFileTypeBTC:
					return kFileTypeCRE;
				case kFileTypeBTP:
					return kFileTypePLA;
				case kFileTypeBTT:
					return kFileTypeTRG;
				default:
					break;
			}
			break;

		default:
			break;
	}

	return type;
}

Common::UString setFileType(const Common::UString &path, FileType type) {
	Common::UString ext;

	for (int i = 0; i < ARRAYSIZE(fileExtensions); i++)
		if (fileExtensions[i].type == type) {
			ext = fileExtensions[i].extension;
			break;
		}

	return Common::FilePath::changeExtension(path, ext);
}

Common::UString getPlatformDescription(Platform platform) {
	static const char *names[] = {
		"Windows", "Nintendo DS", "Mac OS X", "Xbox", "PlayStation 3", "Xbox 360", "GNU/Linux", "Unknown"
	};

	return names[platform];
}

} // End of namespace Aurora
