dnl
dnl Configure script for bash-5.2
dnl
dnl report bugs to chet@po.cwru.edu
dnl
dnl Process this file with autoconf to produce a configure script.

# Copyright (C) 1987-2022 Free Software Foundation, Inc.

#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.

AC_REVISION([for Bash 5.2, version 5.046])dnl

define(bashvers, 5.2)
define(relstatus, release)

AC_INIT([bash], bashvers-relstatus, [bug-bash@gnu.org])

dnl make sure we are using a recent autoconf version
AC_PREREQ(2.69)

AC_CONFIG_SRCDIR(shell.h)
dnl where to find install.sh, config.sub, and config.guess
AC_CONFIG_AUX_DIR(./support)
AC_CONFIG_HEADERS(config.h)

dnl checks for version info
BASHVERS=bashvers
RELSTATUS=relstatus

dnl defaults for debug settings
case "$RELSTATUS" in
alp*|bet*|dev*|rc*|releng*|maint*)	DEBUG='-DDEBUG' MALLOC_DEBUG='-DMALLOC_DEBUG' ;;
*)	DEBUG= MALLOC_DEBUG= ;;
esac

dnl canonicalize the host and os so we can do some tricky things before
dnl parsing options
AC_CANONICAL_HOST
AC_CANONICAL_BUILD

dnl configure defaults
opt_bash_malloc=yes
opt_afs=no
opt_curses=no
opt_with_installed_readline=no

#htmldir=

dnl some systems should be configured without the bash malloc by default
dnl and some need a special compiler or loader
dnl look in the NOTES file for more
case "${host_cpu}-${host_os}" in
# mostly obsolete platforms
alpha*-*)	opt_bash_malloc=no ;;	# alpha running osf/1 or linux
*[[Cc]]ray*-*)	opt_bash_malloc=no ;;	# Crays
*-osf1*)	opt_bash_malloc=no ;;	# other osf/1 machines
*-dgux*)	opt_bash_malloc=no ;;	# DG/UX machines
sparc-svr4*)	opt_bash_malloc=no ;;	# sparc SVR4, SVR4.2
m68k-sysv)	opt_bash_malloc=no ;;	# fixes file descriptor leak in closedir
*-bsdi2.1|*-bsdi3.?)	opt_bash_malloc=no ; : ${CC:=shlicc2} ;; # for loadable builtins
*-opennt*|*-interix*)	opt_bash_malloc=no ;;	# Interix, now owned by Microsoft
*-beos*)	opt_bash_malloc=no ;;	# they say it's suitable
# These need additional investigation
sparc-linux*)	opt_bash_malloc=no ;;	# sparc running linux; requires ELF
*-aix*)		opt_bash_malloc=no ;;	# AIX machines
*-cygwin*)	opt_bash_malloc=no ;;	# Cygnus's CYGWIN environment
# These lack a working sbrk(2)
aarch64-freebsd*)	opt_bash_malloc=no ;;
riscv*-freebsd*)	opt_bash_malloc=no ;;
# Mach-derived systems have a ton of extra malloc functions and lack sbrk(2)
*-nextstep*)	opt_bash_malloc=no ;;	# NeXT machines running NeXTstep
*-openstep*)	opt_bash_malloc=no ;;	# i386/Sparc/HP machines running Openstep
*-macos*)	opt_bash_malloc=no ;;	# Apple MacOS X
*-rhapsody*)	opt_bash_malloc=no ;;	# Apple Rhapsody (MacOS X)
*-darwin*)	opt_bash_malloc=no ;;	# Apple Darwin (MacOS X)
*-machten4)	opt_bash_malloc=no ;;	# MachTen 4.x
# Niche or non-mainstream-shell-user systems
*-qnx*)		opt_bash_malloc=no ;;	# QNX 4.2, QNX [67].x
*-nsk*)		opt_bash_malloc=no ;;	# HP NonStop
*-haiku*)	opt_bash_malloc=no ;;	# Haiku OS
*-genode*)	opt_bash_malloc=no ;;	# Genode has no sbrk
esac

# memory scrambling on free()
case "${host_os}" in
sco3.2v5*|sco3.2v4*)	opt_memscramble=no ;;
*)			opt_memscramble=yes ;;
esac

dnl
dnl macros for the bash debugger
dnl
dnl AM_PATH_LISPDIR
AC_ARG_VAR(DEBUGGER_START_FILE, [location of bash debugger initialization file])

dnl arguments to configure
dnl packages
AC_ARG_WITH(afs, AS_HELP_STRING([--with-afs], [if you are running AFS]), opt_afs=$withval)
AC_ARG_WITH(bash-malloc, AS_HELP_STRING([--with-bash-malloc], [use the Bash version of malloc]), opt_bash_malloc=$withval)
AC_ARG_WITH(curses, AS_HELP_STRING([--with-curses], [use the curses library instead of the termcap library]), opt_curses=$withval)
AC_ARG_WITH(gnu-malloc, AS_HELP_STRING([--with-gnu-malloc], [synonym for --with-bash-malloc]), opt_bash_malloc=$withval)
AC_ARG_WITH(installed-readline, AS_HELP_STRING([--with-installed-readline], [use a version of the readline library that is already installed]), opt_with_installed_readline=$withval)

if test "$opt_bash_malloc" = yes; then
	MALLOC_TARGET=malloc
	MALLOC_SRC=malloc.c

	MALLOC_LIB='-lmalloc'
	MALLOC_LIBRARY='$(ALLOC_LIBDIR)/libmalloc.a'
	MALLOC_LDFLAGS='-L$(ALLOC_LIBDIR)'
	MALLOC_DEP='$(MALLOC_LIBRARY)'

	AC_DEFINE(USING_BASH_MALLOC)
else
	MALLOC_LIB=
	MALLOC_LIBRARY=
	MALLOC_LDFLAGS=
	MALLOC_DEP=	
fi

if test "$opt_afs" = yes; then
	AC_DEFINE(AFS)
fi

if test "$opt_curses" = yes; then
	prefer_curses=yes
fi

if test -z "${DEBUGGER_START_FILE}"; then
	DEBUGGER_START_FILE='${datadir}/bashdb/bashdb-main.inc'
fi

dnl optional shell features in config.h.in
opt_minimal_config=no

opt_job_control=yes
opt_alias=yes
opt_readline=yes
opt_history=yes
opt_bang_history=yes
opt_dirstack=yes
opt_restricted=yes
opt_process_subst=yes
opt_prompt_decoding=yes
opt_select=yes
opt_help=yes
opt_array_variables=yes
opt_dparen_arith=yes
opt_extended_glob=yes
opt_brace_expansion=yes
opt_disabled_builtins=no
opt_command_timing=yes
opt_xpg_echo=no
opt_strict_posix=no
opt_cond_command=yes
opt_cond_regexp=yes
opt_coproc=yes
opt_arith_for_command=yes
opt_net_redirs=yes
opt_progcomp=yes
opt_separate_help=no
opt_multibyte=yes
opt_debugger=yes
opt_single_longdoc_strings=yes
opt_casemod_attrs=yes
opt_casemod_expansions=yes
opt_extglob_default=no
opt_dircomplete_expand_default=no
opt_globascii_default=yes
opt_function_import=yes
opt_dev_fd_stat_broken=no
opt_alt_array_impl=no
opt_translatable_strings=yes

dnl modified by alternate array implementation option
ARRAY_O=array.o

dnl options that affect how bash is compiled and linked
opt_static_link=no
opt_profiling=no

dnl argument parsing for optional features
AC_ARG_ENABLE(minimal-config, AS_HELP_STRING([--enable-minimal-config], [a minimal sh-like configuration]), opt_minimal_config=$enableval)

dnl a minimal configuration turns everything off, but features can be
dnl added individually
if test $opt_minimal_config = yes; then
	opt_job_control=no opt_alias=no opt_readline=no
	opt_history=no opt_bang_history=no opt_dirstack=no
	opt_restricted=no opt_process_subst=no opt_prompt_decoding=no
	opt_select=no opt_help=no opt_array_variables=no opt_dparen_arith=no
	opt_brace_expansion=no opt_disabled_builtins=no opt_command_timing=no
	opt_extended_glob=no opt_cond_command=no opt_arith_for_command=no
	opt_net_redirs=no opt_progcomp=no opt_separate_help=no
	opt_multibyte=yes opt_cond_regexp=no opt_coproc=no
	opt_casemod_attrs=no opt_casemod_expansions=no opt_extglob_default=no
	opt_translatable_strings=no
	opt_globascii_default=yes
fi

AC_ARG_ENABLE(alias, AS_HELP_STRING([--enable-alias], [enable shell aliases]), opt_alias=$enableval)
AC_ARG_ENABLE(alt-array-implementation, AS_HELP_STRING([--enable-alt-array-implementation], [enable an alternate array implementation that optimizes speed at the cost of space]), opt_alt_array_impl=$enableval)
AC_ARG_ENABLE(arith-for-command, AS_HELP_STRING([--enable-arith-for-command], [enable arithmetic for command]), opt_arith_for_command=$enableval)
AC_ARG_ENABLE(array-variables, AS_HELP_STRING([--enable-array-variables], [include shell array variables]), opt_array_variables=$enableval)
AC_ARG_ENABLE(bang-history, AS_HELP_STRING([--enable-bang-history], [turn on csh-style history substitution]), opt_bang_history=$enableval)
AC_ARG_ENABLE(brace-expansion, AS_HELP_STRING([--enable-brace-expansion], [include brace expansion]), opt_brace_expansion=$enableval)
AC_ARG_ENABLE(casemod-attributes, AS_HELP_STRING([--enable-casemod-attributes], [include case-modifying variable attributes]), opt_casemod_attrs=$enableval)
AC_ARG_ENABLE(casemod-expansions, AS_HELP_STRING([--enable-casemod-expansions], [include case-modifying word expansions]), opt_casemod_expansions=$enableval)
AC_ARG_ENABLE(command-timing, AS_HELP_STRING([--enable-command-timing], [enable the time reserved word and command timing]), opt_command_timing=$enableval)
AC_ARG_ENABLE(cond-command, AS_HELP_STRING([--enable-cond-command], [enable the conditional command]), opt_cond_command=$enableval)
AC_ARG_ENABLE(cond-regexp, AS_HELP_STRING([--enable-cond-regexp], [enable extended regular expression matching in conditional commands]), opt_cond_regexp=$enableval)
AC_ARG_ENABLE(coprocesses, AS_HELP_STRING([--enable-coprocesses], [enable coprocess support and the coproc reserved word]), opt_coproc=$enableval)
AC_ARG_ENABLE(debugger, AS_HELP_STRING([--enable-debugger], [enable support for bash debugger]), opt_debugger=$enableval)
AC_ARG_ENABLE(dev-fd-stat-broken, AS_HELP_STRING([--enable-dev-fd-stat-broken], [enable this option if stat on /dev/fd/N and fstat on file descriptor N don't return the same results]), opt_dev_fd_stat_broken=$enableval)
AC_ARG_ENABLE(direxpand-default, AS_HELP_STRING([--enable-direxpand-default], [enable the direxpand shell option by default]), opt_dircomplete_expand_default=$enableval)
AC_ARG_ENABLE(directory-stack, AS_HELP_STRING([--enable-directory-stack], [enable builtins pushd/popd/dirs]), opt_dirstack=$enableval)
AC_ARG_ENABLE(disabled-builtins, AS_HELP_STRING([--enable-disabled-builtins], [allow disabled builtins to still be invoked]), opt_disabled_builtins=$enableval)
AC_ARG_ENABLE(dparen-arithmetic, AS_HELP_STRING([--enable-dparen-arithmetic], [include ((...)) command]), opt_dparen_arith=$enableval)
AC_ARG_ENABLE(extended-glob, AS_HELP_STRING([--enable-extended-glob], [include ksh-style extended pattern matching]), opt_extended_glob=$enableval)
AC_ARG_ENABLE(extended-glob-default, AS_HELP_STRING([--enable-extended-glob-default], [force extended pattern matching to be enabled by default]), opt_extglob_default=$enableval)
AC_ARG_ENABLE(function-import, AS_HELP_STRING([--enable-function-import], [allow bash to import exported function definitions by default]), opt_function_import=$enableval)
AC_ARG_ENABLE(glob-asciiranges-default, AS_HELP_STRING([--enable-glob-asciiranges-default], [force bracket range expressions in pattern matching to use the C locale by default]), opt_globascii_default=$enableval)
AC_ARG_ENABLE(help-builtin, AS_HELP_STRING([--enable-help-builtin], [include the help builtin]), opt_help=$enableval)
AC_ARG_ENABLE(history, AS_HELP_STRING([--enable-history], [turn on command history]), opt_history=$enableval)
AC_ARG_ENABLE(job-control, AS_HELP_STRING([--enable-job-control], [enable job control features]), opt_job_control=$enableval)
AC_ARG_ENABLE(multibyte, AS_HELP_STRING([--enable-multibyte], [enable multibyte characters if OS supports them]), opt_multibyte=$enableval)
AC_ARG_ENABLE(net-redirections, AS_HELP_STRING([--enable-net-redirections], [enable /dev/tcp/host/port redirection]), opt_net_redirs=$enableval)
AC_ARG_ENABLE(process-substitution, AS_HELP_STRING([--enable-process-substitution], [enable process substitution]), opt_process_subst=$enableval)
AC_ARG_ENABLE(progcomp, AS_HELP_STRING([--enable-progcomp], [enable programmable completion and the complete builtin]), opt_progcomp=$enableval)
AC_ARG_ENABLE(prompt-string-decoding, AS_HELP_STRING([--enable-prompt-string-decoding], [turn on escape character decoding in prompts]), opt_prompt_decoding=$enableval)
AC_ARG_ENABLE(readline, AS_HELP_STRING([--enable-readline], [turn on command line editing]), opt_readline=$enableval)
AC_ARG_ENABLE(restricted, AS_HELP_STRING([--enable-restricted], [enable a restricted shell]), opt_restricted=$enableval)
AC_ARG_ENABLE(select, AS_HELP_STRING([--enable-select], [include select command]), opt_select=$enableval)
AC_ARG_ENABLE(separate-helpfiles, AS_HELP_STRING([--enable-separate-helpfiles], [use external files for help builtin documentation]), opt_separate_help=$enableval)
AC_ARG_ENABLE(single-help-strings, AS_HELP_STRING([--enable-single-help-strings], [store help documentation as a single string to ease translation]), opt_single_longdoc_strings=$enableval)
AC_ARG_ENABLE(strict-posix-default, AS_HELP_STRING([--enable-strict-posix-default], [configure bash to be posix-conformant by default]), opt_strict_posix=$enableval)
AC_ARG_ENABLE(translatable-strings, AS_HELP_STRING([--enable-translatable-strings], [include support for $"..." translatable strings]), opt_translatable_strings=$enableval)
AC_ARG_ENABLE(usg-echo-default, AS_HELP_STRING([--enable-usg-echo-default], [a synonym for --enable-xpg-echo-default]), opt_xpg_echo=$enableval)
AC_ARG_ENABLE(xpg-echo-default, AS_HELP_STRING([--enable-xpg-echo-default], [make the echo builtin expand escape sequences by default]), opt_xpg_echo=$enableval)

dnl options that alter how bash is compiled and linked
AC_ARG_ENABLE(mem-scramble, AS_HELP_STRING([--enable-mem-scramble], [scramble memory on calls to malloc and free]), opt_memscramble=$enableval)
AC_ARG_ENABLE(profiling, AS_HELP_STRING([--enable-profiling], [allow profiling with gprof]), opt_profiling=$enableval)
AC_ARG_ENABLE(static-link, AS_HELP_STRING([--enable-static-link], [link bash statically, for use as a root shell]), opt_static_link=$enableval)

dnl So-called `precious' variables
AC_ARG_VAR([CC_FOR_BUILD], [C compiler used when compiling binaries used only at build time])
AC_ARG_VAR([CFLAGS_FOR_BUILD], [Compilation options (CFLAGS) used when compiling binaries used only at build time])
AC_ARG_VAR([LDFLAGS_FOR_BUILD], [Linker options (LDFLAGS) used when compiling binaries used only at build time])
AC_ARG_VAR([CPPFLAGS_FOR_BUILD], [C preprocessor options (CPPFLAGS) used when compiling binaries used only at build time])

dnl opt_job_control is handled later, after BASH_JOB_CONTROL_MISSING runs

dnl opt_readline and opt_history are handled later, because AC_PROG_CC needs
dnl to be run before we can check the version of an already-installed readline
dnl library

if test $opt_alias = yes; then
AC_DEFINE(ALIAS)
fi
if test $opt_dirstack = yes; then
AC_DEFINE(PUSHD_AND_POPD)
fi
if test $opt_restricted = yes; then
AC_DEFINE(RESTRICTED_SHELL)
fi
if test $opt_process_subst = yes; then
AC_DEFINE(PROCESS_SUBSTITUTION)
fi
if test $opt_prompt_decoding = yes; then
AC_DEFINE(PROMPT_STRING_DECODE)
fi
if test $opt_select = yes; then
AC_DEFINE(SELECT_COMMAND)
fi
if test $opt_help = yes; then
AC_DEFINE(HELP_BUILTIN)
fi
if test $opt_array_variables = yes; then
AC_DEFINE(ARRAY_VARS)
fi
if test $opt_dparen_arith = yes; then
AC_DEFINE(DPAREN_ARITHMETIC)
fi
if test $opt_brace_expansion = yes; then
AC_DEFINE(BRACE_EXPANSION)
fi
if test $opt_disabled_builtins = yes; then
AC_DEFINE(DISABLED_BUILTINS)
fi
if test $opt_command_timing = yes; then
AC_DEFINE(COMMAND_TIMING)
fi
if test $opt_xpg_echo = yes ; then
AC_DEFINE(DEFAULT_ECHO_TO_XPG)
fi
if test $opt_strict_posix = yes; then
AC_DEFINE(STRICT_POSIX)
fi
if test $opt_extended_glob = yes ; then
AC_DEFINE(EXTENDED_GLOB)
fi
if test $opt_extglob_default = yes; then
AC_DEFINE(EXTGLOB_DEFAULT, 1)
else
AC_DEFINE(EXTGLOB_DEFAULT, 0)
fi
if test $opt_cond_command = yes ; then
AC_DEFINE(COND_COMMAND)
fi
if test $opt_cond_regexp = yes ; then
AC_DEFINE(COND_REGEXP)
fi
if test $opt_coproc = yes; then
AC_DEFINE(COPROCESS_SUPPORT)
fi
if test $opt_arith_for_command = yes; then
AC_DEFINE(ARITH_FOR_COMMAND)
fi
if test $opt_net_redirs = yes; then
AC_DEFINE(NETWORK_REDIRECTIONS)
fi
if test $opt_progcomp = yes; then
AC_DEFINE(PROGRAMMABLE_COMPLETION)
fi
if test $opt_multibyte = no; then
AC_DEFINE(NO_MULTIBYTE_SUPPORT)
fi
if test $opt_debugger = yes; then
AC_DEFINE(DEBUGGER)
fi
if test $opt_casemod_attrs = yes; then
AC_DEFINE(CASEMOD_ATTRS)
fi
if test $opt_casemod_expansions = yes; then
AC_DEFINE(CASEMOD_EXPANSIONS)
fi
if test $opt_dircomplete_expand_default = yes; then
AC_DEFINE(DIRCOMPLETE_EXPAND_DEFAULT)
fi
if test $opt_globascii_default = yes; then
AC_DEFINE(GLOBASCII_DEFAULT, 1)
else
AC_DEFINE(GLOBASCII_DEFAULT, 0)
fi
if test $opt_function_import = yes; then
AC_DEFINE(FUNCTION_IMPORT)
fi
if test $opt_dev_fd_stat_broken = yes; then
AC_DEFINE(DEV_FD_STAT_BROKEN)
fi
if test $opt_alt_array_impl = yes; then
AC_DEFINE(ALT_ARRAY_IMPLEMENTATION)
ARRAY_O=array2.o
fi
if test $opt_translatable_strings = yes; then
AC_DEFINE(TRANSLATABLE_STRINGS)
fi

if test $opt_memscramble = yes; then
AC_DEFINE(MEMSCRAMBLE)
fi

if test "$opt_minimal_config" = yes; then
	TESTSCRIPT=run-minimal
else
	TESTSCRIPT=run-all
fi

HELPDIR= HELPDIRDEFINE= HELPINSTALL= HELPFILES_TARGET=
if test "$opt_separate_help" != no; then
	if test "$opt_separate_help" = "yes" ; then
		HELPDIR='${datadir}/bash/helpfiles'
	else
		HELPDIR=$opt_separate_help
	fi
	HELPDIRDEFINE='-H ${HELPDIR}'
	HELPINSTALL='install-help'
	HELPFILES_TARGET='helpdoc'
fi
HELPSTRINGS=
if test "$opt_single_longdoc_strings" != "yes"; then
	HELPSTRINGS='-S'
fi

dnl now substitute in the values generated by arguments
AC_SUBST(TESTSCRIPT)
AC_SUBST(MALLOC_TARGET)
AC_SUBST(MALLOC_SRC)

AC_SUBST(MALLOC_LIB)
AC_SUBST(MALLOC_LIBRARY)
AC_SUBST(MALLOC_LDFLAGS)
AC_SUBST(MALLOC_DEP)

AC_SUBST(ARRAY_O)

AC_SUBST(htmldir)

AC_SUBST(HELPDIR)
AC_SUBST(HELPDIRDEFINE)
AC_SUBST(HELPINSTALL)
AC_SUBST(HELPFILES_TARGET)
AC_SUBST(HELPSTRINGS)

dnl We want these before the checks, so the checks can modify their values.
if test -z "$CFLAGS"; then
	want_auto_cflags=1
fi

echo ""
echo "Beginning configuration for bash-$BASHVERS-$RELSTATUS for ${host_cpu}-${host_vendor}-${host_os}"
echo ""

dnl compilation checks
dnl AC_PROG_CC sets $cross_compiling to `yes' if cross-compiling for a
dnl different environment
AC_PROG_CC

if test -n "$want_auto_cflags"; then
	AUTO_CFLAGS="-g ${GCC:+-O2}"
	AUTO_LDFLAGS="-g ${GCC:+-O2}"
#	STYLE_CFLAGS="${GCC:+-Wno-parentheses} ${GCC:+-Wno-format-security} ${GCC:+-Wno-tautological-constant-out-of-range-compare}"
	STYLE_CFLAGS="${GCC:+-Wno-parentheses} ${GCC:+-Wno-format-security}"
else
	AUTO_CFLAGS= AUTO_LDFLAGS= STYLE_CFLAGS=
fi

dnl test for Unix variants
AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE

dnl BEGIN changes for cross-building (currently cygwin, minGW, and
dnl (obsolete) BeOS)

SIGNAMES_O=
SIGNAMES_H=lsignames.h

dnl load up the cross-building cache file -- add more cases and cache
dnl files as necessary

dnl Note that host and target machine are the same, and different than the
dnl build machine.
dnl Set SIGNAMES_H based on whether or not we're cross-compiling.

CROSS_COMPILE=
if test "x$cross_compiling" = "xyes"; then
    case "${host}" in
    *-cygwin*)
	cross_cache=${srcdir}/cross-build/cygwin32.cache
	;;
    *-mingw*)
	cross_cache=${srcdir}/cross-build/cygwin32.cache
	;;
    i[[3456]]86-*-beos*)
	cross_cache=${srcdir}/cross-build/x86-beos.cache
	;;
    *-qnx*)
	cross_cache=${srcdir}/cross-build/qnx.cache
	;;
    *)	echo "configure: cross-compiling for $host is not supported" >&2
	;;
    esac
    if test -n "${cross_cache}" && test -r "${cross_cache}"; then
	echo "loading cross-build cache file ${cross_cache}"
	. ${cross_cache}
    fi
    unset cross_cache
    SIGNAMES_O='signames.o'
    CROSS_COMPILE='-DCROSS_COMPILING'
    AC_SUBST(CROSS_COMPILE)
fi
AC_SUBST(SIGNAMES_H)
AC_SUBST(SIGNAMES_O)

dnl END changes for cross-building

dnl default values
CFLAGS=${CFLAGS-"$AUTO_CFLAGS"}
# LDFLAGS=${LDFLAGS="$AUTO_LDFLAGS"}	# XXX

dnl handle options that alter how bash is compiled and linked
dnl these must come after the test for cc/gcc
if test "$opt_profiling" = "yes"; then
	PROFILE_FLAGS=-pg
	case "$host_os" in
	solaris2*|linux*|darwin*)	;;
	*)		opt_static_link=yes ;;
	esac
	DEBUG= MALLOC_DEBUG=
fi

prefer_shared=yes
prefer_static=no

if test "$opt_static_link" = yes; then
	prefer_static=yes
	prefer_shared=no
	# if we're using gcc, add `-static' to LDFLAGS, except on Solaris >= 2
	if test "$GCC" = "yes"; then
		STATIC_LD="-static" 
		case "$host_os" in
		solaris2*|linux*)	;;
		*)		LDFLAGS="$LDFLAGS -static" ;;	# XXX experimental
		esac
	fi
fi

# set the appropriate make variables for building the "build tools"
# modify defaults based on whether or not we are cross compiling, since the
# options for the target host may not be appropriate for the build host
if test "X$cross_compiling" = "Xno"; then
	CC_FOR_BUILD=${CC_FOR_BUILD-'$(CC)'}
	CPPFLAGS_FOR_BUILD=${CPPFLAGS_FOR_BUILD-"$CPPFLAGS"}	# XXX - should it be '$(CPPFLAGS)'
	if test X"$opt_profiling" = Xyes; then
		LDFLAGS_FOR_BUILD=${LDFLAGS_FOR_BUILD-'$(BASE_LDFLAGS)'}
	else
		LDFLAGS_FOR_BUILD=${LDFLAGS_FOR_BUILD-'$(LDFLAGS)'}
	fi
	# CFLAGS set above to default value if not passed in environment
	if test -n "$want_auto_cflags" ; then
		CFLAGS_FOR_BUILD="${CFLAGS}"
		BASE_CFLAGS_FOR_BUILD="${CFLAGS}"
	else
		# passed in environment
		CFLAGS_FOR_BUILD=${CFLAGS-'$(CFLAGS)'}
		BASE_CFLAGS_FOR_BUILD=${CFLAGS-'$(CFLAGS)'}
	fi
	LIBS_FOR_BUILD=${LIBS_FOR_BUILD-'$(LIBS)'}
else
	CC_FOR_BUILD=${CC_FOR_BUILD-"gcc"}
	CPPFLAGS_FOR_BUILD=${CPPFLAGS_FOR_BUILD-""}
	LDFLAGS_FOR_BUILD=${LDFLAGS_FOR_BUILD-""}
	CFLAGS_FOR_BUILD=${CFLAGS_FOR_BUILD="-g"}
	BASE_CFLAGS_FOR_BUILD=${BASE_CFLAGS_FOR_BUILD="-g"}
	LIBS_FOR_BUILD=${LIBS_FOR_BUILD-""}
fi

AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(STATIC_LD)

AC_SUBST(CC_FOR_BUILD)
AC_SUBST(CFLAGS_FOR_BUILD)
AC_SUBST(BASE_CFLAGS_FOR_BUILD)
AC_SUBST(STYLE_CFLAGS)
AC_SUBST(CPPFLAGS_FOR_BUILD)
AC_SUBST(LDFLAGS_FOR_BUILD)
AC_SUBST(LIBS_FOR_BUILD)

AC_PROG_GCC_TRADITIONAL

dnl BEGIN READLINE and HISTORY LIBRARY SECTION
dnl prepare to allow bash to be linked against an already-installed readline

dnl first test that the readline version is new enough to link bash against
if test "$opt_readline" = yes && test "$opt_with_installed_readline" != "no"
then
	# If the user specified --with-installed-readline=PREFIX and PREFIX
	# is not `yes', set ac_cv_rl_prefix to PREFIX
	test $opt_with_installed_readline != "yes" && ac_cv_rl_prefix=$opt_with_installed_readline

	RL_LIB_READLINE_VERSION

	case "$ac_cv_rl_version" in
	8*|9*)	;;
	*)	opt_with_installed_readline=no 
		AC_MSG_WARN([installed readline library is too old to be linked with bash])
		AC_MSG_WARN([using private bash version])
		;;
	esac
fi

TILDE_LIB=-ltilde
if test $opt_readline = yes; then
	AC_DEFINE(READLINE)
	if test "$opt_with_installed_readline" != "no" ; then
		case "$opt_with_installed_readline" in
		yes)	RL_INCLUDE= ;;
		*)	case "$RL_INCLUDEDIR" in
			/usr/include)	;;
			*)		RL_INCLUDE='-I${RL_INCLUDEDIR}' ;;
			esac
			;;
		esac
		READLINE_DEP=
		READLINE_LIB=-lreadline
		# section for OS versions that don't allow unresolved symbols
		# to be compiled into dynamic libraries.
		case "$host_os" in
		cygwin*)	TILDE_LIB= ;;
		esac
	else
		RL_LIBDIR='$(dot)/$(LIBSUBDIR)/readline'
		READLINE_DEP='$(READLINE_LIBRARY)'
		# section for OS versions that ship an older/broken version of
		# readline as a standard dynamic library and don't allow a
		# static version specified as -llibname to override the
		# dynamic version
		case "${host_os}" in
		darwin[[89]]*|darwin10*) READLINE_LIB='${READLINE_LIBRARY}' ;;
		*)		READLINE_LIB=-lreadline ;;
		esac
	fi
else
	RL_LIBDIR='$(dot)/$(LIBSUBDIR)/readline'
	READLINE_LIB= READLINE_DEP=
fi
if test $opt_history = yes || test $opt_bang_history = yes; then
	if test $opt_history = yes; then
		AC_DEFINE(HISTORY)
	fi
	if test $opt_bang_history = yes; then
		AC_DEFINE(BANG_HISTORY)
	fi
 	if test "$opt_with_installed_readline" != "no"; then
		HIST_LIBDIR=$RL_LIBDIR
		HISTORY_DEP=
		HISTORY_LIB=-lhistory
		case "$opt_with_installed_readline" in
		yes)	RL_INCLUDE= ;;
		*)	case "$RL_INCLUDEDIR" in
			/usr/include)	;;
			*)		RL_INCLUDE='-I${RL_INCLUDEDIR}' ;;
			esac
			;;
		esac
	else
		HIST_LIBDIR='$(dot)/$(LIBSUBDIR)/readline'
		HISTORY_DEP='$(HISTORY_LIBRARY)'
		# section for OS versions that ship an older version of
		# readline as a standard dynamic library and don't allow a
		# static version specified as -llibname to override the
		# dynamic version
		case "${host_os}" in
		darwin[[89]]*|darwin10*) HISTORY_LIB='${HISTORY_LIBRARY}' ;;
		*)		HISTORY_LIB=-lhistory ;;
		esac
	fi
else
	HIST_LIBDIR='$(dot)/$(LIBSUBDIR)/readline'
	HISTORY_LIB= HISTORY_DEP=
fi
AC_SUBST(READLINE_LIB)
AC_SUBST(READLINE_DEP)
AC_SUBST(RL_LIBDIR)
AC_SUBST(RL_INCLUDEDIR)
AC_SUBST(RL_INCLUDE)
AC_SUBST(HISTORY_LIB)
AC_SUBST(HISTORY_DEP)
AC_SUBST(HIST_LIBDIR)
AC_SUBST(TILDE_LIB)

dnl END READLINE and HISTORY LIBRARY SECTION

dnl programs needed by the build and install process
AC_PROG_INSTALL
AC_CHECK_TOOL(AR, ar)
dnl Set default for ARFLAGS, since autoconf does not have a macro for it.
dnl This allows people to set it when running configure or make
test -n "$ARFLAGS" || ARFLAGS="cr"
AC_PROG_RANLIB
AC_PROG_YACC
AC_PROG_MAKE_SET

case "$ac_cv_prog_YACC" in
*bison*)	;;
*)	AC_MSG_WARN([bison not available; needed to process parse.y]) ;;
esac

case "$host_os" in
opennt*|interix*)	MAKE_SHELL="$INTERIX_ROOT/bin/sh" ;;
*)			MAKE_SHELL=/bin/sh ;;
esac
AC_SUBST(MAKE_SHELL)

dnl this is similar to the expanded AC_PROG_RANLIB
if test x$SIZE = x; then
	if test x$ac_tool_prefix = x; then
		SIZE=size
	else
		SIZE=${ac_tool_prefix}size
		save_IFS=$IFS ; IFS=:
		size_found=0
		for dir in $PATH; do
			if test -x $dir/$SIZE ; then
				size_found=1
				break
			fi
		done
		if test $size_found -eq 0; then
			SIZE=:
		fi
		IFS=$save_IFS
	fi
fi
AC_SUBST(SIZE)

m4_include([m4/stat-time.m4])
m4_include([m4/timespec.m4])

m4_include([m4/strtoimax.m4])

dnl include files for gettext

m4_include([m4/codeset.m4])
m4_include([m4/extern-inline.m4])
m4_include([m4/fcntl-o.m4])
m4_include([m4/gettext.m4])
m4_include([m4/glibc2.m4])
m4_include([m4/glibc21.m4])
m4_include([m4/host-cpu-c-abi.m4])
m4_include([m4/iconv.m4])
m4_include([m4/intdiv0.m4])
m4_include([m4/intl.m4])
m4_include([m4/intlmacosx.m4])
m4_include([m4/intl-thread-locale.m4])
m4_include([m4/intmax.m4])
m4_include([m4/inttypes-pri.m4])
m4_include([m4/inttypes.m4])
m4_include([m4/inttypes_h.m4])
m4_include([m4/lcmessage.m4])
m4_include([m4/lib-ld.m4])
m4_include([m4/lib-link.m4])
m4_include([m4/lib-prefix.m4])
m4_include([m4/lock.m4])
m4_include([m4/nls.m4])
m4_include([m4/po.m4])
m4_include([m4/printf-posix.m4])
m4_include([m4/progtest.m4])
m4_include([m4/pthread_rwlock_rdlock.m4])
m4_include([m4/size_max.m4])
m4_include([m4/stdint_h.m4])
m4_include([m4/threadlib.m4])
m4_include([m4/uintmax_t.m4])
m4_include([m4/ulonglong.m4])
m4_include([m4/visibility.m4])
m4_include([m4/wchar_t.m4])
m4_include([m4/wint_t.m4])
m4_include([m4/xsize.m4])

dnl C compiler characteristics
AC_C_CONST
AC_C_INLINE
AC_C_BIGENDIAN
AC_C_STRINGIZE
AC_TYPE_LONG_DOUBLE
AC_C_PROTOTYPES
AC_C_CHAR_UNSIGNED
AC_C_VOLATILE
AC_C_RESTRICT

dnl initialize GNU gettext
BASH_GNU_GETTEXT([no-libtool], [need-ngettext], [lib/intl])

dnl header files
AC_HEADER_DIRENT
AC_HEADER_MAJOR

BASH_HEADER_INTTYPES

AC_CHECK_HEADERS(unistd.h stdlib.h stdarg.h varargs.h limits.h string.h \
		 memory.h locale.h termcap.h termio.h termios.h dlfcn.h \
		 stdbool.h stddef.h stdint.h netdb.h pwd.h grp.h strings.h \
		 regex.h syslog.h ulimit.h)
AC_CHECK_HEADERS(sys/pte.h sys/stream.h sys/select.h sys/file.h sys/ioctl.h \
		 sys/mman.h sys/param.h sys/random.h sys/socket.h sys/stat.h \
		 sys/time.h sys/times.h sys/types.h sys/wait.h)
AC_CHECK_HEADERS(netinet/in.h arpa/inet.h)

dnl sys/ptem.h requires definitions from sys/stream.h on systems where it
dnl exists
AC_CHECK_HEADER(sys/ptem.h, , ,[[
#if HAVE_SYS_STREAM_H
#  include <sys/stream.h>
#endif
]])

dnl SunOS 4 needs to include <sys/time.h> before <sys/resource.h> to compile
dnl autoconf complains about presence but inability to compile
AC_CHECK_HEADER(sys/resource.h, AC_DEFINE(HAVE_SYS_RESOURCE_H), [], [[
#if HAVE_SYS_TIME_H
#  include <sys/time.h>
#endif
]])

dnl special checks for libc functions
AC_FUNC_ALLOCA
AC_FUNC_CHOWN
AC_FUNC_GETPGRP
AC_FUNC_VPRINTF
AC_FUNC_STRCOLL

dnl if we're not using the bash malloc but require the C alloca, set things
dnl up to build a libmalloc.a containing only alloca.o

if test "$ac_cv_func_alloca_works" = "no" && test "$opt_bash_malloc" = "no"; then
	MALLOC_TARGET=alloca
	MALLOC_SRC=alloca.c

	MALLOC_LIB='-lmalloc'
	MALLOC_LIBRARY='$(ALLOC_LIBDIR)/libmalloc.a'
	MALLOC_LDFLAGS='-L$(ALLOC_LIBDIR)'
	MALLOC_DEP='$(MALLOC_LIBRARY)'
fi

dnl if vprintf is not in libc, see if it's defined in stdio.h
if test "$ac_cv_func_vprintf" = no; then
    AC_MSG_CHECKING(for declaration of vprintf in stdio.h)
    AC_EGREP_HEADER([[int[ 	]*vprintf[^a-zA-Z0-9]]],stdio.h,ac_cv_func_vprintf=yes)
    AC_MSG_RESULT($ac_cv_func_vprintf)
    if test $ac_cv_func_vprintf = yes; then
	AC_DEFINE(HAVE_VPRINTF)
    fi
fi

if test "$ac_cv_func_vprintf" = no && test "$ac_cv_func__doprnt" = "yes"; then
  AC_LIBOBJ(vprint)
fi

dnl checks for certain version-specific system calls and libc functions
AC_CHECK_FUNC(__setostype, AC_DEFINE(HAVE_SETOSTYPE))
AC_CHECK_FUNC(wait3, AC_DEFINE(HAVE_WAIT3))

dnl checks for missing libc functions
AC_CHECK_FUNC(mkfifo, AC_DEFINE(HAVE_MKFIFO), AC_DEFINE(MKFIFO_MISSING))

dnl checks for system calls
AC_CHECK_FUNCS(dup2 eaccess fcntl getdtablesize getentropy getgroups \
		gethostname getpagesize getpeername getrandom getrlimit \
		getrusage gettimeofday kill killpg lstat pselect readlink \
		select setdtablesize setitimer tcgetpgrp uname ulimit waitpid)
AC_REPLACE_FUNCS(rename)

dnl checks for c library functions
AC_CHECK_FUNCS(bcopy bzero confstr faccessat fnmatch \
		getaddrinfo gethostbyname getservbyname getservent inet_aton \
		imaxdiv memmove pathconf putenv raise random regcomp regexec \
		setenv setlinebuf setlocale setvbuf siginterrupt strchr \
		sysconf syslog tcgetattr times ttyname tzset unsetenv)

AC_CHECK_FUNCS(vasprintf asprintf)
AC_CHECK_FUNCS(isascii isblank isgraph isprint isspace isxdigit)
AC_CHECK_FUNCS(getpwent getpwnam getpwuid)
AC_CHECK_FUNCS(mkstemp mkdtemp)
AC_CHECK_FUNCS(arc4random)

AC_REPLACE_FUNCS(getcwd memset)
AC_REPLACE_FUNCS(strcasecmp strcasestr strerror strftime strnlen strpbrk strstr)
AC_REPLACE_FUNCS(strtod strtol strtoul strtoll strtoull strtoumax)
AC_REPLACE_FUNCS(dprintf)
AC_REPLACE_FUNCS(strchrnul)
AC_REPLACE_FUNCS(strdup)

AC_CHECK_HEADERS(libaudit.h)
AC_CHECK_DECLS([AUDIT_USER_TTY],,, [[#include <linux/audit.h>]])

AC_CHECK_DECLS([confstr])
AC_CHECK_DECLS([printf])
AC_CHECK_DECLS([sbrk])
AC_CHECK_DECLS([setregid])
AC_CHECK_DECLS([strcpy])
AC_CHECK_DECLS([strsignal])

AC_CHECK_FUNCS(setresuid setresgid)

dnl Extra test to detect the horribly broken HP/UX 11.00 strtold(3)
AC_CHECK_DECLS([strtold], [
    AC_MSG_CHECKING([for broken strtold])
    AC_CACHE_VAL(bash_cv_strtold_broken,
	[AC_COMPILE_IFELSE(
		[AC_LANG_PROGRAM(
		[[#include <stdlib.h>]],
		[[long double r; char *foo, bar; r = strtold(foo, &bar);]]
	)],
	[bash_cv_strtold_broken=no],[bash_cv_strtold_broken=yes])
        ]
    )
    AC_MSG_RESULT($bash_cv_strtold_broken)
    if test "$bash_cv_strtold_broken" = "yes" ; then
	AC_DEFINE(STRTOLD_BROKEN)
    fi
])

AC_CHECK_DECLS(strtol)
AC_CHECK_DECLS(strtoll)
AC_CHECK_DECLS(strtoul)
AC_CHECK_DECLS(strtoull)
AC_CHECK_DECLS(strtoumax)

AC_FUNC_MKTIME

dnl
dnl Checks for lib/intl and related code (uses some of the output from
dnl BASH_GNU_GETTEXT)
dnl

AC_CHECK_HEADERS([argz.h errno.h fcntl.h malloc.h stdio_ext.h])

dnl AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_CHECK_FUNCS([__argz_count __argz_next __argz_stringify dcgettext mempcpy \
		munmap mremap stpcpy strcspn])

INTL_DEP= INTL_INC= LIBINTL_H=
if test "x$USE_INCLUDED_LIBINTL" = "xyes"; then
	INTL_DEP='${INTL_LIBDIR}/libintl.a'
	INTL_INC='-I${INTL_LIBSRC} -I${INTL_BUILDDIR}'
	LIBINTL_H='${INTL_BUILDDIR}/libintl.h'
fi
AC_SUBST(INTL_DEP)
AC_SUBST(INTL_INC)
AC_SUBST(LIBINTL_H)

dnl
dnl End of checks needed by files in lib/intl
dnl

BASH_CHECK_MULTIBYTE

dnl checks for the dynamic loading library functions in libc and libdl
if test "$opt_static_link" != yes; then
AC_CHECK_LIB(dl, dlopen)
AC_CHECK_FUNCS(dlopen dlclose dlsym)
fi

dnl network functions -- check for inet_aton again
if test "$ac_cv_func_inet_aton" != 'yes'; then
BASH_FUNC_INET_ATON
fi

dnl libraries
dnl this is reportedly no longer necessary for irix[56].?
case "$host_os" in
irix4*)	AC_CHECK_LIB(sun, getpwent) ;;
esac

dnl check for getpeername in the socket library only if it's not in libc
if test "$ac_cv_func_getpeername" = no; then
	BASH_CHECK_LIB_SOCKET
fi
dnl check for gethostbyname in socket libraries if it's not in libc
if test "$ac_cv_func_gethostbyname" = no; then
	BASH_FUNC_GETHOSTBYNAME
fi

dnl system types
AC_TYPE_GETGROUPS
AC_TYPE_OFF_T
AC_TYPE_MODE_T
AC_TYPE_UID_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINTPTR_T

AC_CHECK_TYPE(ssize_t, int)
AC_CHECK_TYPE(time_t, long)

AC_TYPE_LONG_LONG_INT
AC_TYPE_UNSIGNED_LONG_LONG_INT

BASH_TYPE_SIG_ATOMIC_T

AC_CHECK_SIZEOF(char, 1)
AC_CHECK_SIZEOF(short, 2)
AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)
AC_CHECK_SIZEOF(char *, 4)
AC_CHECK_SIZEOF(size_t, 4)
AC_CHECK_SIZEOF(double, 8)
AC_CHECK_SIZEOF([long long], 8)

AC_CHECK_TYPE(u_int, [unsigned int])
AC_CHECK_TYPE(u_long, [unsigned long])

BASH_TYPE_BITS16_T
BASH_TYPE_U_BITS16_T
BASH_TYPE_BITS32_T
BASH_TYPE_U_BITS32_T
BASH_TYPE_BITS64_T

BASH_TYPE_PTRDIFF_T

dnl structures
AC_HEADER_STAT

dnl system services
AC_SYS_INTERPRETER
if test $ac_cv_sys_interpreter = yes; then
AC_DEFINE(HAVE_HASH_BANG_EXEC)
fi

dnl Miscellaneous Bash tests
if test "$ac_cv_func_lstat" = "no"; then
BASH_FUNC_LSTAT
fi

dnl behavior of system calls and library functions
BASH_FUNC_DUP2_CLOEXEC_CHECK
BASH_SYS_PGRP_SYNC
BASH_SYS_SIGNAL_VINTAGE

dnl checking for the presence of certain library symbols
BASH_SYS_ERRLIST
BASH_SYS_SIGLIST
BASH_UNDER_SYS_SIGLIST

dnl various system types
BASH_CHECK_TYPE(clock_t, [#include <sys/times.h>], long)
BASH_CHECK_TYPE(sigset_t, [#include <signal.h>], int)
if test "$ac_cv_header_sys_socket_h" = "yes"; then
BASH_CHECK_TYPE(socklen_t, [#include <sys/socket.h>], [unsigned int], HAVE_SOCKLEN_T)
fi

AC_CHECK_TYPE(quad_t, AC_DEFINE([HAVE_QUAD_T], [1]), [], )
BASH_TYPE_RLIMIT

AC_TYPE_INTMAX_T
AC_TYPE_UINTMAX_T

AC_CHECK_SIZEOF(intmax_t, 8)

dnl presence and contents of structures used by system calls
BASH_STRUCT_TERMIOS_LDISC
BASH_STRUCT_TERMIO_LDISC
BASH_STRUCT_DIRENT_D_INO
BASH_STRUCT_DIRENT_D_FILENO
BASH_STRUCT_DIRENT_D_NAMLEN
BASH_STRUCT_WINSIZE
BASH_STRUCT_TIMEVAL
AC_CHECK_MEMBERS([struct stat.st_blocks])
AC_STRUCT_TM
AC_STRUCT_TIMEZONE
BASH_STRUCT_TIMEZONE

BASH_STRUCT_WEXITSTATUS_OFFSET

BASH_CHECK_TYPE_STRUCT_TIMESPEC
BASH_STAT_TIME

dnl checks for system calls
BASH_FUNC_SBRK

dnl presence and behavior of C library functions
BASH_FUNC_STRSIGNAL
BASH_FUNC_OPENDIR_CHECK
BASH_FUNC_ULIMIT_MAXFDS
BASH_FUNC_FPURGE
BASH_FUNC_GETENV
if test "$ac_cv_func_getcwd" = "yes"; then
BASH_FUNC_GETCWD
fi
BASH_FUNC_POSIX_SETJMP
BASH_FUNC_STRCOLL
BASH_FUNC_SNPRINTF
BASH_FUNC_VSNPRINTF

BASH_FUNC_STRTOIMAX

dnl If putenv or unsetenv is not present, set the right define so the
dnl prototype and declaration in lib/sh/getenv.c will be standard-conformant

if test "$ac_cv_func_putenv" = "yes"; then
BASH_FUNC_STD_PUTENV
else
AC_DEFINE(HAVE_STD_PUTENV)
fi
if test "$ac_cv_func_unsetenv" = "yes"; then
BASH_FUNC_STD_UNSETENV
else
AC_DEFINE(HAVE_STD_UNSETENV)
fi

BASH_FUNC_PRINTF_A_FORMAT

BASH_FUNC_FNMATCH_EQUIV_FALLBACK

dnl presence and behavior of OS functions
BASH_SYS_REINSTALL_SIGHANDLERS
BASH_SYS_JOB_CONTROL_MISSING
BASH_SYS_NAMED_PIPES

dnl presence of certain CPP defines
AC_HEADER_TIOCGWINSZ
BASH_HAVE_TIOCSTAT
BASH_HAVE_FIONREAD

BASH_CHECK_WCONTINUED

dnl miscellaneous
BASH_CHECK_SPEED_T
BASH_CHECK_GETPW_FUNCS
BASH_CHECK_RTSIGS
BASH_CHECK_SYS_SIGLIST

dnl special checks
case "$host_os" in
hpux*)	BASH_CHECK_KERNEL_RLIMIT ;;
esac

if test "$opt_readline" = yes; then
dnl yuck
case "$host_os" in
aix*)	prefer_curses=yes ;;
esac
BASH_CHECK_LIB_TERMCAP
fi
AC_SUBST(TERMCAP_LIB)
AC_SUBST(TERMCAP_DEP)

BASH_CHECK_DEV_FD
BASH_CHECK_DEV_STDIN
BASH_SYS_DEFAULT_MAIL_DIR

if test "$bash_cv_job_control_missing" = missing; then
	opt_job_control=no
fi

if test "$opt_job_control" = yes; then
AC_DEFINE(JOB_CONTROL)
JOBS_O=jobs.o
else
JOBS_O=nojobs.o
fi

AC_SUBST(JOBS_O)

dnl Defines that we want to propagate to the Makefiles in subdirectories,
dnl like glob and readline

LOCAL_DEFS=-DSHELL

dnl use this section to possibly define more cpp variables, specify local
dnl libraries, and specify any additional local cc or ld flags
dnl
dnl most of these are really old; this should really go away someday

case "${host_os}" in
sysv4.2*)	AC_DEFINE(SVR4_2)
		AC_DEFINE(SVR4) ;;
sysv4*)		AC_DEFINE(SVR4) ;;
sysv5*)		AC_DEFINE(SVR5) ;;
hpux9*)		LOCAL_CFLAGS="-DHPUX9 -DHPUX -DTGETENT_BROKEN -DTGETFLAG_BROKEN" ;;
hpux*)		LOCAL_CFLAGS="-DHPUX -DTGETENT_BROKEN -DTGETFLAG_BROKEN" ;;
dgux*)		LOCAL_CFLAGS=-D_DGUX_SOURCE; LOCAL_LIBS=-ldgc ;;
isc*)		LOCAL_CFLAGS=-Disc386 ;;
rhapsody*)	LOCAL_CFLAGS=-DRHAPSODY ;;
darwin*)	LOCAL_CFLAGS=-DMACOSX ;;
sco3.2v5*)	LOCAL_CFLAGS="-b elf -DWAITPID_BROKEN -DPATH_MAX=1024" ;;
sco3.2v4*)	LOCAL_CFLAGS="-DMUST_UNBLOCK_CHLD -DPATH_MAX=1024" ;;
sco3.2*)	LOCAL_CFLAGS=-DMUST_UNBLOCK_CHLD ;;
sunos4*)	LOCAL_CFLAGS=-DSunOS4 ;;
solaris2.5*)	LOCAL_CFLAGS="-DSunOS5 -DSOLARIS" ;;
solaris2.8*)	LOCAL_CFLAGS=-DSOLARIS  ;;
solaris2.9*)	LOCAL_CFLAGS=-DSOLARIS  ;;
solaris2.10*)	LOCAL_CFLAGS=-DSOLARIS  ;;
solaris2*)	LOCAL_CFLAGS=-DSOLARIS ;;
lynxos*)	LOCAL_CFLAGS=-DRECYCLES_PIDS ;;
linux*)		LOCAL_LDFLAGS=-rdynamic		 # allow dynamic loading
		case "`uname -r`" in
		1.*|2.[[0123]]*)	: ;;
		*)	AC_DEFINE(PGRP_PIPE) ;;
		esac ;;
netbsd*|openbsd*)	LOCAL_CFLAGS="-DDEV_FD_STAT_BROKEN" ;;
freebsd*|midnightbsd*)	LOCAL_CFLAGS='-DHEREDOC_PIPESIZE=4096' ;;
*qnx[[67]]*)	LOCAL_LIBS="-lncurses" ;;
*qnx*)		LOCAL_CFLAGS="-Dqnx -F -3s" LOCAL_LDFLAGS="-3s" LOCAL_LIBS="-lunix -lncurses" ;;
powerux*)	LOCAL_LIBS="-lgen" ;;
cygwin*)	LOCAL_CFLAGS=-DRECYCLES_PIDS ;;
opennt*|interix*) LOCAL_CFLAGS="-DNO_MAIN_ENV_ARG -DBROKEN_DIRENT_D_INO -D_POSIX_SOURCE -D_ALL_SOURCE -DRECYCLES_PIDS" ;;
*openstep*)	LOCAL_CFLAGS="-D__APPLE_CC__" ;;
esac

dnl Stanza for OS/compiler pair-specific flags
case "${host_os}-${CC}" in
aix4.2*-*gcc*)	LOCAL_LDFLAGS="-Xlinker -bexpall -Xlinker -brtl" ;;
aix4.2*)	LOCAL_LDFLAGS="-bexpall -brtl" ;;
bsdi4*-*gcc*)	LOCAL_LDFLAGS="-rdynamic" ;;	# allow dynamic loading, like Linux
bsdi5*-*gcc*)	LOCAL_LDFLAGS="-rdynamic" ;;	# allow dynamic loading, like Linux
hpux11*-gcc*)	LOCAL_LDFLAGS="-Wl,-E" ;;	# allow dynamic loading
esac

dnl FreeBSD-3.x can have either a.out or ELF
case "${host_os}" in
freebsd[[3-9]]*)
		if test -x /usr/bin/objformat && test "`/usr/bin/objformat`" = "elf" ; then
			LOCAL_LDFLAGS=-rdynamic # allow dynamic loading
		fi ;;
freebsdelf*)	LOCAL_LDFLAGS=-rdynamic ;;	# allow dynamic loading
dragonfly*)	LOCAL_LDFLAGS=-rdynamic ;;	# allow dynamic loading
midnightbsd*)	LOCAL_LDFLAGS=-rdynamic ;;	# allow dynamic loading
esac

case "$host_cpu" in
*cray*)		LOCAL_CFLAGS="-DCRAY" ;; # shell var so config.h can use it
esac

case "$host_cpu-$host_os" in
ibmrt-*bsd4*)	LOCAL_CFLAGS="-ma -U__STDC__" ;;
esac

case "$host_cpu-$host_vendor-$host_os" in
m88k-motorola-sysv3)	LOCAL_CFLAGS=-DWAITPID_BROKEN ;;
mips-pyramid-sysv4)	LOCAL_CFLAGS=-Xa ;;
esac

# turn off paren warnings in gcc
#CFLAGS="$CFLAGS ${STYLE_CFLAGS}"

#
# Shared object configuration section.  These values are generated by
# ${srcdir}/support/shobj-conf
#
if test "$ac_cv_func_dlopen" = "yes" && test -f ${srcdir}/support/shobj-conf
then
	AC_MSG_CHECKING(shared object configuration for loadable builtins)
	eval `${CONFIG_SHELL-/bin/sh} ${srcdir}/support/shobj-conf -C "${CC}" -c "${host_cpu}" -o "${host_os}" -v "${host_vendor}"`
	AC_SUBST(SHOBJ_CC)
	AC_SUBST(SHOBJ_CFLAGS)
	AC_SUBST(SHOBJ_LD)
	AC_SUBST(SHOBJ_LDFLAGS)
	AC_SUBST(SHOBJ_XLDFLAGS)
	AC_SUBST(SHOBJ_LIBS)
	AC_SUBST(SHOBJ_STATUS)
	AC_MSG_RESULT($SHOBJ_STATUS)
else
	SHOBJ_STATUS=unsupported
	AC_SUBST(SHOBJ_STATUS)
fi

# try to create a directory tree if the source is elsewhere
# this should be packaged into a script accessible via ${srcdir}/support
case "$srcdir" in
.)	;;
*)	for d in doc tests support lib examples; do	# dirs
		test -d $d || mkdir $d
	done
	for ld in readline glob tilde malloc sh termcap; do	# libdirs
		test -d lib/$ld || mkdir lib/$ld
	done
	test -d examples/loadables || mkdir examples/loadables	# loadable builtins
	test -d examples/loadables/perl || mkdir examples/loadables/perl
	;;
esac

BUILD_DIR=`pwd`
case "$BUILD_DIR" in
*\ *)	BUILD_DIR=`echo "$BUILD_DIR" | sed 's: :\\\\ :g'` ;;
*)	;;
esac

if test -z "$localedir"; then
	localedir='${datarootdir}/locale'
fi
if test -z "$datarootdir"; then
	datarootdir='${prefix}/share'
fi

AC_SUBST(PROFILE_FLAGS)

AC_SUBST(incdir)
AC_SUBST(BUILD_DIR)

# Some versions of autoconf don't substitute these automatically
AC_SUBST(datarootdir)
AC_SUBST(localedir)

# directory where we install dynamically loadable builtins
if test -z "$loadablesdir"; then
	loadablesdir='${libdir}/bash'
fi
AC_SUBST(loadablesdir)
if test -z "$headersdir"; then
	headersdir='$(includedir)/$(PACKAGE_NAME)'
fi
AC_SUBST(headersdir)

AC_SUBST(YACC)
AC_SUBST(AR)
AC_SUBST(ARFLAGS)

AC_SUBST(BASHVERS)
AC_SUBST(RELSTATUS)
AC_SUBST(DEBUG)
AC_SUBST(MALLOC_DEBUG)

AC_SUBST(host_cpu)
AC_SUBST(host_vendor)
AC_SUBST(host_os)

AC_SUBST(LOCAL_LIBS)
AC_SUBST(LOCAL_CFLAGS)
AC_SUBST(LOCAL_LDFLAGS)
AC_SUBST(LOCAL_DEFS)

#AC_SUBST(ALLOCA_SOURCE)
#AC_SUBST(ALLOCA_OBJECT)

AC_CONFIG_FILES([Makefile builtins/Makefile lib/readline/Makefile \
		 lib/glob/Makefile lib/intl/Makefile \
		 lib/malloc/Makefile lib/sh/Makefile lib/termcap/Makefile \
		 lib/tilde/Makefile doc/Makefile support/Makefile \
		 po/Makefile.in examples/loadables/Makefile \
		 examples/loadables/Makefile.inc \
		 examples/loadables/Makefile.sample \
		 examples/loadables/perl/Makefile \
		 support/bash.pc support/bashbug.sh])

dnl Makefile uses this timestamp file to record whether config.h is up to date.
AC_CONFIG_COMMANDS([stamp-h], [echo timestamp > stamp-h])

AC_OUTPUT
