# -*- coding: utf-8 -*-
%define nasm_version 2.16.01
Summary: The Netwide Assembler, a portable x86 assembler with Intel-like syntax
Name: nasm
Version: 2.16.01
Release: 0%{?dist}
License: BSD
Source: http://www.nasm.us/pub/nasm/releasebuilds/%{nasm_version}/nasm-%{nasm_version}.tar.xz
URL: http://www.nasm.us/
BuildRoot: /tmp/rpm-build-nasm
Prefix: %{_prefix}
BuildRequires: perl(bytes)
BuildRequires: perl(Fcntl)
BuildRequires: perl(File::Basename)
BuildRequires: perl(File::Compare)
BuildRequires: perl(File::Copy)
BuildRequires: perl(File::Find)
BuildRequires: perl(File::Path)
BuildRequires: perl(File::Spec)
BuildRequires: perl(File::Temp)
BuildRequires: perl(Font::TTF::Cmap)
BuildRequires: perl(Font::TTF::Font)
BuildRequires: perl(Font::TTF::Head)
BuildRequires: perl(Font::TTF::Hmtx)
BuildRequires: perl(Font::TTF::Maxp)
BuildRequires: perl(Font::TTF::Post)
BuildRequires: perl(Font::TTF::PSNames)
BuildRequires: perl(Getopt::Long)
BuildRequires: perl(integer)
BuildRequires: perl(Pod::Usage)
BuildRequires: perl(sort)
BuildRequires: perl(Sort::Versions)
BuildRequires: autoconf
BuildRequires: automake
BuildRequires: asciidoc
BuildRequires: xmlto
BuildRequires: xz
BuildRequires: perl
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

%package doc
Summary: Detailed manual for the Netwide Assembler
BuildArch: noarch
BuildRequires: ghostscript
BuildRequires: fontconfig
BuildRequires: adobe-source-sans-pro-fonts
BuildRequires: adobe-source-code-pro-fonts

%description
NASM is the Netwide Assembler, a free portable assembler for the Intel
80x86 microprocessor series, using primarily the traditional Intel
instruction mnemonics and syntax.

%description doc
Extensive documentation for the Netwide Assembler (NASM) in HTML and
PDF formats.

%prep
%setup -q -n nasm-%{nasm_version}

%build
sh autogen.sh
%configure --enable-sections
make %{?_smp_mflags} everything

%install
rm -rf "%{buildroot}"
mkdir -p "%{buildroot}"/%{_bindir}
mkdir -p "%{buildroot}"/%{_mandir}/man1
make DESTDIR="%{buildroot}" install

%files
%doc AUTHORS
%{_bindir}/nasm
%{_bindir}/ndisasm
%{_mandir}/man1/nasm.1*
%{_mandir}/man1/ndisasm.1*

%files doc
%doc doc/html doc/nasmdoc.pdf.xz

# This is the upstream spec file; the change log is in git
%changelog
