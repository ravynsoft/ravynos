dnl Local m4 macros for autoconf (used by sudo)
dnl
dnl SPDX-License-Identifier: ISC
dnl
dnl Copyright (c) 1994-1996, 1998-2005, 2007-2023
dnl	Todd C. Miller <Todd.Miller@sudo.ws>
dnl
dnl Permission to use, copy, modify, and distribute this software for any
dnl purpose with or without fee is hereby granted, provided that the above
dnl copyright notice and this permission notice appear in all copies.
dnl
dnl THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
dnl WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
dnl MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
dnl ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
dnl WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
dnl ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
dnl OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
dnl

dnl
dnl check for sendmail in well-known locations
dnl
AC_ARG_VAR([SENDMAILPROG], [The fully-qualified path to the sendmail program to use.])
AC_DEFUN([SUDO_PROG_SENDMAIL], [
    AC_PATH_PROG([SENDMAILPROG], [sendmail], [], [/usr/sbin$PATH_SEPARATOR/usr/lib$PATH_SEPARATOR/usr/etc$PATH_SEPARATOR/usr/ucblib$PATH_SEPARATOR/usr/local/lib$PATH_SEPARATOR/usr/local/bin])
    test -n "${ac_cv_path_SENDMAILPROG}" && SUDO_DEFINE_UNQUOTED(_PATH_SUDO_SENDMAIL, "${ac_cv_path_SENDMAILPROG}")
])dnl

dnl
dnl check for vi in well-known locations
dnl
AC_ARG_VAR([VIPROG], [The fully-qualified path to the vi program to use.])
AC_DEFUN([SUDO_PROG_VI], [
    AC_PATH_PROG([VIPROG], [vi], [], [/usr/bin$PATH_SEPARATOR/bin$PATH_SEPARATOR/usr/ucb$PATH_SEPARATOR/usr/bsd$PATH_SEPARATOR/usr/local/bin])
    test -n "${ac_cv_path_VIPROG}" && SUDO_DEFINE_UNQUOTED(_PATH_VI, "${ac_cv_path_VIPROG}")
])dnl

dnl
dnl check for mv in well-known locations
dnl
AC_ARG_VAR([MVPROG], [The fully-qualified path to the mv program to use.])
AC_DEFUN([SUDO_PROG_MV], [
    AC_PATH_PROG([MVPROG], [mv], [], [/usr/bin$PATH_SEPARATOR/bin$PATH_SEPARATOR/usr/ucb$PATH_SEPARATOR/usr/local/bin])
    test -n "${ac_cv_path_MVPROG}" && SUDO_DEFINE_UNQUOTED(_PATH_MV, "${ac_cv_path_MVPROG}")
])dnl

dnl
dnl check for bourne shell in well-known locations
dnl
AC_ARG_VAR([BSHELLPROG], [The fully-qualified path to the Bourne shell to use.])
AC_DEFUN([SUDO_PROG_BSHELL], [
    AC_PATH_PROG([BSHELLPROG], [sh], [/usr/bin$PATH_SEPARATOR/bin$PATH_SEPARATOR/usr/sbin$PATH_SEPARATOR/sbin])
    test -n "${ac_cv_path_BSHELLPROG}" && SUDO_DEFINE_UNQUOTED(_PATH_BSHELL, "${ac_cv_path_BSHELLPROG}")
])dnl

dnl
dnl Check for path to utmp file if not using getutid(), etc.
dnl
AC_DEFUN([SUDO_PATH_UTMP], [
    AC_CACHE_CHECK([for utmp file path], [sudo_cv_path_UTMP], [
	sudo_cv_path_UTMP=no
	for p in "/var/run/utmp" "/var/adm/utmp" "/etc/utmp"; do
	    if test -r "$p"; then
		sudo_cv_path_UTMP="$p"
		break
	    fi
	done
    ])
    if test X"$sudo_cv_path_UTMP" != X"no"; then
	SUDO_DEFINE_UNQUOTED(_PATH_UTMP, "$sudo_cv_path_UTMP")
    fi
])

dnl
dnl Where the log file goes, use /var/log if it exists, else /{var,usr}/adm
dnl
AC_DEFUN([SUDO_LOGFILE], [
    if test -n "$with_logpath"; then
	logpath="$with_logpath"
    else
	AC_CACHE_CHECK([for log file location], [sudo_cv_log_path], [
	    # Default value of logpath set in configure.ac
	    sudo_cv_log_path="$logpath"
	    for d in /var/log /var/adm /usr/adm; do
		if test -d "$d"; then
		    sudo_cv_log_path="$d/sudo.log"
		    break
		fi
	    done
	])
	logpath="$sudo_cv_log_path"
    fi
    SUDO_DEFINE_UNQUOTED(_PATH_SUDO_LOGFILE, "$logpath")
])

dnl
dnl Detect time zone file directory, if any.
dnl
AC_DEFUN([SUDO_TZDIR], [
    if test -n "$with_tzdir"; then
	tzdir="$with_tzdir"
    else
	AC_CACHE_CHECK([time zone data directory], [sudo_cv_tz_dir], [
	    sudo_cv_tz_dir=no
	    for d in /usr/share /usr/share/lib /usr/lib /etc; do
		if test -d "$d/zoneinfo"; then
		    sudo_cv_tz_dir="$d/zoneinfo"
		    break
		fi
	    done
	])
	tzdir="$sudo_cv_tz_dir"
    fi
    if test X"$tzdir" != X"no"; then
	SUDO_DEFINE_UNQUOTED(_PATH_ZONEINFO, "$tzdir")
    fi
])

dnl
dnl Parent directory for time stamp dir.
dnl
AC_DEFUN([SUDO_RUNDIR], [
    if test -n "$with_rundir"; then
	rundir="$with_rundir"
    elif test -n "$runstatedir" && test "$runstatedir" != '${localstatedir}/run'; then
	rundir="$runstatedir/sudo"
    else
	# No --with-rundir or --runstatedir specified
	AC_CACHE_CHECK([for sudo run dir location], [sudo_cv_run_dir], [
	    sudo_cv_run_dir=no
	    for d in /run /var/run /var/db /var/lib /var/adm /usr/adm; do
		if test -d "$d"; then
		    sudo_cv_run_dir="$d/sudo"
		    break
		fi
	    done
	])
	rundir="$sudo_cv_run_dir"
    fi
    if test X"$rundir" != X"no"; then
	SUDO_DEFINE_UNQUOTED(_PATH_SUDO_TIMEDIR, "$rundir/ts")
	SUDO_DEFINE_UNQUOTED(_PATH_SUDO_LOGSRVD_PID, "$rundir/sudo_logsrvd.pid")
    fi
])

dnl
dnl Parent directory for the lecture status dir.
dnl
AC_DEFUN([SUDO_VARDIR], [
    if test -n "$with_vardir"; then
	vardir="$with_vardir"
    else
	AC_CACHE_CHECK([for sudo var dir location], [sudo_cv_var_dir], [
	    sudo_cv_var_dir=no
	    for d in /var/db /var/lib /var/adm /usr/adm; do
		if test -d "$d"; then
		    sudo_cv_var_dir="$d/sudo"
		    break
		fi
	    done
	])
	vardir="$sudo_cv_var_dir"
    fi
    if test X"$vardir" != X"no"; then
	SUDO_DEFINE_UNQUOTED(_PATH_SUDO_LECTURE_DIR, "$vardir/lectured")
    fi
])

dnl
dnl Where the sudo_logsrvd relay temporary log files go, use
dnl /var/log/sudo_logsrvd if /var/log exists, else
dnl /{var,usr}/adm/sudo_logsrvd
dnl
AC_DEFUN([SUDO_RELAY_DIR], [
    if test "${with_relaydir-yes}" != "yes"; then
	relay_dir="$with_relaydir"
    else
	AC_CACHE_CHECK([for sudo_logsrvd relay dir location], [sudo_cv_relay_dir], [
	    # Default value of relay_dir set in configure.ac
	    sudo_cv_relay_dir="$relay_dir"
	    for d in /var/log /var/adm /usr/adm; do
		if test -d "$d"; then
		    sudo_cv_relay_dir="$d/sudo_logsrvd"
		    break
		fi
	    done
	])
	relay_dir="$sudo_cv_relay_dir"
    fi
    SUDO_DEFINE_UNQUOTED(_PATH_SUDO_RELAY_DIR, "$relay_dir")
])

dnl
dnl Where the I/O log files go, use /var/log/sudo-io if
dnl /var/log exists, else /{var,usr}/adm/sudo-io
dnl
AC_DEFUN([SUDO_IO_LOGDIR], [
    if test "${with_iologdir-yes}" != "yes"; then
	iolog_dir="$with_iologdir"
    else
	AC_CACHE_CHECK([for I/O log dir location], [sudo_cv_iolog_dir], [
	    # Default value of iolog_dir set in configure.ac
	    sudo_cv_iolog_dir="$iolog_dir"
	    for d in /var/log /var/adm /usr/adm; do
		if test -d "$d"; then
		    sudo_cv_iolog_dir="$d/sudo-io"
		    break
		fi
	    done
	])
	iolog_dir="$sudo_cv_iolog_dir"
    fi
    SUDO_DEFINE_UNQUOTED(_PATH_SUDO_IO_LOGDIR, "$iolog_dir")
])

dnl
dnl Where the log files go, use /var/log if it exists, else /{var,usr}/adm
dnl
AC_DEFUN([SUDO_LOGDIR], [
    if test "${with_logdir-yes}" != "yes"; then
	log_dir="$with_logdir"
    else
	AC_CACHE_CHECK([for log dir location], [sudo_cv_log_dir], [
	    # Default value of log_dir set in configure.ac
	    sudo_cv_log_dir="$log_dir"
	    for d in /var/log /var/adm /usr/adm; do
		if test -d "$d"; then
		    sudo_cv_log_dir="$d"
		    break
		fi
	    done
	])
	log_dir="$sudo_cv_log_dir"
    fi
    SUDO_DEFINE_UNQUOTED(_PATH_SUDO_LOGDIR, "$log_dir")
])

dnl
dnl check for working fnmatch(3)
dnl
AC_DEFUN([SUDO_FUNC_FNMATCH], [
    AC_CACHE_CHECK([for working fnmatch with FNM_CASEFOLD],
    sudo_cv_func_fnmatch, [
	AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <fnmatch.h>
int main() { return(fnmatch("/*/bin/echo *", "/usr/bin/echo just a test", FNM_CASEFOLD)); }]])], [sudo_cv_func_fnmatch=yes], [sudo_cv_func_fnmatch=no], [sudo_cv_func_fnmatch=no])
    ])
    AS_IF([test $sudo_cv_func_fnmatch = yes], [$1], [$2])
])

dnl
dnl Attempt to check for working PIE support.
dnl This is a bit of a hack but on Solaris 10 with GNU ld and GNU as
dnl we can end up with strange values from malloc().
dnl A better check would be to verify that ASLR works with PIE.
dnl
AC_DEFUN([SUDO_WORKING_PIE], [
    AC_CACHE_CHECK([for working PIE support], sudo_cv_working_pie, [
	AC_RUN_IFELSE([AC_LANG_SOURCE([AC_INCLUDES_DEFAULT
int main() { char *p = malloc(1024); if (p == NULL) return 1; memset(p, 0, 1024); return 0; }])], [sudo_cv_working_pie=yes], [sudo_cv_working_pie=no], [sudo_cv_working_pie=no])
    ])
    AS_IF([test $sudo_cv_working_pie = yes], [$1], [$2])
])

dnl
dnl check for isblank(3)
dnl
AC_DEFUN([SUDO_FUNC_ISBLANK],
  [AC_CACHE_CHECK([for isblank], [sudo_cv_func_isblank],
    [AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <ctype.h>]], [[return (isblank('a'));]])],
    [sudo_cv_func_isblank=yes], [sudo_cv_func_isblank=no])])
] [
  if test "$sudo_cv_func_isblank" = "yes"; then
    AC_DEFINE(HAVE_ISBLANK, 1, [Define if you have isblank(3).])
  else
    AC_LIBOBJ(isblank)
    SUDO_APPEND_COMPAT_EXP(isblank)
  fi
])

AC_DEFUN([SUDO_CHECK_LIB], [
    _sudo_check_lib_extras=`echo "$5"|sed -e 's/[ 	]*//g' -e 's/-l/_/g'`
    AC_MSG_CHECKING([for $2 in -l$1${5+ }$5])
    AC_CACHE_VAL([sudo_cv_lib_$1''_$2$_sudo_check_lib_extras], [
	SUDO_CHECK_LIB_OLIBS="$LIBS"
	LIBS="$LIBS -l$1${5+ }$5"
	AC_LINK_IFELSE(
	    [AC_LANG_CALL([], [$2])],
	    [eval sudo_cv_lib_$1''_$2$_sudo_check_lib_extras=yes],
	    [eval sudo_cv_lib_$1''_$2$_sudo_check_lib_extras=no]
	)
	LIBS="$SUDO_CHECK_LIB_OLIBS"
    ])
    if eval test \$sudo_cv_lib_$1''_$2$_sudo_check_lib_extras = "yes"; then
	AC_MSG_RESULT([yes])
	$3
    else
	AC_MSG_RESULT([no])
	$4
    fi
])

AC_DEFUN([SUDO_CHECK_NET_FUNC], [
    _LIBS="$LIBS"
    LIBS="${LIBS} ${NET_LIBS}"
    found=true
    AC_CHECK_FUNC([$1], [$2], [
	# Look for $1 in network libraries appending to NET_LIBS as needed.
	# May need to link with -lnsl and -lsocket due to unresolved symbols
	found=false
	for libs in "-lsocket" "-linet" "-lsocket -lnsl" "-lresolv"; do
	    _libs=
	    for lib in $libs; do
		case "$NET_LIBS" in
		    *"$lib"*)   ;;
		    *)		_libs="$_libs $lib";;
		esac
	    done
	    libs="${_libs# }"
	    test -z "$libs" && continue
	    lib="`echo \"$libs\"|sed -e 's/^-l//' -e 's/ .*$//'`"
	    extralibs="`echo \"$libs\"|sed 's/^-l[[^ ]]*//'`"
	    SUDO_CHECK_LIB($lib, $1, [
		found=true
		NET_LIBS="${NET_LIBS}${NET_LIBS+ }$libs"
		INET_PTON_LIBS="$libs"
		case "$libs" in
		    *-lresolv*)
			AC_DEFINE(NEED_RESOLV_H)
			;;
		esac
		break
	    ], [], [$extralibs])
	done
    ])
    LIBS="$_LIBS"
    AS_IF([test $found = true], [$2], [$3])
])

dnl
dnl check unsetenv() return value
dnl
AC_DEFUN([SUDO_FUNC_UNSETENV_VOID],
  [AC_CACHE_CHECK([whether unsetenv returns void], [sudo_cv_func_unsetenv_void],
    [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT], [
        [return unsetenv("FOO");]
      ])
    ],
    [sudo_cv_func_unsetenv_void=no],
    [sudo_cv_func_unsetenv_void=yes])])
    if test $sudo_cv_func_unsetenv_void = yes; then
      AC_DEFINE(UNSETENV_VOID, 1,
        [Define to 1 if the 'unsetenv' function returns void instead of 'int'.])
    fi
  ])

dnl
dnl check putenv() argument for const
dnl
AC_DEFUN([SUDO_FUNC_PUTENV_CONST],
  [AC_CACHE_CHECK([whether putenv takes a const argument], [sudo_cv_func_putenv_const],
    [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT
      int putenv(const char *string) {return 0;}
    ], [])],
    [sudo_cv_func_putenv_const=yes],
    [sudo_cv_func_putenv_const=no])
  ])
  if test $sudo_cv_func_putenv_const = yes; then
    AC_DEFINE(PUTENV_CONST, const, [Define to const if the 'putenv' function takes a const argument.])
  else
    AC_DEFINE(PUTENV_CONST, [])
  fi
])

dnl
dnl check if ioctl() request argument is int.
dnl
AC_DEFUN([SUDO_FUNC_IOCTL_REQ_INT],
  [AC_CACHE_CHECK([whether ioctl() takes an int request argument], [sudo_cv_func_ioctl_req_int],
    [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
      int ioctl(int fd, int req, ...) {return 0;}
    ], [])],
    [sudo_cv_func_ioctl_req_int=yes],
    [sudo_cv_func_ioctl_req_int=no])
  ])
  if test $sudo_cv_func_ioctl_req_int = yes; then
    AC_DEFINE(IOCTL_REQ_CAST, [(int)], [Define to (int) if the 'ioctl' function request takes an int request argument.])
  else
    AC_DEFINE(IOCTL_REQ_CAST, [])
  fi
])

dnl
dnl check whether au_close() takes 3 or 4 arguments
dnl
AC_DEFUN([SUDO_FUNC_AU_CLOSE_SOLARIS11],
[AC_CACHE_CHECK([whether au_close() takes 4 arguments],
sudo_cv_func_au_close_solaris11,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT
#include <bsm/audit.h>
#include <bsm/libbsm.h>
#include <bsm/audit_uevents.h>

int au_close(int d, int keep, au_event_t event, au_emod_t emod) {return 0;}], [])],
    [sudo_cv_func_au_close_solaris11=yes],
    [sudo_cv_func_au_close_solaris11=no])
  ])
  if test $sudo_cv_func_au_close_solaris11 = yes; then
    AC_DEFINE(HAVE_AU_CLOSE_SOLARIS11, 1, [Define to 1 if the 'au_close' functions takes 4 arguments like Solaris 11.])
  fi
])

dnl
dnl Check if the data argument for the sha2 functions is void * or u_char *
dnl
AC_DEFUN([SUDO_FUNC_SHA2_VOID_PTR],
[AC_CACHE_CHECK([whether the data argument of SHA224Update() is void *],
sudo_cv_func_sha2_void_ptr,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT
#include <sha2.h>
void SHA224Update(SHA2_CTX *context, const void *data, size_t len) {return;}], [])],
    [sudo_cv_func_sha2_void_ptr=yes],
    [sudo_cv_func_sha2_void_ptr=no])
  ])
  if test $sudo_cv_func_sha2_void_ptr = yes; then
    AC_DEFINE(SHA2_VOID_PTR, 1,
      [Define to 1 if the sha2 functions use 'const void *' instead of 'const unsigned char'.])
  fi
])

dnl
dnl check for sa_len field in struct sockaddr
dnl
AC_DEFUN([SUDO_SOCK_SA_LEN], [
    AC_CHECK_MEMBER([struct sockaddr.sa_len], 
	[AC_DEFINE(HAVE_STRUCT_SOCKADDR_SA_LEN, 1, [Define if your struct sockaddr has an sa_len field.])],
	[], [
#	  include <sys/types.h>
#	  include <sys/socket.h>] 
    )]
)

dnl
dnl check for sin_len field in struct sockaddr_in
dnl
AC_DEFUN([SUDO_SOCK_SIN_LEN], [
    AC_CHECK_MEMBER([struct sockaddr_in.sin_len],
	[AC_DEFINE(HAVE_STRUCT_SOCKADDR_IN_SIN_LEN, 1, [Define if your struct sockaddr_in has a sin_len field.])],
	[], [
#	  include <sys/types.h>
#	  include <sys/socket.h>]
    )]
)

dnl
dnl There are three different utmp variants we need to check for.
dnl SUDO_CHECK_UTMP_MEMBERS(utmp_type)
dnl
AC_DEFUN([SUDO_CHECK_UTMP_MEMBERS], [
    dnl
    dnl Check for utmp/utmpx/utmps struct members.
    dnl
    AC_CHECK_MEMBER([struct $1.ut_id], [
	AC_DEFINE(HAVE_STRUCT_UTMP_UT_ID, 1, [Define to 1 if 'ut_id' is a member of 'struct utmp'.])
    ], [], [
#	include <sys/types.h>
#	include <$1.h>
    ])
    AC_CHECK_MEMBER([struct $1.ut_pid], [
	AC_DEFINE(HAVE_STRUCT_UTMP_UT_PID, 1, [Define to 1 if 'ut_pid' is a member of 'struct utmp'.])
    ], [], [
#	include <sys/types.h>
#	include <$1.h>
    ])
    AC_CHECK_MEMBER([struct $1.ut_tv], [
	AC_DEFINE(HAVE_STRUCT_UTMP_UT_TV, 1, [Define to 1 if 'ut_tv' is a member of 'struct utmp'.])
    ], [], [
#	include <sys/types.h>
#	include <$1.h>
    ])
    AC_CHECK_MEMBER([struct $1.ut_type], [
	AC_DEFINE(HAVE_STRUCT_UTMP_UT_TYPE, 1, [Define to 1 if 'ut_type' is a member of 'struct utmp'.])
    ], [], [
#	include <sys/types.h>
#	include <$1.h>
    ])
    dnl
    dnl Older struct utmp has ut_name instead of ut_user
    dnl
    if test "$1" = "utmp"; then
	AC_CHECK_MEMBERS([struct utmp.ut_user], [], [], [
#	include <sys/types.h>
#	include <$1.h>
	])
    fi
    dnl
    dnl Check for ut_exit.__e_termination first, then ut_exit.e_termination
    dnl We need to have already defined _GNU_SOURCE on glibc which only has
    dnl __e_termination visible when _GNU_SOURCE is *not* defined.
    dnl
    AC_CHECK_MEMBER([struct $1.ut_exit.__e_termination], [
	AC_DEFINE(HAVE_STRUCT_UTMP_UT_EXIT, 1, [Define to 1 if 'ut_exit' is a member of 'struct utmp'.])
	AC_DEFINE(HAVE_STRUCT_UTMP_UT_EXIT___E_TERMINATION, 1, [Define to 1 if 'ut_exit.__e_termination' is a member of 'struct utmp'.])
    ], [
	AC_CHECK_MEMBER([struct $1.ut_exit.e_termination], [
	    AC_DEFINE(HAVE_STRUCT_UTMP_UT_EXIT, 1, [Define to 1 if 'ut_exit' is a member of 'struct utmp'.])
	    AC_DEFINE(HAVE_STRUCT_UTMP_UT_EXIT_E_TERMINATION, 1, [Define to 1 if 'ut_exit.e_termination' is a member of 'struct utmp'.])
	], [], [
#	    include <sys/types.h>
#	    include <$1.h>
	])
    ], [
#	include <sys/types.h>
#	include <$1.h>
    ])
])

dnl
dnl Append a libpath to an LDFLAGS style variable if not already present.
dnl Also appends to the _R version unless rpath is disabled.
dnl
AC_DEFUN([SUDO_APPEND_LIBPATH], [
    AX_APPEND_FLAG([-L$2], [$1])
    if test X"$enable_rpath" = X"yes"; then
	AX_APPEND_FLAG([-R$2], [$1_R])
    fi
])

dnl
dnl Append one or more symbols to COMPAT_EXP
dnl
AC_DEFUN([SUDO_APPEND_COMPAT_EXP], [
    for _sym in $1; do
	COMPAT_EXP="${COMPAT_EXP}${_sym}
"
    done
])

dnl
dnl Append one or more symbols to INTERCEPT_EXP
dnl
AC_DEFUN([SUDO_APPEND_INTERCEPT_EXP], [
    for _sym in $1; do
	INTERCEPT_EXP="${INTERCEPT_EXP}${_sym}
"
    done
])

dnl
dnl Determine the mail spool location
dnl NOTE: must be run *after* check for paths.h
dnl
AC_DEFUN([SUDO_MAILDIR], [
    AC_CACHE_CHECK([for the user mail spool directory], [sudo_cv_mail_dir], [
	sudo_cv_mail_dir=no
	if test X"$ac_cv_header_paths_h" = X"yes"; then
	    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT
#include <paths.h>],
	    [char *p = _PATH_MAILDIR;])], [sudo_cv_mail_dir="paths.h"], [])
	fi
	if test $sudo_cv_mail_dir = no; then
	    # Solaris has maillock.h which defines MAILDIR
	    AC_CHECK_HEADERS(maillock.h, [
		sudo_cv_mail_dir=maillock.h
	    ], [
		sudo_cv_mail_dir=/var/mail
		for d in /var/mail /var/spool/mail /usr/spool/mail; do
		    if test -d "$d"; then
			sudo_cv_mail_dir="$d"
			break
		    fi
		done
	    ])
	fi
    ])
    case "$sudo_cv_mail_dir" in
    paths.h)
	# _PATH_MAILDIR already present in paths.h.
	;;
    maillock.h)
	# Use MAILDIR from maillock.h
	SUDO_DEFINE(_PATH_MAILDIR, MAILDIR)
	AC_DEFINE(HAVE_MAILLOCK_H)
	;;
    *)
	SUDO_DEFINE_UNQUOTED(_PATH_MAILDIR, "$sudo_cv_mail_dir")
	;;
    esac
])

dnl
dnl Create PVS-Studio.cfg for supported platforms or throw an error.
dnl
AC_DEFUN([SUDO_PVS_STUDIO_CFG], [
    if test X"$enable_pvs_studio" = X"yes"; then
	# Determine preprocessor type
	case "$CC" in
	*clang*) preprocessor=clang;;
	*gcc*) preprocessor=gcc;;
	*)  case `$CC --version 2>&1` in
	    *clang*) preprocessor=clang;;
	    *gcc*) preprocessor=gcc;;
	    *) AC_MSG_ERROR([Compiler must be gcc or clang for PVS-Studio.]);;
	    esac
	    ;;
	esac

	# Determine platform (currently linux or macos)
	case "$host" in
	x86_64-*-linux*) pvs_platform=linux64;;
	*86-*-linux*) pvs_platform=linux32;;
	*-*-darwin*) pvs_platform=macos;;
	*) AC_MSG_ERROR([PVS-Studio does not support $host.]);;
	esac

	# Create a basic PVS-Studio.cfg file
	cat > PVS-Studio.cfg <<-EOF
		preprocessor = $preprocessor
		platform = $pvs_platform
		analysis-mode = 4
		language = C
	EOF

	# Check for a license file in the default location
	if test -f "$HOME/.config/PVS-Studio/PVS-Studio.lic"; then
	    echo "lic-file = $HOME/.config/PVS-Studio/PVS-Studio.lic" >> PVS-Studio.cfg
	fi
    fi
])

AC_DEFUN([SUDO_CPP_VARIADIC_MACROS],
[AC_CACHE_CHECK([for variadic macro support in cpp],
[sudo_cv_cpp_variadic_macros], [
  sudo_cv_cpp_variadic_macros=yes
  if test X"$ac_cv_prog_cc_c99" = X"no"; then
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT
#if defined(__GNUC__) && __GNUC__ == 2
# define sudo_fprintf(fp, fmt...) fprintf((fp), (fmt))
#else
# define sudo_fprintf(fp, ...) fprintf((fp), __VA_ARGS__)
#endif], [sudo_fprintf(stderr, "a %s", "test");
    ])], [], [sudo_cv_cpp_variadic_macros=no])
  fi
  ])
  if test X"$sudo_cv_cpp_variadic_macros" = X"no"; then
    AC_DEFINE([NO_VARIADIC_MACROS], [1], [Define if your C preprocessor does not support variadic macros.])
    AC_MSG_WARN([your C preprocessor doesn't support variadic macros, debugging support will be limited])
    SUDO_APPEND_COMPAT_EXP(sudo_debug_printf_nvm_v1)
  fi
])

dnl
dnl private versions of AC_DEFINE and AC_DEFINE_UNQUOTED that don't support
dnl tracing that we use to define paths for pathnames.h so autoheader doesn't
dnl put them in config.h.in.  An awful hack.
dnl
m4_define([SUDO_DEFINE],
[cat >>confdefs.h <<\EOF
[@%:@define] $1 m4_if($#, 2, [$2], $#, 3, [$2], 1)
EOF
])

m4_define([SUDO_DEFINE_UNQUOTED],
[cat >>confdefs.h <<EOF
[@%:@define] $1 m4_if($#, 2, [$2], $#, 3, [$2], 1)
EOF
])

dnl
dnl Expand Makefile-style variables in $1 and store the result in $2.
dnl Used to expand file paths for use in man pages and pathnames.h.
dnl
AC_DEFUN([SUDO_EXPAND_PATH], [
    $2="$1"
    while :; do
	$2="`echo \"$$2\" | sed -e 's/(/{/g' -e 's/)/}/g'`"
	case "$$2" in
	*\${[[A-Za-z]]*}*)
	    eval $2="$$2"
	    ;;
	*)
	    break
	    ;;
    esac
done
case "$$2" in
    NONE/*)
        $2="${ac_default_prefix}${$2#NONE}"
        ;;
esac
])

dnl
dnl Expand Makefile-style variables in $1, a colon-separated list of paths,
dnl and define the result as a string using the name $2.
dnl
AC_DEFUN([SUDO_DEFINE_PATH], [
    as_save_IFS=$IFS
    IFS=:
    _sudo_define_path_res=
    for as_dir in $1; do
	SUDO_EXPAND_PATH([$as_dir], [_sudo_define_path_exp])
	if test -z "${_sudo_define_path_res}"; then
	    _sudo_define_path_res="${_sudo_define_path_exp}"
	else
	    _sudo_define_path_res="${_sudo_define_path_res}:${_sudo_define_path_exp}"
	fi
    done
    IFS=$as_save_IFS
    SUDO_DEFINE_UNQUOTED($2, "${_sudo_define_path_res}")
])
