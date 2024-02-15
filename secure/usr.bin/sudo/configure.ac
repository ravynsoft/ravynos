dnl
dnl Use the top-level autogen.sh script to generate configure and config.h.in
dnl
dnl SPDX-License-Identifier: ISC
dnl
dnl Copyright (c) 1994-1996, 1998-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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
AC_PREREQ([2.69])
AC_INIT([sudo], [1.9.15p5], [https://bugzilla.sudo.ws/], [sudo])
AC_CONFIG_HEADERS([config.h pathnames.h])
AC_CONFIG_SRCDIR([src/sudo.c])
AC_CONFIG_AUX_DIR([scripts])
dnl
dnl Variables that get substituted in the Makefile and man pages
dnl
AC_SUBST([PROGS], [sudo])dnl
AC_SUBST([SUDO_LDFLAGS])dnl
AC_SUBST([SUDOERS_LDFLAGS])dnl
AC_SUBST([LIBUTIL_LDFLAGS])dnl
AC_SUBST([ZLIB_LDFLAGS])dnl
AC_SUBST([LT_LDFLAGS])dnl
AC_SUBST([LT_LDDEP], ["\$(shlib_exp)"])dnl
AC_SUBST([LT_LDEXPORTS], ["-export-symbols \$(shlib_exp)"])dnl
AC_SUBST([LT_STATIC])dnl
AC_SUBST([LT_DEP_LIBS])dnl
AC_SUBST([COMMON_OBJS])dnl
AC_SUBST([SUDOERS_LT_STATIC])dnl
AC_SUBST([SUDOERS_OBJS])dnl
AC_SUBST([SUDO_OBJS])dnl
AC_SUBST([SUDO_LIBS])dnl
AC_SUBST([SUDOERS_LIBS])dnl
AC_SUBST([STATIC_SUDOERS])dnl
AC_SUBST([NET_LIBS])dnl
AC_SUBST([AFS_LIBS])dnl
AC_SUBST([REPLAY_LIBS])dnl
AC_SUBST([GETGROUPS_LIB])dnl
AC_SUBST([AUTH_OBJS])dnl
AC_SUBST([MANTYPE])dnl
AC_SUBST([MANDIRTYPE])dnl
AC_SUBST([MANCOMPRESS])dnl
AC_SUBST([MANCOMPRESSEXT])dnl
AC_SUBST([POSTINSTALL])dnl
AC_SUBST([SHLIB_ENABLE])dnl
AC_SUBST([SHLIB_MODE])dnl
AC_SUBST([SUDOERS_MODE])dnl
AC_SUBST([SUDOERS_UID])dnl
AC_SUBST([SUDOERS_GID])dnl
AC_SUBST([DEVEL])dnl
AC_SUBST([EXAMPLES])dnl
AC_SUBST([BAMAN], [0])dnl
AC_SUBST([LCMAN], [0])dnl
AC_SUBST([PSMAN], [0])dnl
AC_SUBST([SEMAN], [0])dnl
AC_SUBST([AAMAN], [0])dnl
AC_SUBST([devdir], ['$(srcdir)'])dnl
AC_SUBST([mansectsu])dnl
AC_SUBST([mansectform])dnl
AC_SUBST([mansectmisc])dnl
AC_SUBST([INTERCEPTFILE])dnl
AC_SUBST([INTERCEPTDIR])dnl
AC_SUBST([intercept_file])dnl
AC_SUBST([NOEXECFILE])dnl
AC_SUBST([NOEXECDIR])dnl
AC_SUBST([noexec_file])dnl
AC_SUBST([sesh_file])dnl
AC_SUBST([INSTALL_BACKUP])dnl
AC_SUBST([INSTALL_INTERCEPT])dnl
AC_SUBST([INSTALL_NOEXEC])dnl
AC_SUBST([PRELOAD_MODULE], ['-module'])dnl
AC_SUBST([DONT_LEAK_PATH_INFO])dnl
AC_SUBST([BSDAUTH_USAGE])dnl
AC_SUBST([SELINUX_USAGE])dnl
AC_SUBST([LDAP], ['#'])dnl
AC_SUBST([LOGINCAP_USAGE])dnl
AC_SUBST([ZLIB])dnl
AC_SUBST([ZLIB_SRC])dnl
AC_SUBST([LIBTOOL_DEPS])dnl
AC_SUBST([LIBDL])dnl
AC_SUBST([LIBRT])dnl
AC_SUBST([LIBINTL])dnl
AC_SUBST([LIBCRYPTO])dnl
AC_SUBST([LIBTLS])dnl
AC_SUBST([LIBPTHREAD])dnl
AC_SUBST([SUDO_NLS], [disabled])dnl
AC_SUBST([LOCALEDIR_SUFFIX])dnl
AC_SUBST([COMPAT_TEST_PROGS])dnl
AC_SUBST([SUDOERS_TEST_PROGS])dnl
AC_SUBST([CROSS_COMPILING])dnl
AC_SUBST([ASAN_LDFLAGS])dnl
AC_SUBST([ASAN_CFLAGS])dnl
AC_SUBST([PIE_LDFLAGS])dnl
AC_SUBST([PIE_CFLAGS])dnl
AC_SUBST([HARDENING_LDFLAGS])dnl
AC_SUBST([HARDENING_CFLAGS])dnl
AC_SUBST([INIT_SCRIPT])dnl
AC_SUBST([INIT_DIR])dnl
AC_SUBST([RC_LINK])dnl
AC_SUBST([COMPAT_EXP])dnl
AC_SUBST([TMPFILES_D])dnl
AC_SUBST([exampledir], ['$(docdir)/examples'])dnl
AC_SUBST([adminconfdir], ['$(prefix)/etc'])dnl
AC_SUBST([DIGEST])dnl
AC_SUBST([devsearch])dnl
AC_SUBST([SIGNAME])dnl
AC_SUBST([PYTHON_PLUGIN], ['#'])dnl
AC_SUBST([PYTHON_PLUGIN_SRC])dnl
AC_SUBST([SSL_COMPAT_SRC])dnl
AC_SUBST([LOGSRV])dnl
AC_SUBST([LOGSRV_SRC], ['lib/logsrv'])dnl
AC_SUBST([LOGSRVD_SRC], ['logsrvd'])dnl
AC_SUBST([LOGSRVD_CONF], ['sudo_logsrvd.conf'])dnl
AC_SUBST([LIBLOGSRV], ['$(top_builddir)/lib/logsrv/liblogsrv.la $(top_builddir)/lib/protobuf-c/libprotobuf-c.la'])dnl
AC_SUBST([PPFILES], ['$(srcdir)/etc/sudo.pp'])dnl
AC_SUBST([FUZZ_ENGINE])dnl
AC_SUBST([FUZZ_LD], ['$(CC)'])dnl
AC_SUBST([INTERCEPT_EXP])dnl
dnl
dnl Config file paths
dnl Either a single file or a colon-separated list of paths.
dnl
AC_SUBST([cvtsudoers_conf], ['$(sysconfdir)/cvtsudoers.conf'])dnl
AC_SUBST([sudo_conf], ['$(sysconfdir)/sudo.conf'])dnl
AC_SUBST([sudo_logsrvd_conf], ['$(sysconfdir)/sudo_logsrvd.conf'])dnl
AC_SUBST([sudoers_path], ['$(sysconfdir)/sudoers'])dnl
dnl
dnl Variables that get substituted in docs (not overridden by environment)
dnl
AC_SUBST([iolog_dir])dnl real initial value from SUDO_IO_LOGDIR
AC_SUBST([log_dir])dnl real initial value from SUDO_LOGDIR
AC_SUBST([logpath])dnl real initial value from SUDO_LOGFILE
AC_SUBST([relay_dir])dnl real initial value from SUDO_RELAY_DIR
AC_SUBST([rundir])dnl real initial value from SUDO_RUNDIR
AC_SUBST([vardir])dnl real initial value from SUDO_VARDIR
AC_SUBST([timeout])
AC_SUBST([password_timeout])
AC_SUBST([sudo_umask])
AC_SUBST([umask_override])
AC_SUBST([passprompt])
AC_SUBST([long_otp_prompt])
AC_SUBST([lecture])
AC_SUBST([logfac])
AC_SUBST([goodpri])
AC_SUBST([badpri])
AC_SUBST([loglen])
AC_SUBST([ignore_dot])
AC_SUBST([mail_no_user])
AC_SUBST([mail_no_host])
AC_SUBST([mail_no_perms])
AC_SUBST([mailto])
AC_SUBST([mailsub])
AC_SUBST([badpass_message])
AC_SUBST([fqdn])
AC_SUBST([runas_default])
AC_SUBST([env_editor])
AC_SUBST([env_reset])
AC_SUBST([passwd_tries])
AC_SUBST([timestamp_type])
AC_SUBST([insults])
AC_SUBST([root_sudo])
AC_SUBST([path_info])
AC_SUBST([ldap_conf])
AC_SUBST([ldap_secret])
AC_SUBST([sssd_lib])
AC_SUBST([nsswitch_conf])
AC_SUBST([netsvc_conf])
AC_SUBST([secure_path])
AC_SUBST([editor])
AC_SUBST([pam_session])
AC_SUBST([pam_login_service])
AC_SUBST([plugindir])
AC_SUBST([sudoers_plugin])
AC_SUBST([python_plugin])
#
# Begin initial values for man page substitution
#
iolog_dir=/var/log/sudo-io
log_dir=/var/log
logpath=/var/log/sudo.log
relay_dir=/var/log/sudo_logsrvd
rundir=/var/run/sudo
vardir=/var/adm/sudo
timeout=5
password_timeout=5
sudo_umask=0022
umask_override=off
passprompt="Password: "
long_otp_prompt=off
lecture=once
logfac=auth
goodpri=notice
badpri=alert
loglen=80
ignore_dot=off
mail_no_user=on
mail_no_host=off
mail_no_perms=off
mailto=root
mailsub="*** SECURITY information for %h ***"
badpass_message="Sorry, try again."
fqdn=off
runas_default=root
env_editor=on
env_reset=on
editor=vi
passwd_tries=3
timestamp_type=tty
insults=off
root_sudo=on
path_info=on
ldap_conf=/etc/ldap.conf
ldap_secret=/etc/ldap.secret
netsvc_conf=/etc/netsvc.conf
intercept_file="$libexecdir/sudo/sudo_intercept.so"
noexec_file="$libexecdir/sudo/sudo_noexec.so"
sesh_file="$libexecdir/sudo/sesh"
nsswitch_conf=/etc/nsswitch.conf
secure_path="not set"
pam_session=on
pam_login_service=sudo
plugindir="$libexecdir/sudo"
sudoers_plugin="sudoers.so"
python_plugin="python_plugin.so"
DIGEST=digest.lo
devsearch="/dev/pts:/dev/vt:/dev/term:/dev/zcons:/dev/pty:/dev"
#
# End initial values for man page substitution
#
dnl
dnl Initial values for Makefile variables listed above
dnl May be overridden by environment variables..
dnl
: ${MANDIRTYPE='man'}
: ${SHLIB_MODE='0644'}
: ${SUDOERS_MODE='0440'}
: ${SUDOERS_UID='0'}
: ${SUDOERS_GID='0'}
dnl
dnl Other variables
dnl
CONFIGURE_ARGS="$@"
AUTH_REG=
AUTH_EXCL=
AUTH_EXCL_DEF=
AUTH_DEF=passwd
CHECKSHADOW=true
shadow_funcs=
shadow_libs=
OS_INIT=os_init_common

dnl
dnl libc replacement functions live in libsudo_util.a
dnl
AC_CONFIG_LIBOBJ_DIR(lib/util)

dnl
dnl We must call AC_USE_SYSTEM_EXTENSIONS before the compiler is run.
dnl
AC_USE_SYSTEM_EXTENSIONS

#
# Prior to sudo 1.8.7, sudo stored libexec files in $libexecdir.
# Starting with sudo 1.8.7, $libexecdir/sudo is used so strip
# off an extraneous "/sudo" from libexecdir.
#
case "$libexecdir" in
    */sudo)
	AC_MSG_WARN([libexecdir should not include the "sudo" subdirectory])
	libexecdir=`expr "$libexecdir" : '\\(.*\\)/sudo$'`
	;;
esac

dnl
dnl Deprecated --with options (these all warn or generate an error)
dnl

AC_ARG_WITH(otp-only, [AS_HELP_STRING([--with-otp-only], [deprecated])],
[case $with_otp_only in
    yes)	with_passwd="no"
		AC_MSG_NOTICE([--with-otp-only option deprecated, treating as --without-passwd])
		;;
esac])

AC_ARG_WITH(alertmail, [AS_HELP_STRING([--with-alertmail], [deprecated])],
[case $with_alertmail in
    *)		with_mailto="$with_alertmail"
		AC_MSG_NOTICE([--with-alertmail option deprecated, treating as --mailto])
		;;
esac])

AC_ARG_WITH(pc-insults, [AS_HELP_STRING([--with-pc-insults], [deprecated])],
[case $with_pc_insults in
    yes)	enable_offensive_insults=no
		AC_MSG_NOTICE([--with-pc-insults option deprecated, it is now the default])
		;;
    no)		enable_offensive_insults=yes
		AC_MSG_NOTICE([--without-pc-insults option deprecated, use --enable-offensive-insults])
		;;
esac])

dnl
dnl Options for --with
dnl

AC_ARG_WITH(devel, [AS_HELP_STRING([--with-devel], [add development options])],
[case $with_devel in
    yes)	AC_MSG_NOTICE([setting up for development: -Wall, flex, yacc])
		AX_APPEND_FLAG([-DSUDO_DEVEL], [CPPFLAGS])
		DEVEL="true"
		devdir=.
		;;
    no)		;;
    *)		AC_MSG_WARN([ignoring unknown argument to --with-devel: $with_devel])
		;;
esac])

AC_ARG_WITH(CC, [AS_HELP_STRING([--with-CC], [C compiler to use])],
[case $with_CC in
    *)		AC_MSG_ERROR([the --with-CC option is no longer supported, please pass CC=$with_CC to configure instead.])
		;;
esac])

AC_ARG_WITH(rpath, [AS_HELP_STRING([--with-rpath], [deprecated, use --disable-rpath])],
[AC_MSG_WARN([--with-rpath deprecated, rpath is now the default])])

AC_ARG_WITH(blibpath, [AS_HELP_STRING([--with-blibpath[[=PATH]]], [deprecated])],
[AC_MSG_WARN([--with-blibpath deprecated, use --with-libpath])])

dnl
dnl Handle BSM auditing support.
dnl
AC_ARG_WITH(bsm-audit, [AS_HELP_STRING([--with-bsm-audit], [enable BSM audit support])],
[case $with_bsm_audit in
    yes)	AC_DEFINE(HAVE_BSM_AUDIT)
		SUDOERS_LIBS="${SUDOERS_LIBS} -lbsm"
		SUDOERS_OBJS="${SUDOERS_OBJS} bsm_audit.lo"
		;;
    no)		;;
    *)		AC_MSG_ERROR([--with-bsm-audit does not take an argument.])
		;;
esac])

dnl
dnl Handle Linux auditing support.
dnl
AC_ARG_WITH(linux-audit, [AS_HELP_STRING([--with-linux-audit], [enable Linux audit support])],
[case $with_linux_audit in
    yes)	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <libaudit.h>]], [[int i = AUDIT_USER_CMD; (void)i;]])], [
		    AC_DEFINE(HAVE_LINUX_AUDIT)
		    SUDO_LIBS="${SUDO_LIBS} -laudit"
		    SUDOERS_LIBS="${SUDO_LIBS} -laudit"
		    SUDOERS_OBJS="${SUDOERS_OBJS} linux_audit.lo"
		], [
		    AC_MSG_ERROR([unable to find AUDIT_USER_CMD in libaudit.h for --with-linux-audit])
		])
		;;
    no)		;;
    *)		AC_MSG_ERROR([--with-linux-audit does not take an argument.])
		;;
esac])

dnl
dnl Handle Solaris auditing support.
dnl
AC_ARG_WITH(solaris-audit, [AS_HELP_STRING([--with-solaris-audit], [enable Solaris audit support])],
[case $with_solaris_audit in
    yes)	AC_DEFINE(HAVE_SOLARIS_AUDIT)
		SUDOERS_LIBS="${SUDOERS_LIBS} -lbsm"
		SUDOERS_OBJS="${SUDOERS_OBJS} solaris_audit.lo"
		;;
    no)		;;
    *)		AC_MSG_ERROR([--with-solaris-audit does not take an argument.])
		;;
esac])

dnl
dnl Handle SSSD support.
dnl
AC_ARG_WITH(sssd, [AS_HELP_STRING([--with-sssd], [enable SSSD support])],
[case $with_sssd in
    yes)	SUDOERS_OBJS="${SUDOERS_OBJS} sssd.lo"
		case "$SUDOERS_OBJS" in
		    *ldap_util.lo*) ;;
		    *) SUDOERS_OBJS="${SUDOERS_OBJS} ldap_util.lo";;
		esac
		AC_DEFINE(HAVE_SSSD)
		;;
    no)		;;
    *)		AC_MSG_ERROR([--with-sssd does not take an argument.])
		;;
esac])

AC_ARG_WITH(sssd-conf, [AS_HELP_STRING([--with-sssd-conf], [path to the SSSD config file])])
sssd_conf="/etc/sssd/sssd.conf"
test -n "$with_sssd_conf" && sssd_conf="$with_sssd_conf"
SUDO_DEFINE_UNQUOTED(_PATH_SSSD_CONF, "$sssd_conf", [Path to the SSSD config file])

AC_ARG_WITH(sssd-lib, [AS_HELP_STRING([--with-sssd-lib], [path to the SSSD library])])
sssd_lib="\"LIBDIR\""
test -n "$with_sssd_lib" && sssd_lib="$with_sssd_lib"
SUDO_DEFINE_UNQUOTED(_PATH_SSSD_LIB, "$sssd_lib", [Path to the SSSD library])

AC_ARG_WITH(incpath, [AS_HELP_STRING([--with-incpath], [additional places to look for include files])],
[case $with_incpath in
    yes)	AC_MSG_ERROR([must give --with-incpath an argument.])
		;;
    no)		AC_MSG_ERROR([--without-incpath not supported.])
		;;
    *)		AC_MSG_NOTICE([adding ${with_incpath} to CPPFLAGS])
		for i in ${with_incpath}; do
		    AX_APPEND_FLAG([-I${i}], [CPPFLAGS])
		done
		;;
esac])

AC_ARG_WITH(libpath, [AS_HELP_STRING([--with-libpath], [additional places to look for libraries])],
[case $with_libpath in
    yes)	AC_MSG_ERROR([must give --with-libpath an argument.])
		;;
    no)		AC_MSG_ERROR([--without-libpath not supported.])
		;;
    *)		AC_MSG_NOTICE([adding ${with_libpath} to LDFLAGS])
		;;
esac])

AC_ARG_WITH(libraries, [AS_HELP_STRING([--with-libraries], [additional libraries to link with])],
[case $with_libraries in
    yes)	AC_MSG_ERROR([must give --with-libraries an argument.])
		;;
    no)		AC_MSG_ERROR([--without-libraries not supported.])
		;;
    *)		AC_MSG_NOTICE([adding ${with_libraries} to LIBS])
		;;
esac])

AC_ARG_WITH(csops, [AS_HELP_STRING([--with-csops], [add CSOps standard options])],
[case $with_csops in
    yes)	AC_MSG_NOTICE([adding CSOps standard options])
		CHECKSIA=false
		with_ignore_dot=yes
		insults=on
		with_classic_insults=yes
		with_csops_insults=yes
		with_env_editor=yes
		: ${mansectsu='8'}
		: ${mansectform='5'}
		: ${mansectmisc='7'}
		;;
    no)		;;
    *)		AC_MSG_WARN([ignoring unknown argument to --with-csops: $with_csops])
		;;
esac])

AC_ARG_WITH(passwd, [AS_HELP_STRING([--without-passwd], [don't use passwd/shadow file for authentication])],
[case $with_passwd in
    yes|no)	AUTH_DEF=""
		test "$with_passwd" = "yes" && AUTH_REG="$AUTH_REG passwd"
		;;
    *)		AC_MSG_ERROR([sorry, --with-passwd does not take an argument.])
		;;
esac])

AC_ARG_WITH(skey, [AS_HELP_STRING([--with-skey[[=DIR]]], [enable S/Key support ])],
[case $with_skey in
    no)		;;
    *)		AC_DEFINE(HAVE_SKEY)
		AUTH_REG="$AUTH_REG S/Key"
		;;
esac])

AC_ARG_WITH(opie, [AS_HELP_STRING([--with-opie[[=DIR]]], [enable OPIE support ])],
[case $with_opie in
    no)		;;
    *)		AC_DEFINE(HAVE_OPIE)
		AUTH_REG="$AUTH_REG NRL_OPIE"
		;;
esac])

AC_ARG_WITH(long-otp-prompt, [AS_HELP_STRING([--with-long-otp-prompt], [use a two line OTP (skey/opie) prompt])],
[case $with_long_otp_prompt in
    yes)	AC_DEFINE(LONG_OTP_PROMPT)
		long_otp_prompt=on
		;;
    no)		long_otp_prompt=off
		;;
    *)		AC_MSG_ERROR([--with-long-otp-prompt does not take an argument.])
		;;
esac])

AC_ARG_WITH(SecurID, [AS_HELP_STRING([--with-SecurID[[=DIR]]], [enable SecurID support])],
[case $with_SecurID in
    no)		;;
    *)		AC_DEFINE(HAVE_SECURID)
		AUTH_EXCL="$AUTH_EXCL SecurID"
		;;
esac])

AC_ARG_WITH(fwtk, [AS_HELP_STRING([--with-fwtk[[=DIR]]], [enable FWTK AuthSRV support])],
[case $with_fwtk in
    no)		;;
    *)		AC_DEFINE(HAVE_FWTK)
		AUTH_EXCL="$AUTH_EXCL FWTK"
		;;
esac])

AC_ARG_WITH(kerb5, [AS_HELP_STRING([--with-kerb5[[=DIR]]], [enable Kerberos V support])],
[case $with_kerb5 in
    no)		;;
    *)		AUTH_REG="$AUTH_REG kerb5"
		;;
esac])

AC_ARG_WITH(aixauth, [AS_HELP_STRING([--with-aixauth], [enable AIX general authentication support])],
[case $with_aixauth in
    yes)	AUTH_EXCL="$AUTH_EXCL AIX_AUTH";;
    no)		;;
    *)		AC_MSG_ERROR([--with-aixauth does not take an argument.])
		;;
esac])

AC_ARG_WITH(pam, [AS_HELP_STRING([--with-pam], [enable PAM support])],
[case $with_pam in
    yes)	AUTH_EXCL="$AUTH_EXCL PAM";;
    no)		;;
    *)		AC_MSG_ERROR([--with-pam does not take an argument.])
		;;
esac])

AC_ARG_WITH(AFS, [AS_HELP_STRING([--with-AFS], [enable AFS support])],
[case $with_AFS in
    yes)	AC_DEFINE(HAVE_AFS)
		AUTH_REG="$AUTH_REG AFS"
		;;
    no)		;;
    *)		AC_MSG_ERROR([--with-AFS does not take an argument.])
		;;
esac])

AC_ARG_WITH(DCE, [AS_HELP_STRING([--with-DCE], [enable DCE support])],
[case $with_DCE in
    yes)	AC_DEFINE(HAVE_DCE)
		AUTH_REG="$AUTH_REG DCE"
		;;
    no)		;;
    *)		AC_MSG_ERROR([--with-DCE does not take an argument.])
		;;
esac])

AC_ARG_WITH(logincap, [AS_HELP_STRING([--with-logincap], [enable BSD login class support])],
[case $with_logincap in
    yes|no)	;;
    *)		AC_MSG_ERROR([--with-logincap does not take an argument.])
		;;
esac])

AC_ARG_WITH(bsdauth, [AS_HELP_STRING([--with-bsdauth], [enable BSD authentication support])],
[case $with_bsdauth in
    yes)	AUTH_EXCL="$AUTH_EXCL BSD_AUTH";;
    no)		;;
    *)		AC_MSG_ERROR([--with-bsdauth does not take an argument.])
		;;
esac])

AC_ARG_WITH(project, [AS_HELP_STRING([--with-project], [enable Solaris project support])],
[case $with_project in
    yes|no)	;;
    no)		;;
    *)		AC_MSG_ERROR([--with-project does not take an argument.])
		;;
esac])

AC_ARG_WITH(lecture, [AS_HELP_STRING([--without-lecture], [don't print lecture for first-time sudoer])],
[case $with_lecture in
    yes|short|always)	lecture=once
			;;
    no|none|never)	lecture=never
			AC_DEFINE(NO_LECTURE)
			;;
    *)		AC_MSG_ERROR([unknown argument to --with-lecture: $with_lecture])
		;;
esac])

AC_ARG_WITH(logging, [AS_HELP_STRING([--with-logging], [log via syslog, file, or both])],
[case $with_logging in
    yes)	AC_MSG_ERROR([must give --with-logging an argument.])
		;;
    no)		AC_MSG_ERROR([--without-logging not supported.])
		;;
    syslog)	AC_DEFINE(LOGGING, SLOG_SYSLOG)
		;;
    file)	AC_DEFINE(LOGGING, SLOG_FILE)
		;;
    both)	AC_DEFINE(LOGGING, SLOG_BOTH)
		;;
    *)		AC_MSG_ERROR([unknown argument to --with-logging: $with_logging])
		;;
esac], [
    with_logging=syslog
    AC_DEFINE(LOGGING, SLOG_SYSLOG)
])

AC_ARG_WITH(logfac, [AS_HELP_STRING([--with-logfac], [syslog facility to log with (default is "auth")])],
[case $with_logfac in
    yes)	AC_MSG_ERROR([must give --with-logfac an argument.])
		;;
    no)		AC_MSG_ERROR([--without-logfac not supported.])
		;;
    authpriv|auth|daemon|user|local0|local1|local2|local3|local4|local5|local6|local7)		logfac=$with_logfac
		;;
    *)		AC_MSG_ERROR([$with_logfac is not a supported syslog facility.])
		;;
esac])

AC_ARG_WITH(goodpri, [AS_HELP_STRING([--with-goodpri], [syslog priority for commands (def is "notice")])],
[case $with_goodpri in
    yes)	AC_MSG_ERROR([must give --with-goodpri an argument.])
		;;
    no)		AC_MSG_ERROR([--without-goodpri not supported.])
		;;
    alert|crit|debug|emerg|err|info|notice|warning)
		goodpri=$with_goodpri
		;;
    *)		AC_MSG_ERROR([$with_goodpri is not a supported syslog priority.])
		;;
esac])
AC_DEFINE_UNQUOTED(PRI_SUCCESS, "$goodpri", [The syslog priority sudo will use for successful attempts.])

AC_ARG_WITH(badpri, [AS_HELP_STRING([--with-badpri], [syslog priority for failures (def is "alert")])],
[case $with_badpri in
    yes)	AC_MSG_ERROR([must give --with-badpri an argument.])
		;;
    no)		AC_MSG_ERROR([--without-badpri not supported.])
		;;
    alert|crit|debug|emerg|err|info|notice|warning)
		badpri=$with_badpri
		;;
    *)		AC_MSG_ERROR([$with_badpri is not a supported syslog priority.])
		;;
esac])
AC_DEFINE_UNQUOTED(PRI_FAILURE, "$badpri", [The syslog priority sudo will use for unsuccessful attempts/errors.])

AC_ARG_WITH(logpath, [AS_HELP_STRING([--with-logpath], [path to the sudo log file])],
[case $with_logpath in
    yes)	AC_MSG_ERROR([must give --with-logpath an argument.])
		;;
    no)		AC_MSG_ERROR([--without-logpath not supported.])
		;;
esac])

AC_ARG_WITH(loglen, [AS_HELP_STRING([--with-loglen], [maximum length of a log file line (default is 80)])],
[case $with_loglen in
    yes)	AC_MSG_ERROR([must give --with-loglen an argument.])
		;;
    no)		AC_MSG_ERROR([--without-loglen not supported.])
		;;
    [[0-9]]*)	loglen=$with_loglen
		;;
    *)		AC_MSG_ERROR([you must enter a number, not $with_loglen])
		;;
esac])
AC_DEFINE_UNQUOTED(MAXLOGFILELEN, $loglen, [The max number of chars per log file line (for line wrapping).])

AC_ARG_WITH(ignore-dot, [AS_HELP_STRING([--with-ignore-dot], [ignore '.' in the PATH])],
[case $with_ignore_dot in
    yes)	ignore_dot=on
		AC_DEFINE(IGNORE_DOT_PATH)
		;;
    no)		ignore_dot=off
		;;
    *)		AC_MSG_ERROR([--with-ignore-dot does not take an argument.])
		;;
esac])

AC_ARG_WITH(mail-if-no-user, [AS_HELP_STRING([--without-mail-if-no-user], [do not send mail if user not in sudoers])],
[case $with_mail_if_no_user in
    yes)	mail_no_user=on
		;;
    no)		mail_no_user=off
		;;
    *)		AC_MSG_ERROR([--with-mail-if-no-user does not take an argument.])
		;;
esac])
AS_IF([test "$mail_no_user" = "on"], [AC_DEFINE(SEND_MAIL_WHEN_NO_USER)])

AC_ARG_WITH(mail-if-no-host, [AS_HELP_STRING([--with-mail-if-no-host], [send mail if user in sudoers but not for this host])],
[case $with_mail_if_no_host in
    yes)	mail_no_host=on
		AC_DEFINE(SEND_MAIL_WHEN_NO_HOST)
		;;
    no)		mail_no_host=off
		;;
    *)		AC_MSG_ERROR([--with-mail-if-no-host does not take an argument.])
		;;
esac])

AC_ARG_WITH(mail-if-noperms, [AS_HELP_STRING([--with-mail-if-noperms], [send mail if user not allowed to run command])],
[case $with_mail_if_noperms in
    yes)	mail_noperms=on
		AC_DEFINE(SEND_MAIL_WHEN_NOT_OK)
		;;
    no)		mail_noperms=off
		;;
    *)		AC_MSG_ERROR([--with-mail-if-noperms does not take an argument.])
		;;
esac])

AC_ARG_WITH(mailto, [AS_HELP_STRING([--with-mailto], [who should get sudo mail (default is "root")])],
[case $with_mailto in
    yes)	AC_MSG_ERROR([must give --with-mailto an argument.])
		;;
    no)		AC_MSG_ERROR([--without-mailto not supported.])
		;;
    *)		mailto=$with_mailto
		;;
esac])
AC_DEFINE_UNQUOTED(MAILTO, "$mailto", [The user or email address that sudo mail is sent to.])

AC_ARG_WITH(mailsubject, [AS_HELP_STRING([--with-mailsubject], [subject of sudo mail])],
[case $with_mailsubject in
    yes)	AC_MSG_ERROR([must give --with-mailsubject an argument.])
		;;
    no)		AC_MSG_WARN([sorry, --without-mailsubject not supported.])
		;;
    *)		mailsub="$with_mailsubject"
		;;
esac])
AC_DEFINE_UNQUOTED(MAILSUBJECT, "$mailsub", [The subject of the mail sent by sudo to the MAILTO user/address.])

AC_ARG_WITH(passprompt, [AS_HELP_STRING([--with-passprompt], [default password prompt])],
[case $with_passprompt in
    yes)	AC_MSG_ERROR([must give --with-passprompt an argument.])
		;;
    no)		AC_MSG_WARN([sorry, --without-passprompt not supported.])
		;;
    *)		passprompt="$with_passprompt"
esac])
AC_DEFINE_UNQUOTED(PASSPROMPT, "$passprompt", [The default password prompt.])

AC_ARG_WITH(badpass-message, [AS_HELP_STRING([--with-badpass-message], [message the user sees when the password is wrong])],
[case $with_badpass_message in
    yes)	AC_MSG_ERROR([must give --with-badpass-message an argument.])
		;;
    no)		AC_MSG_WARN([sorry, --without-badpass-message not supported.])
		;;
    *)		badpass_message="$with_badpass_message"
		;;
esac])
AC_DEFINE_UNQUOTED(INCORRECT_PASSWORD, "$badpass_message", [The message given when a bad password is entered.])

AC_ARG_WITH(fqdn, [AS_HELP_STRING([--with-fqdn], [expect fully qualified hosts in sudoers])],
[case $with_fqdn in
    yes)	fqdn=on
		AC_DEFINE(FQDN)
		;;
    no)		fqdn=off
		;;
    *)		AC_MSG_ERROR([--with-fqdn does not take an argument.])
		;;
esac])

AC_ARG_WITH(timedir, [AS_HELP_STRING([--with-timedir=DIR], [deprecated])],
[case $with_timedir in
    *)		AC_MSG_ERROR([--without-timedir no longer supported, see --with-rundir.])
		;;
esac])

AC_ARG_WITH(rundir, [AS_HELP_STRING([--with-rundir=DIR], [directory for sudo-specific files that do not survive a system reboot, e.g. '/var/run/sudo'])],
[case $with_rundir in
    yes)	AC_MSG_ERROR([must give --with-rundir an argument.])
		;;
    no)		AC_MSG_ERROR([--without-rundir not supported.])
		;;
esac])

AC_ARG_WITH(vardir, [AS_HELP_STRING([--with-vardir=DIR], [directory for sudo-specific files that survive a system reboot, e.g. '/var/db/sudo' or '/var/lib/sudo'])],
[case $with_vardir in
    yes)	AC_MSG_ERROR([must give --with-vardir an argument.])
		;;
    no)		AC_MSG_ERROR([--without-vardir not supported.])
		;;
esac])

AC_ARG_WITH(iologdir, [AS_HELP_STRING([--with-iologdir=DIR], [directory to store sudo I/O log files in])],
[case $with_iologdir in
    yes)    ;;
    no)     AC_MSG_ERROR([--without-iologdir not supported.])
	    ;;
esac])

AC_ARG_WITH(relaydir, [AS_HELP_STRING([--with-relaydir=DIR], [directory to store sudo_logsrvd relay temporary files in])],
[case $with_relaydir in
    yes)    ;;
    no)     AC_MSG_ERROR([--without-relaydir not supported.])
	    ;;
esac])

AC_ARG_WITH(tzdir, [AS_HELP_STRING([--with-tzdir=DIR], [path to the time zone data directory])],
[case $with_tzdir in
    yes)	AC_MSG_ERROR([must give --with-tzdir an argument.])
		;;
esac])

AC_ARG_WITH(sendmail, [AS_HELP_STRING([--with-sendmail], [set path to sendmail])
AS_HELP_STRING([--without-sendmail], [do not send mail at all])],
[case $with_sendmail in
    yes)	with_sendmail=""
		;;
    no)		;;
    *)		SUDO_DEFINE_UNQUOTED(_PATH_SUDO_SENDMAIL, "$with_sendmail")
		;;
esac])

AC_ARG_WITH(sudoers-mode, [AS_HELP_STRING([--with-sudoers-mode], [mode of sudoers file (defaults to 0440)])],
[case $with_sudoers_mode in
    yes)	AC_MSG_ERROR([must give --with-sudoers-mode an argument.])
		;;
    no)		AC_MSG_ERROR([--without-sudoers-mode not supported.])
		;;
    [[1-9]]*)	SUDOERS_MODE=0${with_sudoers_mode}
		;;
    0*)		SUDOERS_MODE=$with_sudoers_mode
		;;
    *)		AC_MSG_ERROR([you must use an octal mode, not a name.])
		;;
esac])

AC_ARG_WITH(sudoers-uid, [AS_HELP_STRING([--with-sudoers-uid], [uid that owns sudoers file (defaults to 0)])],
[case $with_sudoers_uid in
    yes)	AC_MSG_ERROR([must give --with-sudoers-uid an argument.])
		;;
    no)		AC_MSG_ERROR([--without-sudoers-uid not supported.])
		;;
    [[0-9]]*)	SUDOERS_UID=$with_sudoers_uid
		;;
    *)		AC_MSG_ERROR([you must use an unsigned numeric uid, not a name.])
		;;
esac])

AC_ARG_WITH(sudoers-gid, [AS_HELP_STRING([--with-sudoers-gid], [gid that owns sudoers file (defaults to 0)])],
[case $with_sudoers_gid in
    yes)	AC_MSG_ERROR([must give --with-sudoers-gid an argument.])
		;;
    no)		AC_MSG_ERROR([--without-sudoers-gid not supported.])
		;;
    [[0-9]]*)	SUDOERS_GID=$with_sudoers_gid
		;;
    *)		AC_MSG_ERROR([you must use an unsigned numeric gid, not a name.])
		;;
esac])

AC_ARG_WITH(umask, [AS_HELP_STRING([--with-umask], [umask with which the prog should run (default is 022)])
AS_HELP_STRING([--without-umask], [Preserves the umask of the user invoking sudo.])],
[case $with_umask in
    yes)	AC_MSG_ERROR([must give --with-umask an argument.])
		;;
    no)		sudo_umask=0777
		;;
    [[0-9]]*)	sudo_umask=$with_umask
		;;
    *)		AC_MSG_ERROR([you must enter a numeric mask.])
		;;
esac])
AC_DEFINE_UNQUOTED(SUDO_UMASK, $sudo_umask, [The umask that the sudo-run prog should use.])

AC_ARG_WITH(umask-override, [AS_HELP_STRING([--with-umask-override], [Use the umask specified in sudoers even if it is less restrictive than the user's.])],
[case $with_umask_override in
    yes)	AC_DEFINE(UMASK_OVERRIDE)
		umask_override=on
		;;
    no)		umask_override=off
		;;
    *)		AC_MSG_ERROR([--with-umask-override does not take an argument.])
		;;
esac])

AC_ARG_WITH(runas-default, [AS_HELP_STRING([--with-runas-default], [User to run commands as (default is "root")])],
[case $with_runas_default in
    yes)	AC_MSG_ERROR([must give --with-runas-default an argument.])
		;;
    no)		AC_MSG_ERROR([--without-runas-default not supported.])
		;;
    *)		runas_default="$with_runas_default"
		;;
esac])
AC_DEFINE_UNQUOTED(RUNAS_DEFAULT, "$runas_default", [The user sudo should run commands as by default.])

AC_ARG_WITH(exempt, [AS_HELP_STRING([--with-exempt=group], [no passwd needed for users in this group])],
[case $with_exempt in
    yes)	AC_MSG_ERROR([must give --with-exempt an argument.])
		;;
    no)		AC_MSG_ERROR([--without-exempt not supported.])
		;;
    *)		AC_DEFINE_UNQUOTED(EXEMPTGROUP, "$with_exempt", [If defined, users in this group need not enter a passwd (ie "sudo").])
		;;
esac])

AC_ARG_WITH(editor, [AS_HELP_STRING([--with-editor=path], [Default editor for visudo (defaults to vi)])],
[case $with_editor in
    yes)	AC_MSG_ERROR([must give --with-editor an argument.])
		;;
    no)		AC_MSG_ERROR([--without-editor not supported.])
		;;
    *)		AC_DEFINE_UNQUOTED(EDITOR, "$with_editor", [A colon-separated list of pathnames to be used as the editor for visudo.])
		editor="$with_editor"
		;;
esac], [AC_DEFINE(EDITOR, _PATH_VI)])

AC_ARG_WITH(env-editor, [AS_HELP_STRING([--with-env-editor], [Use the environment variable EDITOR for visudo])],
[case $with_env_editor in
    yes)	env_editor=on
		;;
    no)		env_editor=off
		;;
    *)		AC_MSG_ERROR([--with-env-editor does not take an argument.])
		;;
esac])
AS_IF([test "$env_editor" = "on"], [AC_DEFINE(ENV_EDITOR)])

AC_ARG_WITH(passwd-tries, [AS_HELP_STRING([--with-passwd-tries], [number of tries to enter password (default is 3)])],
[case $with_passwd_tries in
    yes)	;;
    no)		AC_MSG_ERROR([--without-editor not supported.])
		;;
    [[1-9]]*)	passwd_tries=$with_passwd_tries
		;;
    *)		AC_MSG_ERROR([you must enter the number of tries, > 0])
		;;
esac])
AC_DEFINE_UNQUOTED(TRIES_FOR_PASSWORD, $passwd_tries, [The number of tries a user gets to enter their password.])

AC_ARG_WITH(timeout, [AS_HELP_STRING([--with-timeout], [minutes before sudo asks for passwd again (def is 5 minutes)])],
[case $with_timeout in
    yes)	;;
    no)		timeout=0
		;;
    [[0-9]]*)	timeout=$with_timeout
		;;
    *)		AC_MSG_ERROR([you must enter the number of minutes.])
		;;
esac])
AC_DEFINE_UNQUOTED(TIMEOUT, $timeout, [The number of minutes before sudo asks for a password again.])

AC_ARG_WITH(password-timeout, [AS_HELP_STRING([--with-password-timeout], [passwd prompt timeout in minutes (default is 5 minutes)])],
[case $with_password_timeout in
    yes)	;;
    no)		password_timeout=0
		;;
    [[0-9]]*)	password_timeout=$with_password_timeout
		;;
    *)		AC_MSG_ERROR([you must enter the number of minutes.])
		;;
esac])
AC_DEFINE_UNQUOTED(PASSWORD_TIMEOUT, $password_timeout, [The passwd prompt timeout (in minutes).])

AC_ARG_WITH(tty-tickets, [AS_HELP_STRING([--with-tty-tickets], [use a different ticket file for each tty])],
[case $with_tty_tickets in
    yes)	timestamp_type=tty
		;;
    no)		timestamp_type=global
		;;
    *)		AC_MSG_ERROR([--with-tty-tickets does not take an argument.])
		;;
esac])

AC_ARG_WITH(insults, [AS_HELP_STRING([--with-insults], [insult the user for entering an incorrect password])],
[case $with_insults in
    yes)	insults=on
		AC_DEFINE(USE_INSULTS)
		with_classic_insults=yes
		with_csops_insults=yes
		;;
    disabled)	insults=off
		with_classic_insults=yes
		with_csops_insults=yes
		;;
    no)		insults=off
		;;
    *)		AC_MSG_ERROR([--with-insults does not take an argument.])
		;;
esac])

AC_ARG_WITH(all-insults, [AS_HELP_STRING([--with-all-insults], [include all the sudo insult sets])],
[case $with_all_insults in
    yes)	with_classic_insults=yes
		with_csops_insults=yes
		with_hal_insults=yes
		with_goons_insults=yes
		with_python_insults=yes
		;;
    no)		;;
    *)		AC_MSG_ERROR([--with-all-insults does not take an argument.])
		;;
esac])

AC_ARG_WITH(classic-insults, [AS_HELP_STRING([--with-classic-insults], [include the insults from the "classic" sudo])],
[case $with_classic_insults in
    yes)	AC_DEFINE(CLASSIC_INSULTS)
		;;
    no)		;;
    *)		AC_MSG_ERROR([--with-classic-insults does not take an argument.])
		;;
esac])

AC_ARG_WITH(csops-insults, [AS_HELP_STRING([--with-csops-insults], [include CSOps insults])],
[case $with_csops_insults in
    yes)	AC_DEFINE(CSOPS_INSULTS)
		;;
    no)		;;
    *)		AC_MSG_ERROR([--with-csops-insults does not take an argument.])
		;;
esac])

AC_ARG_WITH(hal-insults, [AS_HELP_STRING([--with-hal-insults], [include 2001-like insults])],
[case $with_hal_insults in
    yes)	AC_DEFINE(HAL_INSULTS)
		;;
    no)		;;
    *)		AC_MSG_ERROR([--with-hal-insults does not take an argument.])
		;;
esac])

AC_ARG_WITH(goons-insults, [AS_HELP_STRING([--with-goons-insults], [include the insults from the "Goon Show"])],
[case $with_goons_insults in
    yes)	AC_DEFINE(GOONS_INSULTS)
		;;
    no)		;;
    *)		AC_MSG_ERROR([--with-goons-insults does not take an argument.])
		;;
esac])

AC_ARG_WITH(python-insults, [AS_HELP_STRING([--with-python-insults], [include the insults from "Monty Python's Flying Circus"])],
[case $with_python_insults in
    yes)	AC_DEFINE(PYTHON_INSULTS)
        ;;
    no)		;;
    *)		AC_MSG_ERROR([--with-python-insults does not take an argument.])
        ;;
esac])

AC_ARG_WITH(nsswitch, [AS_HELP_STRING([--with-nsswitch[[=PATH]]], [path to nsswitch.conf])],
[case $with_nsswitch in
    no)		;;
    yes)	with_nsswitch="/etc/nsswitch.conf"
		;;
    *)		;;
esac])

AC_ARG_WITH(ldap, [AS_HELP_STRING([--with-ldap[[=DIR]]], [enable LDAP support])],
[case $with_ldap in
    no)		;;
    *)		AC_DEFINE(HAVE_LDAP)
		;;
esac])

AC_ARG_WITH(ldap-conf-file, [AS_HELP_STRING([--with-ldap-conf-file], [path to LDAP configuration file])])
test -n "$with_ldap_conf_file" && ldap_conf="$with_ldap_conf_file"
SUDO_DEFINE_UNQUOTED(_PATH_LDAP_CONF, "$ldap_conf", [Path to the ldap.conf file])

AC_ARG_WITH(ldap-secret-file, [AS_HELP_STRING([--with-ldap-secret-file], [path to LDAP secret password file])])
test -n "$with_ldap_secret_file" && ldap_secret="$with_ldap_secret_file"
SUDO_DEFINE_UNQUOTED(_PATH_LDAP_SECRET, "$ldap_secret", [Path to the ldap.secret file])

AC_ARG_WITH(secure-path, [AS_HELP_STRING([--with-secure-path], [override the user's path with a built-in one])],
[case $with_secure_path in
    yes)	with_secure_path="/bin:/usr/ucb:/usr/bin:/usr/sbin:/sbin:/usr/etc:/etc"
		AC_DEFINE_UNQUOTED(SECURE_PATH, "$with_secure_path")
		secure_path="set to $with_secure_path"
		;;
    no)		;;
    *)		AC_DEFINE_UNQUOTED(SECURE_PATH, "$with_secure_path")
		secure_path="set to $with_secure_path"
		;;
esac])

AC_ARG_WITH(interfaces, [AS_HELP_STRING([--without-interfaces], [don't try to read the ip addr of network interfaces])],
[case $with_interfaces in
    yes)	;;
    no)		AC_DEFINE(STUB_LOAD_INTERFACES)
		;;
    *)		AC_MSG_ERROR([--with-interfaces does not take an argument.])
		;;
esac])

AC_ARG_WITH(askpass, [AS_HELP_STRING([--with-askpass=PATH], [Fully qualified pathname of askpass helper])],
[case $with_askpass in
    yes)	AC_MSG_ERROR([--with-askpass takes a path as an argument.])
		;;
    no)		;;
    *)		;;
esac], [
    with_askpass=no
])
AS_IF([test X"$with_askpass" != X"no"], [
    SUDO_DEFINE_UNQUOTED(_PATH_SUDO_ASKPASS, "$with_askpass")
], [
    SUDO_DEFINE_UNQUOTED(_PATH_SUDO_ASKPASS, NULL)
])

AC_ARG_WITH(exampledir, [AS_HELP_STRING([--with-exampledir=DIR], [path to install sudo examples in])],
[case $with_exampledir in
    yes)	AC_MSG_ERROR([must give --with-exampledir an argument.])
		;;
    no)		AC_MSG_ERROR([--without-exampledir not supported.])
		;;
    *)		exampledir="$with_exampledir"
esac])

AC_ARG_WITH(plugindir, [AS_HELP_STRING([--with-plugindir=DIR], [set directory to load plugins from])],
[case $with_plugindir in
    yes)	AC_MSG_ERROR([must give --with-plugindir an argument.])
		;;
    no)		AC_MSG_ERROR([--without-plugindir not supported.])
		;;
    *)		plugindir="$with_plugindir"
		;;
esac])

AC_ARG_WITH(man, [AS_HELP_STRING([--with-man], [manual pages use man macros])],
[case $with_man in
    yes)	MANTYPE=man
		;;
    no)		AC_MSG_ERROR([--without-man not supported.])
		;;
    *)		AC_MSG_WARN([ignoring unknown argument to --with-man: $with_man.])
		;;
esac])

AC_ARG_WITH(mdoc, [AS_HELP_STRING([--with-mdoc], [manual pages use mdoc macros])],
[case $with_mdoc in
    yes)	MANTYPE=mdoc
		;;
    no)		AC_MSG_ERROR([--without-mdoc not supported.])
		;;
    *)		AC_MSG_WARN([ignoring unknown argument to --with-mdoc: $with_mdoc.])
		;;
esac])

dnl
dnl Options for --enable
dnl

AC_ARG_ENABLE(authentication,
[AS_HELP_STRING([--disable-authentication], [Do not require authentication by default])],
[ case "$enableval" in
    yes)	;;
    no)		AC_DEFINE(NO_AUTHENTICATION)
		;;
    *)		AC_MSG_WARN([ignoring unknown argument to --enable-authentication: $enableval])
		;;
  esac
])

AC_ARG_ENABLE(root-mailer,
[AS_HELP_STRING([--disable-root-mailer], [Don't run the mailer as root, run as the user])],
[ case "$enableval" in
    yes)	;;
    no)		AC_DEFINE(NO_ROOT_MAILER)
		;;
    *)		AC_MSG_WARN([ignoring unknown argument to --enable-root-mailer: $enableval])
		;;
  esac
])

AC_ARG_ENABLE(setreuid,
[AS_HELP_STRING([--disable-setreuid], [Don't try to use the setreuid() function])],
[ case "$enableval" in
    no)		SKIP_SETREUID=yes
		;;
    *)		;;
  esac
])

AC_ARG_ENABLE(setresuid,
[AS_HELP_STRING([--disable-setresuid], [Don't try to use the setresuid() function])],
[ case "$enableval" in
    no)		SKIP_SETRESUID=yes
		;;
    *)		;;
  esac
])

AC_ARG_ENABLE(shadow,
[AS_HELP_STRING([--disable-shadow], [Never use shadow passwords])],
[ case "$enableval" in
    yes)	;;
    no)		CHECKSHADOW="false"
		;;
    *)		AC_MSG_WARN([ignoring unknown argument to --enable-shadow: $enableval])
		;;
  esac
])

AC_ARG_ENABLE(root-sudo,
[AS_HELP_STRING([--disable-root-sudo], [Don't allow root to run sudo])],
[ case "$enableval" in
    yes)	;;
    no)		AC_DEFINE(NO_ROOT_SUDO)
		root_sudo=off
		;;
    *)		AC_MSG_ERROR([--enable-root-sudo does not take an argument.])
		;;
  esac
])

AC_ARG_ENABLE(log-host,
[AS_HELP_STRING([--enable-log-host], [Log the hostname in the log file])],
[ case "$enableval" in
    yes)	AC_DEFINE(HOST_IN_LOG)
		;;
    no)		;;
    *)		AC_MSG_WARN([ignoring unknown argument to --enable-log-host: $enableval])
		;;
  esac
])

AC_ARG_ENABLE(noargs-shell,
[AS_HELP_STRING([--enable-noargs-shell], [If sudo is given no arguments run a shell])],
[ case "$enableval" in
    yes)	AC_DEFINE(SHELL_IF_NO_ARGS)
		;;
    no)		;;
    *)		AC_MSG_WARN([ignoring unknown argument to --enable-noargs-shell: $enableval])
		;;
  esac
])

AC_ARG_ENABLE(shell-sets-home,
[AS_HELP_STRING([--enable-shell-sets-home], [Set $HOME to target user in shell mode])],
[ case "$enableval" in
    yes)	AC_DEFINE(SHELL_SETS_HOME)
		;;
    no)		;;
    *)		AC_MSG_WARN([ignoring unknown argument to --enable-shell-sets-home: $enableval])
		;;
  esac
])

AC_ARG_ENABLE(path_info,
[AS_HELP_STRING([--disable-path-info], [Print 'command not allowed' not 'command not found'])],
[ case "$enableval" in
    yes)	;;
    no)		AC_DEFINE(DONT_LEAK_PATH_INFO)
		path_info=off
		;;
    *)		AC_MSG_WARN([ignoring unknown argument to --enable-path-info: $enableval])
		;;
  esac
])

AC_ARG_ENABLE(env_debug,
[AS_HELP_STRING([--enable-env-debug], [Whether to enable environment debugging.])],
[ case "$enableval" in
    yes)	AC_DEFINE(ENV_DEBUG)
		;;
    no)		;;
    *)		AC_MSG_WARN([ignoring unknown argument to --enable-env-debug: $enableval])
		;;
  esac
])

AC_ARG_ENABLE(postinstall,
[AS_HELP_STRING([--enable-postinstall], [Script to run after the install phase])],
[ case "$enableval" in
    yes)	AC_MSG_ERROR([must give --enable-postinstall an argument.])
		;;
    no)		AC_MSG_ERROR([--enable-postinstall not supported.])
		;;
    *)		POSTINSTALL="$enableval"
		;;
  esac
])

AC_ARG_ENABLE(zlib,
[AS_HELP_STRING([--enable-zlib[[=PATH]]], [Whether to enable or disable zlib])],
[], [enable_zlib=yes])
AX_APPEND_FLAG([-DZLIB_CONST], [CPPFLAGS])

AC_ARG_ENABLE(env_reset,
[AS_HELP_STRING([--enable-env-reset], [Whether to enable environment resetting by default.])],
[ case "$enableval" in
    yes)	env_reset=on
		;;
    no)		env_reset=off
		;;
    *)		env_reset=on
    		AC_MSG_WARN([ignoring unknown argument to --enable-env-reset: $enableval])
		;;
  esac
])
AS_IF([test "$env_reset" = "on"], [
    AC_DEFINE(ENV_RESET, 1)
], [
    AC_DEFINE(ENV_RESET, 0)
])

AC_ARG_ENABLE(warnings,
[AS_HELP_STRING([--enable-warnings], [Whether to enable compiler warnings])],
[ case "$enableval" in
    yes)    ;;
    no)	    ;;
    *)	    AC_MSG_WARN([ignoring unknown argument to --enable-warnings: $enableval])
	    ;;
  esac
])

AC_ARG_ENABLE(werror,
[AS_HELP_STRING([--enable-werror], [Whether to enable the -Werror compiler option])],
[ case "$enableval" in
    yes)    ;;
    no)	    ;;
    *)	    AC_MSG_WARN([ignoring unknown argument to --enable-werror: $enableval])
	    ;;
  esac
])

AC_ARG_ENABLE(ssp,
[AS_HELP_STRING([--disable-ssp], [Do not compile using the -fstack-protector option.])],
[], [enable_ssp=yes])

AC_ARG_ENABLE(hardening,
[AS_HELP_STRING([--disable-hardening], [Do not use compiler/linker exploit mitigation options])],
[], [enable_hardening=yes])

AC_ARG_ENABLE(pie,
[AS_HELP_STRING([--enable-pie], [Build sudo as a position independent executable.])])

AC_ARG_ENABLE(sanitizer,
[AS_HELP_STRING([--enable-sanitizer], [Build sudo with sanitizer support.])], [
    AS_IF([test X"$enable_sanitizer" = X"yes"], [
	enable_sanitizer="-fsanitize=address,undefined"
    ])
], [enable_sanitizer=no])

AC_ARG_ENABLE(fuzzer,
[AS_HELP_STRING([--enable-fuzzer], [Build sudo with LLVM libFuzzer support.])],
[], [enable_fuzzer=no])

AC_ARG_ENABLE(fuzzer-engine,
[AS_HELP_STRING([--enable-fuzzer-engine], [Link fuzz targets with the specified fuzzer engine instead of the default.])],
[ case "$enableval" in
    yes)    AC_MSG_ERROR([must give --enable-fuzzer-engine an argument.])
	    ;;
    no)	    ;;
    *)	    FUZZ_ENGINE="$enableval"
	    ;;
  esac
])

AC_ARG_ENABLE(fuzzer-linker,
[AS_HELP_STRING([--enable-fuzzer-linker], [Use the specified linker when building fuzz targets instead of the default C compiler.])],
[ case "$enableval" in
    yes)    AC_MSG_ERROR([must give --enable-fuzzer-linker an argument.])
	    ;;
    no)	    ;;
    *)	    FUZZ_LD="$enableval"
	    ;;
  esac
])

AC_ARG_ENABLE(leaks,
[AS_HELP_STRING([--disable-leaks], [Prevent some harmless memory leaks.])],
[ case "$enableval" in
    yes)    ;;
    no)	    AC_DEFINE(NO_LEAKS)
	    ;;
    *)	    AC_MSG_WARN([ignoring unknown argument to --disable-leaks: $enableval])
	    ;;
  esac
])

AC_ARG_ENABLE(poll,
[AS_HELP_STRING([--disable-poll], [Use select() instead of poll().])])

AC_ARG_ENABLE(admin-flag,
[AS_HELP_STRING([--enable-admin-flag[[=PATH]]], [Whether to create a Ubuntu-style admin flag file])],
[ case "$enableval" in
    yes)    SUDO_DEFINE_UNQUOTED(_PATH_SUDO_ADMIN_FLAG, "~/.sudo_as_admin_successful")
	    ;;
    no)	    ;;
    *)	    SUDO_DEFINE_UNQUOTED(_PATH_SUDO_ADMIN_FLAG, "$enableval")
	    ;;
  esac
])

AC_ARG_ENABLE(nls,
[AS_HELP_STRING([--disable-nls], [Disable natural language support using gettext])],
[], [enable_nls=yes])

AC_ARG_ENABLE(rpath,
[AS_HELP_STRING([--disable-rpath], [Disable passing of -Rpath to the linker])],
[], [enable_rpath=yes])

AC_ARG_ENABLE(static-sudoers,
[AS_HELP_STRING([--enable-static-sudoers], [Build the sudoers policy module as part of the sudo binary instead as a plugin])],
[], [enable_static_sudoers=no])

AC_ARG_ENABLE(shared_libutil,
[AS_HELP_STRING([--disable-shared-libutil], [Disable use of the libsudo_util shared library.])],
[], [enable_shared_libutil=yes])

AC_ARG_ENABLE(tmpfiles.d,
[AS_HELP_STRING([--enable-tmpfiles.d=DIR], [Set the path to the systemd tmpfiles.d directory.])],
[case $enableval in
    yes)	TMPFILES_D=/usr/lib/tmpfiles.d
		;;
    no)		TMPFILES_D=
		;;
    *)		TMPFILES_D="$enableval"
esac], [
    test -f /usr/lib/tmpfiles.d/systemd.conf && TMPFILES_D=/usr/lib/tmpfiles.d
])

AC_ARG_ENABLE(devsearch,
[AS_HELP_STRING([--enable-devsearch=PATH], [The colon-delimited path to search for device nodes when determining the tty name.])],
[case $enableval in
    yes)	# use default value
		;;
    no)		AC_MSG_WARN([ignoring attempt to disable the device search path])
		;;
    *)		devsearch="$enableval"
		;;
esac])
ds="`echo \"$devsearch\"|sed 's@/dev/*\([[^:]]*:*\)@_PATH_DEV \"\1\" @g'`"
SUDO_DEFINE_UNQUOTED(_PATH_SUDO_DEVSEARCH, $ds)

AC_ARG_WITH(selinux, [AS_HELP_STRING([--with-selinux], [enable SELinux support])],
[case $with_selinux in
    yes)	SELINUX_USAGE="[[-r role]] [[-t type]] "
    		AC_DEFINE(HAVE_SELINUX)
		SUDO_LIBS="${SUDO_LIBS} -lselinux"
		SUDO_OBJS="${SUDO_OBJS} selinux.o"
		PROGS="${PROGS} sesh"
		SEMAN=1
		AC_CHECK_LIB([selinux], [setkeycreatecon],
		    [AC_DEFINE(HAVE_SETKEYCREATECON)])
		;;
    no)		;;
    *)		AC_MSG_ERROR([--with-selinux does not take an argument.])
		;;
esac], [with_selinux=no])

AC_ARG_WITH(apparmor, [AS_HELP_STRING([--with-apparmor], [enable AppArmor support])],
[case $with_apparmor in
    yes)        AC_DEFINE(HAVE_APPARMOR)
                AAMAN=1
		SUDO_LIBS="${SUDO_LIBS} -lapparmor"
                SUDO_OBJS="${SUDO_OBJS} apparmor.o"
                ;;
    no)         ;;
    *)          AC_MSG_ERROR([--with-apparmor does not take an argument.])

esac], [with_apparmor=no])

AC_ARG_ENABLE(sasl,
[AS_HELP_STRING([--enable-sasl], [Enable/disable LDAP SASL support])],
[ case "$enableval" in
    yes|no) ;;
    *)	    AC_MSG_WARN([ignoring unknown argument to --enable-sasl: $enableval])
	    ;;
  esac
])

AC_ARG_ENABLE(timestamp-type,
[AS_HELP_STRING([--timestamp-type=TYPE], [Set the default time stamp record type to global, ppid or tty.])],
[ case "$enableval" in
    global|ppid|tty)
	    timestamp_type=$enableval
	    ;;
    *)	    AC_MSG_WARN([ignoring unknown argument to --enable-timestamp-type: $enableval])
	    ;;
  esac
])
AC_DEFINE_UNQUOTED(TIMESTAMP_TYPE, $timestamp_type)

AC_ARG_ENABLE(offensive_insults,
[AS_HELP_STRING([--enable-offensive-insults], [Enable potentially offensive sudo insults.])],
[], [enable_offensive_insults=no])
AS_IF([test "$enable_offensive_insults" = "yes"], [AC_DEFINE(OFFENSIVE_INSULTS)])

AC_ARG_ENABLE(package_build,
[AS_HELP_STRING([--enable-package-build], [Enable options for package building.])],
[], [enable_package_build=no])

dnl
dnl gss_krb5_ccache_name() may not work on Heimdal so we don't use it by default
dnl
AC_ARG_ENABLE(gss_krb5_ccache_name,
[AS_HELP_STRING([--enable-gss-krb5-ccache-name], [Use GSS-API to set the Kerberos V cred cache name])],
[check_gss_krb5_ccache_name=$enableval], [check_gss_krb5_ccache_name=no])

AC_ARG_ENABLE(pvs-studio,
[AS_HELP_STRING([--enable-pvs-studio], [Create a PVS-Studio.cfg file.])])

AC_ARG_ENABLE(log-server,
[AS_HELP_STRING([--disable-log-server], [Disable building the sudo_logsrvd log server.])],
[ case "$enableval" in
    yes)
	    ;;
    no)
	    LOGSRV=#
	    LOGSRVD_SRC=
	    LOGSRVD_CONF=
	    ;;
    *)	    AC_MSG_WARN([ignoring unknown argument to --enable-log-server: $enableval])
	    ;;
  esac
])

AC_ARG_ENABLE(log-client,
[AS_HELP_STRING([--disable-log-client], [Disable sudoers support for using the sudo_logsrvd log server.])],
[ case "$enableval" in
    yes)
	    AC_DEFINE([SUDOERS_LOG_CLIENT])
	    ;;
    no)
	    ;;
    *)	    AC_MSG_WARN([ignoring unknown argument to --enable-log-client: $enableval])
	    ;;
  esac
], [AC_DEFINE([SUDOERS_LOG_CLIENT])])

AS_IF([test X"${enable_log_client}${enable_log_server}" = X"nono"], [
    # No need for liblogsrv.la
    LOGSRV_SRC=
    LIBLOGSRV=
])
AS_IF([test X"$LOGSRVD_SRC" != X""], [
    PPFILES="$PPFILES "'$(srcdir)/etc/sudo-logsrvd.pp'
])

dnl
dnl Do OpenSSL / wolfSSL / gcrypt after logsrv options
dnl
AC_ARG_ENABLE(openssl,
[AS_HELP_STRING([--enable-openssl], [Use OpenSSL's TLS and sha2 functions])],
[], [
    # Enable OpenSSL by default unless logsrvd and client are disabled
    AS_IF([test X"${enable_log_client}${enable_log_server}" != X"nono"], [
	enable_openssl=maybe
    ])
])
AC_ARG_ENABLE(openssl-pkgconfig-template,
[AS_HELP_STRING([--enable-openssl-pkgconfig-template], [A printf format string used to construct the OpenSSL pkg-config name])],
[], [enable_openssl_pkgconfig_template="%s"])

AC_ARG_ENABLE(wolfssl,
[AS_HELP_STRING([--enable-wolfssl], [Use wolfSSL's TLS and sha2 functions])], [
    enable_openssl=no
])

AC_ARG_ENABLE(gcrypt,
[AS_HELP_STRING([--enable-gcrypt], [Use GNU crypt's sha2 functions])], [
    AS_IF([test "${enable_openssl-no}${enable_wolfssl-no}" != "nono"], [
	AC_MSG_WARN([ignoring --enable-gcrypt when OpenSSL or wolfSSL is enabled.])
	enable_gcrypt=no
    ])
])

AC_ARG_ENABLE(python,
[AS_HELP_STRING([--enable-python], [Compile python plugin support])],
[ case "$enableval" in
    yes|no)
	;;
    *) AC_MSG_WARN([ignoring unknown argument to --enable-python: $enableval])
	;;
  esac
])

AC_ARG_ENABLE(adminconf,
[AS_HELP_STRING([--enable-adminconf[[=DIR]]], [Use configuration files from adminconfdir in preference to sysconfdir])],
[ case "$enableval" in
    yes|no)
	;;
    *)	adminconfdir="$enableval"
	enable_adminconf=yes
	;;
  esac
], [enable_adminconf=no])

dnl
dnl C compiler checks
dnl
AC_PROG_CPP
AC_CHECK_TOOL(AR, ar, false)
AC_CHECK_TOOL(RANLIB, ranlib, :)
AS_IF([test X"$AR" = X"false"], [
    AC_MSG_ERROR([the "ar" utility is required to build sudo])
])
AX_PROG_CC_FOR_BUILD

AS_IF([test "x$ac_cv_prog_cc_c89" = "xno"], [
    AC_MSG_ERROR([Sudo version $PACKAGE_VERSION requires an ANSI C compiler to build.])
])

dnl
dnl If the user specified --disable-static, override them or we'll
dnl be unable to build the executables in the sudoers plugin dir.
dnl
AS_IF([test "$enable_static" = "no"], [
    AC_MSG_WARN([ignoring --disable-static, sudo does not install static libs])
    enable_static=yes
])

dnl
dnl Set host variables and m4 macro dir
dnl
AC_CANONICAL_HOST
AC_CONFIG_MACRO_DIR([m4])

dnl
dnl Relies on CC host being set
dnl
SUDO_PVS_STUDIO_CFG

dnl
dnl On HP-UX 11.11 and higher we prefer dlopen() over shl_load().
dnl Libtool defaults to shl_load() so we need to prime the cache
dnl to override that default.
dnl
case "$host_os" in
hpux11.1[[1-9]]|hpux11.[[2-9]][[0-9]]|hpux1[[2-9]].*)
    # Prefer dlopen() over shl_load()
    : ${ac_cv_func_shl_load='no'}
    : ${ac_cv_lib_dld_shl_load='no'}
    ;;
esac

dnl
dnl Libtool init, we require libtool 2.2.6b or higher
dnl
LT_PREREQ([2.2.6b])
LT_INIT([dlopen])

dnl
dnl AIX supports two distinct flavors of shared libraries.
dnl Traditional AIX shared libs are .a files with a .so inside.
dnl AIX SVR4-style shared libs are plain .so files.  The --with-aix-soname
dnl option can be used to select the type.  We need to set the default
dnl values to match.  This must come after the LT_INIT() call.
dnl
case "$host_os" in
aix*)
    AS_IF([test X"$aix_use_runtimelinking" != X"yes"], [
	# Using traditional AIX dynamic shared objects in an archive file.
	noexec_file="$libexecdir/sudo/sudo_noexec.a(sudo_noexec.so)"
	intercept_file="$libexecdir/sudo/sudo_intercept.a(sudo_intercept.so)"
	sudoers_plugin="sudoers.a(sudoers.so)"
	python_plugin="python_plugin.a(python_plugin.so)"
    ])
    ;;
esac
SUDO_DEFINE_UNQUOTED([_PATH_SUDOERS_PLUGIN], ["$sudoers_plugin"])

dnl
dnl Allow the user to specify an alternate libtool.
dnl XXX - should be able to skip LT_INIT if we are using a different libtool
dnl
AC_ARG_WITH(libtool, [AS_HELP_STRING([--with-libtool=PATH], [specify path to libtool])],
[case $with_libtool in
    yes|builtin) ;;
    no)		AC_MSG_ERROR([--without-libtool not supported.])
		;;
    system)	LIBTOOL=libtool
		;;
    *)		LIBTOOL="$with_libtool"
		;;
esac])

dnl
dnl Defer enable_intercept and with_noexec until after libtool magic runs
dnl
AS_IF([test "$enable_shared" = "no"], [
    enable_intercept=no
    with_noexec=no
    enable_dlopen=no
    lt_cv_dlopen=none
    lt_cv_dlopen_libs=
    ac_cv_func_dlopen=no
    LT_LDFLAGS=-static
])
LIBDL="$lt_cv_dlopen_libs"
SHLIB_ENABLE="$enable_dlopen"

AC_ARG_ENABLE(intercept,
[AS_HELP_STRING([--enable-intercept], [fully qualified pathname of sudo_intercept.so])],
[ case "$enableval" in
    yes)	;;
    no)		;;
    *)		intercept_file="$enableval"
		;;
  esac
], [enable_intercept="$intercept_file"])
INTERCEPTFILE="sudo_intercept.so"
INTERCEPTDIR="`echo $intercept_file|sed -e 's:^${\([[^}]]*\)}:$(\1):' -e 's:^\([[^:]]*\)/[[^/]].*$:\1:'`"

AC_ARG_WITH(noexec, [AS_HELP_STRING([--with-noexec[[=PATH]]], [fully qualified pathname of sudo_noexec.so])],
[case $with_noexec in
    yes)	;;
    no)		;;
    *)		noexec_file="$with_noexec"
		;;
esac], [with_noexec="$noexec_file"])
NOEXECFILE="sudo_noexec.so"
NOEXECDIR="`echo $noexec_file|sed -e 's:^${\([[^}]]*\)}:$(\1):' -e 's:^\([[^:]]*\)/[[^/]].*$:\1:'`"

dnl
dnl Find programs we use
dnl
AC_PATH_PROG(SHA1SUM, [sha1sum], [openssl dgst -sha1])
AC_PATH_PROG(UNAMEPROG, [uname], [uname])
AC_PATH_PROG(TRPROG, [tr], [tr])
AC_CACHE_CHECK([for mandoc or nroff], [ac_cv_path_NROFF], [
    AC_PATH_PROGS_FEATURE_CHECK([NROFF], [mandoc nroff], [ac_cv_path_NROFF="$ac_path_NROFF"], [ac_cv_path_NROFF=no])
])
case "$ac_cv_path_NROFF" in
*mandoc|no)
    # Prefer mdoc format for mandoc (or when no formatter is present).
    : ${MANTYPE='mdoc'}
    ;;
*)
    # Check whether nroff supports -mdoc, this may produce incorrect
    # results when cross-compiling.
    test -n "$MANTYPE" && sudo_cv_var_mantype="$MANTYPE"
    AC_CACHE_CHECK([which macro set to use for manual pages],
	[sudo_cv_var_mantype],
	[
	    sudo_cv_var_mantype="man"
	    echo ".Sh NAME" > conftest
	    echo ".Nm sudo" >> conftest
	    echo ".Nd sudo" >> conftest
	    echo ".Sh DESCRIPTION" >> conftest
	    echo "sudo" >> conftest
	    if $ac_cv_path_NROFF -mdoc conftest >/dev/null 2>&1; then
		sudo_cv_var_mantype="mdoc"
	    fi
	    rm -f conftest
	]
    )
    MANTYPE="$sudo_cv_var_mantype"
    ;;
esac

dnl
dnl If a config.cache exists make sure it matches the current host.
dnl
AS_IF([test -n "$sudo_cv_prev_host"], [
    AS_IF([test "$sudo_cv_prev_host" != "$host"], [
	AC_MSG_ERROR([config.cache was created on a different host; remove it and re-run configure.])
    ])
])
sudo_cv_prev_host="$host"

dnl
dnl Store configure arguments for "sudo -V"
dnl
SUDO_DEFINE_UNQUOTED([CONFIGURE_ARGS], "$CONFIGURE_ARGS")

dnl
dnl We want to be able to differentiate between different rev's
dnl
AS_IF([test -n "$host_os"], [
    OSREV=`echo $host_os | sed 's/^[[^0-9\.]]*\([[0-9\.]]*\).*$/\1/'`
    OSMAJOR=`echo $OSREV | sed 's/\..*$//'`
], [
    OSREV=0
    OSMAJOR=0
])

case "$host" in
    *-*-solaris2*)
		AC_DEFINE([PAM_SUN_CODEBASE])

		# illumos has a broken fmemopen(3)
		if test X"`uname -o 2>/dev/null`" = X"illumos"; then
		    : ${ac_cv_func_fmemopen='no'}
		fi

		# Solaris-specific initialization
		OS_INIT=os_init_solaris
		SUDO_OBJS="${SUDO_OBJS} solaris.o"

		# AFS support needs -lucb
		AS_IF([test "$with_AFS" = "yes"], [
		    AFS_LIBS="-lc -lucb"
		])
		: ${mansectsu='1m'}
		: ${mansectform='4'}
		: ${mansectmisc='5'}
		test -z "$with_pam" && AUTH_EXCL_DEF="PAM"
		AC_CHECK_FUNCS([priv_set], [PSMAN=1])
		;;
    *-*-aix*)
		AC_DEFINE([PAM_SUN_CODEBASE])

		# To get all prototypes (so we pass -Wall)
		AC_DEFINE([_LINUX_SOURCE_COMPAT])

		# For AIX we build in support for both LAM and PAM
		# and choose which to use based on auth_type in
		# /etc/security/login.cfg
		AS_IF([test X"${with_pam}${with_aixauth}" = X""], [
		    AUTH_EXCL_DEF="AIX_AUTH PAM"
		])

		# AIX analog of nsswitch.conf, enabled by default
		AC_ARG_WITH(netsvc, [AS_HELP_STRING([--with-netsvc[[=PATH]]], [path to netsvc.conf])],
		[case $with_netsvc in
		    no)		;;
		    yes)	with_netsvc="/etc/netsvc.conf"
				;;
		    *)		;;
		esac])
		AS_IF([test -z "$with_nsswitch" -a -z "$with_netsvc"], [
		    with_netsvc="/etc/netsvc.conf"
		])

		# cfmakeraw is broken on AIX (and is not documented)
		: ${ac_cv_func_cfmakeraw='no'}

		# strnlen/strndup may be broken on AIX < 6 depending
		# on the libc version, use our own.
		AS_IF([test $OSMAJOR -lt 6], [
		    : ${ac_cv_func_strnlen='no'}
		])

		# fmemopen(3) has a bug wrt feof() on some AIX versions.
		# https://www.ibm.com/support/pages/apar/IJ11845
		: ${ac_cv_func_fmemopen='no'}

		# getdelim() may or may not be present on AIX <= 6.1.
		# bos610 is missing getdelim but bos61J has it.
		AS_IF([test "$enable_package_build" = "yes"], [
		    AS_IF([test $OSMAJOR -le 6], [
			: ${ac_cv_func_getdelim='no'}
		    ])
		])

		# memset_s() may or may ont be present on AIX <= 7.1.
		# bos710 is missing memset_s but bos71L has it.
		AS_IF([test "$enable_package_build" = "yes"], [
		    AS_IF([test $OSMAJOR -le 7], [
			: ${ac_cv_func_memset_s='no'}
		    ])
		])

		# Remove timedir on boot, AIX does not have /var/run
		INIT_SCRIPT=aix.sh
		INIT_DIR=/etc/rc.d/init.d
		RC_LINK=/etc/rc.d/rc2.d/S90sudo

		# AIX-specific functions
		AC_CHECK_FUNCS([getuserattr setrlimit64])
		AC_CHECK_FUNCS([setauthdb],
		    [AC_CHECK_TYPES([authdb_t], [], [], [#include <usersec.h>])])

		COMMON_OBJS="${COMMON_OBJS} aix.lo"
		SUDO_APPEND_COMPAT_EXP(aix_prep_user_v1 aix_restoreauthdb_v1 aix_setauthdb_v1 aix_setauthdb_v2 aix_getauthregistry_v1)

		# These prototypes may be missing
		AC_CHECK_DECLS([usrinfo], [], [], [
#include <sys/types.h>
#include <uinfo.h>
		])
		AC_CHECK_DECLS([setauthdb], [], [], [
#include <sys/types.h>
#include <usersec.h>
		])
		;;
    *-*-hiuxmpp*)
		# HI-UX/MPP is based on OSF/1

		# ignore envariables wrt dynamic lib path
		AX_APPEND_FLAG([-Wl,-no_library_replacement], [SUDO_LDFLAGS])

		shadow_funcs="getprpwnam dispcrypt"
		shadow_libs="-lsecurity"

		: ${mansectsu='8'}
		: ${mansectform='4'}
		: ${mansectmisc='5'}
		;;
    *-*-hpux*)
		AC_DEFINE([PAM_SUN_CODEBASE])

		# AFS support needs -lBSD
		AS_IF([test "$with_AFS" = "yes"], 
		    AFS_LIBS="-lc -lBSD"
		])
		: ${mansectsu='1m'}
		: ${mansectform='4'}
		: ${mansectmisc='5'}

		# HP-UX does not clear /var/run so we need to do it
		INIT_SCRIPT=hpux.sh
		INIT_DIR=/sbin/init.d
		RC_LINK=/sbin/rc2.d/S900sudo

		# HP-UX shared libs must be executable.
		# Load time is much greater if writable so use 0555.
		SHLIB_MODE=0555

		# HP-UX won't unlink a shared lib that is open
		INSTALL_BACKUP='~'

		# The HP bundled compiler cannot generate shared libs
		AS_IF([test -z "$GCC"], [
		    AC_CACHE_CHECK([for HP bundled C compiler],
			[sudo_cv_var_hpccbundled],
			[if $CC -V 2>&1 | grep '^(Bundled)' >/dev/null 2>&1; then
			    sudo_cv_var_hpccbundled=yes
			else
			    sudo_cv_var_hpccbundled=no
			fi]
		    )
		    AS_IF([test "$sudo_cv_var_hpccbundled" = "yes"], [
			AC_MSG_ERROR([The HP bundled C compiler is unable to build Sudo, you must use gcc or the HP ANSI C compiler instead.])
		    ])
		])

		# Build PA-RISC1.1 objects for better portability
		case "$host_cpu" in
		    hppa[[2-9]]*)
			_CFLAGS="$CFLAGS"
			AS_IF([test -n "$GCC"], [
			    portable_flag="-march=1.1"
			], [
			    portable_flag="+DAportable"
			])
			CFLAGS="$CFLAGS $portable_flag"
			AC_CACHE_CHECK([whether $CC understands $portable_flag],
			    [sudo_cv_var_daportable],
			    [AC_LINK_IFELSE(
				[AC_LANG_PROGRAM([[]], [[]])],
				    [sudo_cv_var_daportable=yes],
				    [sudo_cv_var_daportable=no]
				)
			    ]
			)
			AS_IF([test X"$sudo_cv_var_daportable" != X"yes"], [
			    CFLAGS="$_CFLAGS"
			])
			;;
		esac

		case "$host_os" in
			hpux10.*)
			    shadow_funcs="getprpwnam iscomsec"
			    shadow_libs="-lsec"
			    # HP-UX 10.x doesn't support LD_PRELOAD
			    with_noexec=no
			;;
			*)
			    shadow_funcs="getspnam iscomsec"
			    shadow_libs="-lsec"
			    test -z "$with_pam" && AUTH_EXCL_DEF="PAM"
			;;
		esac
		AC_CHECK_HEADERS([utmps.h])
		AC_CHECK_FUNCS([pstat_getproc gethrtime])
		;;
    *-dec-osf*)
		# ignore envariables wrt dynamic lib path
		AX_APPEND_FLAG([-Wl,-no_library_replacement], [SUDO_LDFLAGS])

		: ${CHECKSIA='true'}
		AC_ARG_ENABLE(sia,
		[AS_HELP_STRING([--disable-sia], [Disable SIA on Digital UNIX])],
		[ case "$enableval" in
		    yes)	CHECKSIA=true
				;;
		    no)		CHECKSIA=false
				;;
		    *)		AC_MSG_WARN([ignoring unknown argument to --enable-sia: $enableval])
				;;
		  esac
		])

		shadow_funcs="getprpwnam dispcrypt"
		# OSF/1 4.x and higher need -ldb too
		AS_IF([test $OSMAJOR -lt 4], [
		    shadow_libs="-lsecurity -laud -lm"
		], [
		    shadow_libs="-lsecurity -ldb -laud -lm"
		])

		# use SIA by default, if we have it
		test "$CHECKSIA" = "true" && AUTH_EXCL_DEF="SIA"

		#
		# Some versions of Digital Unix ship with a broken
		# copy of prot.h, which we need for shadow passwords.
		# XXX - make should remove this as part of distclean
		#
		AC_MSG_CHECKING([for broken prot.h])
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/security.h>
#include <prot.h>
		]], [[return(0);]])], [AC_MSG_RESULT(no)], [AC_MSG_RESULT([yes, fixing locally])
		sed 's:<acl.h>:<sys/acl.h>:g' < /usr/include/prot.h > prot.h
		])

		: ${mansectsu='8'}
		: ${mansectform='4'}
		: ${mansectmisc='5'}
		;;
    *-*-irix*)
		AC_DEFINE([_BSD_TYPES])
		AS_IF([test "$prefix" = "/usr/local" -a "$mandir" = '${datarootdir}/man'], [
		    AS_IF([test -d "/usr/share/man/local"], [
			mandir="/usr/share/man/local"
		    ], [
			mandir="/usr/man/local"
		    ])
		])
		# IRIX <= 4 needs -lsun
		AS_IF([test "$OSMAJOR" -le 4], [
		    AC_CHECK_LIB([sun], [getpwnam], [LIBS="${LIBS} -lsun"])
		])

		: ${mansectsu='1m'}
		: ${mansectform='4'}
		: ${mansectmisc='5'}
		;;
    *-*-linux*|*-*-k*bsd*-gnu)
		shadow_funcs="getspnam"
		test -z "$with_pam" && AUTH_EXCL_DEF="PAM"
		# Check for SECCOMP_MODE_FILTER in linux/seccomp.h
		AC_CHECK_DECLS([SECCOMP_MODE_FILTER], [], [], [
#include <sys/types.h>
#include <sys/prctl.h>
#include <asm/unistd.h>
#include <linux/seccomp.h>
#include <linux/filter.h>
		])
		# We call getrandom via syscall(3) in case it is not in libc
		AC_CHECK_HEADERS([linux/random.h sys/syscall.h])

		# Only use our replacement functions when not fuzzing,
		# they may skew the coverage reports.
		AS_IF([test X"$enable_fuzzer" = X"no"], [
		    # glibc's getentropy() emulation may fail on older kernels.
		    # We use our own getentropy() by default on Linux.
		    : ${ac_cv_func_getentropy='no'}

		    # glibc's arc4random() may fail in chroot on older kernels.
		    # We use our own arc4random() by default on Linux.
		    : ${ac_cv_func_arc4random='no'}

		    # glibc's closefrom() emulation may fail in chroot.
		    # We use our own closefrom() by default on Linux.
		    : ${ac_cv_func_closefrom='no'}
		])

		# Linux 3.2 supports reading/writing a another process
		# without using ptrace(2).
		AC_CHECK_FUNCS([process_vm_readv])
		;;
    *-*-gnu*)
		# lockf() is broken on the Hurd
		ac_cv_func_lockf=no
		;;
    *-*-sco*|*-sco-*)
		shadow_funcs="getprpwnam"
		shadow_libs="-lprot -lx"
		: ${mansectsu='1m'}
		: ${mansectform='4'}
		: ${mansectmisc='5'}
		;;
    m88k-motorola-sysv*)
		# motorolla's cc (a variant of gcc) does -O but not -O2
		CFLAGS=`echo $CFLAGS | sed 's/-O2/-O/g'`
		: ${mansectsu='1m'}
		: ${mansectform='4'}
		: ${mansectmisc='5'}
		;;
    *-sequent-sysv*)
		shadow_funcs="getspnam"
		shadow_libs="-lsec"
		: ${mansectsu='1m'}
		: ${mansectform='4'}
		: ${mansectmisc='5'}
		;;
    *-ncr-sysv4*|*-ncr-sysvr4*)
		AC_CHECK_LIB([c89], [strcasecmp], [LIBS="${LIBS} -lc89"])
		: ${mansectsu='1m'}
		: ${mansectform='4'}
		: ${mansectmisc='5'}
		;;
    *-ccur-sysv4*|*-ccur-sysvr4*)
		LIBS="${LIBS} -lgen"
		: ${mansectsu='1m'}
		: ${mansectform='4'}
		: ${mansectmisc='5'}
		;;
    *-*-bsdi*)
		SKIP_SETREUID=yes
		# Check for newer BSD auth API
		AS_IF([test -z "$with_bsdauth"], [
		    AC_CHECK_FUNCS([auth_challenge], [AUTH_EXCL_DEF="BSD_AUTH"])
		])
		;;
    *-*-freebsd*)
		AC_DEFINE([_BSD_SOURCE])

		# FreeBSD has a real setreuid(2) starting with 2.1 and
		# backported to 2.0.5.  We just take 2.1 and above...
		case "$OSREV" in
		0.*|1.*|2.0*)
		    SKIP_SETREUID=yes
		    ;;
		esac
		AS_IF([test "${with_skey-'no'}" = "yes"], [
		     SUDOERS_LIBS="${SUDOERS_LIBS} -lmd"
		])
		CHECKSHADOW="false"
		test -z "$with_pam" && AUTH_EXCL_DEF="PAM"
		: ${with_logincap='maybe'}

		# Examples go in share/examples/sudo
		AS_IF([test X"$with_exampledir" = X""], [
		    exampledir='$(datarootdir)/examples/$(PACKAGE_TARNAME)'
		])
		;;
    *-*-*openbsd*)
		AC_DEFINE([_BSD_SOURCE])

		# OpenBSD-specific initialization
		OS_INIT=os_init_openbsd
		SUDO_OBJS="${SUDO_OBJS} openbsd.o"

		# OpenBSD has a real setreuid(2) starting with 3.3 but
		# we will use setresuid(2) instead.
		SKIP_SETREUID=yes

		# OpenBSD >= 3.0 supports BSD auth
		AS_IF([test -z "$with_bsdauth"], [
		    AS_IF([test "$OSMAJOR" -ge 3], [
			AUTH_EXCL_DEF="BSD_AUTH"
		    ])
		])
		: ${with_logincap='maybe'}

		# Newer OpenBSD only fills in pw_password for getpwnam_shadow()
		shadow_funcs="getpwnam_shadow"

		# Examples go in share/examples/sudo
		AS_IF([test X"$with_exampledir" = X""], [
		    exampledir='$(datarootdir)/examples/$(PACKAGE_TARNAME)'
		])
		;;
    *-*-*netbsd*)
		# NetBSD has a real setreuid(2) starting with 1.3.2
		case "$OSREV" in
		0.9*|1.[[012]]*|1.3|1.3.1)
		    SKIP_SETREUID=yes
		    ;;
		esac
		CHECKSHADOW="false"
		test -z "$with_pam" && AUTH_EXCL_DEF="PAM"
		: ${with_logincap='maybe'}

		# For reallocarray()
		AC_DEFINE([_OPENBSD_SOURCE])

		# Examples go in share/examples/sudo
		AS_IF([test X"$with_exampledir" = X""], [
		    exampledir='$(datarootdir)/examples/$(PACKAGE_TARNAME)'
		])
		;;
    *-*-dragonfly*)
		AC_DEFINE([_BSD_SOURCE])

		AS_IF([test "${with_skey-'no'}" = "yes"], [
		     SUDOERS_LIBS="${SUDOERS_LIBS} -lmd"
		])
		CHECKSHADOW="false"
		test -z "$with_pam" && AUTH_EXCL_DEF="PAM"
		: ${with_logincap='yes'}

		# Examples go in share/examples/sudo
		AS_IF([test X"$with_exampledir" = X""], [
		    exampledir='$(datarootdir)/examples/$(PACKAGE_TARNAME)'
		])
		;;
    *-*-*bsd*)
		CHECKSHADOW="false"
		# Examples go in share/examples/sudo
		AS_IF([test X"$with_exampledir" = X""], [
		    exampledir='$(datarootdir)/examples/$(PACKAGE_TARNAME)'
		])
		;;
    *-*-darwin*)
		# Darwin has a real setreuid(2) starting with 9.0
		AS_IF([test $OSMAJOR -lt 9], [
		    SKIP_SETREUID=yes
		])
		CHECKSHADOW="false"
		test -z "$with_pam" && AUTH_EXCL_DEF="PAM"
		: ${with_logincap='yes'}
		# Darwin has a broken poll(), Apple radar 3710161
		: ${enable_poll='no'}

		# Build sudo_noexec.so as a shared library, not a module.
		# On Darwin, modules and shared libraries are incompatible.
		PRELOAD_MODULE=

		# Mach monotonic timer that runs while sleeping
		AC_CHECK_FUNCS([mach_continuous_time])

		# Undocumented API that dynamically allocates the groups.
		AC_CHECK_FUNCS([getgrouplist_2], [AC_CHECK_DECLS([getgrouplist_2])])

		# We use proc_pidinfo() to emulate closefrom() on macOS.
		AC_CHECK_HEADERS([libproc.h], [AC_CHECK_FUNCS([proc_pidinfo])])

		# We cannot directly access environ from a shared object
		AC_CHECK_HEADERS([crt_externs.h], [AC_CHECK_FUNCS([_NSGetEnviron])])

		# We need to force a flat namespace to make libc
		# symbol hooking work like it does on ELF.
		AX_CHECK_LINK_FLAG([-Wl,-force_flat_namespace], [AX_APPEND_FLAG([-Wl,-force_flat_namespace], [SUDO_LDFLAGS])])

		# Examples go in share/examples/sudo
		AS_IF([test X"$with_exampledir" = X""], [
		    exampledir='$(datarootdir)/examples/$(PACKAGE_TARNAME)'
		])
		;;
    *-*-nextstep*)
		# lockf() is broken on the NeXT
		ac_cv_func_lockf=no
		;;
    *-*-*sysv4*)
		: ${mansectsu='1m'}
		: ${mansectform='4'}
		: ${mansectmisc='5'}
		;;
    *-*-*sco3.2*)	# SCO OpenServer 5
		: ${mansectsu='1'}
		: ${mansectform='4'}
		: ${mansectmisc='5'}
		shadow_funcs="getprpwnam"
		shadow_libs="-lprot"
		;;
# UnixWare 7.x, OpenUNIX 8
    *-*-*sysv5*)
		: ${mansectsu='1'}
		: ${mansectform='4'}
		: ${mansectmisc='5'}
	case "$host" in
	*-*-sysv5SCO_SV*)	# SCO OpenServer 6.x
		shadow_funcs="getprpwnam"
		shadow_libs="-lprot"
		;;
	*)	shadow_funcs="getspnam"
		test -z "$with_pam" && AUTH_EXCL_DEF="PAM"
		;;
	esac
		;;
    *-*-sysv*)
		: ${mansectsu='1m'}
		: ${mansectform='4'}
		: ${mansectmisc='5'}
		;;
esac

dnl
dnl Check for mixing mutually exclusive and regular auth methods
dnl
AUTH_REG=${AUTH_REG# }
AUTH_EXCL=${AUTH_EXCL# }
AS_IF([test -n "$AUTH_EXCL" -a -n "$AUTH_REG"], [
    AC_MSG_ERROR([Cannot mix mutually exclusive ($AUTH_EXCL) and regular ($AUTH_REG) authentication methods])
])
dnl
dnl Only one of S/Key and OPIE may be specified
dnl
AS_IF([test X"${with_skey}${with_opie}" = X"yesyes"], [
    AC_MSG_ERROR([cannot use both S/Key and OPIE])
])

dnl
dnl Use BSD-style man sections by default
dnl
: ${mansectsu='8'}
: ${mansectform='5'}
: ${mansectmisc='7'}

dnl
dnl Add in any libpaths or libraries specified via configure
dnl
AS_IF([test -n "$with_libpath"], [
    for i in ${with_libpath}; do
	SUDO_APPEND_LIBPATH(LDFLAGS, [$i])
    done
])
AS_IF([test -n "$with_libraries"], [
    for i in ${with_libraries}; do
	case $i in
	    -l*)	;;
	    *.a)	;;
	    *.o)	;;
	    *)	i="-l${i}";;
	esac
	LIBS="${LIBS} ${i}"
    done
])

dnl
dnl C compiler checks (to be done after os checks)
dnl AC_PROG_CC_STDC is deprecated in autoconf 2.70 and above.
dnl
m4_bmatch(m4_defn([AC_AUTOCONF_VERSION]), [^2\.69], [AC_PROG_CC_STDC])
AC_C_CONST
AC_C_RESTRICT
AC_C_INLINE
AC_C_VOLATILE
SUDO_CPP_VARIADIC_MACROS

dnl
dnl Program checks
dnl
AC_PROG_AWK
AC_PROG_YACC
AC_PATH_PROG([FLEX], [flex], [flex])
SUDO_PROG_MV
SUDO_PROG_BSHELL
AS_IF([test -z "$with_sendmail"], [
    SUDO_PROG_SENDMAIL
    with_sendmail="$ac_cv_path_SENDMAILPROG"
])
SUDO_PROG_VI
dnl
dnl Use fully-qualified path to vi in the manual
dnl
AS_IF([test -z "$with_editor"], [
    editor="$ac_cv_path_VIPROG"
])
dnl
dnl Check for authpriv support in syslog
dnl
AS_IF([test X"$with_logfac" = X""], [
    AC_CHECK_DECL([LOG_AUTHPRIV], [logfac=authpriv], [], [#include <syslog.h>])
])
AC_DEFINE_UNQUOTED(LOGFAC, "$logfac", [The syslog facility sudo will use.])
dnl
dnl Header file checks
dnl
AC_HEADER_DIRENT
AC_HEADER_STDBOOL
AC_HEADER_MAJOR
AC_CHECK_HEADERS_ONCE([netgroup.h paths.h spawn.h wordexp.h sys/sockio.h sys/bsdtypes.h sys/select.h sys/stropts.h sys/sysmacros.h sys/statvfs.h])
AS_IF([test X"$ac_cv_header_utmps_h" != X"yes"], [
    AC_CHECK_HEADERS([utmpx.h])
])
AC_CHECK_HEADERS([endian.h] [sys/endian.h] [machine/endian.h], [break])
AC_CHECK_HEADERS([procfs.h] [sys/procfs.h], [AC_CHECK_MEMBERS(struct psinfo.pr_ttydev, [AC_CHECK_FUNCS([_ttyname_dev])], [], [AC_INCLUDES_DEFAULT
#ifdef HAVE_PROCFS_H
#include <procfs.h>
#endif
#ifdef HAVE_SYS_PROCFS_H
#include <sys/procfs.h>
#endif
])]
break)
#
# Check for large file and 64-bit time support.
#
AC_SYS_LARGEFILE
m4_ifdef([AC_SYS_YEAR2038], [AC_SYS_YEAR2038], [
    # GNU libc only allows setting _TIME_BITS when FILE_OFFSET_BITS is also set.    # GNU libc defines __TIMESIZE on systems where _TIME_BITS can be set.
    AS_IF([test X"$ac_cv_sys_file_offset_bits" = X"yes"], [
	AC_CHECK_DECL(__TIMESIZE, [
	    AC_DEFINE([_TIME_BITS], [64], [Number of bits in a timestamp, on hosts where this is settable.])
	], [], [
AC_INCLUDES_DEFAULT
#include <time.h>
	])
    ])
])

#
# Don't allow undefined symbols, even in shared libraries, if possible.
# This will detect missing symbols at build-time instead of run-time
# but is incompatible with the sanitizers/fuzzers.
# We must set this *before* the library tests.
#
AS_IF([test -n "$GCC" -a X"${enable_sanitizer}${enable_fuzzer}" = X"nono"], [
    AS_CASE([$host_os], [darwin*], [
	# On macOS 13, using "-undefined dynamic_lookup" produces a
	# warning.  Use the -no-undefined libtool option to avoid this.
	    AX_APPEND_FLAG([-no-undefined], [LT_LDFLAGS])
    ], [
	# On FreeBSD and Dragonfly, environ is filled in by the dynamic loader
	# so -Wl,--no-undefined causes a link error when environ is used.
	# https://bugs.freebsd.org/bugzilla/show_bug.cgi?id=263265
	# We use errno because OpenBSD shared libraries don't explicitly
	# link with libc, which can result in undefined reference errors.
	AC_CACHE_CHECK([the linker accepts -Wl,--no-undefined],
	    [sudo_cv_var_ld___no_undefined],
	    [
		sudo_cv_var_ld___no_undefined=no
		_CFLAGS="$CFLAGS"
		CFLAGS="$CFLAGS $lt_prog_compiler_pic"
		_LDFLAGS="$LDFLAGS"
		LDFLAGS="$LDFLAGS $lt_prog_compiler_pic -shared -Wl,--no-undefined"
		AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <errno.h>
    extern char **environ;]], [[int ret = ((long)environ & 0xff) + errno; return ret;]])],
		    [sudo_cv_var_ld___no_undefined=yes])
		CFLAGS="$_CFLAGS"
		LDFLAGS="$_LDFLAGS"
	    ]
	)
	AS_IF([test "$sudo_cv_var_ld___no_undefined" = "yes"], [
	    AX_APPEND_FLAG([-Wl,--no-undefined], [LDFLAGS])
	])
    ])
])

#
# HP-UX may need to define _XOPEN_SOURCE_EXTENDED to expose MSG_WAITALL.
# Also, HP-UX 11.23 has a broken sys/types.h when large files support
# is enabled and _XOPEN_SOURCE_EXTENDED is not also defined.
# The following test will define _XOPEN_SOURCE_EXTENDED in either case.
#
case "$host_os" in
    hpux*)
	AC_CACHE_CHECK([whether sys/socket.h needs _XOPEN_SOURCE_EXTENDED for MSG_WAITALL], [sudo_cv_xopen_source_extended],
	[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT
[#	include <sys/socket.h>]], [[int a = MSG_WAITALL; return a;]])],
	[sudo_cv_xopen_source_extended=no], [
	    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#define _XOPEN_SOURCE_EXTENDED
	    AC_INCLUDES_DEFAULT
[#	    include <sys/socket.h>
#	    include <net/if.h>]], [[int a = MSG_WAITALL; return a;]])],
	    [sudo_cv_xopen_source_extended=yes],
	    [sudo_cv_xopen_source_extended=error])
	])])
	AS_IF([test "$sudo_cv_xopen_source_extended" = "yes"], [
	    AC_DEFINE([_XOPEN_SOURCE_EXTENDED])
	])
	;;
esac
SUDO_MAILDIR
AS_IF([test ${with_logincap-'no'} != "no"], [
    AC_CHECK_HEADERS([login_cap.h], [
	LOGINCAP_USAGE='[[-c class]] '
	LCMAN=1
	with_logincap=yes
    ])
    AS_IF([test "${with_logincap}" = "yes"], [
	# setusercontext() is in libutil on NetBSD, FreeBSD, Dragonfly BSD.
	_LIBS="$LIBS"
	AC_SEARCH_LIBS([setusercontext], [util])
	LIBS="$_LIBS"
	AS_IF([test "${ac_cv_search_setusercontext}" != "none required"], [
	    SUDO_LIBS="${SUDO_LIBS} -lutil"
	    SUDOERS_LIBS="${SUDOERS_LIBS} -lutil"
	])
    ])
])
AS_IF([test ${with_project-'no'} != "no"], [
    AC_CHECK_HEADER(project.h, [
	AC_CHECK_LIB([project], [setproject], [
	    AC_DEFINE(HAVE_PROJECT_H)
	    SUDO_LIBS="${SUDO_LIBS} -lproject"
	])
    ], [with_project=no])
])
dnl
dnl typedef checks
dnl
AS_IF([test X"${ac_cv_header_stdint_h}${ac_cv_header_inttypes_h}" = X"nono"], [
    AC_TYPE_INT8_T
    AC_TYPE_UINT8_T
    AC_TYPE_INT16_T
    AC_TYPE_UINT16_T
    AC_TYPE_INT32_T
    AC_TYPE_UINT32_T
    AC_TYPE_INT64_T
    AC_TYPE_UINT64_T
    AC_TYPE_INTMAX_T
    AC_TYPE_UINTMAX_T
])
AC_CHECK_TYPES([sig_atomic_t], [], [], [
AC_INCLUDES_DEFAULT
#include <signal.h>])
AC_CHECK_TYPES([struct in6_addr], [], [], [#include <sys/types.h>
#include <netinet/in.h>])
AC_CHECK_TYPES(socklen_t, [], [], [
AC_INCLUDES_DEFAULT
#include <sys/socket.h>])
SUDO_SOCK_SA_LEN
SUDO_SOCK_SIN_LEN
AC_CHECK_SIZEOF([long])
AC_CHECK_SIZEOF([long long])
AC_CHECK_SIZEOF([id_t])
AC_CHECK_SIZEOF([time_t])
AC_CHECK_SIZEOF([uid_t])
# sudo current assumes uid_t can be cast to unsigned int without problems
AS_IF([test $ac_cv_sizeof_uid_t -gt 4], [AC_MSG_ERROR([sudo does not support systems where uid_t is larger than 32-bit])])
AS_IF([test X"$ac_cv_header_utmps_h" = X"yes"], [
    SUDO_CHECK_UTMP_MEMBERS([utmps])
], [test X"$ac_cv_header_utmpx_h" = X"yes"], [
    SUDO_CHECK_UTMP_MEMBERS([utmpx])
], [
    SUDO_CHECK_UTMP_MEMBERS([utmp])
])

dnl
dnl Default values for LD_PRELOAD and related settings.
dnl
RTLD_PRELOAD_VAR="LD_PRELOAD"
if test $ac_cv_sizeof_long -eq 4; then
    RTLD_PRELOAD_VAR_32="LD_PRELOAD"
else
    RTLD_PRELOAD_VAR_64="LD_PRELOAD"
fi
RTLD_PRELOAD_ENABLE_VAR=
RTLD_PRELOAD_DELIM=":"
RTLD_PRELOAD_DEFAULT=

dnl
dnl System-specific LD_PRELOAD equivalents.
dnl The below tests rely on ac_cv_sizeof_long being defined.
dnl
case "$host" in
    *-*-solaris2*)
		# LD_PRELOAD is space-delimited
		RTLD_PRELOAD_DELIM=" "
		RTLD_PRELOAD_VAR_32="LD_PRELOAD_32"
		RTLD_PRELOAD_VAR_64="LD_PRELOAD_64"
		;;
    *-*-aix*)
		# LDR_PRELOAD and LDR_PRELOAD64 are only supported on 
		# AIX 5.3 and above.
		case "$OSREV" in
		    [[1-4]].*|5.[[1-2]]*)
			with_noexec=no
			;;
		    *)
			# AIX uses LDR_PRELOAD for 32-bit executables
			# and LDR_PRELOAD64 for 64-bit executable.
			RTLD_PRELOAD_VAR_32="LDR_PRELOAD"
			RTLD_PRELOAD_VAR_64="LDR_PRELOAD64"
			if test $ac_cv_sizeof_long -eq 4; then
			    RTLD_PRELOAD_VAR="LDR_PRELOAD"
			else
			    RTLD_PRELOAD_VAR="LDR_PRELOAD64"
			fi
			;;
		esac
		;;
    *-dec-osf*|*-*-hiuxmpp*|*-*-irix*)
		# ":DEFAULT" must be appended to _RLD_LIST
		RTLD_PRELOAD_VAR="_RLD_LIST"
		RTLD_PRELOAD_DEFAULT="DEFAULT"
		;;
    *-*-darwin*)
		# Darwin 8 and above can interpose library symbols cleanly
		AS_IF([test $OSMAJOR -ge 8], [
		    AC_DEFINE(HAVE___INTERPOSE)
		    dlyld_interpose=yes
		], [
		    RTLD_PRELOAD_ENABLE_VAR="DYLD_FORCE_FLAT_NAMESPACE"
		])
		RTLD_PRELOAD_VAR="DYLD_INSERT_LIBRARIES"

		# Build sudo_noexec.so as a shared library, not a module.
		# On Darwin, modules and shared libraries are incompatible.
		PRELOAD_MODULE=
		;;
    *-*-nextstep*)
		RTLD_PRELOAD_VAR="DYLD_INSERT_LIBRARIES"
		RTLD_PRELOAD_ENABLE_VAR="DYLD_FORCE_FLAT_NAMESPACE"
		;;
esac

dnl
dnl Library preloading to support NOEXEC
dnl
AS_IF([test X"$enable_intercept" = X"no"], [
    intercept_file=disabled
])
AS_IF([test X"$with_noexec" = X"no"], [
    noexec_file=disabled
])
AS_IF([test X"${intercept_file} ${noexec_file}" != X"disabled disabled"], [
    SUDO_DEFINE_UNQUOTED(RTLD_PRELOAD_VAR, "$RTLD_PRELOAD_VAR")
    if test -n "$RTLD_PRELOAD_VAR_32"; then
	SUDO_DEFINE_UNQUOTED(RTLD_PRELOAD_VAR_32, "$RTLD_PRELOAD_VAR_32")
    fi
    if test -n "$RTLD_PRELOAD_VAR_64"; then
	SUDO_DEFINE_UNQUOTED(RTLD_PRELOAD_VAR_64, "$RTLD_PRELOAD_VAR_64")
    fi
    SUDO_DEFINE_UNQUOTED(RTLD_PRELOAD_DELIM, '$RTLD_PRELOAD_DELIM')
    AS_IF([test -n "$RTLD_PRELOAD_DEFAULT"], [
	SUDO_DEFINE_UNQUOTED(RTLD_PRELOAD_DEFAULT, "$RTLD_PRELOAD_DEFAULT")
    ])
    AS_IF([test -n "$RTLD_PRELOAD_ENABLE_VAR"], [
	SUDO_DEFINE_UNQUOTED(RTLD_PRELOAD_ENABLE_VAR, "$RTLD_PRELOAD_ENABLE_VAR")
    ])
])

dnl
dnl Python plugin support
dnl
AS_IF([test ${enable_python-'no'} = "yes"], [
    AM_PATH_PYTHON([3])

    AC_ARG_VAR([PYTHON_INCLUDE], [Include flags for python, bypassing python-config])
    AC_ARG_VAR([PYTHON_LIBS], [Linker flags for python, bypassing python-config])
    AC_ARG_VAR([PYTHON_CONFIG], [Path to python-config])

    AS_IF([test -z "$PYTHON_INCLUDE" || test -z "$PYTHON_LIBS"], [
      AS_IF([test -z "$PYTHON_CONFIG"], [
        AC_PATH_PROGS([PYTHON_CONFIG],
                      [python$PYTHON_VERSION-config python-config],
                      [no],
                      [`dirname $PYTHON`])
        AS_IF([test "$PYTHON_CONFIG" = no], [AC_MSG_ERROR([cannot find python-config for $PYTHON.])])
      ])
    ])

    AS_IF([test -z "$PYTHON_INCLUDE"], [
      # Pull out python include path, ignore other flags
      PYTHON_INCLUDE=`$PYTHON_CONFIG --cflags | tr " " "\n" | grep "^-I" | sort -u | tr "\n" " "`
    ])

    AS_IF([test -z "$PYTHON_LIBS"], [
      # Newer versions of python3-config need --embed to include libpython
      if $PYTHON_CONFIG 2>&1 | grep embed >/dev/null; then
        PY_EMBED=--embed
      else
        PY_EMBED=
      fi
      PYTHON_LIBS=`$PYTHON_CONFIG --ldflags $PY_EMBED`
      PYTHON_LIBS=`$PYTHON_CONFIG --ldflags $PY_EMBED | tr " " "\n" | grep "^-[[lL]]" | tr "\n" " "`
    ])

    PPFILES="$PPFILES "'$(srcdir)/etc/sudo-python.pp'
    PYTHON_PLUGIN_SRC=plugins/python
    PYTHON_PLUGIN=
    AC_CONFIG_FILES([$PYTHON_PLUGIN_SRC/Makefile])
])

dnl
dnl Function checks
dnl
AC_FUNC_GETGROUPS
AC_FUNC_FSEEKO
AC_CHECK_FUNCS_ONCE([faccessat fexecve fmemopen killpg nl_langinfo renameat strtoull wordexp])
AC_CHECK_FUNCS([execvpe], [SUDO_APPEND_INTERCEPT_EXP(execvpe)])
AC_CHECK_FUNCS([pread], [
    # pread/pwrite on 32-bit HP-UX 11.x may not support large files
    case "$host_os" in
	hpux*)
	    AC_CHECK_FUNCS([pread64 pwrite64], [
		AC_CHECK_DECLS([pread64, pwrite64])
	    ])
	    ;;
    esac
], [
    AC_LIBOBJ(pread)
    SUDO_APPEND_COMPAT_EXP(sudo_pread)
])
AC_CHECK_FUNCS([pwrite], [], [
    AC_LIBOBJ(pwrite)
    SUDO_APPEND_COMPAT_EXP(sudo_pwrite)
])
AC_CHECK_FUNCS([cfmakeraw], [], [
    AC_LIBOBJ(cfmakeraw)
    SUDO_APPEND_COMPAT_EXP(sudo_cfmakeraw)
])
AC_CHECK_FUNCS([localtime_r], [], [
    AC_LIBOBJ(localtime_r)
    SUDO_APPEND_COMPAT_EXP(sudo_localtime_r)
])
AC_CHECK_FUNCS([gmtime_r], [], [
    AC_LIBOBJ(gmtime_r)
    SUDO_APPEND_COMPAT_EXP(sudo_gmtime_r)
])
AC_CHECK_FUNCS([timegm], [], [
    AC_LIBOBJ(timegm)
    SUDO_APPEND_COMPAT_EXP(sudo_timegm)
])
AC_CHECK_FUNCS([getgrouplist], [], [
    case "$host_os" in
    aix*)
	AC_CHECK_FUNCS([getgrset])
	;;
    *)
	AC_CHECK_FUNC([nss_search], [
	    AC_CHECK_FUNC([_nss_XbyY_buf_alloc], [
		# Solaris
		AC_CHECK_FUNC([_nss_initf_group], [
		    AC_CHECK_HEADERS([nss_dbdefs.h])
		    AC_DEFINE([HAVE_NSS_SEARCH])
		    AC_DEFINE([HAVE__NSS_XBYY_BUF_ALLOC])
		    AC_DEFINE([HAVE__NSS_INITF_GROUP])
		], [
		    AC_CHECK_HEADERS([nss_dbdefs.h], [
			# Older Solaris does not export _nss_initf_group
			# but we can use our own.
			AC_DEFINE([HAVE_NSS_SEARCH])
			AC_DEFINE([HAVE__NSS_XBYY_BUF_ALLOC])
		    ])
		])
	    ], [
		dnl HP-UX support disabled until "group: compat" fixed
		dnl # HP-UX
		dnl AC_CHECK_FUNC([__nss_XbyY_buf_alloc], [
		dnl     AC_CHECK_FUNC([__nss_initf_group], [
		dnl	AC_CHECK_HEADERS([nss_dbdefs.h])
		dnl	AC_DEFINE([HAVE_NSS_SEARCH])
		dnl	AC_DEFINE([HAVE___NSS_XBYY_BUF_ALLOC])
		dnl	AC_DEFINE([HAVE___NSS_INITF_GROUP])
		dnl    ])
		dnl])
		:
	    ])
	])
	;;
    esac
    SUDO_APPEND_COMPAT_EXP(sudo_getgrouplist)
])
AC_CHECK_FUNCS([getdelim], [
    # Out of date gcc fixed includes may result in missing getdelim() prototype
    AC_CHECK_DECLS([getdelim])
], [
    AC_LIBOBJ(getdelim)
    SUDO_APPEND_COMPAT_EXP(sudo_getdelim)
    COMPAT_TEST_PROGS="${COMPAT_TEST_PROGS}${COMPAT_TEST_PROGS+ }getdelim_test"
])
AC_CHECK_FUNCS([getusershell], [
    # Older Solaris has getusershell() et al but does not declare it.
    AC_CHECK_DECLS([getusershell])
], [
    AC_LIBOBJ(getusershell)
    SUDO_APPEND_COMPAT_EXP(sudo_getusershell)
])
AC_CHECK_FUNCS([reallocarray], [], [
    AC_LIBOBJ(reallocarray)
    SUDO_APPEND_COMPAT_EXP(sudo_reallocarray)
])
AC_CHECK_FUNCS([arc4random], [
    AC_CHECK_FUNCS([arc4random_uniform], [], [
	AC_LIBOBJ(arc4random_uniform)
	SUDO_APPEND_COMPAT_EXP(sudo_arc4random_uniform)
    ])
    AC_CHECK_FUNCS([arc4random_buf], [], [
	AC_LIBOBJ(arc4random_buf)
	SUDO_APPEND_COMPAT_EXP(sudo_arc4random_buf)
    ])
], [
    AC_LIBOBJ(arc4random)
    SUDO_APPEND_COMPAT_EXP(sudo_arc4random)
    SUDO_APPEND_COMPAT_EXP(sudo_arc4random_buf)
    AC_LIBOBJ(arc4random_uniform)
    SUDO_APPEND_COMPAT_EXP(sudo_arc4random_uniform)
    # arc4random.c needs getentropy()
    AC_CHECK_FUNCS([getentropy], [
	AC_CHECK_HEADERS([sys/random.h])
    ], [
	AC_LIBOBJ(getentropy)
	SUDO_APPEND_COMPAT_EXP(sudo_getentropy)
	AC_CHECK_FUNCS([getauxval])
    ])
    # arc4random.c wants pthread_atfork
    AC_CHECK_HEADERS([pthread.h], [
	AC_CHECK_LIB([pthread], [main], [LIBPTHREAD="-lpthread"])
	AC_CHECK_FUNCS([pthread_atfork])
    ])
])
AX_GCC_BUILTIN(__builtin_clz)
AX_GCC_BUILTIN(__builtin_clzl)

utmp_style=LEGACY
AC_CHECK_FUNCS([getutsid getutxid getutid], [utmp_style=POSIX; break])
AS_IF([test "$utmp_style" = "LEGACY"], [
    AC_CHECK_FUNCS([getttyent ttyslot], [break])
])

AC_CHECK_FUNCS([sysctl], [
    AC_CHECK_FUNCS([devname])
    # Check for the various flavors of kinfo_proc
    found=false
    AC_CHECK_MEMBER([struct kinfo_proc.ki_structsize], [
	AC_DEFINE(HAVE_KINFO_PROC_FREEBSD)
	found=true
    ], [], [
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/user.h>
    ])
    AS_IF([test "$found" = "false"], [
	AC_CHECK_MEMBER([struct kinfo_proc.kp_paddr], [
	    AC_DEFINE(HAVE_KINFO_PROC_DFLY)
	    found=true
	], [], [
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/user.h>
	])
    ])
    AS_IF([test "$found" = "false"], [
	AC_CHECK_MEMBER([struct kinfo_proc2.p_paddr], [
	    AC_DEFINE(HAVE_KINFO_PROC2_NETBSD)
	    found=true
	], [], [
#include <sys/param.h>
#include <sys/sysctl.h>
	])
    ])
    AS_IF([test "$found" = "false"], [
	AC_CHECK_MEMBER([struct kinfo_proc.p_paddr], [
	    AC_DEFINE(HAVE_KINFO_PROC_OPENBSD)
	    found=true
	], [], [
#include <sys/param.h>
#include <sys/sysctl.h>
	])
    ])
    AS_IF([test "$found" = "false"], [
	AC_CHECK_MEMBER([struct kinfo_proc.kp_proc], [
	    AC_DEFINE(HAVE_KINFO_PROC_44BSD)
	    found=true
	], [], [
#include <sys/param.h>
#include <sys/sysctl.h>
	])
    ])
])

AC_CHECK_FUNCS([openpty], [AC_CHECK_HEADERS([libutil.h util.h pty.h], [break])], [
    AC_CHECK_LIB([util], [openpty], [
	AC_CHECK_HEADERS([libutil.h util.h pty.h], [break])
	case "$SUDO_LIBS" in
	    *-lutil*) ;;
	    *) SUDO_LIBS="${SUDO_LIBS} -lutil";;
	esac
	AC_DEFINE(HAVE_OPENPTY)
    ], [
	AC_CHECK_FUNCS([_getpty], [], [
	    AC_CHECK_FUNCS([grantpt], [
		AC_CHECK_FUNCS([posix_openpt])
	    ], [
		AC_CHECK_FUNCS([revoke])
	    ])
	])
    ])
])
AC_CHECK_FUNCS([unsetenv], [SUDO_FUNC_UNSETENV_VOID], [])
SUDO_FUNC_PUTENV_CONST
SUDO_FUNC_IOCTL_REQ_INT
AS_IF([test -z "$SKIP_SETRESUID"], [
    AC_CHECK_FUNCS([setresuid], [
	SKIP_SETREUID=yes
	AC_CHECK_DECLS([setresuid])
	AC_CHECK_FUNCS([getresuid], [AC_CHECK_DECLS([getresuid])])
    ])
])
AS_IF([test -z "$SKIP_SETRESUID"], [
    AC_CHECK_FUNCS([setreuid])
])
AC_CHECK_FUNCS_ONCE([seteuid])
AS_IF([test X"$with_interfaces" != X"no"], [
    AC_CHECK_FUNCS([getifaddrs], [AC_CHECK_FUNCS([freeifaddrs])])
])
AC_CHECK_FUNCS([lockf], [break])
AC_CHECK_FUNCS([innetgr], [
    AC_CHECK_DECLS([innetgr], [], [], [
AC_INCLUDES_DEFAULT
#ifdef HAVE_NETGROUP_H
# include <netgroup.h>
#else
# include <netdb.h>
#endif /* HAVE_NETGROUP_H */
])], [
    AC_CHECK_FUNCS([_innetgr], [
	AC_CHECK_DECLS([_innetgr], [], [], [
AC_INCLUDES_DEFAULT
#ifdef HAVE_NETGROUP_H
# include <netgroup.h>
#else
# include <netdb.h>
#endif /* HAVE_NETGROUP_H */
	])
    ])
])
AC_CHECK_FUNCS([getdomainname], [
    AC_CHECK_DECLS([getdomainname], [], [], [
AC_INCLUDES_DEFAULT
#include <netdb.h>
    ])
], [
    AC_CHECK_FUNCS([sysinfo], [AC_CHECK_HEADERS([sys/systeminfo.h])])
])
AC_CHECK_FUNCS([utimensat], [], [
    AC_LIBOBJ(utimens)
    SUDO_APPEND_COMPAT_EXP(sudo_utimensat)
    AC_CHECK_FUNCS([utimes])
])
AC_CHECK_FUNCS([futimens], [], [
    AC_LIBOBJ(utimens)
    SUDO_APPEND_COMPAT_EXP(sudo_futimens)
    AC_CHECK_FUNCS([futimes futimesat futime], [break])
])
AC_CHECK_FUNCS([explicit_bzero], [], [
    AC_LIBOBJ(explicit_bzero)
    SUDO_APPEND_COMPAT_EXP(sudo_explicit_bzero)
    AC_CHECK_FUNCS([explicit_memset memset_explicit memset_s bzero], [break])
])
SUDO_FUNC_FNMATCH([AC_DEFINE(HAVE_FNMATCH)], [
    AC_LIBOBJ(fnmatch)
    SUDO_APPEND_COMPAT_EXP(sudo_fnmatch)
    COMPAT_TEST_PROGS="${COMPAT_TEST_PROGS}${COMPAT_TEST_PROGS+ }fnm_test"
])
SUDO_FUNC_ISBLANK
AC_CHECK_FUNCS([glob], [], [
    AC_LIBOBJ(glob)
    SUDO_APPEND_COMPAT_EXP(sudo_glob sudo_globfree)
])
AC_CHECK_FUNCS([memrchr], [], [
    AC_LIBOBJ(memrchr)
    SUDO_APPEND_COMPAT_EXP(sudo_memrchr)
])
AC_CHECK_FUNCS([freezero], [], [
    AC_LIBOBJ(freezero)
    SUDO_APPEND_COMPAT_EXP(sudo_freezero)
])
AC_CHECK_FUNCS(nanosleep, [], [
    # On Solaris, nanosleep is in librt
    AC_CHECK_LIB([rt], [nanosleep], [
	AC_DEFINE(HAVE_NANOSLEEP)
	LIBRT="-lrt"
    ], [
	AC_LIBOBJ(nanosleep)
	SUDO_APPEND_COMPAT_EXP(sudo_nanosleep)
    ])
])
AC_CHECK_FUNCS([fchownat], [], [
    AC_LIBOBJ(fchownat)
    SUDO_APPEND_COMPAT_EXP(sudo_fchownat)
])
AC_CHECK_FUNCS([mkdirat], [], [
    AC_LIBOBJ(mkdirat)
    SUDO_APPEND_COMPAT_EXP(sudo_mkdirat)
])
AC_CHECK_FUNCS([openat], [], [
    AC_LIBOBJ(openat)
    SUDO_APPEND_COMPAT_EXP(sudo_openat)
])
AC_CHECK_FUNCS([unlinkat], [], [
    AC_LIBOBJ(unlinkat)
    SUDO_APPEND_COMPAT_EXP(sudo_unlinkat)
])
AC_CHECK_FUNCS([fchmodat], [], [
    AC_LIBOBJ(fchmodat)
    SUDO_APPEND_COMPAT_EXP(sudo_fchmodat)
])
AC_CHECK_FUNCS([fstatat], [], [
    AC_LIBOBJ(fstatat)
    SUDO_APPEND_COMPAT_EXP(sudo_fstatat)
])
AC_CHECK_FUNCS([dup3], [], [
    AC_LIBOBJ(dup3)
    SUDO_APPEND_COMPAT_EXP(sudo_dup3)
])
AC_CHECK_FUNCS([pipe2], [], [
    AC_LIBOBJ(pipe2)
    SUDO_APPEND_COMPAT_EXP(sudo_pipe2)
])
AC_CHECK_FUNCS([pw_dup], [], [
    AC_LIBOBJ(pw_dup)
    SUDO_APPEND_COMPAT_EXP(sudo_pw_dup)
])
AC_CHECK_FUNCS([realpath], [], [
    AC_LIBOBJ(realpath)
    SUDO_APPEND_COMPAT_EXP(sudo_realpath)
])
AC_CHECK_FUNCS([strlcpy], [], [
    AC_LIBOBJ(strlcpy)
    SUDO_APPEND_COMPAT_EXP(sudo_strlcpy)
])
AC_CHECK_FUNCS([strlcat], [], [
    AC_LIBOBJ(strlcat)
    SUDO_APPEND_COMPAT_EXP(sudo_strlcat)
])
AC_CHECK_FUNC([strnlen], [AC_FUNC_STRNLEN], [AC_LIBOBJ(strnlen)])
AS_IF([test X"$ac_cv_func_strnlen_working" = X"yes"], [
    AC_DEFINE(HAVE_STRNLEN)
    AC_CHECK_FUNCS([strndup], [], [
	AC_LIBOBJ(strndup)
	SUDO_APPEND_COMPAT_EXP(sudo_strndup)
    ])
], [
    # Broken or missing strnlen, use our own.
    SUDO_APPEND_COMPAT_EXP(sudo_strnlen)
    # Avoid libc strndup() since it is usually implemented using strnlen()
    AC_LIBOBJ(strndup)
    SUDO_APPEND_COMPAT_EXP(sudo_strndup)
])
AC_CHECK_FUNCS([clock_gettime], [], [
    # On Solaris, clock_gettime is in librt
    AC_CHECK_LIB([rt], [clock_gettime], [
	AC_DEFINE(HAVE_CLOCK_GETTIME)
	LIBRT="-lrt"
    ])
])
AC_CHECK_FUNCS([getopt_long], [], [
    AC_LIBOBJ(getopt_long)
    SUDO_APPEND_COMPAT_EXP(sudo_getopt_long sudo_getopt_long_only)
    AC_CACHE_CHECK([for optreset], sudo_cv_optreset, [
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[extern int optreset; optreset = 1; return optreset;]])], [sudo_cv_optreset=yes], [sudo_cv_optreset=no])])
    AS_IF([test "$sudo_cv_optreset" = "yes"], [
	AC_DEFINE(HAVE_OPTRESET)
    ])
])
AC_CHECK_FUNCS([closefrom], [], [AC_LIBOBJ(closefrom)
    SUDO_APPEND_COMPAT_EXP(sudo_closefrom)
    AC_CHECK_DECL(F_CLOSEM, AC_DEFINE(HAVE_FCNTL_CLOSEM), [
	# Linux has a special header for close_range(2).
	AC_CHECK_FUNCS([close_range], [
	    case "$host_os" in
	    linux*) AC_CHECK_HEADERS([linux/close_range.h]);;
	    esac
	])
    ], [
#	include <limits.h>
#	include <fcntl.h> ])
    COMPAT_TEST_PROGS="${COMPAT_TEST_PROGS}${COMPAT_TEST_PROGS+ }closefrom_test"
])
sudo_mktemp=no
case "$host_os" in
    darwin*)
	# macOS has these but uses a _np (non-portable) suffix
	AC_CHECK_FUNCS([mkdtempat_np mkostempsat_np], [], [sudo_mktemp=yes; break])
	;;
    *)
	AC_CHECK_FUNCS([mkdtempat mkostempsat], [], [sudo_mktemp=yes; break])
	;;
esac
# If any of the mktemp family are missing we use our own.
AS_IF([test X"$sudo_mktemp" = X"yes"], [
    AC_LIBOBJ(mktemp)
    SUDO_APPEND_COMPAT_EXP(sudo_mkdtemp sudo_mkdtempat sudo_mkostempsat sudo_mkstemp sudo_mkstemps)
    COMPAT_TEST_PROGS="${COMPAT_TEST_PROGS}${COMPAT_TEST_PROGS+ }mktemp_test"
])
AX_FUNC_SNPRINTF
AS_IF([test X"$ac_cv_have_working_snprintf$ac_cv_have_working_vsnprintf" = X"yesyes"], [
    # System has a C99-compliant v?snprintf(), check for v?asprintf()
    AC_CHECK_FUNCS([asprintf], [], [
	AC_LIBOBJ(snprintf)
	SUDO_APPEND_COMPAT_EXP(sudo_asprintf)
    ])
    AC_CHECK_FUNCS([vasprintf], [], [
	AC_LIBOBJ(snprintf)
	SUDO_APPEND_COMPAT_EXP(sudo_vasprintf)
    ])
], [
    # Missing or non-compliant v?snprintf(), assume missing/bad v?asprintf()
    SUDO_APPEND_COMPAT_EXP(sudo_snprintf sudo_vsnprintf sudo_asprintf sudo_vasprintf)
])
AC_CHECK_MEMBERS([struct tm.tm_gmtoff], [], [], [
AC_INCLUDES_DEFAULT
#include <errno.h>
])
AC_CHECK_MEMBER([struct stat.st_mtim],
    [AC_DEFINE(HAVE_ST_MTIM)]
    [AC_CHECK_MEMBER([struct stat.st_mtim.st__tim], AC_DEFINE(HAVE_ST__TIM))],
    [AC_CHECK_MEMBER([struct stat.st_mtimespec],
	[AC_DEFINE([HAVE_ST_MTIMESPEC])],
	[AC_CHECK_MEMBER([struct stat.st_nmtime], AC_DEFINE(HAVE_ST_NMTIME))])
    ]
)
dnl
dnl 4.4BSD-based systems can force the password or group file to be held open
dnl
AC_CHECK_FUNCS([setpassent setgroupent])
dnl
dnl Function checks for sudo_noexec
dnl
AS_IF([test X"$with_noexec" != X"no"], [
    # Check for non-standard exec functions
    AC_CHECK_FUNCS([exect execvP execvpe])
    # Check for posix_spawn, and posix_spawnp
    AS_IF([test X"$ac_cv_header_spawn_h" = X"yes"], [
	AC_CHECK_FUNCS([posix_spawn posix_spawnp])
    ])
])

dnl
dnl Check for the dirfd function/macro.  If not found, look for dd_fd in DIR.
dnl
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <$ac_header_dirent>]], [[DIR *d; (void)dirfd(d);]])], [AC_DEFINE(HAVE_DIRFD)], [AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <$ac_header_dirent>]], [[DIR d; memset(&d, 0, sizeof(d)); return(d.dd_fd);]])], [AC_DEFINE(HAVE_DD_FD)], [])])
AC_CHECK_MEMBERS([struct dirent.d_type, struct dirent.d_namlen], [], [], [
AC_INCLUDES_DEFAULT
#include <$ac_header_dirent>
])

dnl
dnl Check for OpenSSL or wolfSSL
dnl
SUDO_CHECK_OPENSSL

dnl
dnl Check for sha2 functions if not using openssl or wolfssl
dnl
AS_IF([test "$DIGEST" = "digest.lo"], [
    AS_IF([test "${enable_gcrypt-no}" != no], [
	# Use gcrypt's sha2 functions
	AC_DEFINE(HAVE_GCRYPT)
	DIGEST=digest_gcrypt.lo
	LIBCRYPTO="-lgcrypt"
	AS_IF([test "$enable_gcrypt" != "yes"], [
	    AX_APPEND_FLAG([-I${enable_gcrypt}/include], [CPPFLAGS])
	    SUDO_APPEND_LIBPATH(LDFLAGS, [${enable_gcrypt}/lib])
	])
    ], [
	# Use sudo's sha2 functions if not in libmd or libc.
	FOUND_SHA2=no
	AC_CHECK_HEADER([sha2.h], [
	    FOUND_SHA2=yes
	    AC_CHECK_FUNCS([SHA224Update], [SUDO_FUNC_SHA2_VOID_PTR], [
		# On some systems, SHA224Update is in libmd
		AC_CHECK_LIB([md], [SHA224Update], [
		    AC_DEFINE(HAVE_SHA224UPDATE)
		    SUDO_FUNC_SHA2_VOID_PTR
		    LIBCRYPTO="-lmd"
		], [
		    # Does not have SHA224Update
		    FOUND_SHA2=no
		])
	    ])
	])
	AS_IF([test X"$FOUND_SHA2" = X"no"], [
	    AC_LIBOBJ(sha2)
	    SUDO_APPEND_COMPAT_EXP(sudo_SHA224Final sudo_SHA224Init sudo_SHA224Pad sudo_SHA224Transform sudo_SHA224Update sudo_SHA256Final sudo_SHA256Init sudo_SHA256Pad sudo_SHA256Transform sudo_SHA256Update sudo_SHA384Final sudo_SHA384Init sudo_SHA384Pad sudo_SHA384Transform sudo_SHA384Update sudo_SHA512Final sudo_SHA512Init sudo_SHA512Pad sudo_SHA512Transform sudo_SHA512Update)
	])
    ])
])

dnl
dnl Network functions may live in libsocket, libinet, libnsl, or libresolv.
dnl Order is important here.
dnl
SUDO_CHECK_NET_FUNC([socket])

dnl
dnl We need to keep track of inet_pton()'s dependent libraries for
dnl our getaddrinfo() replacement.
dnl
ONET_LIBS="$NET_LIBS"
SUDO_CHECK_NET_FUNC([inet_pton], [
    AC_DEFINE(HAVE_INET_PTON)
    INET_PTON_LIBS=
    for lib in $NET_LIBS; do
	case "$ONET_LIBS" in
	    *"$lib"*) ;;
	    *) INET_PTON_LIBS="${INET_PTON_LIBS}${INET_PTON_LIBS+ }$lib";;
	esac
    done
], [
    AC_LIBOBJ(inet_pton)
    SUDO_APPEND_COMPAT_EXP(sudo_inet_pton)
])

dnl
dnl The check for inet_ntop() must follow inet_pton() for INET_PTON_LIBS.
dnl
SUDO_CHECK_NET_FUNC([inet_ntop], [
    AC_DEFINE(HAVE_INET_NTOP)
], [
    AC_LIBOBJ(inet_ntop)
    SUDO_APPEND_COMPAT_EXP(sudo_inet_ntop)
])

dnl
dnl The check for syslog() must be last so we can detect INET_PTON_LIBS.
dnl
SUDO_CHECK_NET_FUNC([syslog])

dnl
dnl Check for getaddrinfo and add any required libs to NET_LIBS.
dnl If it was added to LIBOBJS we need to export the symbols.
dnl
OLIBS="$LIBS"
GETADDRINFO_LIBS=
AX_FUNC_GETADDRINFO
case " $LIBOBJS " in
    *" getaddrinfo.$ac_objext "* )
	SUDO_APPEND_COMPAT_EXP(sudo_getaddrinfo sudo_freeaddrinfo sudo_gai_strerror)
	# We need libsudo_util to pull in dependent libraries for
	# inet_pton(), gethostbyname(), and getservbyname()
	AS_IF([test -n "${INET_PTON_LIBS}"], [
	    LT_DEP_LIBS="${LT_DEP_LIBS}${LT_DEP_LIBS+ }${INET_PTON_LIBS}"
	    LIBS="${LIBS}${LIBS+ }${INET_PTON_LIBS}"
	])
	SUDO_CHECK_NET_FUNC([gethostbyname])
	;;
    *)
	for lib in $LIBS; do
	    case "$OLIBS" in
		*"$lib"*) ;;
		*) GETADDRINFO_LIBS="${GETADDRINFO_LIBS}${GETADDRINFO_LIBS+ }$lib";;
	    esac
	done
	AS_IF([test -n "${GETADDRINFO_LIBS}"], [
	    # We need libsudo_util to pull in dependent libraries for
	    # gai_strerror()
	    LT_DEP_LIBS="${LT_DEP_LIBS}${LT_DEP_LIBS+ }${GETADDRINFO_LIBS}"
	    LIBS="${LIBS}${LIBS+ }${GETADDRINFO_LIBS}"

	    # Add to NET_LIBS if necessary
	    for lib in $GETADDRINFO_LIBS; do
		case "$NET_LIBS" in
		    *"$lib"*)   ;;
		    *)		NET_LIBS="${NET_LIBS}${NET_LIBS+ }$lib";;
		esac
	    done
	])
	;;
esac
LIBS="$OLIBS"

dnl
dnl Check for va_copy or __va_copy in stdarg.h
dnl
AC_CACHE_CHECK([for va_copy], sudo_cv_func_va_copy, [
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <stdarg.h>
	va_list ap1, ap2;]], [[va_copy(ap1, ap2);]])],
	[sudo_cv_func_va_copy=yes], [sudo_cv_func_va_copy=no])
])
AS_IF([test "$sudo_cv_func_va_copy" = "yes"], [
    AC_DEFINE(HAVE_VA_COPY)
], [
    AC_CACHE_CHECK([for __va_copy], sudo_cv_func___va_copy, [
	AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <stdarg.h>
	    va_list ap1, ap2;]], [[__va_copy(ap1, ap2);]])],
	    [sudo_cv_func___va_copy=yes], [sudo_cv_func___va_copy=no])
    ])
    AS_IF([test "$sudo_cv_func___va_copy" = "yes"], [
	AC_DEFINE(HAVE___VA_COPY)
    ])
])

dnl
dnl Check for getprogname()/setprogname() or __progname
dnl
AC_CHECK_FUNCS([getprogname], [
    AC_CHECK_FUNCS([setprogname], [], [SUDO_APPEND_COMPAT_EXP(sudo_setprogname)])
], [
    AC_CACHE_CHECK([for __progname], sudo_cv___progname, [
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[extern char *__progname; if (__progname[0] == '\0') return 1;]])], [sudo_cv___progname=yes], [sudo_cv___progname=no])])
    AS_IF([test "$sudo_cv___progname" = "yes"], [
	AC_DEFINE(HAVE___PROGNAME)
    ])
    SUDO_APPEND_COMPAT_EXP(sudo_getprogname)
    SUDO_APPEND_COMPAT_EXP(sudo_setprogname)
])

dnl
dnl Check for __func__ or __FUNCTION__
dnl
AC_CACHE_CHECK([for __func__], sudo_cv___func__, [
    AS_IF([test X"$ac_cv_prog_cc_c99" = X"no"], [
	AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[if (__func__[0] == '\0') return 1;]])], [sudo_cv___func__=yes], [sudo_cv___func__=no])
    ], [
	# C99 and higher support __func__
	sudo_cv___func__=yes
    ])
])
AS_IF([test "$sudo_cv___func__" = "yes"], [
    AC_DEFINE(HAVE___FUNC__)
], [test -n "$GCC"], [
    AC_CACHE_CHECK([for __FUNCTION__], sudo_cv___FUNCTION__, [
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[if(__FUNCTION__[0] == '\0') return 1;]])], [sudo_cv___FUNCTION__=yes], [sudo_cv___FUNCTION__=no])])
    AS_IF([test "$sudo_cv___FUNCTION__" = "yes"], [
	AC_DEFINE(HAVE___FUNC__)
	AC_DEFINE(__func__, __FUNCTION__, [Define to __FUNCTION__ if your compiler supports __FUNCTION__ but not __func__])
    ])
])

SUDO_CHECK_GETTEXT

dnl
dnl Deferred zlib option processing.
dnl By default we use the system zlib if it is present.
dnl If a directory was specified for zlib (or we are use sudo's version),
dnl prepend the include dir to make sure we get the right zlib header.
dnl
case "$enable_zlib" in
    yes)
	AC_CHECK_LIB([z], [gzclearerr], [
	    AC_CHECK_HEADERS([zlib.h], [ZLIB="-lz"], [enable_zlib=builtin])
	])
	;;
    no)
	;;
    system)
	AC_DEFINE(HAVE_ZLIB_H)
	ZLIB="-lz"
	;;
    static|shared|builtin)
	# handled below
	;;
    *)
	AC_DEFINE(HAVE_ZLIB_H)
	AX_APPEND_FLAG([-I${enable_zlib}/include], [CPPFLAGS])
	SUDO_APPEND_LIBPATH(ZLIB, [$enable_zlib/lib])
	ZLIB="${ZLIB} -lz"
	;;
esac
case "$enable_zlib" in
    builtin|static|dynamic)
	AC_DEFINE(HAVE_ZLIB_H)
	# XXX - can't use AX_APPEND_FLAG due to use of $(top_foo) and quoting
	CPPFLAGS='-I$(top_builddir)/lib/zlib -I$(top_srcdir)/lib/zlib '"${CPPFLAGS}"
	ZLIB="${ZLIB}"' $(top_builddir)/lib/zlib/libsudo_z.la'
	ZLIB_SRC=lib/zlib
	AC_CONFIG_HEADERS([lib/zlib/zconf.h])
	AC_CONFIG_FILES([lib/zlib/Makefile])
	AS_IF([test X"$enable_shared" = X"no" -o "$enable_zlib" = "static"], [
	    AS_IF([test "$enable_zlib" = "shared"], [
		AC_MSG_ERROR([unable to build shared libraries on this system])
	    ])
	    # Build as convenience library
	    ZLIB_LDFLAGS=-no-install
	])
	;;
esac

dnl
dnl Check for NSIG, _NSIG or __NSIG declarations in signal.h
dnl
AC_CHECK_DECLS([NSIG], [], [
    AC_CHECK_DECLS([_NSIG], [], [
	AC_CHECK_DECLS([__NSIG], [], [], [
AC_INCLUDES_DEFAULT
#include <signal.h>
	])
    ], [
AC_INCLUDES_DEFAULT
#include <signal.h>
    ])
], [
AC_INCLUDES_DEFAULT
#include <signal.h>
])

dnl
dnl Check for errno declaration in errno.h
dnl
AC_CHECK_DECLS([errno], [], [], [
AC_INCLUDES_DEFAULT
#include <errno.h>
])

dnl
dnl Check for h_errno declaration in netdb.h
dnl
AC_CHECK_DECLS([h_errno], [], [], [
AC_INCLUDES_DEFAULT
#include <netdb.h>
])

dnl
dnl Check for incomplete limits.h and missing SIZE_MAX.
dnl
AC_CHECK_DECLS([LLONG_MAX, LLONG_MIN, ULLONG_MAX, PATH_MAX, SSIZE_MAX], [], [], [
#include <sys/types.h>
#include <limits.h>
])
AC_CHECK_DECLS([SIZE_MAX], [], [], [
#include <sys/types.h>
#include <limits.h>
#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif
])
dnl
dnl Try to find equivalents for missing types
dnl
AS_IF([test "$ac_cv_have_decl_LLONG_MAX" != "yes"], [
    AC_CHECK_DECLS([QUAD_MAX], [], [], [[
#include <sys/types.h>
#include <limits.h>
    ]])
])
AS_IF([test "$ac_cv_have_decl_LLONG_MIN" != "yes"], [
    AC_CHECK_DECLS([QUAD_MIN], [], [], [[
#include <sys/types.h>
#include <limits.h>
    ]])
])
AS_IF([test "$ac_cv_have_decl_ULLONG_MAX" != "yes"], [
    AC_CHECK_DECLS([UQUAD_MAX], [], [], [[
#include <sys/types.h>
#include <limits.h>
    ]])
])
AS_IF([test "$ac_cv_have_decl_SIZE_MAX" != "yes"], [
    AC_CHECK_DECLS([SIZE_T_MAX], [], [], [[
#include <sys/types.h>
#include <limits.h>
    ]])
])
AS_IF([test "$ac_cv_have_decl_PATH_MAX" != "yes"], [
    AC_CHECK_DECLS([_POSIX_PATH_MAX], [], [], [[
#include <sys/types.h>
#include <limits.h>
    ]])
])

dnl
dnl Check for strsignal() or sys_siglist
dnl
AC_CHECK_FUNCS([strsignal], [], [
    AC_LIBOBJ(strsignal)
    SUDO_APPEND_COMPAT_EXP(sudo_strsignal)
    HAVE_SIGLIST="false"
    AC_CHECK_DECLS([sys_siglist], [HAVE_SIGLIST="true"], [
	AC_CHECK_DECLS([_sys_siglist], [HAVE_SIGLIST="true"], [], [
AC_INCLUDES_DEFAULT
#include <signal.h>
	])
    ], [
AC_INCLUDES_DEFAULT
#include <signal.h>
    ])
    AS_IF([test "$HAVE_SIGLIST" != "true"], [
	AC_LIBOBJ(siglist)
    ])
])

dnl
dnl Check for sig2str() and str2sig(), sys_signame or sys_sigabbrev
dnl
AC_CHECK_FUNCS([sig2str], [
    AC_CHECK_DECLS(SIG2STR_MAX, [], [], [
#	include <signal.h>
])], [
    AC_LIBOBJ(sig2str)
    SUDO_APPEND_COMPAT_EXP(sudo_sig2str)
])
AC_CHECK_FUNCS([str2sig], [], [
    AC_LIBOBJ(str2sig)
    SUDO_APPEND_COMPAT_EXP(sudo_str2sig)
])

dnl
dnl Check for sys_signame or sys_sigabbrev if missing sig2str() or str2sig().
dnl Also enable unit tests for sig2str() and str2sig().
dnl
AS_IF([test x"${ac_cv_func_sig2str}${ac_cv_func_str2sig}" != x"yesyes"], [
    AC_CHECK_FUNCS([sigabbrev_np])
    AS_IF([test x"${ac_cv_func_sigabbrev_np}" != x"yes"], [
	COMPAT_TEST_PROGS="${COMPAT_TEST_PROGS}${COMPAT_TEST_PROGS+ }strsig_test"
	HAVE_SIGNAME="false"
	AC_CHECK_DECLS([sys_signame], [HAVE_SIGNAME="true"], [
	    AC_CHECK_DECLS([_sys_signame], [HAVE_SIGNAME="true"], [
		AC_CHECK_DECLS([sys_sigabbrev], [HAVE_SIGNAME="true"], [], [
AC_INCLUDES_DEFAULT
#include <signal.h>
		])
	    ], [
AC_INCLUDES_DEFAULT
#include <signal.h>
	    ])
	], [
AC_INCLUDES_DEFAULT
#include <signal.h>
	])
	AS_IF([test "$HAVE_SIGNAME" != "true"], [
	    AC_CACHE_CHECK([for undeclared sys_sigabbrev],
		[sudo_cv_var_sys_sigabbrev],
		[AC_LINK_IFELSE(
		    [AC_LANG_PROGRAM([[extern char **sys_sigabbrev;]], [[return sys_sigabbrev[1];]])],
			[sudo_cv_var_sys_sigabbrev=yes],
			[sudo_cv_var_sys_sigabbrev=no]
		    )
		]
	    )
	    AS_IF([test "$sudo_cv_var_sys_sigabbrev" = yes], [
		AC_DEFINE(HAVE_SYS_SIGABBREV)
	    ], [
		AC_LIBOBJ(signame)
		SIGNAME=signame.lo
	    ])
	])
    ])
])

dnl
dnl Check for dl_iterate_phdr, may require -ldl
dnl
OLIBS="$LIBS"
LIBS="$LIBS $lt_cv_dlopen_libs"
AC_CHECK_FUNCS([dl_iterate_phdr])
LIBS="$OLIBS"

dnl
dnl nsswitch.conf and its equivalents
dnl
AS_IF([test ${with_netsvc-"no"} != "no"], [
    SUDO_DEFINE_UNQUOTED(_PATH_NETSVC_CONF, "${with_netsvc-/etc/netsvc.conf}")
    netsvc_conf=${with_netsvc-/etc/netsvc.conf}
], [test ${with_nsswitch-"yes"} != "no"], [
    SUDO_DEFINE_UNQUOTED(_PATH_NSSWITCH_CONF, "${with_nsswitch-/etc/nsswitch.conf}")
    nsswitch_conf=${with_nsswitch-/etc/nsswitch.conf}
])

dnl
dnl Mutually exclusive auth checks come first, followed by
dnl non-exclusive ones.  Note: passwd must be last of all!
dnl

dnl
dnl Convert default authentication methods to with_* if
dnl no explicit authentication scheme was specified.
dnl
AS_IF([test -z "${AUTH_EXCL}${AUTH_REG}" -a -n "$AUTH_EXCL_DEF"], [
    for auth in $AUTH_EXCL_DEF; do
	case $auth in
	    AIX_AUTH)	with_aixauth=maybe;;
	    BSD_AUTH)	with_bsdauth=maybe;;
	    PAM)	with_pam=maybe;;
	    SIA)	CHECKSIA=true;;
	esac
    done
])

dnl
dnl PAM support.  Systems that use PAM by default set with_pam=default
dnl and we do the actual tests here.
dnl
AS_IF([test ${with_pam-"no"} != "no"], [
    #
    # Check for pam_start() in libpam first, then for pam_appl.h.
    #
    found_pam_lib=no
    AC_CHECK_LIB([pam], [pam_start], [found_pam_lib=yes], [], [$lt_cv_dlopen_libs])
    #
    # Some PAM implementations (macOS for example) put the PAM headers
    # in /usr/include/pam instead of /usr/include/security...
    #
    found_pam_hdrs=no
    AC_CHECK_HEADERS([security/pam_appl.h] [pam/pam_appl.h], [found_pam_hdrs=yes; break])
    AS_IF([test "$found_pam_lib" = "yes" -a "$found_pam_hdrs" = "yes"], [
	# Found both PAM libs and headers
	with_pam=yes
    ], [test "$with_pam" = "yes"], [
	AS_IF([test "$found_pam_lib" = "no"], [
	    AC_MSG_ERROR([--with-pam specified but unable to locate PAM development library.])
	])
	AS_IF([test "$found_pam_hdrs" = "no"], [
	    AC_MSG_ERROR([--with-pam specified but unable to locate PAM development headers.])
	])
    ], [test "$found_pam_lib" != "$found_pam_hdrs"], [
	AS_IF([test "$found_pam_lib" = "no"], [
	    AC_MSG_ERROR([found PAM headers but no PAM development library; specify --without-pam to build without PAM])
	])
	AS_IF([test "$found_pam_hdrs" = "no"], [
	    AC_MSG_ERROR([found PAM library but no PAM development headers; specify --without-pam to build without PAM])
	])
    ])

    AS_IF([test "$with_pam" = "yes"], [
	# Older PAM implementations lack pam_getenvlist
	OLIBS="$LIBS"
	LIBS="$LIBS -lpam $lt_cv_dlopen_libs"
	AC_CHECK_FUNCS([pam_getenvlist])
	LIBS="$OLIBS"

	# We already link with -ldl if needed (see LIBDL below)
	SUDOERS_LIBS="${SUDOERS_LIBS} -lpam"
	AC_DEFINE(HAVE_PAM)
	AUTH_OBJS="$AUTH_OBJS pam.lo";
	AUTH_EXCL=PAM

	AC_ARG_WITH(pam-login, [AS_HELP_STRING([--with-pam-login], [enable specific PAM session for sudo -i])],
	[case $with_pam_login in
	    yes)	AC_DEFINE([HAVE_PAM_LOGIN])
			pam_login_service="sudo-i"
			;;
	    no)		;;
	    *)		AC_MSG_ERROR([--with-pam-login does not take an argument.])
			;;
	esac])

	AC_ARG_ENABLE(pam_session,
	[AS_HELP_STRING([--disable-pam-session], [Disable PAM session support])],
	    [ case "$enableval" in
		yes)	;;
		no)	AC_DEFINE(NO_PAM_SESSION)
			pam_session=off
			;;
		*)	AC_MSG_WARN([ignoring unknown argument to --enable-pam-session: $enableval])
			;;
	    esac])
    ])
])

dnl
dnl AIX general authentication
dnl We may build in support for both AIX LAM and PAM and select
dnl which one to use at run-time.
dnl
AS_IF([test ${with_aixauth-'no'} != "no"], [
    AC_CHECK_FUNCS([authenticate], [with_aixauth=yes])
    AS_IF([test "${with_aixauth}" = "yes"], [
	AC_MSG_NOTICE([using AIX general authentication])
	AC_DEFINE(HAVE_AIXAUTH)
	AUTH_OBJS="$AUTH_OBJS aix_auth.lo";
	SUDOERS_LIBS="${SUDOERS_LIBS} -ls"
	AUTH_EXCL=AIX_AUTH
    ])
])

dnl
dnl BSD authentication
dnl If set to "maybe" only enable if no other exclusive method in use.
dnl
AS_IF([test ${with_bsdauth-'no'} != "no"], [
    AC_CHECK_HEADER(bsd_auth.h, AC_DEFINE(HAVE_BSD_AUTH_H)
	[AUTH_OBJS="$AUTH_OBJS bsdauth.lo"]
	[BSDAUTH_USAGE='[[-a type]] ']
	[AUTH_EXCL=BSD_AUTH; BAMAN=1],
	[AC_MSG_ERROR([BSD authentication was specified but bsd_auth.h could not be found])])
])

dnl
dnl SIA authentication for Tru64 Unix
dnl
AS_IF([test ${CHECKSIA-'false'} = "true"], [
    AC_CHECK_FUNCS([sia_ses_init], [found=true], [found=false])
    AS_IF([test "$found" = "true"], [
	AUTH_EXCL=SIA
	AUTH_OBJS="$AUTH_OBJS sia.lo"
    ])
])

dnl
dnl extra FWTK libs + includes
dnl
AS_IF([test ${with_fwtk-'no'} != "no"], [
    AS_IF([test "$with_fwtk" != "yes"], [
	SUDO_APPEND_LIBPATH(SUDOERS_LDFLAGS, [${with_fwtk}])
	AX_APPEND_FLAG([-I${with_fwtk}], [CPPFLAGS])
	with_fwtk=yes
    ])
    SUDOERS_LIBS="${SUDOERS_LIBS} -lauth -lfwall"
    AUTH_OBJS="$AUTH_OBJS fwtk.lo"
])

dnl
dnl extra SecurID lib + includes
dnl
AS_IF([test ${with_SecurID-'no'} != "no"], [
    AS_IF([test "$with_SecurID" != "yes"], [], [test -d /usr/ace/examples], [
	with_SecurID=/usr/ace/examples
    ], [
	with_SecurID=/usr/ace
    ])
    AX_APPEND_FLAG([-I${with_SecurID}], [CPPFLAGS])
    SUDO_APPEND_LIBPATH(SUDOERS_LDFLAGS, [${with_SecurID}])
    SUDOERS_LIBS="${SUDOERS_LIBS} -laceclnt -lpthread"
    AUTH_OBJS="$AUTH_OBJS securid5.lo";
])

dnl
dnl Non-mutually exclusive auth checks come next.
dnl Note: passwd must be last of all!
dnl

dnl
dnl Convert default authentication methods to with_* if
dnl no explicit authentication scheme was specified.
dnl
AS_IF([test -z "${AUTH_EXCL}" -a -n "$AUTH_DEF"], [
    for auth in $AUTH_DEF; do
	case $auth in
	    passwd)	: ${with_passwd='maybe'};;
	esac
    done
])

dnl
dnl Kerberos V
dnl There is an easy way and a hard way...
dnl
AS_IF([test ${with_kerb5-'no'} != "no"], [
    AC_CHECK_PROG(KRB5CONFIG, krb5-config, yes, "")
    AS_IF([test -n "$KRB5CONFIG"], [
	AC_DEFINE(HAVE_KERB5)
	AUTH_OBJS="$AUTH_OBJS kerb5.lo"
	for f in `krb5-config --cflags`; do
	    AX_APPEND_FLAG([$f], [CPPFLAGS])
	done
	SUDOERS_LIBS="$SUDOERS_LIBS `krb5-config --libs`"
	dnl
	dnl Try to determine whether we have Heimdal or MIT Kerberos
	dnl
	AC_MSG_CHECKING(whether we are using Heimdal)
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <krb5.h>]], [[const char *tmp = heimdal_version;]])], [
		AC_MSG_RESULT(yes)
		AC_DEFINE(HAVE_HEIMDAL)
	    ], [
		AC_MSG_RESULT(no)
	    ]
	)
    ], [
	AC_DEFINE(HAVE_KERB5)
	dnl
	dnl Use the specified directory, if any, else search for correct inc dir
	dnl
	AS_IF([test "$with_kerb5" = "yes"], [
	    found=no
	    O_CPPFLAGS="$CPPFLAGS"
	    for dir in "" "kerberosV/" "krb5/" "kerberos5/" "kerberosv5/"; do
		CPPFLAGS="$O_CPPFLAGS -I/usr/include/${dir}"
		AC_PREPROC_IFELSE([AC_LANG_PROGRAM([[#include <krb5.h>]])], [found=yes; break])
	    done
	    AS_IF([test X"$found" = X"no"], [
		CPPFLAGS="$O_CPPFLAGS"
		AC_MSG_WARN([unable to locate Kerberos V include files, you will have to edit the Makefile and add -I/path/to/krb/includes to CPPFLAGS])
	    ])
	], [
	    dnl XXX - try to include krb5.h here too
	    SUDO_APPEND_LIBPATH(SUDOERS_LDFLAGS, [${with_kerb5}/lib])
	    AX_APPEND_FLAG([-I${with_kerb5}/include], [CPPFLAGS])
	])

	dnl
	dnl Try to determine whether we have Heimdal or MIT Kerberos
	dnl
	AC_MSG_CHECKING(whether we are using Heimdal)
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <krb5.h>]], [[const char *tmp = heimdal_version;]])], [
		AC_MSG_RESULT(yes)
		AC_DEFINE(HAVE_HEIMDAL)
		# XXX - need to check whether -lcrypo is needed!
		SUDOERS_LIBS="${SUDOERS_LIBS} -lkrb5 -lcrypto -ldes -lcom_err -lasn1"
		AC_CHECK_LIB([roken], [main], [SUDOERS_LIBS="${SUDOERS_LIBS} -lroken"])
	    ], [
		AC_MSG_RESULT(no)
		SUDOERS_LIBS="${SUDOERS_LIBS} -lkrb5 -lk5crypto -lcom_err"
		AC_CHECK_LIB([krb5support], [main], [SUDOERS_LIBS="${SUDOERS_LIBS} -lkrb5support"])
	])
	AUTH_OBJS="$AUTH_OBJS kerb5.lo"
    ])
    _LIBS="$LIBS"
    LIBS="${LIBS} ${SUDOERS_LIBS}"
    AC_CHECK_FUNCS([krb5_verify_user krb5_init_secure_context])
    AC_CHECK_FUNCS([krb5_get_init_creds_opt_alloc], [
	AC_CACHE_CHECK([whether krb5_get_init_creds_opt_free takes a context],
	    sudo_cv_krb5_get_init_creds_opt_free_two_args, [
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <krb5.h>]],
			[[krb5_get_init_creds_opt_free(NULL, NULL);]]
		    )],
		    [sudo_cv_krb5_get_init_creds_opt_free_two_args=yes],
		    [sudo_cv_krb5_get_init_creds_opt_free_two_args=no]
		)
	    ]
	)
    ])
    AS_IF([test X"$sudo_cv_krb5_get_init_creds_opt_free_two_args" = X"yes"], [
  	AC_DEFINE(HAVE_KRB5_GET_INIT_CREDS_OPT_FREE_TWO_ARGS)
    ])
    LIBS="$_LIBS"
    AC_ARG_ENABLE(kerb5-instance,
    [AS_HELP_STRING([--enable-kerb5-instance], [instance string to append to the username (separated by a slash)])],
	[ case "$enableval" in
	    yes)	AC_MSG_ERROR([must give --enable-kerb5-instance an argument.])
			;;
	    no)		;;
	    *)		SUDO_DEFINE_UNQUOTED(SUDO_KRB5_INSTANCE, "$enableval")
			;;
	esac])
])

dnl
dnl extra AFS libs and includes
dnl
AS_IF([test ${with_AFS-'no'} = "yes"], [
    # looks like the "standard" place for AFS libs is /usr/afsws/lib
    AFSLIBDIRS="/usr/lib/afs /usr/afsws/lib /usr/afsws/lib/afs"
    for i in $AFSLIBDIRS; do
	AS_IF([test -d ${i}], [
	    SUDO_APPEND_LIBPATH(SUDOERS_LDFLAGS, [$i])
	    FOUND_AFSLIBDIR=true
	])
    done
    AS_IF([test -z "$FOUND_AFSLIBDIR"], [
	AC_MSG_WARN([unable to locate AFS libraries, you will have to edit the Makefile and add -L/path/to/afs/libs to SUDOERS_LDFLAGS or rerun configure with the --with-libpath options.])
    ])

    # Order is important here.  Note that we build AFS_LIBS from right to left
    # since AFS_LIBS may be initialized with BSD compat libs that must go last
    AFS_LIBS="-laudit ${AFS_LIBS}"
    for i in $AFSLIBDIRS; do
	AS_IF([test -f ${i}/util.a], [
	    AFS_LIBS="${i}/util.a ${AFS_LIBS}"
	    FOUND_UTIL_A=true
	    break;
	])
    done
    AS_IF([test -z "$FOUND_UTIL_A"], [
	AFS_LIBS="-lutil ${AFS_LIBS}"
    ])
    AFS_LIBS="-lkauth -lprot -lubik -lauth -lrxkad -lsys -ldes -lrx -llwp -lcom_err ${AFS_LIBS}"

    # AFS includes may live in /usr/include on some machines...
    for i in /usr/afsws/include; do
	AS_IF([test -d ${i}], [
	    AX_APPEND_FLAG([-I${i}], [CPPFLAGS])
	    FOUND_AFSINCDIR=true
	])
    done

    AS_IF([test -z "$FOUND_AFSLIBDIR"], [
	AC_MSG_WARN([unable to locate AFS include dir, you may have to edit the Makefile and add -I/path/to/afs/includes to CPPFLAGS or rerun configure with the --with-incpath options.])
    ])

    AUTH_OBJS="$AUTH_OBJS afs.lo"
])

dnl
dnl extra DCE obj + lib
dnl Order of libs in HP-UX 10.x is important, -ldce must be last.
dnl
AS_IF([test ${with_DCE-'no'} = "yes"], [
    DCE_OBJS="${DCE_OBJS} dce_pwent.o"
    SUDOERS_LIBS="${SUDOERS_LIBS} -ldce"
    AUTH_OBJS="$AUTH_OBJS dce.lo"
])

dnl
dnl extra S/Key lib and includes
dnl
AS_IF([test "${with_skey-'no'}" = "yes"], [
    O_LDFLAGS="$LDFLAGS"
    AS_IF([test "$with_skey" != "yes"], [
	AX_APPEND_FLAG([-I${with_skey}/include], [CPPFLAGS])
	LDFLAGS="$LDFLAGS -L${with_skey}/lib"
	SUDO_APPEND_LIBPATH(SUDOERS_LDFLAGS, [${with_skey}/lib])
	AC_CHECK_HEADER([skey.h], [found=yes], [found=no], [#include <stdio.h>])
    ], [
	found=no
	O_CPPFLAGS="$CPPFLAGS"
	for dir in "" "/usr/local" "/usr/contrib"; do
	    test -n "$dir" && CPPFLAGS="$O_CPPFLAGS -I${dir}/include"
	    AC_CHECK_HEADER([skey.h], [found=yes; break], [], [#include <stdio.h>])
	done
	AS_IF([test "$found" = "no" -o -z "$dir"], [
	    CPPFLAGS="$O_CPPFLAGS"
	], [
	    LDFLAGS="$LDFLAGS -L${dir}/lib"
	    SUDO_APPEND_LIBPATH(SUDOERS_LDFLAGS, [${dir}/lib])
	])
	AS_IF([test "$found" = "no"], [
	    AC_MSG_WARN([unable to locate skey.h, you will have to edit the Makefile and add -I/path/to/skey/includes to CPPFLAGS])
	])
    ])
    AC_CHECK_LIB([skey], [main], [found=yes], [AC_MSG_WARN([unable to locate libskey.a, you will have to edit the Makefile and add -L/path/to/skey/lib to SUDOERS_LDFLAGS])])
    AC_CHECK_LIB([skey], [skeyaccess], [AC_DEFINE(HAVE_SKEYACCESS)])

    AC_MSG_CHECKING([for RFC1938-compliant skeychallenge])
    AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM([[
#	    include <stdio.h>
#	    include <skey.h>]],
	    [[skeychallenge(NULL, NULL, NULL, 0);]]
	)], [
	    AC_DEFINE(HAVE_RFC1938_SKEYCHALLENGE)
	    AC_MSG_RESULT([yes])
	], [
	    AC_MSG_RESULT([no])
	]
    )

    LDFLAGS="$O_LDFLAGS"
    SUDOERS_LIBS="${SUDOERS_LIBS} -lskey"
    AUTH_OBJS="$AUTH_OBJS rfc1938.lo"
])

dnl
dnl extra OPIE lib and includes
dnl
AS_IF([test "${with_opie-'no'}" = "yes"], [
    O_LDFLAGS="$LDFLAGS"
    AS_IF([test "$with_opie" != "yes"], [
	AX_APPEND_FLAG([-I${with_opie}/include], [CPPFLAGS])
	LDFLAGS="$LDFLAGS -L${with_opie}/lib"
	SUDO_APPEND_LIBPATH(SUDOERS_LDFLAGS, [${with_opie}/lib])
	AC_PREPROC_IFELSE([AC_LANG_PROGRAM([[#include <opie.h>]])], [found=yes], [found=no])
    ], [
	found=no
	O_CPPFLAGS="$CPPFLAGS"
	for dir in "" "/usr/local" "/usr/contrib"; do
	    test -n "$dir" && CPPFLAGS="$O_CPPFLAGS -I${dir}/include"
	    AC_PREPROC_IFELSE([AC_LANG_PROGRAM([[#include <opie.h>]])], [found=yes; break])
	done
	AS_IF([test "$found" = "no" -o -z "$dir"], [
	    CPPFLAGS="$O_CPPFLAGS"
	], [
	    LDFLAGS="$LDFLAGS -L${dir}/lib"
	    SUDO_APPEND_LIBPATH(SUDOERS_LDFLAGS, [${dir}/lib])
	])
	AS_IF([test "$found" = "no"], [
	    AC_MSG_WARN([unable to locate opie.h, you will have to edit the Makefile and add -I/path/to/opie/includes to CPPFLAGS])
	])
    ])
    AC_CHECK_LIB([opie], [main], [found=yes], [AC_MSG_WARN([unable to locate libopie.a, you will have to edit the Makefile and add -L/path/to/opie/lib to SUDOERS_LDFLAGS])])
    LDFLAGS="$O_LDFLAGS"
    SUDOERS_LIBS="${SUDOERS_LIBS} -lopie"
    AUTH_OBJS="$AUTH_OBJS rfc1938.lo"
])

dnl
dnl Check for shadow password routines if we have not already done so.
dnl If there is a specific list of functions to check we do that first.
dnl Otherwise, we check for SVR4-style and then SecureWare-style.
dnl
AS_IF([test ${with_passwd-'no'} != "no"], [
    dnl
    dnl if crypt(3) not in libc, look elsewhere
    dnl
    _LIBS="$LIBS"
    AC_SEARCH_LIBS([crypt], [crypt crypt_d ufc], [
	test "${ac_cv_search_crypt}" != "none required" && shadow_libs="${shadow_libs} ${ac_cv_search_crypt}"
    ])
    AS_IF([test "${ac_cv_search_crypt}" != "no"], [
	AC_DEFINE(HAVE_CRYPT)
    ])
    LIBS="$_LIBS"

    AS_IF([test "$CHECKSHADOW" = "true" -a -n "$shadow_funcs"], [
	_LIBS="$LIBS"
	LIBS="$LIBS $shadow_libs"
	found=no
	for func in $shadow_funcs; do
	    AC_CHECK_FUNC([$func], [
		dnl Enumerate shadow functions instead of using:
		dnl     AC_DEFINE_UNQUOTED(AS_TR_CPP([HAVE_$func]))
		dnl for autoheader's sake and to catch template omissions.
		case "$func" in
		    dispcrypt)
			AC_DEFINE(HAVE_DISPCRYPT)
			;;
		    getprpwnam)
			AC_DEFINE(HAVE_GETPRPWNAM)
			SECUREWARE=1
			;;
		    getpwnam_shadow)
			AC_DEFINE(HAVE_GETPWNAM_SHADOW)
			;;
		    getspnam)
			AC_DEFINE(HAVE_GETSPNAM)
			;;
		    iscomsec)
			AC_DEFINE(HAVE_ISCOMSEC)
			;;
		    *)
			AC_MSG_ERROR([unhandled shadow password function $func])
			;;
		esac
		found=yes
	    ])
	done
	AS_IF([test "$found" = "no"], [
	    shadow_libs=
	])
	CHECKSHADOW=false
	LIBS="$_LIBS"
    ])
    AS_IF([test "$CHECKSHADOW" = "true"], [
	AC_SEARCH_LIBS([getspnam], [gen shadow], [
	    AC_DEFINE(HAVE_GETSPNAM)
	    test "${ac_cv_search_getspnam}" != "none required" && shadow_libs="${shadow_libs} ${ac_cv_search_getspnam}"
	    CHECKSHADOW=false
	])
    ])
    AS_IF([test "$CHECKSHADOW" = "true"], [
	AC_SEARCH_LIBS([getprpwnam], [sec security prot], [
	    AC_DEFINE(HAVE_GETPRPWNAM)
	    test "${ac_cv_search_getprpwnam}" != "none required" && shadow_libs="${shadow_libs} ${ac_cv_search_getprpwnam}"
	    SECUREWARE=1
	    CHECKSHADOW=false
	])
    ])
    AS_IF([test -n "$shadow_libs"], [
	# sudoers needs to link with shadow libs for password auth
	SUDOERS_LIBS="$SUDOERS_LIBS $shadow_libs"
    ])
    AS_IF([test -n "$SECUREWARE"], [
	_LIBS="$LIBS"
	LIBS="$LIBS $shadow_libs"
	AC_CHECK_FUNCS([bigcrypt])
	AUTH_OBJS="$AUTH_OBJS secureware.lo"
	# set_auth_parameters() and initprivs() are called from sudo.c
	AC_CHECK_FUNCS([set_auth_parameters initprivs], [test -n "$shadow_libs" && SUDO_LIBS="$SUDO_LIBS $shadow_libs"])
	LIBS="$_LIBS"
    ])
])

dnl
dnl Solaris 11 added a 4th argument to the au_close() function
dnl
AS_IF([test X"$with_bsm_audit" = X"yes"], [
    SUDO_FUNC_AU_CLOSE_SOLARIS11
])

dnl
dnl Choose event subsystem backend: poll or select
dnl
AS_IF([test X"$enable_poll" = X""], [
    AC_CHECK_FUNCS([ppoll poll], [enable_poll=yes; break], [enable_poll=no])
], [test X"$enable_poll" = X"yes"], [
    AC_CHECK_FUNCS([ppoll], [], AC_DEFINE(HAVE_POLL))
])
AS_IF([test "$enable_poll" = "yes"], [
    COMMON_OBJS="${COMMON_OBJS} event_poll.lo"
], [
    AC_CHECK_FUNCS([pselect])
    COMMON_OBJS="${COMMON_OBJS} event_select.lo"
])

dnl
dnl If LDAP support is enabled, add sudo ldap objects to SUDOERS_OBJS
dnl and add LDAP libraries to SUDOERS_LDFLAGS SUDOERS_LIBS.
dnl
SUDO_CHECK_LDAP

#
# How to do dynamic object loading.
# We support dlopen() and sh_load(), else fall back to static loading.
#
case "$lt_cv_dlopen" in
    dlopen)
	AC_DEFINE(HAVE_DLOPEN)
	AS_IF([test "$enable_static_sudoers" = "yes"], [
	    AC_DEFINE(STATIC_SUDOERS_PLUGIN)
	    SUDO_OBJS="${SUDO_OBJS} preload.o"
	    STATIC_SUDOERS="\$(top_builddir)/plugins/sudoers/sudoers.la"
	    AX_APPEND_FLAG([-no-install], [SUDOERS_LDFLAGS])
	    SUDOERS_LT_STATIC="--tag=disable-shared"
	    LT_STATIC=""
	], [
	    SUDOERS_LT_STATIC="--tag=disable-static"
	    LT_STATIC="--tag=disable-static"
	])
	;;
    shl_load)
	AC_DEFINE(HAVE_SHL_LOAD)
	AS_IF([test "$enable_static_sudoers" = "yes"], [
	    AC_DEFINE(STATIC_SUDOERS_PLUGIN)
	    SUDO_OBJS="${SUDO_OBJS} preload.o"
	    STATIC_SUDOERS="\$(top_builddir)/plugins/sudoers/sudoers.la"
	    AX_APPEND_FLAG([-no-install], [SUDOERS_LDFLAGS])
	    SUDOERS_LT_STATIC="--tag=disable-shared"
	    LT_STATIC=""
	], [
	    SUDOERS_LT_STATIC="--tag=disable-static"
	    LT_STATIC="--tag=disable-static"
	])
	;;
    *)
	AS_IF([test X"${ac_cv_func_dlopen}" = X"yes"], [
	    AC_MSG_ERROR([dlopen present but libtool doesn't appear to support your platform.])
	])
	# Preload sudoers module symbols
	AC_DEFINE(STATIC_SUDOERS_PLUGIN)
	SUDO_OBJS="${SUDO_OBJS} preload.o"
	STATIC_SUDOERS="\$(top_builddir)/plugins/sudoers/sudoers.la"
	LT_STATIC=""
	;;
esac

#
# The check_symbols test can only succeed with a dynamic sudoers plugin.
#
if test X"$STATIC_SUDOERS" = X""; then
    SUDOERS_TEST_PROGS="${SUDOERS_TEST_PROGS}${SUDOERS_TEST_PROGS+ }check_symbols"
fi

#
# We can only disable linking with the shared libsudo_util if
# sudoers is linked statically too.
#
AS_IF([test "$enable_shared_libutil" = "no"], [
    AS_IF([test X"$STATIC_SUDOERS" = X""], [
	AC_MSG_ERROR([--disable-shared-libutil may only be specified with --enable-static-sudoers or when dynamic linking is disabled.])
    ], [
	# Do not install libsudo_util.
	AX_APPEND_FLAG([-no-install], [LIBUTIL_LDFLAGS])
    ])
])

# On HP-UX, you cannot dlopen() a shared object that uses pthreads unless
# the main program is linked against -lpthread.  We have no knowledge of
# what libraries a plugin may depend on (e.g. HP-UX LDAP which uses pthreads)
# so always link against -lpthread on HP-UX if it is available.
# This check should go after all other libraries tests.
case "$host_os" in
    hpux*)
	AC_CHECK_LIB([pthread], [main], [SUDO_LIBS="${SUDO_LIBS} -lpthread"])
	AC_DEFINE(_REENTRANT)
	;;
esac

dnl
dnl Check for log file, timestamp and iolog locations
dnl
AS_IF([test "$utmp_style" = "LEGACY"], [SUDO_PATH_UTMP])
SUDO_LOGDIR
SUDO_LOGFILE
SUDO_RELAY_DIR
SUDO_RUNDIR
SUDO_VARDIR
SUDO_IO_LOGDIR
SUDO_TZDIR

dnl
dnl Turn warnings into errors.
dnl All compiler/loader tests after this point will fail if
dnl a warning is displayed (normally, warnings are not fatal).
dnl
AC_LANG_WERROR

dnl
dnl Don't use sys/sysctl.h if it is marked deprecated (Linux).
dnl This test relies on AC_LANG_WERROR
dnl
AC_CHECK_HEADERS([sys/sysctl.h])

dnl
dnl If compiler supports the -static-libgcc flag use it unless we have
dnl GNU ld (which can avoid linking in libgcc when it is not needed).
dnl This test relies on AC_LANG_WERROR
dnl
AS_IF([test -n "$GCC" -a "$lt_cv_prog_gnu_ld" != "yes"], [
    AX_CHECK_COMPILE_FLAG([-static-libgcc], [AX_APPEND_FLAG([-Wc,-static-libgcc], [LT_LDFLAGS])])
])

dnl
dnl We want to use DT_RUNPATH in preference to DT_RPATH in ELF binaries.
dnl Otherwise, LD_LIBRARY_PATH does not work when running the tests.
dnl We don't do this on NetBSD where RPATH already supports LD_LIBRARY_PATH.
dnl
case "$host_os" in
    netbsd*)
	;;
    *)
	AX_CHECK_LINK_FLAG([-Wl,--enable-new-dtags], [AX_APPEND_FLAG([-Wl,--enable-new-dtags], [LDFLAGS])])
	;;
esac

SUDO_CHECK_PIE_SUPPORT
SUDO_SYMBOL_VISIBILITY
SUDO_CHECK_SANITIZER
SUDO_CHECK_HARDENING

dnl
dnl Use passwd auth module?
dnl
case "$with_passwd" in
yes|maybe)
    AUTH_OBJS="$AUTH_OBJS getspwuid.lo passwd.lo"
    AS_IF([test "${ac_cv_search_crypt}" = "no"], [
	AC_MSG_WARN([no crypt function found, assuming plaintext passwords])
    ])
    ;;
*)
    AC_DEFINE(WITHOUT_PASSWD)
    AS_IF([test -z "$AUTH_OBJS"], [
	AC_MSG_ERROR([no authentication methods defined.])
    ])
    ;;
esac
AUTH_OBJS=${AUTH_OBJS# }

dnl
dnl LIBS may contain duplicates from SUDO_LIBS, SUDOERS_LIBS, or NET_LIBS
dnl
AS_IF([test -n "$LIBS"], [
    L="$LIBS"
    LIBS=
    for l in ${L}; do
	dupe=0
	for sl in ${SUDO_LIBS} ${SUDOERS_LIBS} ${NET_LIBS}; do
	    test $l = $sl && dupe=1
	done
	test $dupe = 0 && LIBS="${LIBS} $l"
    done
])

dnl
dnl OS-specific initialization
dnl
AC_DEFINE_UNQUOTED(os_init, $OS_INIT, [Define to an OS-specific initialization function or 'os_init_common'.])

dnl
dnl We add -Wall and -Werror after all tests so they don't cause failures
dnl
AS_IF([test -n "$GCC"], [
    #
    # The fallthrough attribute is supported by gcc 7.0 and clang 10.
    # This test relies on AC_LANG_WERROR.
    #
    AC_CACHE_CHECK([whether $CC supports the fallthrough attribute],
	[sudo_cv_var_fallthrough_attribute],
	[AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
	    int main(int argc, char *argv[])
	    {
		int num = argc + 1;
		switch (num) {
		    case 1:
			num = 0;
			__attribute__((__fallthrough__));
		    case 0:
			num++;
		}
		return num;
	    }
	]])],
	[
	    sudo_cv_var_fallthrough_attribute=yes
	],
	[
	    sudo_cv_var_fallthrough_attribute=no]
	)]
    )
    AS_IF([test X"$sudo_cv_var_fallthrough_attribute" = X"yes"], [
	AC_DEFINE(HAVE_FALLTHROUGH_ATTRIBUTE)
    ])
    AS_IF([test X"$enable_warnings" = X"yes" -o X"$with_devel" = X"yes"], [
	dnl
	dnl Default warnings for development use.
	dnl
	AX_APPEND_FLAG([-Wall], [CFLAGS])
	AX_APPEND_FLAG([-Wmissing-prototypes], [CFLAGS])
	AX_APPEND_FLAG([-Wno-unknown-pragmas], [CFLAGS])
	AX_APPEND_FLAG([-Wpointer-arith], [CFLAGS])
	AX_APPEND_FLAG([-Wshadow], [CFLAGS])
	AX_APPEND_FLAG([-Wsign-compare], [CFLAGS])
	AX_APPEND_FLAG([-Wwrite-strings], [CFLAGS])
	AX_CHECK_COMPILE_FLAG([-Wvla], [AX_APPEND_FLAG([-Wvla], [CFLAGS])])
	AX_CHECK_COMPILE_FLAG([-Walloca], [AX_APPEND_FLAG([-Walloca], [CFLAGS])])
	AX_CHECK_COMPILE_FLAG([-Wtrampolines], [AX_APPEND_FLAG([-Wtrampolines], [CFLAGS])])
	AS_IF([test X"$sudo_cv_var_fallthrough_attribute" = X"yes"], [
	    AX_APPEND_FLAG([-Wimplicit-fallthrough], [CFLAGS])
	])
    ])
    AS_IF([test X"$enable_werror" = X"yes"], [
	AX_APPEND_FLAG([-Werror], [CFLAGS])
    ])
    case "$host_os" in
	# Avoid unwanted warnings on macOS
	darwin*) AX_APPEND_FLAG([-Wno-deprecated-declarations], [CFLAGS]);;
    esac
])

dnl
dnl Skip regress tests and sudoers validation checks if cross compiling.
dnl
CROSS_COMPILING="$cross_compiling"

dnl
dnl Set exec_prefix
dnl
test "$exec_prefix" = "NONE" && exec_prefix='$(prefix)'

dnl
dnl Expand exec_prefix in variables used by the manual pages
dnl
oexec_prefix="$exec_prefix"
if test "$exec_prefix" = '$(prefix)'; then
    if test "$prefix" = "NONE"; then
	exec_prefix="$ac_default_prefix"
    else
	exec_prefix="$prefix"
    fi
fi

# Update exec_prefix in intercept_file
_intercept_file=
while test X"$intercept_file" != X"$_intercept_file"; do
    _intercept_file="$intercept_file"
    eval "intercept_file=\"$_intercept_file\""
done

# Update exec_prefix in noexec_file
_noexec_file=
while test X"$noexec_file" != X"$_noexec_file"; do
    _noexec_file="$noexec_file"
    eval "noexec_file=\"$_noexec_file\""
done

# Update exec_prefix in sesh_file
_sesh_file=
while test X"$sesh_file" != X"$_sesh_file"; do
    _sesh_file="$sesh_file"
    eval "sesh_file=\"$_sesh_file\""
done

# Update exec_prefix in plugindir
_plugindir=
while test X"$plugindir" != X"$_plugindir"; do
    _plugindir="$plugindir"
    eval "plugindir=\"$_plugindir\""
done
exec_prefix="$oexec_prefix"

dnl
dnl Defer setting _PATH_SUDO_NOEXEC, etc until after exec_prefix is set
dnl
AS_IF([test X"$enable_intercept" != X"no"], [
    SUDO_OBJS="${SUDO_OBJS} intercept.pb-c.o"
    PROGS="${PROGS} sudo_intercept.la"
    INSTALL_INTERCEPT="install-intercept"

    SUDO_DEFINE_UNQUOTED(_PATH_SUDO_INTERCEPT, "$intercept_file", [The fully qualified pathname of sudo_intercept.so])
])
AS_IF([test X"$with_noexec" != X"no"], [
    PROGS="${PROGS} sudo_noexec.la"
    INSTALL_NOEXEC="install-noexec"

    SUDO_DEFINE_UNQUOTED(_PATH_SUDO_NOEXEC, "$noexec_file", [The fully qualified pathname of sudo_noexec.so])
])
AS_IF([test X"$with_selinux" != X"no"], [
    SUDO_DEFINE_UNQUOTED(_PATH_SUDO_SESH, "$sesh_file")
], [
    SUDO_DEFINE_UNQUOTED(_PATH_SUDO_SESH, NULL)
])
AS_IF([test X"$enable_shared" != X"no"], [
    SUDO_DEFINE_UNQUOTED(_PATH_SUDO_PLUGIN_DIR, "$plugindir/")
    AC_DEFINE(ENABLE_SUDO_PLUGIN_API, 1, [Define to 1 to enable sudo's plugin interface.])
], [
    SUDO_DEFINE_UNQUOTED(_PATH_SUDO_PLUGIN_DIR, NULL)
])

dnl
dnl Add -R options to LDFLAGS, etc.
dnl
if test X"$LDFLAGS_R" != X""; then
    LDFLAGS="$LDFLAGS $LDFLAGS_R"
fi
if test X"$SUDOERS_LDFLAGS_R" != X""; then
    SUDOERS_LDFLAGS="$SUDOERS_LDFLAGS $SUDOERS_LDFLAGS_R"
fi
if test X"$ZLIB_R" != X""; then
    ZLIB="$ZLIB_R $ZLIB"
fi
if test X"$LIBCRYPTO_R" != X""; then
    LIBCRYPTO="$LIBCRYPTO_R $LIBCRYPTO"
fi
if test X"$LIBTLS_R" != X""; then
    LIBTLS="$LIBTLS_R $LIBTLS"
fi

dnl
dnl Trim leading spaces
dnl
CFLAGS=${CFLAGS# }
CPPFLAGS=${CPPFLAGS# }
LDFLAGS=${LDFLAGS# }
SUDO_LDFLAGS=${SUDO_LDFLAGS# }
SUDOERS_LDFLAGS=${SUDOERS_LDFLAGS# }
LIBS=${LIBS# }
SUDO_LIBS=${SUDO_LIBS# }
SUDOERS_LIBS=${SUDOERS_LIBS# }

dnl
dnl Override default configure dirs for the Makefile
dnl
if test X"$prefix" = X"NONE"; then
    test X"$mandir" = X'${datarootdir}/man' && mandir='$(prefix)/man'
else
    test X"$mandir" = X'${datarootdir}/man' && mandir='$(datarootdir)/man'
fi
test X"$bindir" = X'${exec_prefix}/bin' && bindir='$(exec_prefix)/bin'
test X"$sbindir" = X'${exec_prefix}/sbin' && sbindir='$(exec_prefix)/sbin'
test X"$libexecdir" = X'${exec_prefix}/libexec' && libexecdir='$(exec_prefix)/libexec'
test X"$includedir" = X'${prefix}/include' && includedir='$(prefix)/include'
test X"$datarootdir" = X'${prefix}/share' && datarootdir='$(prefix)/share'
test X"$docdir" = X'${datarootdir}/doc/${PACKAGE_TARNAME}' && docdir='$(datarootdir)/doc/$(PACKAGE_TARNAME)'
test X"$localedir" = X'${datarootdir}/locale' && localedir='$(datarootdir)/locale'
test X"$localstatedir" = X'${prefix}/var' && localstatedir='$(prefix)/var'
test X"$runstatedir" = X'${localstatedir}/run' && runstatedir='$(localstatedir)/run'
test X"$adminconfdir" = X'${prefix}/etc' && adminconfdir='$(prefix)/etc'
test X"$sysconfdir" = X'${prefix}/etc' && sysconfdir='/etc'

# The configuration file search path is to check adminconfdir first and
# fall back to sysconfdir.  This can support systems with read-only
# sysconfdir (/etc) that contains a set of default configuration files.
SUDO_EXPAND_PATH([$sysconfdir], [_sysconfdir])
SUDO_EXPAND_PATH([$adminconfdir], [_adminconfdir])
AS_IF([test $enable_adminconf = yes], [
    # Only use adminconfdir if different from sysconfdir
    AS_IF([test X"$_sysconfdir" != X"$_adminconfdir"], [
	cvtsudoers_conf='$(adminconfdir)/cvtsudoers.conf:'$cvtsudoers_conf
	sudo_conf='$(adminconfdir)/sudo.conf:'$sudo_conf
	sudo_logsrvd_conf='$(adminconfdir)/sudo_logsrvd.conf:'$sudo_logsrvd_conf
	sudoers_path='$(adminconfdir)/sudoers:'$sudoers_path
    ])
])

# Expand config file paths for use in pathnames.h (after config dir override)
SUDO_DEFINE_PATH([$cvtsudoers_conf], [_PATH_CVTSUDOERS_CONF])
SUDO_DEFINE_PATH([$sudo_conf], [_PATH_SUDO_CONF])
SUDO_DEFINE_PATH([$sudo_logsrvd_conf], [_PATH_SUDO_LOGSRVD_CONF])
SUDO_DEFINE_PATH([$sudoers_path], [_PATH_SUDOERS])

# Convert exampledir to something that can be used in the man pages
SUDO_EXPAND_PATH([$exampledir], [EXAMPLES])

dnl
dnl Substitute into the Makefile and man pages
dnl
AS_IF([test X"$INIT_SCRIPT" != X""], [
    AC_CONFIG_FILES([etc/init.d/$INIT_SCRIPT])
], [test X"$TMPFILES_D" != X""], [
    AC_CONFIG_FILES([etc/init.d/sudo.conf])
])

AC_CONFIG_FILES([Makefile docs/Makefile examples/Makefile examples/sudoers examples/sudo.conf examples/sudo_logsrvd.conf examples/syslog.conf include/Makefile lib/eventlog/Makefile lib/fuzzstub/Makefile lib/iolog/Makefile lib/logsrv/Makefile lib/protobuf-c/Makefile lib/ssl_compat/Makefile lib/util/Makefile lib/util/regress/harness lib/util/util.exp logsrvd/Makefile src/intercept.exp src/sudo_usage.h src/Makefile plugins/audit_json/Makefile plugins/sample/Makefile plugins/group_file/Makefile plugins/sample_approval/Makefile plugins/system_group/Makefile plugins/sudoers/Makefile plugins/sudoers/regress/harness plugins/sudoers/sudoers])
AC_CONFIG_COMMANDS([harness], [chmod +x lib/util/regress/harness plugins/sudoers/regress/harness])

AC_OUTPUT

dnl
dnl Summarize configuration
dnl
if test ${LIBTLS+y}; then
    have_tls=yes
fi
echo "" >&AS_MESSAGE_FD
echo "Configured Sudo version $PACKAGE_VERSION" >&AS_MESSAGE_FD
echo " Compiler settings:" >&AS_MESSAGE_FD
echo "  compiler			: $CC" >&AS_MESSAGE_FD
echo "  compiler options		: $CFLAGS" >&AS_MESSAGE_FD
echo "  preprocessor options		: $CPPFLAGS" >&AS_MESSAGE_FD
echo "  front-end libraries		: $SUDO_LIBS" >&AS_MESSAGE_FD
echo "  front-end linker options	: $SUDO_LDFLAGS" >&AS_MESSAGE_FD
echo "  network libraries		: $NET_LIBS" >&AS_MESSAGE_FD
echo "  Crypto library		: $LIBCRYPTO" >&AS_MESSAGE_FD
echo "  TLS libraries			: $LIBTLS" >&AS_MESSAGE_FD
echo "  extra libraries		: $LIBS" >&AS_MESSAGE_FD
echo "  extra linker options		: $LDFLAGS" >&AS_MESSAGE_FD
echo "  sudoers libraries		: $SUDOERS_LIBS" >&AS_MESSAGE_FD
echo "  sudoers linker options	: $SUDOERS_LDFLAGS" >&AS_MESSAGE_FD
if test "${enable_sanitizer-no}" != "no"; then
    echo "  sanitizer options		: ${enable_sanitizer}" >&AS_MESSAGE_FD
fi
if test X"$FUZZ_LD" != X"\$(CC)"; then
    echo "  fuzzing linker		: ${FUZZ_LD}" >&AS_MESSAGE_FD
fi
if test X"$FUZZ_ENGINE" != X"\$(top_builddir)/lib/fuzzstub/libsudo_fuzzstub.la"; then
    echo "  fuzzing engine		: ${FUZZ_ENGINE}" >&AS_MESSAGE_FD
fi
echo " Plugin options:" >&AS_MESSAGE_FD
echo "  plugin support		: ${SHLIB_ENABLE}" >&AS_MESSAGE_FD
echo "  Sudoers plugin static		: ${enable_static_sudoers-no}" >&AS_MESSAGE_FD
echo "  Python plugin			: ${enable_python-no}" >&AS_MESSAGE_FD
if test "${enable_python-no}" != "no"; then
    echo "   Python CFLAGS		: ${PYTHON_INCLUDE}" >&AS_MESSAGE_FD
    echo "   Python LDFLAGS		: ${PYTHON_LIBS}" >&AS_MESSAGE_FD
fi
echo " Optional features:" >&AS_MESSAGE_FD
echo "  log client			: ${enable_log_client-yes}" >&AS_MESSAGE_FD
echo "  log server			: ${enable_log_server-yes}" >&AS_MESSAGE_FD
echo "  log client/server TLS		: ${have_tls-no}" >&AS_MESSAGE_FD
case "$host_os" in
    linux*) echo "  SELinux RBAC			: ${with_selinux-yes}" >&AS_MESSAGE_FD;;
esac
echo " Optional sudoers back-ends:" >&AS_MESSAGE_FD
echo "  LDAP				: ${with_ldap-no}" >&AS_MESSAGE_FD
if test "${with_ldap-no}" != "no"; then
    echo "   ldap configuration		: ${ldap_conf}" >&AS_MESSAGE_FD
    echo "   ldap secret			: ${ldap_secret}" >&AS_MESSAGE_FD
    echo "   SASL authentication		: ${enable_sasl-no}" >&AS_MESSAGE_FD
fi
echo "  SSSD				: ${with_sssd-no}" >&AS_MESSAGE_FD
if test "${with_sssd-no}" != "no"; then
    echo "   SSSD config path		: ${sssd_conf}" >&AS_MESSAGE_FD
    if test "${sssd_lib}" = \""LIBDIR\""; then
	echo "   SSSD lib dir			: ${libdir}" >&AS_MESSAGE_FD
    else
	echo "   SSSD lib dir			: ${sssd_lib}" >&AS_MESSAGE_FD
    fi
fi
echo " Authentication options:" >&AS_MESSAGE_FD
echo "  require authentication	: ${enable_authentication-yes}" >&AS_MESSAGE_FD
auth_methods=`echo "$AUTH_OBJS" | sed -e 's/\.lo//g' -e 's/getspwuid *//'`
echo "  authentication methods	: ${auth_methods}" >&AS_MESSAGE_FD
if test "${with_pam-no}" = "yes"; then
    echo "  pam session support		: ${pam_session}" >&AS_MESSAGE_FD
    echo "  pam login service		: ${pam_login_service}" >&AS_MESSAGE_FD
fi
if test "${with_kerb5-no}" != "no"; then
    echo "  kerb5 instance string	: ${with_kerb5-none}" >&AS_MESSAGE_FD
fi
if test "${with_opie-no}-${with_skey-no}" != "no-no"; then
    echo "  long OTP prompt		: ${long_otp_prompt-no}" >&AS_MESSAGE_FD
fi
echo "  group exempt from passwords	: ${with_exempt-none}" >&AS_MESSAGE_FD
echo "  password prompt		: ${passprompt}" >&AS_MESSAGE_FD
echo "  password prompt timeout	: ${password_timeout} minutes" >&AS_MESSAGE_FD
echo "  password tries		: ${passwd_tries}" >&AS_MESSAGE_FD
echo "  bad password message		: ${badpass_message}" >&AS_MESSAGE_FD
if test "$insults" = "on"; then
    i=""
    test "$enable_offensive_insults" = "yes" && i="offensive ${i}"
    test "$with_python_insults" = "yes" && i="python ${i}"
    test "$with_goons_insults" = "yes" && i="goons ${i}"
    test "$with_hal_insults" = "yes" && i="hal ${i}"
    test "$with_csops_insults" = "yes" && i="csops ${i}"
    test "$with_classic_insults" = "yes" && i="classic ${i}"
else
    i=no
fi
echo "  insults			: $i" >&AS_MESSAGE_FD
echo "  display lecture		: ${lecture}" >&AS_MESSAGE_FD
echo "  timestamp (credential) type	: ${timestamp_type}" >&AS_MESSAGE_FD
echo "  timestamp (credential) timeout: ${timeout} minutes" >&AS_MESSAGE_FD
echo " Logging options:" >&AS_MESSAGE_FD
echo "  logging default		: ${with_logging}" >&AS_MESSAGE_FD
echo "  syslog facility		: ${logfac}" >&AS_MESSAGE_FD
echo "  syslog priority allowed	: ${goodpri}" >&AS_MESSAGE_FD
echo "  syslog priority denied	: ${badpri}" >&AS_MESSAGE_FD
echo "  log file path			: ${logpath}" >&AS_MESSAGE_FD
echo "  log file includes hostname	: ${enable_log_host-no}" >&AS_MESSAGE_FD
echo "  log file line length		: ${loglen}" >&AS_MESSAGE_FD
echo "  compress I/O logs		: ${enable_zlib}" >&AS_MESSAGE_FD
case "$host_os" in
    linux*) echo "  Linux audit			: ${with_linux_audit-no}" >&AS_MESSAGE_FD;;
    solaris2.11*) echo "  Solaris audit			: ${with_solaris_audit-no}" >&AS_MESSAGE_FD;;
    *) echo "  BSM audit			: ${with_bsm_audit-no}" >&AS_MESSAGE_FD;;
esac
echo "  run mailer as root		: ${enable_root_mailer-yes}" >&AS_MESSAGE_FD
echo "  warning/error mail recipient	: ${mailto}" >&AS_MESSAGE_FD
echo "  warning/error mail subject	: ${mailsub}" >&AS_MESSAGE_FD
echo "  mail if user not in sudoers	: ${mail_no_user}" >&AS_MESSAGE_FD
echo "  mail if user not on host	: ${mail_no_host}" >&AS_MESSAGE_FD
echo "  mail if command not allowed	: ${mail_no_perms}" >&AS_MESSAGE_FD
echo " Pathnames:" >&AS_MESSAGE_FD
echo "  prefix			: $prefix" >&AS_MESSAGE_FD
echo "  sysconfdir			: $_sysconfdir" >&AS_MESSAGE_FD
if test "${enable_adminconf-no}" != "no"; then
    echo "  adminconfdir			: $_adminconfdir" >&AS_MESSAGE_FD
fi
echo "  log directory			: ${log_dir}" >&AS_MESSAGE_FD
echo "  run directory			: ${rundir}" >&AS_MESSAGE_FD
echo "  var directory			: ${vardir}" >&AS_MESSAGE_FD
echo "  I/O log directory		: ${iolog_dir}" >&AS_MESSAGE_FD
echo "  sudo_logsrvd relay directory	: ${relay_dir}" >&AS_MESSAGE_FD
if test X"$with_exampledir" != X""; then
    echo "  exampledir			: $exampledir" >&AS_MESSAGE_FD
fi
echo "  plugin directory		: ${plugindir}" >&AS_MESSAGE_FD
echo "  sudoers plugin		: ${sudoers_plugin}" >&AS_MESSAGE_FD
if test "${enable_python-no}" != "no"; then
    echo "  python plugin			: ${python_plugin}" >&AS_MESSAGE_FD
fi
echo "  sudoers file			: ${sudoers_path}" >&AS_MESSAGE_FD
echo "  cvtsudoers.conf file		: ${cvtsudoers_conf}" >&AS_MESSAGE_FD
echo "  sudo.conf file		: ${sudo_conf}" >&AS_MESSAGE_FD
echo "  sudo_logsrvd.conf file	: ${sudo_logsrvd_conf}" >&AS_MESSAGE_FD
echo "  path to sendmail		: ${with_sendmail}" >&AS_MESSAGE_FD
echo "  time zone directory		: ${tzdir}" >&AS_MESSAGE_FD
if test -n "$TMPFILES_D"; then
    echo "  systemd tempfiles dir	: ${TMPFILES_D}" >&AS_MESSAGE_FD
fi
if test ${with_netsvc-"no"} != "no"; then
    echo "  netsvc file			: ${netsvc_conf}" >&AS_MESSAGE_FD
elif test ${with_nsswitch-"yes"} != "no"; then
    echo "  nsswitch file			: ${nsswitch_conf}" >&AS_MESSAGE_FD
fi
echo "  intercept file		: ${intercept_file}" >&AS_MESSAGE_FD
echo "  noexec file			: ${noexec_file}" >&AS_MESSAGE_FD
echo "  secure path			: ${with_secure_path-no}" >&AS_MESSAGE_FD
echo "  askpass helper file		: ${with_askpass-no}" >&AS_MESSAGE_FD
echo "  device search path		: ${devsearch}" >&AS_MESSAGE_FD
echo " Other options:" >&AS_MESSAGE_FD
if test "${with_devel-no}" != "no"; then
    echo "  development build		: ${with_devel}" >&AS_MESSAGE_FD
fi
case "$host_os" in
    solaris2*) echo "  Solaris project support	: ${with_project-no}" >&AS_MESSAGE_FD;;
esac
if test "${with_logincap+set}" = "set"; then
    echo "  /etc/login.conf support	: ${with_logincap}" >&AS_MESSAGE_FD
fi
echo "  fully-qualified domain names	: ${fqdn}" >&AS_MESSAGE_FD
echo "  default umask			: ${sudo_umask}" >&AS_MESSAGE_FD
echo "  umask override		: ${umask_override}" >&AS_MESSAGE_FD
echo "  default runas user		: ${runas_default}" >&AS_MESSAGE_FD
echo "  probe network interfaces	: ${with_interfaces-yes}" >&AS_MESSAGE_FD
echo "  allow root to run sudo	: ${root_sudo}" >&AS_MESSAGE_FD
echo "  reset environment for commands: ${env_reset}" >&AS_MESSAGE_FD
echo "  run shell if no args		: ${enable_noargs_shell-no}" >&AS_MESSAGE_FD
echo "  ignore '.' or '' in \$PATH	: ${ignore_dot}" >&AS_MESSAGE_FD
echo "  disable path info		: ${enable_path_info-no}" >&AS_MESSAGE_FD
echo "  sudoers file mode		: ${SUDOERS_MODE}" >&AS_MESSAGE_FD
echo "  sudoers file owner		: ${SUDOERS_UID}:${SUDOERS_GID}" >&AS_MESSAGE_FD
echo "  default visudo editor		: ${editor}" >&AS_MESSAGE_FD
echo "  visudo supports \$EDITOR	: ${env_editor}" >&AS_MESSAGE_FD
if test "${enable_env_debug+set}" = "set"; then
    echo "  environment debugging	: ${enable_env_debug-no}" >&AS_MESSAGE_FD
fi
echo "" >&AS_MESSAGE_FD

dnl
dnl Display any warnings/info the user needs to know about at the end.
dnl
AS_IF([test "$openssl_missing" = "yes"], [
    AC_MSG_WARN([OpenSSL dev libraries not found, Sudo logsrv connections will not be encrypted.])
])
AS_IF([test "$with_pam" = "yes"], [
    case $host_os in
	hpux*)
	    AS_IF([test -f /usr/lib/security/libpam_hpsec.so.1], [
		AC_MSG_NOTICE([you may wish to add the following line to /etc/pam.conf])
		AC_MSG_NOTICE([sudo session required libpam_hpsec.so.1 bypass_umask bypass_last_login])
	    ])
	    ;;
	linux*)
	    AC_MSG_NOTICE([you will need to customize examples/pam.conf and install it as /etc/pam.d/sudo])
	    ;;
    esac
])
dnl
dnl Warn user if they may need to clear rundir manually.
dnl
case "$rundir" in
    /run/*|/var/run/*)
	clear_rundir=0
	;;
    *)
	clear_rundir=1
	;;
esac
AS_IF([test $clear_rundir -eq 1], [
    AC_MSG_NOTICE([warning: the $rundir/ts directory must be cleared at boot time.])
    AC_MSG_NOTICE([         You may need to create a startup item to do this.])
])

dnl
dnl Autoheader templates
dnl
AH_TEMPLATE(CLASSIC_INSULTS, [Define to 1 if you want the insults from the "classic" version sudo.])
AH_TEMPLATE(CSOPS_INSULTS, [Define to 1 if you want insults culled from the twisted minds of CSOps.])
AH_TEMPLATE(DONT_LEAK_PATH_INFO, [Define to 1 if you want sudo to display "command not allowed" instead of "command not found" when a command cannot be found.])
AH_TEMPLATE(ENV_DEBUG, [Define to 1 to enable environment function debugging.])
AH_TEMPLATE(ENV_EDITOR, [Define to 1 if you want visudo to honor the EDITOR and VISUAL env variables.])
AH_TEMPLATE(FQDN, [Define to 1 if you want to require fully qualified hosts in sudoers.])
AH_TEMPLATE(ENV_RESET, [Define to 1 to enable environment resetting by default.])
AH_TEMPLATE(PYTHON_INSULTS, [Define to 1 if you want insults from "Monty Python's Flying Circus".])
AH_TEMPLATE(GOONS_INSULTS, [Define to 1 if you want insults from the "Goon Show".])
AH_TEMPLATE(HAL_INSULTS, [Define to 1 if you want 2001-like insults.])
AH_TEMPLATE(HAVE_AFS, [Define to 1 if you use AFS.])
AH_TEMPLATE(HAVE_AIXAUTH, [Define to 1 if you use AIX general authentication.])
AH_TEMPLATE(HAVE_BSD_AUTH_H, [Define to 1 if you use BSD authentication.])
AH_TEMPLATE(HAVE_BSM_AUDIT, [Define to 1 to enable BSM audit support.])
AH_TEMPLATE(HAVE_CRYPT, [Define to 1 if you have the 'crypt' function.])
AH_TEMPLATE(HAVE_DCE, [Define to 1 if you use OSF DCE.])
AH_TEMPLATE(HAVE_DD_FD, [Define to 1 if your 'DIR' contains dd_fd.])
AH_TEMPLATE(HAVE_DIRFD, [Define to 1 if you have the 'dirfd' function or macro.])
AH_TEMPLATE(HAVE_DISPCRYPT, [Define to 1 if you have the 'dispcrypt' function.])
AH_TEMPLATE(HAVE_DLOPEN, [Define to 1 if you have the 'dlopen' function.])
AH_TEMPLATE(HAVE_FCNTL_CLOSEM, [Define to 1 if your system has the F_CLOSEM fcntl.])
AH_TEMPLATE(HAVE_FNMATCH, [Define to 1 if you have the 'fnmatch' function.])
AH_TEMPLATE(HAVE_FWTK, [Define to 1 if you use the FWTK authsrv daemon.])
AH_TEMPLATE(HAVE_GETPRPWNAM, [Define to 1 if you have the 'getprpwnam' function.  (SecureWare-style shadow passwords).])
AH_TEMPLATE(HAVE_GETPWNAM_SHADOW, [Define to 1 if you have the 'getpwnam_shadow' function.])
AH_TEMPLATE(HAVE_GETSPNAM, [Define to 1 if you have the 'getspnam' function (SVR4-style shadow passwords).])
AH_TEMPLATE(HAVE_GSS_KRB5_CCACHE_NAME, [Define to 1 if you have the 'gss_krb5_ccache_name' function.])
AH_TEMPLATE(HAVE_HEIMDAL, [Define to 1 if your Kerberos is Heimdal.])
AH_TEMPLATE(HAVE_INET_NTOP, [Define to 1 if you have the 'inet_ntop' function.])
AH_TEMPLATE(HAVE_INET_PTON, [Define to 1 if you have the 'inet_pton' function.])
AH_TEMPLATE(HAVE_ISCOMSEC, [Define to 1 if you have the 'iscomsec' function. (HP-UX >= 10.x check for shadow enabled).])
AH_TEMPLATE(HAVE_KERB5, [Define to 1 if you use Kerberos V.])
AH_TEMPLATE(HAVE_KRB5_GET_INIT_CREDS_OPT_ALLOC, [Define to 1 if you have the 'krb5_get_init_creds_opt_alloc' function.])
AH_TEMPLATE(HAVE_KRB5_GET_INIT_CREDS_OPT_FREE_TWO_ARGS, [Define to 1 if your 'krb5_get_init_creds_opt_free' function takes two arguments.])
AH_TEMPLATE(HAVE_KRB5_INIT_SECURE_CONTEXT, [Define to 1 if you have the 'krb5_init_secure_context' function.])
AH_TEMPLATE(HAVE_KRB5_VERIFY_USER, [Define to 1 if you have the 'krb5_verify_user' function.])
AH_TEMPLATE(HAVE_LBER_H, [Define to 1 if your LDAP needs <lber.h>. (OpenLDAP does not).])
AH_TEMPLATE(HAVE_LDAP, [Define to 1 if you use LDAP for sudoers.])
AH_TEMPLATE(HAVE_LIBINTL_H, [Define to 1 if you have the <libintl.h> header file.])
AH_TEMPLATE(HAVE_LINUX_AUDIT, [Define to 1 to enable Linux audit support.])
AH_TEMPLATE(HAVE_SSSD, [Define to 1 to enable SSSD support.])
AH_TEMPLATE(HAVE_OPIE, [Define to 1 if you use NRL OPIE.])
AH_TEMPLATE(HAVE_OPTRESET, [Define to 1 if you have the 'optreset' symbol.])
AH_TEMPLATE(HAVE_PAM, [Define to 1 if you use PAM authentication.])
AH_TEMPLATE(HAVE_PAM_LOGIN, [Define to 1 if you use a specific PAM session for sudo -i.])
AH_TEMPLATE(HAVE_PROJECT_H, [Define to 1 if you have the <project.h> header file.])
AH_TEMPLATE(HAVE_SECURID, [Define to 1 if you use SecurID for authentication.])
AH_TEMPLATE(HAVE_SELINUX, [Define to 1 to enable SELinux RBAC support.])
AH_TEMPLATE(HAVE_SETKEYCREATECON, [Define to 1 if you have the 'setkeycreatecon' function.])
AH_TEMPLATE(HAVE_APPARMOR, [Define to 1 to enable AppArmor support.])
AH_TEMPLATE(HAVE_SHL_LOAD, [Define to 1 if you have the 'shl_load' function.])
AH_TEMPLATE(HAVE_SKEY, [Define to 1 if you use S/Key.])
AH_TEMPLATE(HAVE_SKEYACCESS, [Define to 1 if your S/Key library has skeyaccess().])
AH_TEMPLATE(HAVE_RFC1938_SKEYCHALLENGE, [Define to 1 if the skeychallenge() function is RFC1938-compliant and takes 4 arguments.])
AH_TEMPLATE(HAVE_SOLARIS_AUDIT, [Define to 1 to enable Solaris audit support.])
AH_TEMPLATE(HAVE_ST__TIM, [Define to 1 if your struct stat uses an st__tim union.])
AH_TEMPLATE(HAVE_ST_MTIM, [Define to 1 if your struct stat has an st_mtim member.])
AH_TEMPLATE(HAVE_ST_MTIMESPEC, [Define to 1 if your struct stat has an st_mtimespec member.])
AH_TEMPLATE(HAVE_ST_NMTIME, [Define to 1 if your struct stat has an st_nmtime member.])
AH_TEMPLATE(HAVE___PROGNAME, [Define to 1 if your crt0.o defines the __progname symbol for you.])
AH_TEMPLATE(HOST_IN_LOG, [Define to 1 if you want the hostname to be entered into the log file.])
AH_TEMPLATE(IGNORE_DOT_PATH, [Define to 1 if you want to ignore '.' and empty PATH elements.])
AH_TEMPLATE(LOGGING, [Define to SLOG_SYSLOG, SLOG_FILE, or SLOG_BOTH.])
AH_TEMPLATE(LONG_OTP_PROMPT, [Define to 1 if you want a two line OTP (S/Key or OPIE) prompt.])
AH_TEMPLATE(NO_AUTHENTICATION, [Define to 1 if you don't want sudo to prompt for a password by default.])
AH_TEMPLATE(NO_LEAKS, [Define to 1 if you want sudo to free up memory before exiting.])
AH_TEMPLATE(NO_LECTURE, [Define to 1 if you don't want users to get the lecture the first time they use sudo.])
AH_TEMPLATE(NO_PAM_SESSION, [Define to 1 if you don't want to use sudo's PAM session support.])
AH_TEMPLATE(NO_ROOT_MAILER, [Define to avoid running the mailer as root.])
AH_TEMPLATE(NO_ROOT_SUDO, [Define to 1 if root should not be allowed to use sudo.])
AH_TEMPLATE(TIMESTAMP_TYPE, [Define to global, ppid or tty to set the default timestamp record type.])
AH_TEMPLATE(OFFENSIVE_INSULTS, [Define to 1 to include offensive insults from the classic version of sudo.])
AH_TEMPLATE(SECURE_PATH, [A colon-separated list of directories to override the user's PATH with.])
AH_TEMPLATE(SEND_MAIL_WHEN_NOT_OK, [Define to 1 to send mail when the user is not allowed to run a command.])
AH_TEMPLATE(SEND_MAIL_WHEN_NO_HOST, [Define to 1 to send mail when the user is not allowed to run sudo on this host.])
AH_TEMPLATE(SEND_MAIL_WHEN_NO_USER, [Define to 1 to send mail when the user is not in the sudoers file.])
AH_TEMPLATE(SHELL_IF_NO_ARGS, [Define to 1 if you want sudo to start a shell if given no arguments.])
AH_TEMPLATE(SHELL_SETS_HOME, [Define to 1 if you want sudo to set $HOME in shell mode.])
AH_TEMPLATE(STATIC_SUDOERS_PLUGIN, [Define to 1 to compile the sudoers plugin statically into the sudo binary.])
AH_TEMPLATE(STUB_LOAD_INTERFACES, [Define to 1 if the code in interfaces.c does not compile for you.])
AH_TEMPLATE(UMASK_OVERRIDE, [Define to 1 to use the umask specified in sudoers even when it is less restrictive than the invoking user's.])
AH_TEMPLATE(USE_INSULTS, [Define to 1 if you want to insult the user for entering an incorrect password.])
AH_TEMPLATE(USE_STOW, [Define to 1 if you use GNU stow packaging.])
AH_TEMPLATE(WITHOUT_PASSWD, [Define to avoid using the passwd/shadow file for authentication.])
AH_TEMPLATE(HAVE___FUNC__, [Define to 1 if the compiler supports the C99 __func__ variable.])
AH_TEMPLATE(HAVE___INTERPOSE, [Define to 1 if you have dyld with __interpose attribute support.])
AH_TEMPLATE(SUDO_KRB5_INSTANCE, [An instance string to append to the username (separated by a slash) for Kerberos V authentication.])
AH_TEMPLATE(RTLD_PRELOAD_VAR, [The environment variable that controls preloading of dynamic objects.])
AH_TEMPLATE(RTLD_PRELOAD_VAR_32, [The environment variable that controls preloading of 32-bit dynamic objects.])
AH_TEMPLATE(RTLD_PRELOAD_VAR_64, [The environment variable that controls preloading of 64-bit dynamic objects.])
AH_TEMPLATE(RTLD_PRELOAD_ENABLE_VAR, [An extra environment variable that is required to enable preloading (if any).])
AH_TEMPLATE(RTLD_PRELOAD_DELIM, [The delimiter to use when defining multiple preloaded objects.])
AH_TEMPLATE(RTLD_PRELOAD_DEFAULT, [The default value of preloaded objects (if any).])
AH_TEMPLATE(HAVE_DSO_VISIBILITY, [Define to 1 if the compiler supports the __visibility__ attribute.])
AH_TEMPLATE(HAVE_SYS_SIGABBREV, [Define to 1 if your libc has the 'sys_sigabbrev' symbol.])
AH_TEMPLATE(HAVE_NSS_SEARCH, [Define to 1 if you have the 'nss_search' function.])
AH_TEMPLATE(HAVE__NSS_INITF_GROUP, [Define to 1 if you have the '_nss_initf_group' function.])
AH_TEMPLATE(HAVE___NSS_INITF_GROUP, [Define to 1 if you have the '__nss_initf_group' function.])
AH_TEMPLATE(HAVE__NSS_XBYY_BUF_ALLOC, [Define to 1 if you have the '_nss_XbyY_buf_alloc' function.])
AH_TEMPLATE(HAVE___NSS_XBYY_BUF_ALLOC, [Define to 1 if you have the '__nss_XbyY_buf_alloc' function.])
AH_TEMPLATE(NEED_RESOLV_H, [Define to 1 if resolv.h must be included to get the 'inet_ntop' or 'inet_pton' function prototypes.])
AH_TEMPLATE(HAVE_STRNLEN, [Define to 1 if you have the 'strnlen' function.])
AH_TEMPLATE(PAM_SUN_CODEBASE, [Define to 1 if your system uses a Solaris-derived PAM and not Linux-PAM or OpenPAM.])
AH_TEMPLATE(HAVE_KINFO_PROC_44BSD, [Define to 1 if your system has a 4.4BSD-style kinfo_proc struct.])
AH_TEMPLATE(HAVE_KINFO_PROC_FREEBSD, [Define to 1 if your system has a FreeBSD-style kinfo_proc struct.])
AH_TEMPLATE(HAVE_KINFO_PROC_DFLY, [Define to 1 if your system has a Dragonfly-style kinfo_proc struct.])
AH_TEMPLATE(HAVE_KINFO_PROC2_NETBSD, [Define to 1 if your system has a NetBSD-style kinfo_proc2 struct.])
AH_TEMPLATE(HAVE_KINFO_PROC_OPENBSD, [Define to 1 if your system has an OpenBSD-style kinfo_proc struct.])
AH_TEMPLATE(HAVE_OPENSSL, [Define to 1 if you are using OpenSSL's TLS and sha2 functions.])
AH_TEMPLATE(HAVE_WOLFSSL, [Define to 1 if you are using wolfSSL's TLS and sha2 functions.])
AH_TEMPLATE(HAVE_GCRYPT, [Define to 1 if you are using gcrypt's sha2 functions.])
AH_TEMPLATE(HAVE_SSL_CTX_SET_MIN_PROTO_VERSION, [Define to 1 if you have the 'SSL_CTX_set_min_proto_version' function or macro.])
AH_TEMPLATE(HAVE_SSL_CTX_SET_CIPHERSUITES, [Define to 1 if you have the 'SSL_CTX_set_ciphersuites' function or macro.])
AH_TEMPLATE(SUDOERS_LOG_CLIENT, [Define to 1 to compile support for sudo_logsrvd in the sudoers plugin.])
AH_TEMPLATE(HAVE_FALLTHROUGH_ATTRIBUTE, [Define to 1 if the compiler supports the fallthrough attribute.])
AH_TEMPLATE(HAVE_VA_COPY, [Define to 1 if you have the 'va_copy' function.])
AH_TEMPLATE(HAVE___VA_COPY, [Define to 1 if you have the '__va_copy' function.])

dnl
dnl Bits to copy verbatim into config.h.in
dnl
AH_TOP([#ifndef SUDO_CONFIG_H
#define SUDO_CONFIG_H

/* Configure script arguments used to build sudo. */
#undef CONFIGURE_ARGS])

AH_BOTTOM([#ifndef HAVE_SIG_ATOMIC_T
typedef int			sig_atomic_t;
#endif

#ifndef HAVE_SOCKLEN_T
typedef unsigned int		socklen_t;
#endif

#ifndef __GNUC_PREREQ__
# ifdef __GNUC__
#  define __GNUC_PREREQ__(ma, mi) \
	((__GNUC__ > (ma)) || (__GNUC__ == (ma) && __GNUC_MINOR__ >= (mi)))
# else
#  define __GNUC_PREREQ__(ma, mi)	0
# endif
#endif

/* Define away __attribute__ for non-gcc or old gcc. */
#if !defined(__attribute__) && !__GNUC_PREREQ__(2, 5)
# define __attribute__(x)
#endif

/* For functions that call exit() directly. */
#ifdef __has_c_attribute
# if __has_c_attribute(__noreturn__)
#  define sudo_noreturn		[[__noreturn__]]
# endif
#endif
#ifndef sudo_noreturn
# if __GNUC_PREREQ__(2, 5)
#  define sudo_noreturn		__attribute__((__noreturn__))
# else
#  define sudo_noreturn
# endif
#endif

/* For malloc-like functions that return uninitialized or zeroed memory. */
#if __GNUC_PREREQ__(2, 96)
# define sudo_malloclike	__attribute__((__malloc__))
#else
# define sudo_malloclike
#endif

/* Compile-time checking for function arguments that must not be NULL. */
#if __GNUC_PREREQ__(3, 3)
# define sudo_attr_nonnull(_a)	__attribute__((__nonnull__ (_a)))
#else
# define sudo_attr_nonnull(_a)
#endif

/* For catching format string mismatches. */
#if __GNUC_PREREQ__(2, 7)
# define sudo_printflike(_f, _v)	__attribute__((__format__ (__printf__, _f, _v))) sudo_attr_nonnull(_f)
# define sudo_printf0like(_f, _v)	__attribute__((__format__ (__printf__, _f, _v)))
# define sudo_attr_fmt_arg(_f)		__attribute__((__format_arg__ (_f)))
#else
# define sudo_printflike(_f, _v)
# define sudo_printf0like(_f, _v)
# define sudo_attr_fmt_arg(_f)
#endif

/* C23 defines a fallthrough attribute, gcc 7.0 and clang 10 have their own. */
#ifdef __has_c_attribute
# if __has_c_attribute(__fallthrough__)
# define FALLTHROUGH		[[__fallthrough__]]
# endif
#endif
#ifndef FALLTHROUGH
# if defined(HAVE_FALLTHROUGH_ATTRIBUTE)
#  define FALLTHROUGH		__attribute__((__fallthrough__))
# else
#  define FALLTHROUGH		do { } while (0)
# endif
#endif

/* Symbol visibility controls. */
#ifdef HAVE_DSO_VISIBILITY
# if defined(__GNUC__)
#  define sudo_dso_public __attribute__((__visibility__("default")))
# elif defined(__SUNPRO_C)
#  define sudo_dso_public __global
# else
#  define sudo_dso_public __declspec(dllexport)
# endif
#else
# define sudo_dso_public
#endif

/* BSD compatibility on some SVR4 systems. */
#ifdef __svr4__
# define BSD_COMP
#endif

/* Enable BSD extensions on systems that have them.  */
#ifndef _BSD_SOURCE
# undef _BSD_SOURCE
#endif

/* Enable OpenBSD extensions on NetBSD.  */
#ifndef _OPENBSD_SOURCE
# undef _OPENBSD_SOURCE
#endif

/* Enable BSD types on IRIX.  */
#ifndef _BSD_TYPES
# undef _BSD_TYPES
#endif

/* Enable Linux-compatible extensions on AIX.  */
#ifndef _LINUX_SOURCE_COMPAT
# undef _LINUX_SOURCE_COMPAT
#endif

/* Enable unlimited getgroups(2) support on macOS. */
#ifndef _DARWIN_UNLIMITED_GETGROUPS
# undef _DARWIN_UNLIMITED_GETGROUPS
#endif

/* Enable prototypes in GCC fixed includes on older systems.  */
#ifndef __USE_FIXED_PROTOTYPES__
# undef __USE_FIXED_PROTOTYPES__
#endif

/* Enable XPG4v2 extensions to POSIX, needed for MSG_WAITALL on older HP-UX.  */
#ifndef _XOPEN_SOURCE_EXTENDED
# undef _XOPEN_SOURCE_EXTENDED
#endif

/* Enable reentrant versions of the standard C API (obsolete).  */
#ifndef _REENTRANT
# undef _REENTRANT
#endif

/* Enable "safer" versions of the standard C API (ISO C11).  */
#ifndef __STDC_WANT_LIB_EXT1__
# undef __STDC_WANT_LIB_EXT1__
#endif

/* Prevent static analyzers from genering bogus memory leak warnings. */
#if defined(__COVERITY__) && !defined(NO_LEAKS)
# define NO_LEAKS
#endif

#endif /* SUDO_CONFIG_H */])
