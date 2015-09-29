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
