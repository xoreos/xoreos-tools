# If you want to build the current git checkout, run "build-from-git.sh".
# If you want to build the last stable release of xoreos instead,
# build from this specfile directly.

# Globals, overridden by build script.
%global snapshot 0

Name:           xoreos-tools
Version:        0.0.5

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

# Boost dependencies.
BuildRequires:  boost-devel, boost-system, boost-filesystem, boost-atomic,
BuildRequires:  boost-regex, boost-locale

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
* erf: Create BioWare ERF archives
* fixpremiumgff: Repair BioWare GFF files in NWN premium module HAKs
* unerf: Extract BioWare ERF archives
* unherf: Extract BioWare HERF archives
* unrim: Extract BioWare RIM archives
* unnds: Extract Nintendo DS roms
* unnsbtx: Extract Nintendo NSBTX textures into TGA images
* unkeybif: Extract BioWare KEY/BIF archives
* unobb: Extract Aspyr's OBB virtual filesystem
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
%configure --with-release=xFedora

# When building in place we want to do a make clean.
make clean

make %{?_smp_mflags}

%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot}
# We'll get the documentation manually.
rm %{buildroot}%{_pkgdocdir}/*

%files

# Binaries.
%{_bindir}/*

# Man pages.
%{_mandir}/man1/*

%doc *.md AUTHORS ChangeLog TODO
%license COPYING

%changelog
* Tue Jul 03 2018 Sven Hesse <drmccoy@drmccoy.de> 0.0.5-1
- New upstream release.

* Mon Feb 15 2016 Ben Rosser <rosser.bjr@gmail.com> 0.0.4-1
- Initial package.
