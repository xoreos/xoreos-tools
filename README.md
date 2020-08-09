xoreos-tools README
===================

A collection of tools to help with the reverse-engineering of BioWare's
Aurora engine games. xoreos-tools is part of the xoreos project; please
see the [xoreos website](https://xoreos.org/) and its [GitHub
repositories](https://github.com/xoreos) for details, especially [the
main README.md](https://github.com/xoreos/xoreos/blob/master/README.md).

The tools included here are licensed under the terms of the [GNU General
Public License version 3](https://www.gnu.org/licenses/agpl-3.0.html)
or (at your option) any later version.

Currently, the following tools are included:

* gff2xml: Convert BioWare GFF to XML
* tlk2xml: Convert BioWare TLK to XML
* ssf2xml: Convert BioWare SSF to XML
* fev2xml: Convert FMOD FEV to XML
* xml2gff: Convert XML back to BioWare GFF
* xml2tlk: Convert XML back to BioWare TLK
* xml2ssf: Convert XML back to BioWare SSF
* convert2da: Convert BioWare 2DA/GDA to 2DA/CSV
* fixpremiumgff: Repair BioWare GFF files in NWN premium module HAKs
* fixnwn2xml: Convert Obsidian NWN2 XML to valid XML
* unerf: Extract BioWare ERF archives
* unherf: Extract BioWare HERF archives
* unrim: Extract BioWare RIM archives
* unnds: Extract Nintendo DS roms
* unnsbtx: Extract Nintendo NSBTX textures into TGA images
* unkeybif: Extract BioWare KEY/BIF archives
* unobb: Extract Aspyr's OBB virtual filesystem
* untws: Extract CDProjectRed's TheWitcherSave archives
* erf: Create BioWare ERF archives
* rim: Create BioWare RIM archives
* tws: Create CDProjectRed TheWitcherSave archives
* keybif: Create BioWare KEY/BIF archives
* desmall: Decompress "small" (Nintendo DS LZSS, types 0x00 and 0x10) files
* xoreostex2tga: Convert BioWare's texture formats into TGA
* nbfs2tga: Convert Nintendo's raw NBFS images into TGA
* ncgr2tga: Convert Nintendo's NCGR images into TGA
* cbgt2tga: Convert CBGT images into TGA
* cdpth2tga: Convert CDPTH depth images into TGA
* ncsdis: Disassemble NWScript bytecode
* ncsdecomp: Decompile NWScript bytecode


Status [![Build Status](https://travis-ci.org/xoreos/xoreos-tools.svg?branch=master)](https://travis-ci.org/xoreos/xoreos-tools) [![Build status (AppVeyor)](https://ci.appveyor.com/api/projects/status/yaw9e79y8ffpqijo?svg=true)](https://ci.appveyor.com/project/DrMcCoy/xoreos-tools/branch/master) [![Coverity Status](https://scan.coverity.com/projects/3296/badge.svg)](https://scan.coverity.com/projects/3296)
------


Getting xoreos-tools
--------------------

You can get xoreos-tools in multiple ways:

You can download an archive with a binary of the latest release from our
[downloads page](https://xoreos.org/downloads/index.html). This includes
binaries for Microsoft Windows, Mac OS X and GNU/Linux, as well as packages
for various GNU/Linux distributions. All of them are available for both 32-
and 64-bit x86 architectures.

Or, if you're running Arch Linux, you can install xoreos-tools directly from the
[AUR](https://aur.archlinux.org/packages/xoreos-tools/).

Or, if you're running Gentoo Linux, you can install xoreos-tools directly from our
[overlay](https://github.com/xoreos/gentoo-overlay).

Lastly, you can compile xoreos-tools yourself; either from a release source package,
found on our [downloads page](https://xoreos.org/downloads/index.html), or a
fresh [repository](https://github.com/xoreos/xoreos-tools) checkout. For details
on how to compile xoreos on various operating system, please read the
[Compiling xoreos-tools](https://wiki.xoreos.org/index.php?title=Compiling_xoreos-tools)
page on our wiki.


TLK language IDs and encodings
------------------------------

Aurora games use numerical language IDs to identify which language a
TLK file holds. Unfortunately, those language IDs vary between games,
and so does the encoding used for strings in those TLK files. There
is no way to autodetect this information, so it has to be provided
to tools handling those files, in one way or another.

For the tools tlk2xml and xml2tlk, you can specify this encoding
either directly, or by giving the game the TLK is from. Please note
that this does not work for Sonic Chronicles: The Dark Brotherhood,
because its TLK files do not provide a language ID.

Neverwinter Nights, Neverwinter Nights 2, Knights of the Old Republic,
Knights of the Old Republic 2:

| Language ID | Language              | Encoding |
|------------:|:----------------------|:---------|
|           0 | English               | CP-1252  |
|           1 | French                | CP-1252  |
|           2 | German                | CP-1252  |
|           3 | Italian               | CP-1252  |
|           4 | Spanish               | CP-1252  |
|           5 | Polish                | CP-1250  |
|         128 | Korean                | CP-949   |
|         129 | Chinese (Traditional) | CP-950   |
|         130 | Chinese (Simplified)  | CP-936   |
|         131 | Japanese              | CP-932   |

Jade Empire:

| Language ID | Language              | Encoding |
|------------:|:----------------------|:---------|
|           0 | English               | UTF-8    |
|           1 | French                | UTF-8    |
|           2 | German                | UTF-8    |
|           3 | Italian               | UTF-8    |
|           4 | Spanish               | UTF-8    |
|           5 | Polish                | UTF-8    |
|           6 | Czech                 | UTF-8    |
|           7 | Hungarian             | UTF-8    |
|         130 | Chinese (Simplified)  | UTF-8    |
|         132 | Russian               | UTF-8    |

The Witcher:

| Language ID | Language              | Encoding |
|------------:|:----------------------|:---------|
|           0 | "Debug"               | UTF-8    |
|           3 | English               | UTF-8    |
|           5 | Polish                | UTF-8    |
|          10 | German                | UTF-8    |
|          11 | French                | UTF-8    |
|          12 | Spanish               | UTF-8    |
|          13 | Italian               | UTF-8    |
|          14 | Russian               | UTF-8    |
|          15 | Czech                 | UTF-8    |
|          16 | Hungarian             | UTF-8    |
|          20 | Korean                | UTF-8    |
|          21 | Chinese (Traditional) | UTF-8    |
|          22 | Chinese (Simplified)  | UTF-8    |

Sonic Chronicles: The Dark Brotherhood:

| Language ID | Language              | Encoding |
|------------:|:----------------------|:---------|
|           - | English               | CP-1252  |
|           - | French                | CP-1252  |
|           - | German                | CP-1252  |
|           - | Italian               | CP-1252  |
|           - | Spanish               | CP-1252  |
|           - | Japanese              | UTF-8    |

Dragon Age: Origins, Dragon Age II:

| Language ID | Language              | Encoding |
|------------:|:----------------------|:---------|
|           0 | English               | UTF-16LE |
|           1 | French                | UTF-16LE |
|           2 | Russian               | UTF-16LE |
|           3 | Italian               | UTF-16LE |
|           4 | German                | UTF-16LE |
|           5 | Polish                | UTF-16LE |
|           6 | Spanish               | UTF-16LE |
|           7 | Czech                 | UTF-16LE |
|           8 | Hungarian             | UTF-16LE |
|           9 | Korean                | UTF-16LE |
|          10 | Japanese              | UTF-16LE |


Links
-----

- [xoreos website](https://xoreos.org/)
- [xoreos wiki](https://wiki.xoreos.org/)
- [Main source repository](https://github.com/xoreos/xoreos-tools)
- [All xoreos repositories](https://github.com/xoreos/)


Contact
-------

To contact us, please either write to [mailing list](https://xoreos.org/mailman/listinfo/xoreos-devel),
or join our IRC channel #xoreos on Freenode.
