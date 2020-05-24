# If you want to build the current git checkout, run "build-from-git.sh".
# If you want to build the last stable release of xoreos instead,
# build from this specfile directly.

# Globals, overridden by build script.
%global snapshot 0

Name:           xoreos-tools
Version:        0.0.6

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

BuildRequires:  gcc-c++, make
BuildRequires:  zlib-devel, xz-devel, libxml2-devel
BuildRequires:  libtool, gettext

# Boost dependencies.
BuildRequires:  boost-devel, boost-system, boost-filesystem
BuildRequires:  boost-locale

#Requires:

%description
A collection of tools to help with the reverse-engineering of BioWare's
Aurora engine games.

xoreos-tools is part of the xoreos project.

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
* Mon Aug 03 2020 Sven Hesse <drmccoy@drmccoy.de> 0.0.6-1
- New upstream release.

* Tue Jul 03 2018 Sven Hesse <drmccoy@drmccoy.de> 0.0.5-1
- New upstream release.

* Mon Feb 15 2016 Ben Rosser <rosser.bjr@gmail.com> 0.0.4-1
- Initial package.
