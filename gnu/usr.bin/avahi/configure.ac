# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# This file is part of avahi.
#
# avahi is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# avahi is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
# License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with avahi; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.

AC_PREREQ(2.63)
AC_INIT([avahi],[0.8],[avahi (at) lists (dot) freedesktop (dot) org])
AC_CONFIG_SRCDIR([avahi-core/server.c])
AC_CONFIG_MACRO_DIR([common])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign 1.11 -Wall -Wno-portability silent-rules tar-pax])

AC_SUBST(PACKAGE_URL, [http://avahi.org/])

AC_SUBST(LIBAVAHI_COMMON_VERSION_INFO, [8:4:5])
AC_SUBST(LIBAVAHI_CORE_VERSION_INFO, [8:0:1])
AC_SUBST(LIBAVAHI_CLIENT_VERSION_INFO, [5:9:2])
AC_SUBST(LIBAVAHI_GLIB_VERSION_INFO, [1:2:0])
AC_SUBST(LIBAVAHI_LIBEVENT_VERSION_INFO, [1:0:0])
AC_SUBST(LIBAVAHI_GOBJECT_VERSION_INFO, [0:5:0])
AC_SUBST(LIBAVAHI_QT3_VERSION_INFO, [1:2:0])
AC_SUBST(LIBAVAHI_QT4_VERSION_INFO, [1:2:0])
AC_SUBST(LIBAVAHI_QT5_VERSION_INFO, [1:2:0])
AC_SUBST(LIBAVAHI_UI_VERSION_INFO, [1:4:1])

# Do not touch these, since they we took this version-info from upstream HOWL/Bonjour
AC_SUBST(LIBAVAHI_COMPAT_LIBDNS_SD_VERSION_INFO, [1:0:0])
AC_SUBST(LIBAVAHI_COMPAT_HOWL_VERSION_INFO, [0:0:0])
AC_SUBST(HOWL_COMPAT_VERSION, [0.9.8])

AC_CANONICAL_HOST

AM_SILENT_RULES([yes])

AC_CHECK_PROG([STOW], [stow], [yes], [no])

AS_IF([test "x$STOW" = "xyes" && test -d /usr/local/stow], [
        AC_MSG_NOTICE([*** Found /usr/local/stow: default install prefix set to /usr/local/stow/${PACKAGE_NAME}-${PACKAGE_VERSION} ***])
        ac_default_prefix="/usr/local/stow/${PACKAGE_NAME}-${PACKAGE_VERSION}"
])

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_CXX
AM_PROG_CC_C_O
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CPP
AC_PROG_MKDIR_P
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_GCC_TRADITIONAL

# -fstack-protector
AC_ARG_ENABLE([stack-protector],
    [AS_HELP_STRING([--disable-stack-protector],
        [Disable GCC's/libc's stack-smashing protection])],
    [case "${enableval}" in
         yes) enable_ssp=yes ;;
          no) enable_ssp=no ;;
           *) AC_MSG_ERROR([invalid value ${enableval} for --disable-stack-protector]) ;;
     esac],
    [enable_ssp=yes])

if test x"$enable_ssp" = x"yes" && test x"$GCC" != x"yes"; then
    AC_MSG_NOTICE([Disabling stack-smashing protection because compiler is not GCC])
    enable_ssp=no
fi

if test x"$enable_ssp" = x"yes"; then
    # Check for broken ssp in libc: http://www.avahi.org/ticket/105
    # libc's brokenness will get in the way regardless of whether -lssp is
    # provided, but provide it anyway (otherwise non-libc ssp would wrongly
    # break here)

    # Get -lssp if it exists
    GCC_STACK_PROTECT_LIB

    AC_MSG_CHECKING([whether stack-smashing protection is available])
    ssp_old_cflags="$CFLAGS"
    ssp_old_ldflags="$LDFLAGS"
    CFLAGS="$CFLAGS -Werror -fstack-protector-all -fPIC"
    LDFLAGS="$LDFLAGS -Wl,-z,defs"
    cat confdefs.h > conftest.c
    cat >>conftest.c <<_ACEOF
void test_broken_ssp(c)
    const char *c;
{
    char arr[[123]], *p; /* beware of possible double-braces if copying this */
    for (p = arr; *c; ++p) {
        *p = *c;
        ++c;
    }
}
_ACEOF
    rm -f conftest.o

    if $CC -c $CFLAGS $CPPFLAGS -o conftest.o conftest.c >/dev/null 2>&1; then
        AC_MSG_RESULT([yes])
        AC_MSG_CHECKING([whether stack-smashing protection is buggy])
        if $CC -o conftest.so $LDFLAGS -shared conftest.o $LIBS >/dev/null 2>&1; then
            AC_MSG_RESULT([no])
        else
            AC_MSG_RESULT([yes])
            enable_ssp=no
        fi
    else
        AC_MSG_RESULT([no])
    fi

    rm -f conftest.c conftest.o conftest.so

    CFLAGS="$ssp_old_cflags"
    LDFLAGS="$ssp_old_ldflags"
fi

if test x"$enable_ssp" = x"yes"; then
    # Do this the long way so we don't call GCC_STACK_PROTECT_LIB twice
    GCC_STACK_PROTECT_CC

    AC_LANG_PUSH([C++])
    GCC_STACK_PROTECT_CXX
    AC_LANG_POP([C++])
    # XXX: Update the enable_ssp value now for output later?
fi

# libtool stuff
AC_PROG_LIBTOOL

AC_CACHE_CHECK([whether the C++ compiler works], [avahi_cv_sys_cxx_works], [
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([int main() { return 0; }], [avahi_cv_sys_cxx_works=yes],
        [avahi_cv_sys_cxx_works=no])
    AC_LANG_POP([C++])
    ])
[ if [ "x$avahi_cv_sys_cxx_works" = "xno" ]; then ]
    AC_MSG_FAILURE([The C++ compiler does not work])
[ fi ]

ACX_PTHREAD(,AC_MSG_ERROR([Missing POSIX Threads support]))

#
# Check for netlink.h
#
AC_CHECK_HEADER(linux/netlink.h,
HAVE_NETLINK=yes
AC_DEFINE([HAVE_NETLINK],[],[Support for Linux netlink])
, [], [
#include <sys/socket.h>
#include <asm/types.h>
])

AM_CONDITIONAL(HAVE_NETLINK, [ test x"$HAVE_NETLINK" = xyes ])

#
# Check for net/route.h
#
AC_CHECK_HEADER(net/route.h,
HAVE_PF_ROUTE=yes
AC_DEFINE([HAVE_PF_ROUTE],[],[Support for PF_ROUTE])
, [], [
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if_dl.h>
])

AM_CONDITIONAL(HAVE_PF_ROUTE, [ test x"$HAVE_PF_ROUTE" = xyes ])

#
# Check for sys/filio.h; needed for FIONREAD on Solaris
#
AC_CHECK_HEADER(sys/filio.h,
HAVE_SYS_FILIO_H=yes
AC_DEFINE([HAVE_SYS_FILIO_H],[],[Support for sys/filio.h])
, [], [
])

AM_CONDITIONAL(HAVE_SYS_FILIO_H, [ test x"$HAVE_SYS_FILIO_H" = xyes ])

#
# Check for sys/sysctl.h; not present on Solaris
#
AC_CHECK_HEADER(sys/sysctl.h,
HAVE_SYS_SYSCTL=yes
AC_DEFINE([HAVE_SYS_SYSCTL_H],[],[Support for sys/sysctl.h])
, [], [
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
])

AM_CONDITIONAL(HAVE_SYS_SYSCTL_H, [ test x"$HAVE_SYS_SYSCTL_H" = xyes ])

#
# Check for lifconf struct; only present on Solaris
#
AC_MSG_CHECKING(for struct lifconf)
AC_CACHE_VAL(avahi_cv_has_struct_lifconf,
[AC_TRY_COMPILE(
[#include <sys/socket.h>
#include <net/if.h>
],[sizeof (struct lifconf);],
avahi_cv_has_struct_lifconf=yes,avahi_cv_has_struct_lifconf=no)])
AC_MSG_RESULT($avahi_cv_has_struct_lifconf)
if test $avahi_cv_has_struct_lifconf = yes; then
    AC_DEFINE(HAVE_STRUCT_LIFCONF,1,[Define if there is a struct lifconf.])
fi

#
# Check for struct ip_mreqn
#
AC_MSG_CHECKING(for struct ip_mreqn)
AC_TRY_COMPILE([#include <netinet/in.h>], [
	struct ip_mreqn mreq;
	mreq.imr_address.s_addr = 0;
], [
	# Yes, we have it...
	AC_MSG_RESULT(yes)
	AC_DEFINE([HAVE_STRUCT_IP_MREQN],[],[Support for struct ip_mreqn])
], [
	# We'll just have to try and use struct ip_mreq
	AC_MSG_RESULT(no)
	AC_MSG_CHECKING(for struct ip_mreq)
	AC_TRY_COMPILE([#include <netinet/in.h>], [
		struct ip_mreq mreq;
		mreq.imr_interface.s_addr = 0;
	], [
		# Yes, we have it...
		AC_MSG_RESULT(yes)
		AC_DEFINE([HAVE_STRUCT_IP_MREQ],[],[Support for struct ip_mreq])
	], [
		# No multicast support
			AC_MSG_RESULT(no)
	])
])

#
# Detecting the linux distribution for specific things like init scripts.
#
AC_ARG_WITH(distro, AS_HELP_STRING([--with-distro=DISTRO],[Specify the distribution to target: One of lfs, debian, gentoo, archlinux, fedora, mandriva, darwin, netbsd, freebsd, slackware or none]))
if test "z$with_distro" = "z"; then
    if test "$cross_compiling" = yes; then
        AC_MSG_WARN([Target distribution cannot be reliably detected when cross-compiling. You should specify it with --with-distro (see $0 --help for recognized distros)])
    else
        AC_CHECK_FILE(/etc/lfs-release,with_distro="lfs")
        AC_CHECK_FILE(/etc/SuSE-release,with_distro="suse")
        AC_CHECK_FILE(/etc/gentoo-release,with_distro="gentoo")
        AC_CHECK_FILE(/etc/arch-release,with_distro="archlinux")
        AC_CHECK_FILE(/etc/debian_version,with_distro="debian")
        AC_CHECK_FILE(/etc/redhat-release,with_distro="fedora")
        AC_CHECK_FILE(/etc/mandriva-release,with_distro="mandriva")
        AC_CHECK_FILE(/etc/slackware-version,with_distro="slackware")
    fi
    if test "z$with_distro" = "z"; then
        with_distro=`uname -s`
    fi
fi
with_distro=`echo ${with_distro} | tr '[[:upper:]]' '[[:lower:]]' `

case $with_distro in
    lfs|debian|gentoo|archlinux|suse|fedora|mandriva|darwin|freebsd|slackware|none)
     ;;
    netbsd)
     AC_MSG_WARN([Your distribution (${with_distro}) is supported but no init script exist yet! (patches welcome)])
     ;;
    linux)
     AC_MSG_ERROR([Linux distribution autodetection failed, you must specify the distribution to target using --with-distro=DISTRO, set DISTRO to none if your distribution is not supported.])
     ;;
    *)
     AC_MSG_ERROR([Your distribution (${with_distro}) is not yet supported, init scripts and D-Bus configuration will not be installed! (patches welcome), you can specify --with-distro=none to skip this check])
     ;;
esac

AM_CONDITIONAL(TARGET_LFS, test x"$with_distro" = xlfs)
AM_CONDITIONAL(TARGET_SUSE, test x"$with_distro" = xsuse)
AM_CONDITIONAL(TARGET_GENTOO, test x"$with_distro" = xgentoo)
AM_CONDITIONAL(TARGET_DEBIAN, test x"$with_distro" = xdebian)
AM_CONDITIONAL(TARGET_ARCHLINUX, test x"$with_distro" = xarchlinux)
AM_CONDITIONAL(TARGET_FEDORA, test x"$with_distro" = xfedora)
AM_CONDITIONAL(TARGET_MANDRIVA, test x"$with_distro" = xmandriva)
AM_CONDITIONAL(TARGET_DARWIN, test x"$with_distro" = xdarwin)
AM_CONDITIONAL(TARGET_NETBSD, test x"$with_distro" = xnetbsd)
AM_CONDITIONAL(TARGET_FREEBSD, test x"$with_distro" = xfreebsd)
AM_CONDITIONAL(TARGET_SLACKWARE, test x"$with_distro" = xslackware)

test_gcc_flag() {
    AC_LANG_CONFTEST([int main() {}])
    $CC -c conftest.c $CFLAGS $@ > /dev/null 2> /dev/null
    ret=$?
    rm -f conftest.o
    return $ret
}

# If using GCC specify some additional parameters
if test "x$GCC" = "xyes" ; then

    DESIRED_FLAGS="-Wall -W -Wextra -pedantic -pipe -Wformat -Wold-style-definition -Wdeclaration-after-statement -Wfloat-equal -Wmissing-declarations -Wmissing-prototypes -Wstrict-prototypes -Wredundant-decls -Wmissing-noreturn -Wshadow -Wendif-labels -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings -fdiagnostics-show-option -Wno-cast-qual -fno-strict-aliasing"

    if test "x$HAVE_NETLINK" = "xyes" ; then
        # Test whether rtnetlink.h can be included when compiled with -std=c99
        # some distributions (e.g. archlinux) have broken headers that dont
        # define __u64 with -std=c99
        AC_MSG_CHECKING([checking whether rtnetlink.h can be included with -std=c99])
        OLDCFLAGS="$CFLAGS"
        CFLAGS="-std=c99"
        AC_TRY_COMPILE([#include <linux/rtnetlink.h>], [],
            use_stdc99=yes, use_stdc99=no)

        if test x"$use_stdc99" = xyes; then
            DESIRED_FLAGS="-std=c99 $DESIRED_FLAGS"
            AC_MSG_RESULT([yes])
        else
            AC_MSG_RESULT([no])
        fi

        CFLAGS="$OLDCFLAGS"
    else
        DESIRED_FLAGS="-std=c99 $DESIRED_FLAGS"
    fi

    for flag in $DESIRED_FLAGS ; do
        AC_MSG_CHECKING([whether $CC accepts $flag])
        if test_gcc_flag $flag ; then
           CFLAGS="$CFLAGS $flag"
           AC_MSG_RESULT([yes])
        else
           AC_MSG_RESULT([no])
        fi
    done
fi

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h limits.h netinet/in.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h netdb.h syslog.h])
AC_HEADER_STDBOOL

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_HEADER_SYS_WAIT

 # Solaris stuff
 AC_SEARCH_LIBS([inet_ntop],[nsl])
 AC_SEARCH_LIBS([recv],[socket])
 AC_CHECK_DECL([CMSG_SPACE],,CFLAGS="$CFLAGS -D_XOPEN_SOURCE=500 -D__EXTENSIONS__", [[#include <sys/socket.h>]])

# Checks for library functions.
AC_FUNC_MEMCMP
AC_FUNC_SELECT_ARGTYPES
# avahi_malloc actually returns NULL for avahi_malloc(0), so it does not matter
# whether libc's malloc does too. (Same for realloc.)
#AC_FUNC_MALLOC
#AC_FUNC_REALLOC
AC_CHECK_FUNCS([gethostname memchr memmove memset mkdir select socket strchr strcspn strdup strerror strrchr strspn strstr uname setresuid setreuid setresgid setregid strcasecmp gettimeofday putenv strncasecmp strlcpy gethostbyname seteuid setegid setproctitle getprogname])

AC_FUNC_CHOWN
AC_FUNC_STAT
AC_TYPE_MODE_T
AC_TYPE_PID_T

AC_CHECK_DECLS(environ)

# check if gcc's -fvisibility is supported
CHECK_VISIBILITY_HIDDEN

enable_chroot=yes
AC_CHECK_HEADERS([sys/capability.h],,enable_chroot=no)
AC_CHECK_HEADERS([sys/prctl.h],,enable_chroot=no)
AC_CHECK_FUNCS([chroot],,enable_chroot=no)

AM_CONDITIONAL(ENABLE_CHROOT, test "x$enable_chroot" = "xyes")

if test "x$enable_chroot" = "xyes" ; then
   AC_DEFINE([ENABLE_CHROOT], 1, [Enable chroot() usage])
fi

AC_CHECK_LIB(dl, dlopen, [ AC_CHECK_HEADERS(dlfcn.h, HAVE_DLOPEN=yes, HAVE_DLOPEN=no) ], HAVE_DLOPEN=no)
if test "x$HAVE_DLOPEN" = "xyes" ; then
    AC_DEFINE([HAVE_DLOPEN],1,[Have dlopen()])
fi
AM_CONDITIONAL(HAVE_DLOPEN, test "x$HAVE_DLOPEN" = "xyes")

have_inotify=no
AC_CHECK_HEADERS([sys/inotify.h], [have_inotify=yes])

AM_CONDITIONAL(HAVE_INOTIFY, test "x$have_inotify" = "xyes")

if test "x$have_inotify" = "xyes" ; then
   AC_DEFINE([HAVE_INOTIFY], 1, [Enable Linux inotify() usage])
fi

have_kqueue=yes
AC_CHECK_FUNCS([kqueue],,have_kqueue=no)

AM_CONDITIONAL(HAVE_KQUEUE, test "x$have_kqueue" = "xyes")

if test "x$have_kqueue" = "xyes" ; then
    AC_DEFINE([HAVE_KQUEUE], 1, [Enable BSD kqueue() usage])
fi

GETTEXT_PACKAGE=avahi
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["$GETTEXT_PACKAGE"],[Gettext package])
AM_GNU_GETTEXT_VERSION([0.19.8])
AM_GNU_GETTEXT([external])

avahilocaledir='${prefix}/${DATADIRNAME}/locale'
AC_SUBST(avahilocaledir)

# Check for pkg-config manually first, as if its not installed the
# PKG_PROG_PKG_CONFIG macro won't be defined.
AC_CHECK_PROG(have_pkg_config, pkg-config, yes, no)

if test x"$have_pkg_config" = "xno"; then
    AC_MSG_ERROR(pkg-config is required to install this program)
fi

PKG_PROG_PKG_CONFIG

#
# Check for GLIB 2.0
#
AC_ARG_ENABLE(glib,
        AS_HELP_STRING([--disable-glib],[Disable use of GLib]),
        [case "${enableval}" in
                yes) HAVE_GLIB=yes ;;
                no)  HAVE_GLIB=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-glib) ;;
        esac],
        [HAVE_GLIB=yes])

if test "x$HAVE_GLIB" = "xyes" ; then
        PKG_CHECK_MODULES(GLIB20, [ glib-2.0 >= 2.4.0 ])
        AC_SUBST(GLIB20_CFLAGS)
        AC_SUBST(GLIB20_LIBS)
fi
AM_CONDITIONAL(HAVE_GLIB, test "x$HAVE_GLIB" = "xyes")

#
# Check for GLIB's gobject 2.0
#
AC_ARG_ENABLE(gobject,
        AS_HELP_STRING([--disable-gobject],[Disable use of GLib GObject]),
        [case "${enableval}" in
                yes) HAVE_GOBJECT=yes ;;
                no)  HAVE_GOBJECT=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-gobject) ;;
        esac],
        [HAVE_GOBJECT=yes])

if test "x$HAVE_GOBJECT" = "xyes" ; then
        PKG_CHECK_MODULES(GOBJECT, [ glib-2.0 >= 2.4.0 gobject-2.0 ])
        AC_SUBST(GOBJECT_CFLAGS)
        AC_SUBST(GOBJECT_LIBS)
fi
AM_CONDITIONAL(HAVE_GOBJECT, test "x$HAVE_GOBJECT" = "xyes")

#
# Introspection support.
#
GOBJECT_INTROSPECTION_CHECK([0.9.5])

#
# Check for libevent 2.0.21
#
AC_ARG_ENABLE(libevent,
	AS_HELP_STRING([--disable-libevent],[Disable use of libevent]),
	[case "${enableval}" in
		yes) HAVE_LIBEVENT=yes ;;
		no)  HAVE_LIBEVENT=no ;;
		*) AC_MSG_ERROR(bad value ${enableval} for --enable-libevent) ;;
	esac],
	[HAVE_LIBEVENT=yes])

if test "x$HAVE_LIBEVENT" = "xyes" ; then
	PKG_CHECK_MODULES(LIBEVENT, [ libevent >= 2.0.21 ])
	AC_SUBST(LIBEVENT_CFLAGS)
	AC_SUBST(LIBEVENT_LIBS)
fi
AM_CONDITIONAL(HAVE_LIBEVENT, test "x$HAVE_LIBEVENT" = "xyes")

#
# Check for Qt 3
#
AC_ARG_ENABLE(qt3,
        AS_HELP_STRING([--enable-qt3],[Enable building of Qt3 mainloop integration]),
        [case "${enableval}" in
                yes) HAVE_QT3=yes ;;
                no)  HAVE_QT3=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-qt3) ;;
        esac],
        [HAVE_QT3=no])

if test "x$HAVE_QT3" = "xyes" ; then
        PKG_CHECK_MODULES( QT3, [ qt-mt >= 3.0.0 ])
        AC_SUBST(QT3_CFLAGS)
        AC_SUBST(QT3_LIBS)
	    QT3_PREFIX="`$PKG_CONFIG --variable=prefix qt-mt`/bin"
        AC_PATH_PROGS(MOC_QT3, [moc-qt3 moc], no, [$QT3_PREFIX])
        if test "$MOC_QT3" = no; then
            AC_MSG_ERROR([Could not find QT3 moc])
        fi
        AC_SUBST(MOC_QT3)
fi
AM_CONDITIONAL(HAVE_QT3, test "x$HAVE_QT3" = "xyes")

#
# Check for Qt 4
#
AC_ARG_ENABLE(qt4,
        AS_HELP_STRING([--disable-qt4],[Disable building of Qt4Core mainloop integration]),
        [case "${enableval}" in
                yes) HAVE_QT4=yes ;;
                no)  HAVE_QT4=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-qt4) ;;
        esac],
        [HAVE_QT4=no])

if test "x$HAVE_QT4" = "xyes" ; then
        PKG_CHECK_MODULES( QT4, [ QtCore >= 4.0.0 ])
        AC_SUBST(QT4_CFLAGS)
        AC_SUBST(QT4_LIBS)
	    QT4_PREFIX="`$PKG_CONFIG --variable=prefix QtCore`/bin"
        AC_PATH_PROGS(MOC_QT4, [moc-qt4 moc], no, [$QT4_PREFIX])
        if test "$MOC_QT4" = no; then
            AC_MSG_ERROR([Could not find QT4 moc])
        fi
        AC_SUBST(MOC_QT4)
fi
AM_CONDITIONAL(HAVE_QT4, test "x$HAVE_QT4" = "xyes")

#
# Check for Qt 5
#
AC_ARG_ENABLE(qt5,
        AS_HELP_STRING([--disable-qt5],[Disable building of Qt5Core mainloop integration]),
        [case "${enableval}" in
                yes) HAVE_QT5=yes ;;
                no)  HAVE_QT5=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-qt5) ;;
        esac],
        [HAVE_QT5=yes])

if test "x$HAVE_QT5" = "xyes" ; then
        PKG_CHECK_MODULES( QT5, [ Qt5Core >= 5.0.0 ])
        AC_AVAHI_QT_ADD_PIC_IF_NEEDED
        AC_SUBST(QT5_CFLAGS)
        AC_SUBST(QT5_LIBS)
        QT5_PREFIX="`$PKG_CONFIG --variable=host_bins Qt5Core`"
        AC_PATH_PROGS(MOC_QT5, [moc-qt5 moc], no, [$QT5_PREFIX])
        if test "$MOC_QT5" = no; then
            AC_MSG_ERROR([Could not find QT5 moc])
        fi
        AC_SUBST(MOC_QT5)
fi
AM_CONDITIONAL(HAVE_QT5, test "x$HAVE_QT5" = "xyes")

#
# Check for GTK+ 2.0
#
AC_ARG_ENABLE(gtk,
        AS_HELP_STRING([--disable-gtk],[Disable use of GTK+ 2]),
        [case "${enableval}" in
                yes) HAVE_GTK=yes ;;
                no)  HAVE_GTK=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-gtk) ;;
        esac],
        [HAVE_GTK=no])

if test "x$HAVE_GTK" = "xyes" ; then
        # Check for GTK 2.0
        PKG_CHECK_MODULES(GTK20, [ gtk+-2.0 >= 2.14.0 ])
        AC_SUBST(GTK20_CFLAGS)
        AC_SUBST(GTK20_LIBS)
fi
AM_CONDITIONAL(HAVE_GTK, test "x$HAVE_GTK" = "xyes")

#
# Check for GTK+ 3.0
#
AC_ARG_ENABLE(gtk3,
        AS_HELP_STRING([--disable-gtk3],[Disable use of GTK+ 3]),
        [case "${enableval}" in
                yes) HAVE_GTK3=yes ;;
                no)  HAVE_GTK3=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-gtk3) ;;
        esac],
        [HAVE_GTK3=yes])

if test "x$HAVE_GTK3" = "xyes" ; then
        # Check for GTK 3.0
        PKG_CHECK_MODULES(GTK30, [ gtk+-3.0 ])
        AC_SUBST(GTK30_CFLAGS)
        AC_SUBST(GTK30_LIBS)
fi
AM_CONDITIONAL(HAVE_GTK3, test "x$HAVE_GTK3" = "xyes")

AM_CONDITIONAL(HAVE_GTK2OR3, test "x$HAVE_GTK3" = "xyes" -o "x$HAVE_GTK" = "xyes" )

#
# D-Bus
#
AC_ARG_ENABLE(dbus,
        AS_HELP_STRING([--disable-dbus],[Disable use of D-Bus]),
        [case "${enableval}" in
                yes) HAVE_DBUS=yes ;;
                no)  HAVE_DBUS=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-dbus) ;;
        esac],
        [HAVE_DBUS=yes])

AC_ARG_WITH(dbus-sys, AS_HELP_STRING([--with-dbus-sys=<dir>], [Path to D-Bus system.d directory]))
AC_ARG_WITH(dbus-system-socket, AS_HELP_STRING([--with-dbus-system-address=<address>], [Path to the D-Bus system socket, you probably want to put unix:path= at the start. Only needed for very old D-Bus releases]))

DBUS_VERSION="Disabled"
DBUS_SYS_DIR="Disabled"
DBUS_SYSTEM_BUS_DEFAULT_ADDRESS="Disabled"
if test "x$HAVE_DBUS" = "xyes" ; then
    PKG_CHECK_MODULES(DBUS, [ dbus-1 >= 0.34 ])

    AC_DEFINE(HAVE_DBUS, 1, [Whether we have D-Bus or not])

    DBUS_VERSION=`$PKG_CONFIG dbus-1 --modversion`
    DBUS_VERSION_MAJOR=`echo $DBUS_VERSION | awk -F. '{print $1}'`
    DBUS_VERSION_MINOR=`echo $DBUS_VERSION | awk -F. '{print $2}'`
    DBUS_VERSION_MICRO=`echo $DBUS_VERSION | awk -F. '{print $3}'`
    if test "z$DBUS_VERSION_MAJOR" = "z"; then
        DBUS_VERSION_MAJOR="0"
    fi
    if test "z$DBUS_VERSION_MINOR" = "z"; then
        DBUS_VERSION_MINOR="0"
    fi
    if test "z$DBUS_VERSION_MICRO" = "z"; then
        DBUS_VERSION_MICRO="0"
    fi

    if test "z$DBUS_VERSION_MAJOR" = "z0" -a "z$DBUS_VERSION_MINOR" = "z0" -a "z$DBUS_VERSION_MICRO" = "z0"; then
        echo "Error: Couldn't determine the version of your D-Bus package."
        echo "  This is probably an error in this script, please report it"
        echo "  along with the following information:"
        echo "      Base D-Buss version ='$DBUS_VERSION'"
        echo "      DBUS_VERSION_MAJOR='$DBUS_VERSION_MAJOR'"
        echo "      DBUS_VERSION_MINOR='$DBUS_VERSION_MINOR'"
        echo "      DBUS_VERSION_MICRO='$DBUS_VERSION_MICRO'"
        exit 1
    else
        echo "Your D-Bus version is $DBUS_VERSION_MAJOR,$DBUS_VERSION_MINOR,$DBUS_VERSION_MICRO."
        DBUS_CFLAGS="$DBUS_CFLAGS -DDBUS_VERSION_MAJOR=$DBUS_VERSION_MAJOR"
        DBUS_CFLAGS="$DBUS_CFLAGS -DDBUS_VERSION_MINOR=$DBUS_VERSION_MINOR"
        DBUS_CFLAGS="$DBUS_CFLAGS -DDBUS_VERSION_MICRO=$DBUS_VERSION_MICRO"
    fi

    DBUS_CFLAGS="$DBUS_CFLAGS -DDBUS_API_SUBJECT_TO_CHANGE"
    AC_SUBST(DBUS_CFLAGS)
    AC_SUBST(DBUS_LIBS)

    if ! test -z "$with_dbus_sys" ; then
        DBUS_SYS_DIR="$with_dbus_sys"
    else
        DBUS_SYS_DIR="${sysconfdir}/dbus-1/system.d"
    fi
    AC_SUBST(DBUS_SYS_DIR)

    if ! test -z "$with_dbus_system_address" ; then
        DBUS_SYSTEM_BUS_DEFAULT_ADDRESS="$with_dbus_system_address"
    else
        # This is ugly, but D-Bus doesn't export this address for us
        # so we have to guess, pretty much all setups i've seen have
        # it in /var/lib/dbus or /var/run/dbus, and its defaulted to
        # /var/run upstream so we will try guess first then default
        # to /var/run/dbus.

        DBUS_SYSTEM_BUS_DEFAULT_ADDRESS="unix:path=/var/run/dbus/system_bus_socket"
        TRY_SOCKETS="/var/lib/dbus/system_bus_socket /var/run/dbus/system_bus_socket ${localstatedir}/run/dbus/system_bus_socket ${prefix}/var/run/dbus/system_bus_socket"
        for sock in $TRY_SOCKETS; do
            if test -S $sock; then
                DBUS_SYSTEM_BUS_DEFAULT_ADDRESS="unix:path=$sock"
            fi
        done
    fi
    AC_SUBST(DBUS_SYSTEM_BUS_DEFAULT_ADDRESS)

    SAVED_LIBS="$LIBS"
    LIBS="$LIBS $DBUS_LIBS"
    AC_CHECK_FUNCS([dbus_connection_close dbus_bus_get_private])
    LIBS="$SAVED_LIBS"
fi
AM_CONDITIONAL(HAVE_DBUS, test "x$HAVE_DBUS" = "xyes")

#
# Expat
#
AC_ARG_WITH(xml, AS_HELP_STRING([--with-xml=[expat/bsdxml/none]],[XML library to use]))
use_expat=false
use_bsdxml=false

# See what we have
AC_CHECK_LIB(expat, XML_ParserCreate, [ AC_CHECK_HEADERS(expat.h, have_expat=true, have_expat=false) ], have_expat=false)
AC_CHECK_LIB(bsdxml, XML_ParserCreate, [ AC_CHECK_HEADERS(bsdxml.h, have_bsdxml=true, have_bsdxml=false) ], have_bsdxml=false)

if test "x$with_xml" = "xnone"; then
        :
elif test "x$with_xml" = "xexpat"; then
	use_expat=true
	if ! $have_expat ; then
		AC_MSG_ERROR([*** libexpat requested, but not found ***])
	fi
elif test "x$with_xml" = "xbsdxml"; then
	use_bsdxml=true
	if ! $have_bsdxml ; then
		AC_MSG_ERROR([*** libbsdxml requested, but not found ***])
	fi
elif test "x$with_xml" != "x"; then
	AC_MSG_ERROR([*** unknown with-xml option ***])
else
	if $have_expat ; then
		use_expat=true
	elif $have_bsdxml ; then
		use_bsdxml=true
	else
		AC_MSG_ERROR([*** neither libexpat not libbsdxml could be found ***])
	fi
fi

if $use_expat; then
	with_xml=expat
	XML_CFLAGS=-DUSE_EXPAT_H
	XML_LIBS=-lexpat
fi
if $use_bsdxml; then
	with_xml=bsdxml
	XML_CFLAGS=-DUSE_BSDXML_H
	XML_LIBS=-lbsdxml
fi
AC_SUBST(XML_LIBS)
AC_SUBST(XML_CFLAGS)

if $use_expat || $use_bsdxml; then
	HAVE_XML=yes
fi

AM_CONDITIONAL(HAVE_XML, test "x$HAVE_XML" = "xyes")

#
# GDBM
#
#
# Check for dbm
#
AC_ARG_ENABLE(dbm,
        AS_HELP_STRING([--enable-dbm],[Enable use of DBM]),
        [case "${enableval}" in
                yes) HAVE_DBM=yes ;;
                no)  HAVE_DBM=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-dbm) ;;
        esac],
        [HAVE_DBM=no])

AC_ARG_ENABLE(gdbm,
        AS_HELP_STRING([--disable-gdbm],[Disable use of GDBM]),
        [case "${enableval}" in
                yes) HAVE_GDBM=yes ;;
                no)  HAVE_GDBM=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --disable-gdbm) ;;
        esac],
        [HAVE_GDBM=yes])

if test "x$HAVE_GDBM" = "xyes" ; then
    if test "x$HAVE_DBM" = "xyes" ; then
        AC_MSG_ERROR([*** --enable-gdbm and --enable-dbm both specified ***])
    fi
    AC_CHECK_LIB(gdbm, gdbm_open, [ AC_CHECK_HEADERS(gdbm.h, have_gdbm=true, have_gdbm=false) ], have_gdbm=false)

    if ! $have_gdbm ; then
        AC_MSG_ERROR([*** libgdbm not found ***])
    fi
    AC_DEFINE([HAVE_GDBM],[],[Support for GDBM])
else
    if test "x$HAVE_DBM" = "xyes" ; then
        AC_CHECK_HEADERS(ndbm.h, have_dbm=true, have_dbm=false)

        if ! $have_dbm ; then
            AC_MSG_ERROR([*** dbm not found ***])
        fi
        AC_DEFINE([HAVE_DBM],[],[Support for DBM])
    fi
fi
AM_CONDITIONAL(HAVE_GDBM, test "x$HAVE_GDBM" = "xyes")
AM_CONDITIONAL(HAVE_DBM, test "x$HAVE_DBM" = "xyes")

#
# libdaemon
#
AC_ARG_ENABLE(libdaemon,
        AS_HELP_STRING([--disable-libdaemon],[Disable use of libdaemon]),
        [case "${enableval}" in
                yes) HAVE_LIBDAEMON=yes ;;
                no)  HAVE_LIBDAEMON=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-libdaemon) ;;
        esac],
        [HAVE_LIBDAEMON=yes])

if test "x$HAVE_LIBDAEMON" = "xyes" ; then
    PKG_CHECK_MODULES(LIBDAEMON, [ libdaemon >= 0.14 ])
    AC_SUBST(LIBDAEMON_CFLAGS)
    AC_SUBST(LIBDAEMON_LIBS)
fi
AM_CONDITIONAL(HAVE_LIBDAEMON, test "x$HAVE_LIBDAEMON" = "xyes")

#
# Python stuff
#
AC_ARG_ENABLE(python,
    AS_HELP_STRING([--disable-python], [Disable scripts that depends on python]),
    [case "${enableval}" in
          yes) HAVE_PYTHON=yes ;;
          no)  HAVE_PYTHON=no ;;
          *) AC_MSG_ERROR([bad value ${enableval} for --enable-python]) ;;
    esac],[HAVE_PYTHON=yes])

HAVE_PYTHON_DBUS=no
HAVE_PYGOBJECT=no

if test "x$HAVE_PYTHON" = "xyes" ; then
    AM_PATH_PYTHON([2.6])

    AC_ARG_ENABLE(pygobject,
        AS_HELP_STRING([--disable-pygobject],[Disable use of Python GObject]),
        [case "${enableval}" in
                yes) HAVE_PYGOBJECT=yes ;;
                no)  HAVE_PYGOBJECT=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-pygobject) ;;
        esac],
        [HAVE_PYGOBJECT=yes])

    if test "x$HAVE_PYGOBJECT" = "xyes" ; then
        PKG_CHECK_MODULES([PYGOBJECT],[pygobject-3.0],,
            [AC_MSG_ERROR(Could not find Python GObject)])
    fi


    if test "x$HAVE_DBUS" = "xyes" ; then
        AC_ARG_ENABLE(python-dbus,
	    AS_HELP_STRING([--disable-python-dbus],[Disable use of D-Bus in Python]),
            [case "${enableval}" in
                   yes) HAVE_PYTHON_DBUS=yes ;;
                   no)  HAVE_PYTHON_DBUS=no ;;
                   *) AC_MSG_ERROR(bad value ${enableval} for --enable-python-dbus) ;;
            esac],
            [HAVE_PYTHON_DBUS=yes])

        if test "x$HAVE_PYTHON_DBUS" = "xyes"; then
            AM_CHECK_PYMOD(dbus,,,[AC_MSG_ERROR(Could not find Python module dbus)])
        fi

        AM_CHECK_PYMOD(socket,,,[AC_MSG_ERROR(Could not find Python module socket)])
        if test "x$HAVE_GDBM" = "xyes" || test "x$HAVE_DBM" = "xyes"; then
            AM_CHECK_PYMOD(anydbm,,,[
                AM_CHECK_PYMOD(dbm,,,[AC_MSG_ERROR(Could not find Python module dbm)])
            ])
        fi
    fi
fi
AM_CONDITIONAL(HAVE_PYTHON, [test "x$HAVE_PYTHON" = "xyes" ])
AM_CONDITIONAL(HAVE_PYGOBJECT, test "x$HAVE_PYGOBJECT" = "xyes")
AM_CONDITIONAL(HAVE_PYTHON_DBUS, test "x$HAVE_PYTHON_DBUS" = "xyes")

#
# Check for mono stuff
#
HAVE_MONO=no
if test "x$HAVE_DBUS" = "xyes" ; then
    AC_ARG_ENABLE(mono,
            AS_HELP_STRING([--disable-mono],[Disable mono bindings]),
            [case "${enableval}" in
                    yes) HAVE_MONO=yes ;;
                    no)  HAVE_MONO=no ;;
                    *) AC_MSG_ERROR(bad value ${enableval} for --enable-mono) ;;
            esac],
            [HAVE_MONO=yes])

    if test "x$HAVE_MONO" = "xyes" ; then
        AC_PATH_PROG(MCS, mcs)
        if test "x$MCS" = "x" ; then
            AC_MSG_ERROR([Can not find "mcs" - The Mono C-Sharp Compiler) in your PATH])
        fi

        AC_PATH_PROG(GACUTIL, gacutil)
        if test "x$GACUTIL" = "x" ; then
            AC_MSG_ERROR([Can not find "gacutil" in your PATH])
        fi

        AC_SUBST(MCS)
        AC_SUBST(GACUTIL)
    fi
fi
AM_CONDITIONAL(HAVE_MONO, test "x$HAVE_MONO" = "xyes")

#
# Check for monodoc stuff
#
HAVE_MONODOC=no
AC_ARG_ENABLE(monodoc,
        AS_HELP_STRING([--disable-monodoc],[Disable documentation for mono bindings]),
        [case "${enableval}" in
                yes) HAVE_MONODOC=yes ;;
                no)  HAVE_MONODOC=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-monodoc) ;;
        esac],
        [HAVE_MONODOC=yes])

if test "x$HAVE_MONO" = "xyes" && test "x$HAVE_MONODOC" = "xyes" ; then
        PKG_CHECK_MODULES(MONODOC, [monodoc >= 1.1.8])
        MONODOC_DIR=`$PKG_CONFIG --variable=sourcesdir monodoc`

        AC_PATH_PROG(MONODOCER, monodocer)
        AC_PATH_PROG(MDASSEMBLER, mdassembler)

        AC_SUBST(MONODOC_DIR)
        AC_SUBST(MONODOCER)
        AC_SUBST(MDASSEMBLER)
fi
AM_CONDITIONAL(HAVE_MONODOC, test "x$HAVE_MONODOC" = "xyes")

#
# Build autoipd?
#
AC_ARG_ENABLE(autoipd,
        AS_HELP_STRING([--disable-autoipd],[Disable building of avahi-autoipd]),
        [case "${enableval}" in
                yes) ENABLE_AUTOIPD=yes ;;
                no)  ENABLE_AUTOIPD=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --disable-autoipd) ;;
        esac],
        [ENABLE_AUTOIPD=yes])

AM_CONDITIONAL(ENABLE_AUTOIPD, test "x$ENABLE_AUTOIPD" = "xyes")

#
# Defining users and groups
#
AC_ARG_WITH(avahi_user, AS_HELP_STRING([--with-avahi-user=<user>],[User for running avahi-daemon (avahi)]))
if test -z "$with_avahi_user" ; then
    AVAHI_USER=avahi
else
    AVAHI_USER=$with_avahi_user
fi
AC_SUBST(AVAHI_USER)
AC_DEFINE_UNQUOTED(AVAHI_USER,"$AVAHI_USER", [User for running the Avahi daemon])

AC_ARG_WITH(avahi_group,AS_HELP_STRING([--with-avahi-group=<group>],[Group for running avahi-daemon (avahi)]))
if test -z "$with_avahi_group" ; then
    AVAHI_GROUP=avahi
else
    AVAHI_GROUP=$with_avahi_group
fi
AC_SUBST(AVAHI_GROUP)
AC_DEFINE_UNQUOTED(AVAHI_GROUP,"$AVAHI_GROUP", [Group for Avahi])

AC_ARG_WITH(avahi_priv_access_group,AS_HELP_STRING([--with-avahi-priv-access-group=<group>],[Priviliged access group for Avahi clients (netdev)]))
if test -z "$with_avahi_priv_access_group" ; then
    AVAHI_PRIV_ACCESS_GROUP=netdev
else
    AVAHI_PRIV_ACCESS_GROUP=$with_avahi_priv_access_group
fi
AC_SUBST(AVAHI_PRIV_ACCESS_GROUP)
AC_DEFINE_UNQUOTED(AVAHI_PRIV_ACCESS_GROUP,"$AVAHI_PRIV_ACCESS_GROUP", [Privileged access group for Avahi clients])

AC_ARG_WITH(autoipd_user, AS_HELP_STRING([--with-autoipd-user=<user>],[User for running the avahi-autoipd daemon (avahi-autoipd)]))
if test -z "$with_autoipd_user" ; then
    AVAHI_AUTOIPD_USER=avahi-autoipd
else
    AVAHI_AUTOIPD_USER=$with_autoipd_user
fi
AC_SUBST(AVAHI_AUTOIPD_USER)
AC_DEFINE_UNQUOTED(AVAHI_AUTOIPD_USER,"$AVAHI_AUTOIPD_USER", [User for running the avahi-autoipd daemon])

AC_ARG_WITH(autoipd_group,AS_HELP_STRING([--with-autoipd-group=<group>],[Group for running the avahi-autoipd daemon (avahi-autoipd)]))
if test -z "$with_autoipd_group" ; then
    AVAHI_AUTOIPD_GROUP=avahi-autoipd
else
    AVAHI_AUTOIPD_GROUP=$with_autoipd_group
fi
AC_SUBST(AVAHI_AUTOIPD_GROUP)
AC_DEFINE_UNQUOTED(AVAHI_AUTOIPD_GROUP,"$AVAHI_AUTOIPD_GROUP", [Group for running the avahi-autoipd daemon])

#
# Avahi runtime dir
#
avahi_runtime_dir="/run"
avahi_socket="${avahi_runtime_dir}/avahi-daemon/socket"
AC_SUBST(avahi_runtime_dir)
AC_SUBST(avahi_socket)

#
# Avahi interfaces dir
#
if test "x$HAVE_PYTHON_DBUS" = "xyes" -o "x$HAVE_GTK" = "xyes" -o "x$HAVE_GTK3" = "xyes"; then
	interfacesdir="${datadir}/${PACKAGE}/interfaces/"
	AC_SUBST(interfacesdir)
fi

#
# Doxygen
#
DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(ON)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN(avahi, doxygen.cfg, doxygen)

AC_ARG_ENABLE(core-docs,
        AS_HELP_STRING([--enable-core-docs],[Enable building of documentation for avahi-core]),
[case "${enableval}" in
  yes) ENABLE_CORE_DOCS=yes ;;
  no)  ENABLE_CORE_DOCS=no ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-core-docs]) ;;
esac],[ENABLE_CORE_DOCS=no])

AM_CONDITIONAL([ENABLE_CORE_DOCS], [test "x$ENABLE_CORE_DOCS" = xyes])

#
# Build and Install man pages
#
AC_ARG_ENABLE(manpages,
        AS_HELP_STRING([--disable-manpages],[Disable building and installation of man pages]),
[case "${enableval}" in
  yes) manpages=yes ;;
  no)  manpages=no ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --disable-manpages]) ;;
esac],[manpages=yes])

if test x$manpages = xyes ; then
    #
    # XMLTOMAN manpage generation
    #
    AC_ARG_ENABLE(xmltoman,
    AS_HELP_STRING([--disable-xmltoman],[Disable rebuilding of man pages with xmltoman]),
    [case "${enableval}" in
      yes) xmltoman=yes ;;
      no)  xmltoman=no ;;
      *) AC_MSG_ERROR([bad value ${enableval} for --disable-xmltoman]) ;;
    esac],[xmltoman=yes])

    if test x$xmltoman = xyes ; then
        AC_CHECK_PROG(have_xmltoman, xmltoman, yes, no)
    fi

    if test x$have_xmltoman = xno -o x$xmltoman = xno; then
        if ! test -e $srcdir/man/avahi-daemon.8 ; then
            AC_MSG_ERROR([*** xmltoman was not found or was disabled, it is required to build the manpages as they have not been pre-built, install xmltoman, pass --disable-manpages or dont pass --disable-xmltoman])
            exit 1
        fi
        AC_MSG_WARN([*** Not rebuilding man pages as xmltoman is not found ***])
        xmltoman=no
    fi
fi
AM_CONDITIONAL([USE_XMLTOMAN], [test "x$xmltoman" = xyes])
AM_CONDITIONAL([BUILD_MANPAGES], [test "x$manpages" = xyes])

#
# Conditionally compile test and example programs
#
AC_ARG_ENABLE(tests,
        AS_HELP_STRING([--enable-tests],[Enable building of tests and examples]),
        [case "${enableval}" in
                yes) ENABLE_TESTS=yes ;;
                no)  ENABLE_TESTS=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-tests) ;;
        esac],
        [ENABLE_TESTS=no])

AM_CONDITIONAL([ENABLE_TESTS], [test "x$ENABLE_TESTS" = "xyes"])

#
# Optionally enable libdns_sd compatibility support
#
AC_ARG_ENABLE(compat-libdns_sd,
        AS_HELP_STRING([--enable-compat-libdns_sd],[Enable compatibility layer for libdns_sd]),
        [case "${enableval}" in
                yes) ENABLE_COMPAT_LIBDNS_SD=yes ;;
                no)  ENABLE_COMPAT_LIBDNS_SD=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-compat-libdns_sd) ;;
        esac],
        [ENABLE_COMPAT_LIBDNS_SD=no])

AM_CONDITIONAL([ENABLE_COMPAT_LIBDNS_SD], [test "x$ENABLE_COMPAT_LIBDNS_SD" = "xyes"])

#
# Optionally enable HOWL compatibility support
#
AC_ARG_ENABLE(compat-howl,
        AS_HELP_STRING([--enable-compat-howl],[Enable compatibility layer for HOWL]),
        [case "${enableval}" in
                yes) ENABLE_COMPAT_HOWL=yes ;;
                no)  ENABLE_COMPAT_HOWL=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-compat-howl) ;;
        esac],
        [ENABLE_COMPAT_HOWL=no])

AM_CONDITIONAL([ENABLE_COMPAT_HOWL], [test "x$ENABLE_COMPAT_HOWL" = "xyes"])

#
# systemd
#
AC_ARG_WITH([systemdsystemunitdir],
        AS_HELP_STRING([--with-systemdsystemunitdir=DIR], [Directory for systemd service files]),
        [],
        [with_systemdsystemunitdir=$($PKG_CONFIG --variable=systemdsystemunitdir systemd)])
if test "x$with_systemdsystemunitdir" != xno; then
        AC_SUBST([systemdsystemunitdir], [$with_systemdsystemunitdir])
fi
AM_CONDITIONAL(HAVE_SYSTEMD, [test -n "$with_systemdsystemunitdir" -a "x$with_systemdsystemunitdir" != xno ])

# ==========================================================================
AC_CONFIG_FILES([
Makefile
avahi-common/Makefile
avahi-core/Makefile
avahi-glib/Makefile
avahi-gobject/Makefile
avahi-qt/Makefile
avahi-daemon/Makefile
avahi-daemon/avahi-dbus.conf
avahi-discover-standalone/Makefile
avahi-client/Makefile
initscript/Makefile
initscript/debian/Makefile
initscript/gentoo/Makefile
initscript/suse/Makefile
initscript/fedora/Makefile
initscript/lfs/Makefile
initscript/mandriva/Makefile
initscript/darwin/Makefile
initscript/freebsd/Makefile
initscript/slackware/Makefile
avahi-dnsconfd/Makefile
avahi-utils/Makefile
avahi-python/Makefile
avahi-python/avahi/Makefile
avahi-python/avahi-discover/Makefile
examples/Makefile
common/Makefile
man/Makefile
tests/Makefile
service-type-database/Makefile
avahi-sharp/Makefile
avahi-ui-sharp/Makefile
avahi-compat-libdns_sd/Makefile
avahi-compat-howl/Makefile
avahi-compat-howl/samples/Makefile
avahi-autoipd/Makefile
avahi-ui/Makefile
avahi-libevent/Makefile
po/Makefile.in
])
AC_OUTPUT

# ==========================================================================
echo "
 ---{ $PACKAGE_NAME $VERSION }---

    prefix:                                    ${prefix}
    sysconfdir:                                ${sysconfdir}
    localstatedir:                             ${localstatedir}
    avahi socket:                              ${avahi_socket}
    dbus-1 system.d dir:                       ${DBUS_SYS_DIR}
    dbus-1 version:                            ${DBUS_VERSION}
    dbus-1 system socket:                      ${DBUS_SYSTEM_BUS_DEFAULT_ADDRESS}
    C Compiler:                                ${CC}
    CFLAGS:                                    ${CFLAGS}
    Enable GLIB:                               ${HAVE_GLIB}
    Enable GLIB GObject:                       ${HAVE_GOBJECT}
    Enable GObject Introspection:              ${found_introspection}
    Enable GTK 2.0:                            ${HAVE_GTK}
    Enable GTK 3.0:                            ${HAVE_GTK3}
    Enable D-Bus:                              ${HAVE_DBUS}
    With XML:                                  ${with_xml}
    Enable GDBM:                               ${HAVE_GDBM}
    Enable DBM:                                ${HAVE_DBM}
    Enable libdaemon:                          ${HAVE_LIBDAEMON}
    Enable Python:                             ${HAVE_PYTHON}
    Enable pygobject:                          ${HAVE_PYGOBJECT}
    Enable python-dbus:                        ${HAVE_PYTHON_DBUS}
    Enable QT3:                                ${HAVE_QT3}
    Enable QT4:                                ${HAVE_QT4}
    Enable QT5:                                ${HAVE_QT5}
    Enable Mono:                               ${HAVE_MONO}
    Enable Monodoc:                            ${HAVE_MONODOC}
    Distribution/OS:                           ${with_distro}
    User for avahi-daemon:                     ${AVAHI_USER}
    Group for avahi-daemon:                    ${AVAHI_GROUP}
    Priviliged access group for Avahi clients: ${AVAHI_PRIV_ACCESS_GROUP}
    User for avahi-autoipd:                    ${AVAHI_AUTOIPD_USER}
    Group for avahi-autoipd:                   ${AVAHI_AUTOIPD_GROUP}
    Enable chroot():                           ${enable_chroot}
    Enable Linux inotify:                      ${have_inotify}
    Enable stack-smashing protection:          ${enable_ssp}
    systemd unit directory:                    ${with_systemdsystemunitdir}
"

BUILD_DAEMON="no   (You need libdaemon and expat/bsdxml!)"

if test "x$HAVE_XML" = "xyes" -a "x$HAVE_LIBDAEMON" = "xyes" ; then
    BUILD_DAEMON=yes
fi

BUILD_PYTHON="no   (You need python, pygobject and python-dbus!)"

if test "x$BUILD_DAEMON" = "xyes" -a "x$HAVE_DBUS" = "xyes" -a "x$HAVE_PYTHON" = "xyes" -a "x$HAVE_PYTHON_DBUS" = "xyes" -a "x$HAVE_PYGOBJECT" = "xyes" ; then
    BUILD_PYTHON=yes
fi

BUILD_CLIENT="no   (You need avahi-daemon and D-Bus!)"

if test "x$HAVE_DBUS" = "xyes" ; then
    BUILD_CLIENT=yes
fi

if test "x$ENABLE_COMPAT_LIBDNS_SD" = "xyes" -a "x$BUILD_CLIENT" != "xyes" ; then
   ENABLE_COMPAT_LIBDNS_SD="no   (You need libavahi-client!)"
fi
if test "x$ENABLE_COMPAT_HOWL" = "xyes" -a "x$BUILD_CLIENT" != "xyes" ; then
   ENABLE_COMPAT_HOWL="no   (You need libavahi-client!)"
fi
if test "x$ENABLE_AUTOIPD" = "xyes" -a "x$HAVE_LIBDAEMON" != "xyes" ; then
   ENABLE_AUTOIPD="no   (You need libdaemon!)"
fi

HAVE_GTK2OR3=no
if test "x$HAVE_GTK" = "xyes" -o "x$HAVE_GTK3" = "xyes" ; then
   HAVE_GTK2OR3=yes
fi

BUILD_UI="no"
if test "x$HAVE_GTK2OR3" = "xyes" -a "x$BUILD_CLIENT" = "xyes" ; then
   BUILD_UI="yes"
fi

BUILD_GOBJECT="no"
if test "x$BUILD_CLIENT" = "xyes" -a "x$HAVE_GOBJECT" = "xyes" ; then
   BUILD_GOBJECT="yes"
fi

echo "\
    Building libavahi-core              yes
    Building avahi-daemon:              ${BUILD_DAEMON}
    Building avahi-dnsconfd:            ${BUILD_DAEMON}
    Building libavahi-client:           ${BUILD_CLIENT}
    Building avahi-utils:               ${BUILD_CLIENT}
    Building avahi-python:              ${BUILD_PYTHON}
    Building libavahi-glib:             ${HAVE_GLIB}
    Building libavahi-gobject:          ${BUILD_GOBJECT}
    Building avahi-discover-standalone: ${HAVE_GTK2OR3}
    Building libavahi-qt3:              ${HAVE_QT3}
    Building libavahi-qt4:              ${HAVE_QT4}
    Building libavahi-qt5:              ${HAVE_QT5}
    Building avahi-sharp:               ${HAVE_MONO}
    Building avahi-compat-libdns_sd:    ${ENABLE_COMPAT_LIBDNS_SD}
    Building avahi-compat-howl:         ${ENABLE_COMPAT_HOWL}
    Building tests:                     ${ENABLE_TESTS}
    Building avahi-core documentation:  ${ENABLE_CORE_DOCS}
    Building avahi-autoipd:             ${ENABLE_AUTOIPD}
    Building libavahi-ui:               ${BUILD_UI}
    Building libavahi-libevent:		${HAVE_LIBEVENT}
"
