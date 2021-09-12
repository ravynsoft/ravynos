# norootforbuild

%define libname libqtermwidget0

Name:		qtermwidget
Summary:	Qt4 terminal widget
Version:	0.2.0
Release:	1
License:	GPL
Source:		%{name}-%{version}.tar.bz2
Group:		Utility
URL:		https://github.com/qterminal
Vendor:		petr@yarpen.cz


%if 0%{?fedora_version}
	%define breq qt4-devel
    %define pref %{buildroot}/usr
%endif
%if 0%{?mandriva_version}
	%define breq libqt4-devel
    %define pref %{buildroot}/usr
%endif
%if 0%{?suse_version}
	%define breq libqt4-devel
    %define pref %{_prefix}
%endif


BuildRequires:	gcc-c++, %{breq}, cmake
BuildRoot:	%{_tmppath}/%{name}-%{version}-build
Prefix:		%{_prefix}

%description
QTermWidget is an opensource project based on KDE4 Konsole application. The main goal of this project is to provide unicode-enabled, embeddable QT4 widget for using as a built-in console (or terminal emulation widget).
Of course I`m aware about embedding abilities of original Konsole, but once I had Qt without KDE, and it was a serious problem. I decided not to rely on a chance. I could not find any interesting related project, so I had to write it.
The original Konsole`s code was rewritten entirely with QT4 only; also I have to include in the project some parts of code from kde core library. All code dealing with user interface parts and session management was removed (maybe later I bring it back somehow), and the result is quite useful, I suppose.
This library was compiled and tested on three linux systems, based on 2.4.32, 2.6.20, 2.6.23 kernels, x86 and amd64. Ther is also a sample application provided for quick start.

%package -n %{libname}
Summary:	Qt4 terminal widget - base package
Group:		"Development/Libraries/C and C++"
%description -n %{libname}
QTermWidget is an opensource project based on KDE4 Konsole application.
The main goal of this project is to provide unicode-enabled, embeddable
QT4 widget for using as a built-in console (or terminal emulation widget).

%package devel
Summary:	Qt4 terminal widget - development package
Group:		"Development/Libraries/C and C++"
Requires:	%{libname}
%description devel
Development package for QTermWidget. Contains headers and dev-libs.

%prep
%setup

%build
cmake \
    -DCMAKE_C_FLAGS="%{optflags}" \
    -DCMAKE_CXX_FLAGS="%{optflags}" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_INSTALL_PREFIX=%{pref} \
     %{_builddir}/%{name}-%{version}

%{__make} %{?jobs:-j%jobs}


%install
%makeinstall


%clean
%{__rm} -rf %{buildroot}

%post -n %{libname}
ldconfig

%postun -n %{libname}
ldconfig

%files -n %{libname}
%defattr(-,root,root,-)
%doc AUTHORS LICENSE Changelog INSTALL README
%{_libdir}/lib%{name}.so.%{version}
%{_datadir}/%{name}
%{_datadir}/%{name}/*

%files devel
%defattr(-,root,root,-)
%{_includedir}/*.h
%{_libdir}/*.so
%{_libdir}/*.so.0

%changelog
* Mon Oct 29 2010 Petr Vanek <petr@scribus.info> 0.2
- version bump, cmake builds

* Sat Jul 26 2008 TI_Eugene <ti.eugene@gmail.com> 0.100
- Initial build
