# If you want to build the current git checkout, run "build-from-git.sh".
# If you want to build the last stable release of xoreos instead,
# build from this specfile directly.

# Globals, overridden by build script.
%global snapshot 0

Name:           xoreos-tools
Version:        0.0.4

# This is a bit ugly but it works.
%if "%{snapshot}" == "0"
Release:        1%{?dist}
%else
Release:        1.%{snapshot}%{?dist}
%endif

Summary:        Tools to help the development of xoreos

License:        GPLv3
URL:            https://xoreos.org/
Source0:        https://github.com/xoreos/xoreos-tools/releases/download/%{version}/%{name}-%{version}.tar.gz

BuildRequires:  zlib-devel, libxml2-devel
BuildRequires:  libtool, gettext

#Requires:

%description
A collection of tools to help with the reverse-engineering of BioWare's
Aurora engine games. xoreos-tools is part of the xoreos project; please
see the xoreos website and its GitHub repositories for details,
especially the main README.md.

Currently, the following tools are included:

* gff2xml: Convert BioWare GFF to XML
* tlk2xml: Convert BioWare TLK to XML
* ssf2xml: Convert BioWare SSF to XML
* xml2tlk: Convert XML back to BioWare TLK
* xml2ssf: Convert XML back to BioWare SSF
* convert2da: Convert BioWare 2DA/GDA to 2DA/CSV
* fixpremiumgff: Repair BioWare GFF files in NWN premium module HAKs
* unerf: Extract BioWare ERF archives
* unherf: Extract BioWare HERF archives
* unrim: Extract BioWare RIM archives
* unnds: Extract Nintendo DS roms
* unnsbtx: Extract Nintendo NSBTX textures into TGA images
* unkeybif: Extract BioWare KEY/BIF archives
* desmall: Decompress "small" (Nintendo DS LZSS, types 0x00 and 0x10) files
* xoreostex2tga: Convert BioWare's texture formats into TGA
* nbfs2tga: Convert Nintendo's raw NBFS images into TGA
* ncgr2tga: Convert Nintendo's NCGR images into TGA
* cbgt2tga: Convert CBGT images into TGA
* cdpth2tga: Convert CDPTH depth images into TGA
* ncsdis: Disassemble NWScript bytecode

%prep
%setup -q

%build
./autogen.sh
%configure

# When building in place we want to do a make clean.
make clean

make %{?_smp_mflags}

%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot}
# We'll get the documentation manually.
rm %{buildroot}%{_pkgdocdir}/*

%files

# Scripts.
%{_bindir}/cbgt2tga
%{_bindir}/cdpth2tga
%{_bindir}/convert2da
%{_bindir}/desmall
%{_bindir}/fixpremiumgff
%{_bindir}/gff2xml
%{_bindir}/nbfs2tga
%{_bindir}/ncgr2tga
%{_bindir}/ncsdis
%{_bindir}/tlk2xml
%{_bindir}/ssf2xml
%{_bindir}/unerf
%{_bindir}/unherf
%{_bindir}/unkeybif
%{_bindir}/unnds
%{_bindir}/unnsbtx
%{_bindir}/unrim
%{_bindir}/xml2tlk
%{_bindir}/xml2ssf
%{_bindir}/xoreostex2tga

# man pages.
%{_mandir}/man1/cbgt2tga.1*
%{_mandir}/man1/cdpth2tga.1*
%{_mandir}/man1/convert2da.1*
%{_mandir}/man1/desmall.1*
%{_mandir}/man1/fixpremiumgff.1.*
%{_mandir}/man1/gff2xml.1.*
%{_mandir}/man1/nbfs2tga.1.*
%{_mandir}/man1/ncgr2tga.1.*
%{_mandir}/man1/ncsdis.1.*
%{_mandir}/man1/tlk2xml.1.*
%{_mandir}/man1/ssf2xml.1.*
%{_mandir}/man1/unerf.1.*
%{_mandir}/man1/unherf.1.*
%{_mandir}/man1/unkeybif.1.*
%{_mandir}/man1/unnds.1.*
%{_mandir}/man1/unnsbtx.1.*
%{_mandir}/man1/unrim.1.*
%{_mandir}/man1/xml2tlk.1.*
%{_mandir}/man1/xml2ssf.1.*
%{_mandir}/man1/xoreostex2tga.1.*

%doc *.md AUTHORS ChangeLog TODO
%license COPYING*

%changelog
* Mon Feb 15 2016 Ben Rosser <rosser.bjr@gmail.com> 0.0.4-1
- Initial package.
