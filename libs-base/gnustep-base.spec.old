# This package is not relocatable
%define ver	0.6.5
%define date	20000217
%define prefix 	/usr
%define gsr 	%{prefix}/GNUstep
%define libcombo gnu-gnu-gnu-xgps
Name: 		gnustep-base
Version: 	%{ver}
Release: 	1
Source: 	ftp://ftp.gnustep.org/pub/gnustep/core/gstep-base-%{ver}.tar.gz
Copyright: 	GPL
Group: 		Development/Tools
Summary: 	GNUstep Base library package
Packager:	Christopher Seawood <cls@seawood.org>
Distribution:	Seawood's Random RPMS (%{_buildsym})
Vendor:		The Seawood Project
URL:		http://www.gnustep.org/
BuildRoot: 	/var/tmp/build-%{name}
Conflicts:	gnustep-core
Requires:	gnustep-make

%description
The GNUstep Base Library is a library of general-purpose, non-graphical
Objective C objects.  For example, it includes classes for strings,
object collections, byte streams, typed coders, invocations,
notifications, notification dispatchers, moments in time, network ports,
remote object messaging support (distributed objects), event loops, and
random number generators.
Library combo is %{libcombo}.
%{_buildblurb}

%package devel
Summary: GNUstep Base headers and development libs.
Group: Development/Libraries
Requires: %{name} = %{ver}, gnustep-make-devel
Conflicts: gnustep-core

%description devel
Header files required to build applications against the GNUstep Base library.
Library combo is %{libcombo}.
%{_buildblurb}

%prep
%setup -q -n gstep-%{ver}/base
%patch -p2 -b .unicode

%build
if [ -z "$GNUSTEP_SYSTEM_ROOT" ]; then
   . %{gsr}/Makefiles/GNUstep.sh 
fi
CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%{gsr} --with-library-combo=%{libcombo}
make

%install
rm -rf $RPM_BUILD_ROOT
if [ -z "$GNUSTEP_SYSTEM_ROOT" ]; then
   . %{gsr}/Makefiles/GNUstep.sh 
fi
make install GNUSTEP_INSTALLATION_DIR=${RPM_BUILD_ROOT}%{gsr}

%ifos Linux
cat > mygnustep.init.in << EOF
#!/bin/sh
#
# gnustep daemons
#
# chkconfig: 2345 35 65
# description: Starts gnustep daemons
#
# Source function library.
. /etc/rc.d/init.d/functions

case "\$1" in
  start)
        echo -n "Starting gnustep services: "
        daemon %{gsr}/Tools/GSARCH/GSOS/gdomap
        echo
        touch /var/lock/subsys/gnustep
        ;;

  stop)
        echo -n "Stopping gnustep services: "
        killproc gdomap
        echo
        rm -f /var/lock/subsys/gnustep
        ;;

   status)
        status gdomap   
        ;;

   restart|reload)
        \$0 stop
        \$0 start
        ;;

    *)
        echo "Usage: gnustep {start|stop|status|restart|reload}"
        exit 1
esac 
EOF

sed -e "s|GSARCH|${GNUSTEP_HOST_CPU}|g" -e "s|GSOS|${GNUSTEP_HOST_OS}|g" < mygnustep.init.in > mygnustep.init
mkdir -p ${RPM_BUILD_ROOT}/etc/rc.d/init.d
mv mygnustep.init ${RPM_BUILD_ROOT}/etc/rc.d/init.d/gnustep
%endif

cat > filelist.rpm.in << EOF
%defattr (-, bin, bin)
%doc ANNOUNCE AUTHORS COPYING* ChangeLog* INSTALL* NEWS README Version
%config %{gsr}/Libraries/Resources/NSTimeZones/localtime
%ifos Linux
%config /etc/rc.d/init.d/gnustep
%endif

%dir %{gsr}/Libraries
%dir %{gsr}/Libraries/Resources
%dir %{gsr}/Libraries/Resources/NSTimeZones
%dir %{gsr}/Libraries/GSARCH
%dir %{gsr}/Libraries/GSARCH/GSOS
%dir %{gsr}/Libraries/GSARCH/GSOS/%{libcombo}
%dir %{gsr}/Tools
%dir %{gsr}/Tools/GSARCH
%dir %{gsr}/Tools/GSARCH/GSOS
%dir %{gsr}/Tools/GSARCH/GSOS/%{libcombo}

%{gsr}/Libraries/Resources/NSCharacterSets
%{gsr}/Libraries/Resources/NSTimeZones/README
%{gsr}/Libraries/Resources/NSTimeZones/abbreviations
%{gsr}/Libraries/Resources/NSTimeZones/regions
%{gsr}/Libraries/Resources/NSTimeZones/zones
%{gsr}/Libraries/Resources/NSTimeZones/*.m
%{gsr}/Libraries/GSARCH/GSOS/%{libcombo}/lib*.so.*

%{gsr}/Tools/dread
%{gsr}/Tools/dwrite
%{gsr}/Tools/dremove
%{gsr}/Tools/gdnc
%{gsr}/Tools/plparse
%{gsr}/Tools/sfparse
%{gsr}/Tools/pldes
%{gsr}/Tools/plser
%{gsr}/Tools/GSARCH/GSOS/%{libcombo}/*

%attr(4755, root, root) %{gsr}/Tools/GSARCH/GSOS/gdomap

EOF

cat > filelist-devel.rpm.in  << EOF
%defattr(-, bin, bin)
%dir %{gsr}/Headers
%dir %{gsr}/Headers/gnustep

%{gsr}/Headers/gnustep/Foundation
%{gsr}/Headers/gnustep/base
%{gsr}/Headers/gnustep/unicode
%{gsr}/Headers/GSARCH
%{gsr}/Libraries/GSARCH/GSOS/%{libcombo}/lib*.so

EOF

sed -e "s|GSARCH|${GNUSTEP_HOST_CPU}|" -e "s|GSOS|${GNUSTEP_HOST_OS}|" < filelist.rpm.in > filelist.rpm
sed -e "s|GSARCH|${GNUSTEP_HOST_CPU}|" -e "s|GSOS|${GNUSTEP_HOST_OS}|" < filelist-devel.rpm.in > filelist-devel.rpm

echo 'GMT' > $RPM_BUILD_ROOT/%{gsr}/Libraries/Resources/NSTimeZones/localtime

%post
if [ -z "$GNUSTEP_SYSTEM_ROOT" ]; then
   . %{gsr}/Makefiles/GNUstep.sh 
fi
grep -q '^gdomap' /etc/services || (echo "gdomap 538/tcp # GNUstep distrib objects" >> /etc/services && echo "gdomap 538/udp # GNUstep distrib objects" >> /etc/services)
%ifos Linux
grep -q '%{gsr}/Libraries/$GNUSTEP_HOST_CPU/$GNUSTEP_HOST_OS/gnu-gnu-gnu-xgps' /etc/ld.so.conf || echo "%{gsr}/Libraries/$GNUSTEP_HOST_CPU/$GNUSTEP_HOST_OS/%{libcombo}" >> /etc/ld.so.conf
/sbin/ldconfig
/sbin/chkconfig --add gnustep
%endif

%preun
if [ -z "$GNUSTEP_SYSTEM_ROOT" ]; then
   . %{gsr}/Makefiles/GNUstep.sh 
fi
if [ $1 = 0 ]; then
    /sbin/chkconfig --del gnustep
    mv -f /etc/services /etc/services.orig
    grep -v "^gdomap 538" /etc/services.orig > /etc/services
    rm -f /etc/services.orig
fi


%postun
if [ -z "$GNUSTEP_SYSTEM_ROOT" ]; then
   . %{gsr}/Makefiles/GNUstep.sh 
fi
if [ $1 = 0 ]; then
%ifos Linux
    mv -f /etc/ld.so.conf /etc/ld.so.conf.orig
    grep -v "^%{gsr}/Libraries/$GNUSTEP_HOST_CPU/$GNUSTEP_HOST_OS/%{libcombo}$" /etc/ld.so.conf.orig > /etc/ld.so.conf
    rm -f /etc/ld.so.conf.orig
    /sbin/ldconfig
%endif
fi

%clean
rm -rf $RPM_BUILD_ROOT

%files -f filelist.rpm
%files -f filelist-devel.rpm devel

%changelog
* Sat Sep 18 1999 Christopher Seawood <cls@seawood.org>
- Version 0.6.0
- Added unicode patch to make sure unicode headers were installed

* Sat Aug 07 1999 Christopher Seawood <cls@seawood.org>
- Updated to cvs dawn_6 branch

* Fri Jun 25 1999 Christopher Seawood <cls@seawood.org>
- Split into separate rpm from gnustep-core
- Build from cvs snapshot
- Added header patch
- Split into main & -devel packages

