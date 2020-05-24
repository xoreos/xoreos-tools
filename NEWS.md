Changes in xoreos-tools version 0.0.6
=====================================

This release of the xoreos-tools package features nine new tools:
unobb, untws, rim, keybif, tws, fixnwn2xml, xml2gff, fev2xml and ncsdecomp.

The first two new tools, unobb and untws, are new archive unpackers. unobb
extracts "obb" files found in Aspyr's Android and iOS ports of the BioWare
games, which can be either plain ZIP files or, more interesting, a virtual
filesystem. untws extracts save files from The Witcher.

The next three tools, rim, keybif and tws are archive packers. rim is the
counterpart to unrim, creating RIM archives. keybif is the counterpart to
unkeybif, creating KEY/BIF archives (and lzma-compressed KEY/BZF archives found
in Aspyr's mobile ports). However, V1.1 files for The Witcher are not yet
supported. And lastly, tws, is the counterpart to the new untws tool, creating
save files for The Witcher.

Next up, fixnwn2xml takes the non-standard XML files in NWN2 and turns them into
valid, standards-compliant XML files.

xml2gff is the counterpart to gff2xml, taking an XML file and turning it back
into a GFF. Only GFF3 (GFF V3.2/V3.3) are supported for now, so neither Sonic
nor the Dragon Age games (which use GFF V4.0/V4.1) are supported at the moment.

Another work-in-progress tool is fev2xml, which reads the FMOD event file format
FEV and creates a human-readable XML file. Only the FEV1 version is supported
and only a fraction of its features at that.

Likewise, ncsdecomp is the start of an NWScript bytecode compiler, built on the
foundations of our NWScript bytecode disassembler. It's highly experimental and
we give no guarantees that it works correctly at all.

In addition to these new tools, there are some new minor features and bugfixes:
- unerf can now extract ERF V2.1
- erf can now create ERF V2.0 and V2.2
- xoreostex2tga now supports animated TPCs and swizzled Xbox SBMs
- gff2xml now supports SAC files and big-endian GFF4s
- tlk2xml now supports big-endian GFF4s
- The character encoding matrix for Jade Empire is now correct

In an attempt to modernize a bit, xoreos-tools now requires a C++11-capable
compiler. This should hopefully not be a huge problem.


Changes in xoreos-tools version 0.0.5
=====================================

This release of the xoreos-tool package features three new tools:
ssf2xml, xml2ssf and erf.

The first toool, ssf2xml, takes a sound set file as used in the two Neverwinter
Nights and the two Knights of the Old Republic games and converts it into a
user-readable XML file. That file can then be edited and, with the help of the
second new tool, xml2ssf, converted back into a game-readable SSF file.

The third new tool is a packer for the ERF archive format. It is the counterpart
of the already existing unerf tool, which extracts ERF archives. However, unlike
the unerf tool, the erf tool can only create ERF archives of the version 1.0,
as used by Neverwinter Nights, Knights of the Old Republic I and II, Jade Empire
and The Witcher. Later versions of the format are not yet supported.

Apart from that, this release of course also includes a ton of user-invisible
code quality and documentation fixes.


Changes in xoreos-tools version 0.0.4
=====================================

This release of the xoreos-tools package features two new tools:
fixpremiumgff and ncsdis.

The first tool, fixpremiumgff, can restore the deliberately broken GFF files
found in the BioWare premium modules for Neverwinter Nights. The resulting
GFF files can then be edited as normal.

The second tool, ncsdis, is a disassembler for the stack-based bytecode of
BioWare's NWScript scripting language. It supports the scripts of all games
targeted by xoreos and can disassemble them into a full assembly listing.
It can also produce a control flow graph in the DOT description language,
which can then be plotted into an image by using the dot tools from the
GraphViz suite (<http://graphviz.org/>).

Moreover, this release includes a lot of user-invisible code documentation
and quality fixes.


Changes in xoreos-tools version 0.0.3
=====================================

This release of the xoreos-tools package features a new xml2tlk tool that
can convert XML files created by the tlk2xml tool back into a talk table
TLK file. Please note that, at the moment, only non-GFF'd TLK files can be
written, as used by the two Neverwinter Nights games, the two Knights of
the Old Republic games, Jade Empire and The Witcher. TLK files as used by
Sonic Chronicles: The Dark Brotherhood and the two Dragon Age games can not
be written (they can, however, be read with the tlk2xml tool).

Additionally, the convert2da tool gained the ability to write binary 2DA
files, as used by the two Knights of the Old Republic games; and
xoreostex2tga can now correctly read TPC cube maps.


Changes in xoreos-tools version 0.0.2
=====================================

This is the first official release of xoreos-tools, together with xoreos.

Included are:

- Tools for handling resource archives
  - unkeybif
  - unerf
  - unrim
  - unherf
  - unnds
  - desmall
- Tools converting basic resource formats into human-readable form
  - gff2xml
  - tlk2xml
  - convert2da
- Tools converting graphics formats into TGA
  - xoreostex2tga
  - unnsbtx
  - nbfs2tga
  - ncgr2tga
  - cbgt2tga
  - cdpth2tga
