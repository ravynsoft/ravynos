#!/bin/sh

# quote() - Creates a shell literal
# Usage:  echo "...` quote "..." `..."
quote() {
	case "$1" in
	'') echo "''" ;;
	*)  echo "$1" | sed 's/\([^a-zA-Z0-9.:_/-]\)/\\\1/g' ;;
	esac
}

case $PERL_CONFIG_SH in
'')
	if test -f config.sh
		then TOP=.
	else
		echo "Can't find config.sh."; exit 1
	fi
	. $TOP/config.sh
	;;
esac

case "$osname" in
amigaos)
	devtty=CONSOLE:
	;;
*)
	devtty=/dev/tty
	;;
esac

Makefile=Makefile

: This forces SH files to create target in same directory as SH file.
: This is so that make depend always knows where to find SH derivatives.
case "$0" in
*/*) cd `expr X$0 : 'X\(.*\)/'` ;;
esac

pwd="`pwd`"
linklibperl='$(LIBPERL)'
linklibperl_nonshr=''
shrpldflags='$(LDDLFLAGS)'
ldlibpth=''
DPERL_EXTERNAL_GLOB='-DPERL_EXTERNAL_GLOB'
DPERL_IS_MINIPERL='-DPERL_IS_MINIPERL'
case "$useshrplib" in
true)
	# Prefix all runs of 'miniperl' and 'perl' with
	# $ldlibpth so that ./perl finds *this* shared libperl.
	case "$LD_LIBRARY_PATH" in
	'')  ldlibpth="LD_LIBRARY_PATH=` quote "$pwd" `" ;;
	*)   ldlibpth="LD_LIBRARY_PATH=` quote "$pwd" `:` quote "$LD_LIBRARY_PATH" `" ;;
	esac

	pldlflags="$cccdlflags"
	static_ldflags=''
	case "${osname}${osvers}" in
	darwin*)
		shrpldflags="${ldflags} -dynamiclib \
                            -compatibility_version \
				${api_revision}.${api_version}.${api_subversion} \
			     -current_version \
				${revision}.${patchlevel}.${subversion}"
		case "$osvers" in
	        1[5-9]*|[2-9]*)
			shrpldflags="$shrpldflags -install_name `pwd`/\$@ -Xlinker -headerpad_max_install_names"
			exeldflags="-Xlinker -headerpad_max_install_names"
			;;
		*)
			shrpldflags="$shrpldflags -install_name \$(shrpdir)/\$@"
			;;
		esac
		;;
	cygwin*)
		shrpldflags="$shrpldflags -Wl,--out-implib=libperl.dll.a"
		linklibperl="-L. -lperl"
		;;
	sunos*)
		linklibperl="-lperl"
		;;
	netbsd*|freebsd[234]*|openbsd*|dragonfly*|bitrig*)
		linklibperl="-L. -lperl"
		;;
	interix*)
		linklibperl="-L. -lperl"
		shrpldflags="$shrpldflags -Wl,--image-base,0x57000000"
		;;
	aix*)
		case "$cc" in
		gcc*)
			shrpldflags="-shared -Wl,-H512 -Wl,-T512 -Wl,-bhalt:4 -Wl,-bM:SRE -Wl,-bE:perl.exp"
			case "$osvers" in
			3*)	shrpldflags="$shrpldflags -e _nostart"
				;;
			*)	shrpldflags="$shrpldflags -Wl,-bnoentry"
				;;
			esac
			shrpldflags="$shrpldflags $ldflags $perllibs $cryptlib"
			linklibperl="-L $archlibexp/CORE -L `pwd | sed 's/\/UU$//'` -lperl"
			linklibperl_nonshr='-lperl_nonshr'
			;;
		*)
			shrpldflags="-H512 -T512 -bhalt:4 -bM:SRE -bE:perl.exp"
			case "$osvers" in
			3*)	shrpldflags="$shrpldflags -e _nostart"
				;;
			*)	shrpldflags="$shrpldflags -b noentry"
				;;
			esac
			shrpldflags="$shrpldflags $ldflags $perllibs $cryptlib"
			linklibperl="-L $archlibexp/CORE -L `pwd | sed 's/\/UU$//'` -lperl"
			linklibperl_nonshr='-lperl_nonshr'
			;;
		esac
		;;
	hpux*)
		linklibperl="-L `pwd | sed 's/\/UU$//'` -Wl,+s -Wl,+b$archlibexp/CORE -lperl"
		;;
	os390*)
            case "$use64bitall" in
            define|true|[yY]*) shrpldflags='-Wl,LP64,dll'
                   linklibperl='libperl.x'
                   ;;
            *)     shrpldflags='-Wl,XPLINK,dll'
	           linklibperl='libperl.x'
	           ;;
            esac
            ;;
	esac
	case "$ldlibpthname" in
	'') ;;
	*)
	    case "$osname" in
	    os2)
	        ldlibpth=''
	        ;;
	    *)
		eval "ldlibpthval=\"\$$ldlibpthname\""

		case "$ldlibpthval" in
		'')  ldlibpth="$ldlibpthname=` quote "$pwd" `" ;;
		*)   ldlibpth="$ldlibpthname=` quote "$pwd" `:` quote "$ldlibpthval" `" ;;
		esac

		;;
	    esac

	    ;;
        esac

	case "$osname" in
	linux)
	    # If there is a pre-existing $libperl from a previous
	    # installation, Linux needs to use LD_PRELOAD to
	    # override the LD_LIBRARY_PATH setting.  See the
	    # INSTALL file, under "Building a shared perl library".
	    # If there is no pre-existing $libperl, we don't need
	    # to do anything further.
	    if test -f $archlib/CORE/$libperl; then
		rm -f preload
		cat <<'EOT' > preload
#! /bin/sh
lib=$1
shift
test -r $lib && export LD_PRELOAD="$lib $LD_PRELOAD"
exec "$@"
EOT
		chmod 755 preload
		ldlibpth="$ldlibpth `pwd`/preload `pwd`/$libperl"
	    fi
	    ;;
	os390)	test -f /bin/env && ldlibpth="/bin/env $ldlibpth"
		;;
	esac

	;;

*)	pldlflags=''
	static_ldflags='CCCDLFLAGS='
	;;
esac

# ccdlflags, not to be confused with cccdlflags:
case "${osname}" in
aix)
	# In AIX we need to change this for building Perl itself from
	# the config.sh definition (which is for building external
	# extensions *after* Perl has been built and installed)
	ccdlflags=`echo $ccdlflags|sed -e 's@-bE:.*/perl\.exp@-bE:perl.exp@'`
	;;
esac

: is Cwd static or dynamic
static_cwd='define'
list_util_dep='$(PERL_EXE)'
for f in $dynamic_ext; do
   case $f in
       Cwd) static_cwd='undef' ;;
       List/Util) list_util_dep=lib/auto/List/Util/Util.$dlext ;;
   esac
done

: Prepare dependency lists for Makefile.  Non-Unix systems likely will also
: require changes in make_ext.pl
dynamic_list=' '
dynamic_ext_re="lib/auto/re/re.$dlext"
extra_dep='
cpan/Pod-Simple/pm_to_blib: dist/if/pm_to_blib
ext/Pod-Functions/pm_to_blib: cpan/Pod-Simple/pm_to_blib cpan/Pod-Escapes/pm_to_blib pod/perlfunc.pod
cpan/IO-Compress/pm_to_blib: dist/lib/pm_to_blib
'
for f in $dynamic_ext; do
    : the dependency named here will never exist
      base=`echo "$f" | sed 's/.*\///'`
    this_target="lib/auto/$f/$base.$dlext"
    dynamic_list="$dynamic_list $this_target"

    : Parallel makes reveal that we have some interdependencies
    case $f in
	Math/BigInt/FastCalc|Devel/NYTProf) extra_dep="$extra_dep
$this_target: $list_util_dep" ;;
	Unicode/Normalize) extra_dep="$extra_dep
$this_target: uni.data" ;;
    esac
done

static_list=' '
for f in $static_ext; do
	base=`echo "$f" | sed 's/.*\///'`
	static_list="$static_list lib/auto/$f/$base\$(LIB_EXT)"
    : Parallel makes reveal that we have some interdependencies
    this_target="lib/auto/$f/$base\$(LIB_EXT)"
    case $f in
	Math/BigInt/FastCalc|Devel/NYTProf) extra_dep="$extra_dep
$this_target: lib/auto/List/Util/Util\$(LIB_EXT)" ;;
	Unicode/Normalize) extra_dep="$extra_dep
$this_target: uni.data" ;;
    esac
done

nonxs_list=' '
for f in $nonxs_ext; do
    p=`echo "$f" | tr / -`
    for d in ext dist cpan; do
	if test -d $d/$p; then
	    nonxs_list="$nonxs_list $d/$p/pm_to_blib"
	fi
    done
done

dtrace_h=''

# three object files generated by 'dtrace -G' when dtrace is enabled
dtrace_perllib_o=''
dtrace_mini_o=''
dtrace_main_o=''

case "$usedtrace" in
define|true)
	dtrace_h='perldtrace.h'
	case "$dtraceobject" in
	define)
		dtrace_perllib_o='dtrace_perllib$(OBJ_EXT)'
		dtrace_mini_o='dtrace_mini$(OBJ_EXT)'
		dtrace_main_o='dtrace_main$(OBJ_EXT)'
		;;
	esac
	;;
esac

echo "Extracting $Makefile (with variable substitutions)"
$spitshell >$Makefile <<!GROK!THIS!
# $Makefile
# This file is derived from Makefile.SH.  Any changes made here will
# be lost the next time you run Configure.
#  Makefile is used to generate $firstmakefile.  The only difference
#  is that $firstmakefile has the dependencies filled in at the end.

CC = $cc
LD = $ld

LDFLAGS = $ldflags
CLDFLAGS = $ldflags

mallocsrc = $mallocsrc
mallocobj = $mallocobj
LNS = $lns
# NOTE: some systems don't grok "cp -f". XXX Configure test needed?
CPS = $cp
RMS = rm -f
ranlib = $ranlib
ECHO = $echo

# The following are mentioned only to make metaconfig include the
# appropriate questions in Configure.  If you want to change these,
# edit config.sh instead, or specify --man1dir=/wherever on
# installman commandline.
bin = $installbin
scriptdir = $scriptdir
shrpdir = $archlibexp/CORE
privlib = $installprivlib
man1dir = $man1dir
man1ext = $man1ext
man3dir = $man3dir
man3ext = $man3ext

# The following are used to build and install shared libraries for
# dynamic loading.
LDDLFLAGS = $lddlflags
SHRPLDFLAGS = $shrpldflags
CCDLFLAGS = $ccdlflags
DLSUFFIX = .$dlext
PLDLFLAGS = $pldlflags
LIBPERL = $libperl
LLIBPERL= $linklibperl
SHRPENV = $shrpenv

# Static targets are ordinarily built without CCCDLFLAGS.  However,
# if building a shared libperl.so that might later be linked into
# another application, then it might be appropriate to also build static
# extensions (usually just DynaLoader) with relocatable code (e.g. -fPIC
# for GNU cc).
STATIC_LDFLAGS = $static_ldflags

# The following is used to include the current directory in
# the dynamic loader path you are building a shared libperl.
LDLIBPTH = $ldlibpth

# Sometimes running an executable is an adventure.
RUN = 

# When cross-compiling we want to use a (mini)perl built for the host, not the target
HOST_PERL = $hostperl
HOST_GENERATE = $hostgenerate
SYSTEM_PERL = $perl

# These variables may need to be manually set for non-Unix systems.
AR = $full_ar
HOST_EXE_EXT = $_exe
EXE_EXT = $_exe
LIB_EXT = $_a
OBJ_EXT = $_o

# Macros to invoke a copy of miniperl during the build.  Targets which
# are built using these macros should depend on \$(MINIPERL_EXE)
MINIPERL_EXE = miniperl\$(EXE_EXT)
MINIPERL = \$(LDLIBPTH) ./miniperl\$(EXE_EXT) -Ilib

# Macros to invoke sort the MANIFEST during build
MANIFEST_SRT = MANIFEST.srt

!GROK!THIS!

case "$useshrplib$osname" in
truedarwin)
	$spitshell >>$Makefile <<!GROK!THIS!
PERL_EXE_LDFLAGS=$exeldflags
!GROK!THIS!
	;;
esac

case "$usecrosscompile$perl" in
define?*)
	$spitshell >>$Makefile <<!GROK!THIS!
# Macros to invoke a copy of our fully operational perl during the build.
PERL_EXE = perl\$(EXE_EXT)
RUN_PERL = \$(LDLIBPTH) \$(RUN) $perl\$(EXE_EXT)
!GROK!THIS!
	;;
*)
	$spitshell >>$Makefile <<!GROK!THIS!
# Macros to invoke a copy of our fully operational perl during the build.
PERL_EXE = perl\$(EXE_EXT)
RUN_PERL = \$(LDLIBPTH) \$(RUN) ./perl\$(EXE_EXT) -Ilib -I.
!GROK!THIS!
	;;
esac

$spitshell >>$Makefile <<!GROK!THIS!
# Macros to run our tests
RUN_TESTS = \$(LDLIBPTH) ./runtests

dynamic_ext = $dynamic_list
dynamic_ext_re = $dynamic_ext_re
static_ext = $static_list
nonxs_ext = $nonxs_list
ext = \$(dynamic_ext) \$(static_ext) \$(nonxs_ext)
DYNALOADER = DynaLoader\$(OBJ_EXT)

libs = $perllibs $cryptlib

public = \$(PERL_EXE) utilities

shellflags = $shellflags

# The /dev/tty or the moral equivalent.
devtty = $devtty

# This is set to  MAKE=$make if your $make command doesn't
# do it for you.
$make_set_make

# Mention $gmake here so it gets probed for by Configure.

!GROK!THIS!

case "${osname}" in
linux*|darwin)
$spitshell >>$Makefile <<!GROK!THIS!
# If you're going to use valgrind and it can't be invoked as plain valgrind
# then you'll need to change this, or override it on the make command line.
VALGRIND ?= valgrind
VG_TEST  ?= ./perl -e 1 2>/dev/null

!GROK!THIS!
	;;
esac

case "$dtracexnolibs" in
define) xnolibs=-xnolibs ;;
*) xnolibs= ;;
esac

$spitshell >>$Makefile <<!GROK!THIS!
DTRACE = $dtrace $xnolibs
DTRACE_H = $dtrace_h

DTRACE_PERLLIB_O = $dtrace_perllib_o # "dtrace -G" output for perllib_objs
DTRACE_MINI_O    = $dtrace_mini_o    # "dtrace -G" output for common and mini
DTRACE_MAIN_O    = $dtrace_main_o    # "dtrace -G" output for perlmain.o

FIRSTMAKEFILE = $firstmakefile

# Any special object files needed by this architecture, e.g. os2/os2.obj
ARCHOBJS = $archobjs

.SUFFIXES: .c \$(OBJ_EXT) .i .s .c.depends

# grrr
SHELL = $sh

# how to tr(anslate) newlines
TRNL = '$trnl'

OPTIMIZE = $optimize

EXTRAS = $extras

INSTALLPREFIXEXP = $prefix

!GROK!THIS!
# not used by Makefile but by installperl;
# mentioned here so that metaconfig picks these up
# $installusrbinperl
# $versiononly

case "${osname}:${osvers}" in
darwin:*)
$spitshell >>$Makefile <<EOF

# Your locales are broken (osname $osname, osvers $osvers)
# and to avoid the numerous
# perl: warning: Setting locale failed.
# warnings during the build process we reset the locale variables.

LC_ALL=C
LANG=C
LANGUAGE=C
EOF
	;;
esac

## In the following dollars and backticks do not need the extra backslash.
$spitshell >>$Makefile <<'!NO!SUBS!'

CCCMD    = sh $(shellflags) cflags "optimize='$(OPTIMIZE)'" $@

CCCMDSRC = sh $(shellflags) cflags "optimize='$(OPTIMIZE)'" $<

CONFIGPM_FROM_CONFIG_SH = lib/Config.pm lib/Config_heavy.pl
CONFIGPM = $(CONFIGPM_FROM_CONFIG_SH) lib/Config_git.pl

CONFIGPOD = lib/Config.pod

CONFIGH = config.h
!NO!SUBS!

SH_to_target() {
    echo $@ | sed -e s/\\\.SH//g -e s/_/./g
}

SH='Makefile.SH cflags.SH config_h.SH makedepend.SH myconfig.SH runtests.SH pod/Makefile.SH'
shextract=`SH_to_target $SH`

## In the following dollars and backticks do not need the extra backslash.
$spitshell >>$Makefile <<!GROK!THIS!

private = preplibrary \$(CONFIGPM) \$(CONFIGPOD) git_version.h lib/buildcustomize.pl

# Files to be built with variable substitution before miniperl
# is available.
sh = $SH

shextract = $shextract
!GROK!THIS!

# Source files where we build a variant for miniperl:
mini_special='op perl universal'
for file in $mini_special; do
    mini_special_c="$mini_special_c ${file}mini.c"
    mini_only_objs="$mini_only_objs ${file}mini\$(OBJ_EXT)"
    main_only_objs="$main_only_objs ${file}\$(OBJ_EXT)"
    ctags_exclude="$ctags_exclude --exclude=${file}mini.c"
done

## In the following dollars and backticks do not need the extra backslash.
$spitshell >>$Makefile <<'!NO!SUBS!'

addedbyconf = UU $(shextract)

# Unicode data files generated by mktables
unidatafiles = lib/unicore/Decomposition.pl lib/unicore/TestProp.pl \
	lib/unicore/CombiningClass.pl lib/unicore/Name.pl \
	lib/unicore/UCD.pl lib/unicore/Name.pm \
	lib/unicore/Heavy.pl lib/unicore/mktables.lst

# Directories of Unicode data files generated by mktables
unidatadirs = lib/unicore/To lib/unicore/lib

h1 = EXTERN.h INTERN.h XSUB.h av.h $(CONFIGH) cop.h cv.h dosish.h
h2 = embed.h form.h gv.h handy.h hv.h hv_func.h keywords.h mg.h op.h opcode.h
h3 = pad.h patchlevel.h perl.h perlapi.h perly.h pp.h proto.h regcomp.h regcomp_internal.h
h4 = regexp.h scope.h sv.h unixish.h util.h iperlsys.h thread.h
h5 = utf8.h warnings.h mydtrace.h op_reg_common.h l1_char_class_tab.h
h6 = charclass_invlists.h
h = $(h1) $(h2) $(h3) $(h4) $(h5) $(h6)

c1 = av.c scope.c op.c peep.c doop.c doio.c dump.c gv.c hv.c mg.c reentr.c mro_core.c perl.c
c2 = perly.c pp.c pp_hot.c pp_ctl.c pp_sys.c utf8.c sv.c
c3 = taint.c toke.c util.c deb.c run.c builtin.c universal.c class.c pad.c globals.c keywords.c
c4 = perlio.c numeric.c mathoms.c locale.c pp_pack.c pp_sort.c caretx.c dquote.c time64.c
c5 = regcomp.c regcomp_debug.c regcomp_invlist.c regcomp_study.c regcomp_trie.c regexec.c
c6 = $(mallocsrc)
c_base  = $(c1) $(c2) $(c3) $(c4) $(c5) $(c6)

!NO!SUBS!

$spitshell >>$Makefile <<!GROK!THIS!
mini_only_src  =$mini_special_c
mini_only_objs =$mini_only_objs
main_only_objs =$main_only_objs
!GROK!THIS!

$spitshell >>$Makefile <<'!NO!SUBS!'

c = $(c_base) miniperlmain.c $(mini_only_src)

obj1  = $(mallocobj) gv$(OBJ_EXT) toke$(OBJ_EXT) perly$(OBJ_EXT) pad$(OBJ_EXT)
obj2  = regcomp$(OBJ_EXT) regcomp_debug$(OBJ_EXT) regcomp_invlist$(OBJ_EXT) regcomp_study$(OBJ_EXT) regcomp_trie$(OBJ_EXT)
obj3  = regexec$(OBJ_EXT) dump$(OBJ_EXT) util$(OBJ_EXT) mg$(OBJ_EXT) reentr$(OBJ_EXT) mro_core$(OBJ_EXT)
obj4  = keywords$(OBJ_EXT) builtin$(OBJ_EXT) class$(OBJ_EXT)
obj5  = hv$(OBJ_EXT) av$(OBJ_EXT) run$(OBJ_EXT) pp_hot$(OBJ_EXT) sv$(OBJ_EXT) pp$(OBJ_EXT)
obj6  = scope$(OBJ_EXT) pp_ctl$(OBJ_EXT) pp_sys$(OBJ_EXT) peep$(OBJ_EXT)
obj7  = doop$(OBJ_EXT) doio$(OBJ_EXT) utf8$(OBJ_EXT) taint$(OBJ_EXT)
obj8  = deb$(OBJ_EXT) globals$(OBJ_EXT) perlio$(OBJ_EXT) numeric$(OBJ_EXT) mathoms$(OBJ_EXT)
obj9  = locale$(OBJ_EXT) pp_pack$(OBJ_EXT) pp_sort$(OBJ_EXT) caretx$(OBJ_EXT) dquote$(OBJ_EXT)
obj10 = time64$(OBJ_EXT)

# split the objects into 3 exclusive sets: those used by both miniperl and
# perl, and those used by just one or the other. Doesn't include the
# actual perl(mini)main.o, nor any dtrace objects.

common_objs    = $(obj1) $(obj2) $(obj3) $(obj4) $(obj5) $(obj6) $(obj7) $(obj8) $(obj9) $(obj10) $(ARCHOBJS)

miniperl_objs_nodt = $(mini_only_objs) $(common_objs) miniperlmain$(OBJ_EXT)
perllib_objs_nodt  = $(main_only_objs) $(common_objs)

!NO!SUBS!

# dtrace with -G modifies the source object files, which can cause
# dependency issues, and can cause the dtrace -G to fail on FreeBSD
# so separate the objects generated by $(CC) from those used to link
# the executable when dtrace -G is involved.
#
# $(FOO:op%os=np%ns) isn't generally portable but is portable to
# the makes on darwin, Solaris, FreeBSD and Linux, which is where we
# use dtrace

case "$usedtrace:$dtraceobject" in
define:define)
    $spitshell >>$Makefile <<'!NO!SUBS!'

miniperl_dtrace_objs = $(miniperl_objs_nodt:%=mpdtrace/%)
perllib_dtrace_objs = $(perllib_objs_nodt:%=libpdtrace/%)
perlmain_dtrace_objs = maindtrace/perlmain$(OBJ_EXT)

miniperl_objs = $(miniperl_dtrace_objs) $(DTRACE_MINI_O)
perllib_objs  = $(perllib_dtrace_objs) $(DTRACE_PERLLIB_O)
perlmain_objs = $(perlmain_dtrace_objs) $(DTRACE_MAIN_O)

miniperl_dep = $(DTRACE_MINI_O)
perllib_dep = $(DTRACE_PERLLIB_O)
perlmain_dep = $(DTRACE_MAIN_O)

!NO!SUBS!
    ;;
*)
    $spitshell >>$Makefile <<'!NO!SUBS!'

miniperl_objs = $(miniperl_objs_nodt) $(DTRACE_MINI_O)
perllib_objs  = $(perllib_objs_nodt) $(DTRACE_PERLLIB_O)
perlmain_objs = perlmain$(OBJ_EXT) $(DTRACE_MAIN_O)

miniperl_dep = $(miniperl_objs)
perllib_dep = $(perllib_objs)
perlmain_dep = $(perlmain_objs)

!NO!SUBS!
    ;;
esac

$spitshell >>$Makefile <<'!NO!SUBS!'

perltoc_pod_prereqs = extra.pods pod/perl5382delta.pod pod/perlapi.pod pod/perlintern.pod pod/perlmodlib.pod pod/perluniprops.pod
generated_pods = pod/perltoc.pod $(perltoc_pod_prereqs)
generated_headers = uudmap.h bitcount.h mg_data.h

lintflags = \
    -b \
    -n \
    -p \
    -Ncheck=%all \
    -Nlevel=4 \
    -errchk=parentheses \
    -errhdr=%all \
    -errfmt=src \
    -errtags \
    -erroff=E_ASSIGN_NARROW_CONV \
    -erroff=E_BAD_PTR_CAST \
    -erroff=E_BAD_PTR_CAST_ALIGN \
    -erroff=E_BAD_PTR_INT_COMBINATION \
    -erroff=E_BAD_SIGN_EXTEND \
    -erroff=E_BLOCK_DECL_UNUSED \
    -erroff=E_CASE_FALLTHRU \
    -erroff=E_CONST_EXPR \
    -erroff=E_CONSTANT_CONDITION \
    -erroff=E_END_OF_LOOP_CODE_NOT_REACHED \
    -erroff=E_EQUALITY_NOT_ASSIGNMENT \
    -erroff=E_EXPR_NULL_EFFECT \
    -erroff=E_FALSE_LOGICAL_EXPR \
    -erroff=E_INCL_NUSD \
    -erroff=E_LOOP_EMPTY \
    -erroff=E_MAIN_PARAM \
    -erroff=E_POINTER_TO_OBJECT \
    -erroff=E_PTRDIFF_OVERFLOW \
    -erroff=E_SHIFT_CNT_NEG_TOO_BIG_L \
    -erroff=E_STATIC_UNUSED \
    -erroff=E_TRUE_LOGICAL_EXPR

.c$(OBJ_EXT): 
	@echo `$(CCCMD)` $(PLDLFLAGS) $*.c
	@`$(CCCMD)` $(PLDLFLAGS) $*.c

.c.i:
	@echo `$(CCCMDSRC)` -E $*.c \> $*.i
	@`$(CCCMDSRC)` -E $*.c > $*.i

.c.s:
	@echo `$(CCCMDSRC)` -S $*.c
	@`$(CCCMDSRC)` -S $*.c

all: $(FIRSTMAKEFILE) $(MINIPERL_EXE) $(generated_pods) $(private) $(unidatafiles) $(public) $(dynamic_ext) $(nonxs_ext) extras.make $(MANIFEST_SRT) common_build
	@echo " ";
	@echo "	Everything is up to date. Type '$(MAKE) test' to run test suite."

$(MANIFEST_SRT):	MANIFEST $(PERL_EXE)
	@$(RUN_PERL) Porting/manisort -q || (echo "WARNING: re-sorting MANIFEST"; \
	    $(RUN_PERL) Porting/manisort -q -o MANIFEST; sh -c true)
	@touch $(MANIFEST_SRT)

.PHONY: all utilities

# Both git_version.h and lib/Config_git.pl are built
# by make_patchnum.pl.
git_version.h: lib/Config_git.pl

lib/Config_git.pl: $(MINIPERL_EXE) make_patchnum.pl
	$(MINIPERL) make_patchnum.pl

# make sure that we recompile perl.c if the git version changes
perl$(OBJ_EXT): git_version.h

!NO!SUBS!

# Making utilities requires Cwd.  If we have dynamic
# loading, we only need miniperl and Cwd.$dlext.  If we have static
# loading, we need to build perl first.
case "$usedl$static_cwd" in
defineundef)
    util_deps='$(MINIPERL_EXE) $(CONFIGPM) lib/auto/Cwd/Cwd$(DLSUFFIX) FORCE'
    ;;
definedefine)
    util_deps='$(PERL_EXE) $(CONFIGPM) FORCE'
    ;;
*)  util_deps='$(PERL_EXE) $(CONFIGPM) FORCE'
    ;;
esac

$spitshell >>$Makefile <<!GROK!THIS!
utils/Makefile: \$(MINIPERL_EXE) \$(CONFIGPM) utils/Makefile.PL
	\$(MINIPERL) utils/Makefile.PL

utilities:	utils/Makefile $util_deps
	@echo " "; echo "	Making utilities"; cd utils; \$(LDLIBPTH) \$(MAKE) all

!GROK!THIS!

$spitshell >>$Makefile <<'!NO!SUBS!'

# This is now done by installman only if you actually want the man pages.
#	@echo " "; echo "	Making docs"; cd pod; $(MAKE) all;

# Phony target to force checking subdirectories.
# Apparently some makes require an action for the FORCE target.
.PHONY: FORCE
FORCE:
	@sh -c true
!NO!SUBS!

for file in $mini_special; do
    if $issymlink $file.c; then
        $spitshell >>$Makefile <<!GROK!THIS!

# We do a copy of the $file.c instead of a symlink because gcc gets huffy
# if we have a symlink forest to another disk (it complains about too many
# levels of symbolic links, even if we have only two)

${file}mini.c: $file.c
	\$(RMS) ${file}mini.c
	\$(CPS) ${file}.c ${file}mini.c
!GROK!THIS!
    else
        $spitshell >>$Makefile <<!GROK!THIS!

${file}mini.c: $file.c
	\$(RMS) ${file}mini.c
	\$(LNS) ${file}.c ${file}mini.c
!GROK!THIS!
    fi

    $spitshell >>$Makefile <<!GROK!THIS!

${file}mini\$(OBJ_EXT): ${file}mini.c
	echo @\`\$(CCCMD)\` \$(PLDLFLAGS) $DPERL_IS_MINIPERL $DPERL_EXTERNAL_GLOB ${file}mini.c
	@\`\$(CCCMD)\` \$(PLDLFLAGS) $DPERL_IS_MINIPERL $DPERL_EXTERNAL_GLOB ${file}mini.c
!GROK!THIS!
done

$spitshell >>$Makefile <<'!NO!SUBS!'

cygwin.c: cygwin/cygwin.c
	$(LNS) cygwin/cygwin.c

globals$(OBJ_EXT): $(generated_headers)

uudmap.h mg_data.h: bitcount.h

generate_uudmap$(OBJ_EXT): mg_raw.h

!NO!SUBS!

# If hostgerenate isn't defined, then we have to build
# generate_uudmap and run it. This can potentially mean running
# it on the target system if we're cross-compiling.
# If it is defined, then we just run it locally.
case "$hostgenerate" in
''|'undef')
$spitshell >>$Makefile <<!GROK!THIS!
bitcount.h: generate_uudmap\$(HOST_EXE_EXT)
	$run ./generate_uudmap\$(HOST_EXE_EXT) \$(generated_headers)
	-@for f in \$(generated_headers); do \\
	    $from \$\$f; \\
	done

!GROK!THIS!

$spitshell >>$Makefile <<'!NO!SUBS!'
generate_uudmap$(HOST_EXE_EXT): generate_uudmap$(OBJ_EXT)
	$(CC) -o generate_uudmap$(EXE_EXT) $(LDFLAGS) generate_uudmap$(OBJ_EXT) $(libs)

!NO!SUBS!
;;
*)
$spitshell >>$Makefile <<'!NO!SUBS!'
bitcount.h: generate_uudmap$(HOST_EXE_EXT)
	./generate_uudmap$(HOST_EXE_EXT) $(generated_headers)

generate_uudmap$(HOST_EXE_EXT): generate_uudmap$(OBJ_EXT)
	-@rm generate_uudmap$(HOST_EXE_EXT)
	$(LNS) $(HOST_GENERATE) generate_uudmap$(HOST_EXE_EXT)

!NO!SUBS!
;;
esac

case "$osname" in
amigaos*)
$spitshell >>$Makefile <<'!NO!SUBS!'
perlmain.c: $(MINIPERL_EXE) ext/ExtUtils-Miniperl/pm_to_blib
	$(MINIPERL) -MExtUtils::Miniperl -e 'writemain(\\"perlmain.c", @ARGV)' DynaLoader $(static_ext)

# The file ext.libs is a list of libraries that must be linked in
# for static extensions, e.g. -lm -lgdbm, etc.  The individual
# static extension Makefile's add to it.
ext.libs: $(static_ext)
	-@test -f ext.libs || touch ext.libs

!NO!SUBS!
;;
*)
$spitshell >>$Makefile <<'!NO!SUBS!'
perlmain.c: $(MINIPERL_EXE) ext/ExtUtils-Miniperl/pm_to_blib
	$(MINIPERL) -MExtUtils::Miniperl -e 'writemain(\"perlmain.c", @ARGV)' DynaLoader $(static_ext)

# The file ext.libs is a list of libraries that must be linked in
# for static extensions, e.g. -lm -lgdbm, etc.  The individual
# static extension Makefile's add to it.
ext.libs: $(static_ext)
	-@test -f ext.libs || touch ext.libs

!NO!SUBS!
;;
esac

# How to build libperl.  This is still rather convoluted.
# Load up custom Makefile.SH fragment for shared loading and executables:
case "$osname" in
*)
	Makefile_s="$osname/Makefile.SHs"
	;;
esac

case "$osname" in
aix)
	case "$useshrplib" in
	define|true|[yY]*)
		$spitshell >>$Makefile <<'!NO!SUBS!'

LIBPERLEXPORT		= perl.exp

!NO!SUBS!
		
		;;
	*)	
		$spitshell >>$Makefile <<'!NO!SUBS!'

PERLEXPORT		= perl.exp

!NO!SUBS!
	;;
	esac
	$spitshell >>$Makefile <<'!NO!SUBS!'
perl.exp: $(MINIPERL_EXE) makedef.pl $(CONFIGPM) $(SYM) $(SYMH)
	$(MINIPERL) makedef.pl --sort-fold PLATFORM=aix CC_FLAGS="$(OPTIMIZE)" > perl.exp

!NO!SUBS!
	;;
os2)
	$spitshell >>$Makefile <<'!NO!SUBS!'
MINIPERLEXP		= miniperl

perl5.def: $(MINIPERLEXP) makedef.pl $(CONFIGPM) $(SYM) $(SYMH) miniperl.map
	$(MINIPERL) makedef.pl PLATFORM=os2 -DPERL_DLL=$(PERL_DLL) CC_FLAGS="$(OPTIMIZE)" > perl5.def

!NO!SUBS!
	;;
esac

if test -s $Makefile_s ; then
	. $Makefile_s
	$spitshell >>$Makefile <<!GROK!THIS!

Makefile: $Makefile_s
!GROK!THIS!
else
	case "$dtrace_h" in
	?*)
		$spitshell >>$Makefile <<'!NO!SUBS!'
# dtrace dicards const qualifiers from arguments, put them back
$(DTRACE_H): perldtrace.d
	$(DTRACE) -h -s perldtrace.d -o $(DTRACE_H).in
	sed -e '/const/!s/char \*/const char */g' $(DTRACE_H).in >$(DTRACE_H)
	$(RMS) $(DTRACE_H).in

mydtrace.h: $(DTRACE_H)

!NO!SUBS!
		;;
	esac
	case "$dtraceobject" in
	define)
		$spitshell >>$Makefile <<'!NO!SUBS!'
$(DTRACE_MINI_O): perldtrace.d $(miniperl_objs_nodt)
	-rm -rf mpdtrace
	mkdir mpdtrace
	cp $(miniperl_objs_nodt) mpdtrace/
	$(DTRACE) -G -s perldtrace.d -o $(DTRACE_MINI_O) $(miniperl_dtrace_objs)

$(DTRACE_PERLLIB_O): perldtrace.d $(perllib_objs_nodt)
	-rm -rf libpdtrace
	mkdir libpdtrace
	cp $(perllib_objs_nodt) libpdtrace/
	$(DTRACE) -G -s perldtrace.d -o $(DTRACE_PERLLIB_O) $(perllib_dtrace_objs)

$(DTRACE_MAIN_O): perldtrace.d perlmain$(OBJ_EXT)
	-rm -rf maindtrace
	mkdir maindtrace
	cp perlmain$(OBJ_EXT) maindtrace/
	$(DTRACE) -G -s perldtrace.d -o $(DTRACE_MAIN_O) $(perlmain_dtrace_objs) ||	      \
	  ( $(ECHO) "No probes in perlmain$(OBJ_EXT), generating a dummy $(DTRACE_MAIN_O)" && \
	    $(ECHO) >dtrace_main.c &&							      \
	    `$(CCCMD)` $(PLDLFLAGS) dtrace_main.c &&					      \
	     rm -f dtrace_main.c )

!NO!SUBS!
		;;
    esac
	$spitshell >>$Makefile <<'!NO!SUBS!'
$(LIBPERL): $& $(perllib_dep) $(DYNALOADER) $(LIBPERLEXPORT)
!NO!SUBS!
	case "$useshrplib" in
	true)
		$spitshell >>$Makefile <<'!NO!SUBS!'
	rm -f $@
	$(LD) -o $@ $(SHRPLDFLAGS) $(perllib_objs) $(DYNALOADER) $(libs)
!NO!SUBS!
		case "$osname" in
		aix)
			$spitshell >>$Makefile <<'!NO!SUBS!'
	rm -f libperl$(OBJ_EXT)
	mv $@ libperl$(OBJ_EXT)
	$(AR) qv $(LIBPERL) libperl$(OBJ_EXT)
!NO!SUBS!
			;;
		esac
		;;
	*)
		$spitshell >>$Makefile <<'!NO!SUBS!'
	rm -f $(LIBPERL)
	$(AR) rc $(LIBPERL) $(perllib_objs) $(DYNALOADER)
	@$(ranlib) $(LIBPERL)
!NO!SUBS!
		;;
	esac
	$spitshell >>$Makefile <<'!NO!SUBS!'

# How to build executables.

# The $& notation tells Sequent machines that it can do a parallel make,
# and is harmless otherwise.
# The miniperl -w -MExporter line is a basic cheap test to catch errors
# before make goes on to run preplibrary and then MakeMaker on extensions.
# This is very handy because later errors are often caused by miniperl
# build problems but that's not obvious to the novice.
# The Module used here must not depend on Config or any extensions.

# The seemingly cranky ordering of having $(MINIPERL_EXE) depend on
# lib/buildcustomize.pl despite the reality that lib/buildcustomize.pl needs
# miniperl to exist first permits us to
# a) have one rule generate both miniperl and lib/buildcustomize.pl
#    (so that lib/buildcustomize.pl is always available. This simplifies things)
# b) have the rest of the Makefile depend on the more obvious $(MINIPERL_EXE)

$(MINIPERL_EXE): lib/buildcustomize.pl
	@touch $(MINIPERL_EXE)

!NO!SUBS!

	case "${osname}${osvers}" in
	amigaos*)
		$spitshell >>$Makefile <<'!NO!SUBS!'
lib/buildcustomize.pl: $& $(miniperl_objs) write_buildcustomize.pl
	-@rm -f miniperl.xok
	$(CC) $(CLDFLAGS) -o $(MINIPERL_EXE) \
	    $(miniperl_objs) $(libs)
#	$(LDLIBPTH) ./miniperl$(HOST_EXE_EXT) -w -Ilib -Idist/Exporter/lib -MExporter -e '<?>' || sh -c 'echo >&2 Failed to build miniperl.  Please run make minitest; exit 1'
	$(MINIPERL) -f write_buildcustomize.pl
!NO!SUBS!
		;;
	darwin*)
		case "$osvers" in
		[1-6].*) ;;
		*) case "$ldflags" in
		    *"-flat_namespace"*) ;;
		    *) # to allow opmini.o to override stuff in libperl.dylib
		$spitshell >>$Makefile <<!NO!SUBS!
NAMESPACEFLAGS = -force_flat_namespace
!NO!SUBS!
		       ;;
		    esac
		    ;;
		esac
		$spitshell >>$Makefile <<'!NO!SUBS!'
lib/buildcustomize.pl: $& $(miniperl_objs) write_buildcustomize.pl
	-@rm -f miniperl.xok
	$(CC) $(CLDFLAGS) $(NAMESPACEFLAGS) -o $(MINIPERL_EXE) \
	    $(miniperl_objs) $(libs)
	$(LDLIBPTH) ./miniperl$(HOST_EXE_EXT) -w -Ilib -Idist/Exporter/lib -MExporter -e '<?>' || sh -c 'echo >&2 Failed to build miniperl.  Please run make minitest; exit 1'
	$(MINIPERL) -f write_buildcustomize.pl
!NO!SUBS!
		;;
	*)
		if test "X$hostperl" != X; then
			$spitshell >>$Makefile <<!GROK!THIS!
lib/buildcustomize.pl: \$& \$(miniperl_dep) write_buildcustomize.pl
	-@rm -f miniperl.xok
	-@rm \$(MINIPERL_EXE)
	\$(LNS) \$(HOST_PERL) \$(MINIPERL_EXE)
	\$(LDLIBPTH) ./miniperl\$(HOST_EXE_EXT) -w -Ilib -Idist/Exporter/lib -MExporter -e '<?>' || sh -c 'echo >&2 Failed to build miniperl.  Please run make minitest; exit 1'
	\$(MINIPERL) -f write_buildcustomize.pl 'osname' "$osname"
!GROK!THIS!
		else
			$spitshell >>$Makefile <<'!NO!SUBS!'
lib/buildcustomize.pl: $& $(miniperl_dep) write_buildcustomize.pl
	-@rm -f miniperl.xok
	$(CC) $(CLDFLAGS) -o $(MINIPERL_EXE) \
	    $(miniperl_objs) $(libs)
	$(LDLIBPTH) ./miniperl$(HOST_EXE_EXT) -w -Ilib -Idist/Exporter/lib -MExporter -e '<?>' || sh -c 'echo >&2 Failed to build miniperl.  Please run make minitest; exit 1'
	$(MINIPERL) -f write_buildcustomize.pl
!NO!SUBS!
		fi
		;;
	esac

	$spitshell >>$Makefile <<'!NO!SUBS!'

$(PERL_EXE): $& $(perlmain_dep) $(LIBPERL) $(static_ext) ext.libs $(PERLEXPORT) write_buildcustomize.pl
	-@rm -f miniperl.xok
!NO!SUBS!

        case "$osname" in
	# In AmigaOS the Perl executable needs to be linked with -ldl,
	# but none of the other executables should be.
        amigaos) $spitshell >>$Makefile <<'!NO!SUBS!'
	$(SHRPENV) $(CC) -o perl $(CLDFLAGS) $(CCDLFLAGS) $(perlmain_objs) $(LLIBPERL) $(static_ext) `cat ext.libs` $(libs) -ldl
!NO!SUBS!
        ;;
        os390) $spitshell >>$Makefile <<'!NO!SUBS!'
	$(SHRPENV) $(CC) -o perl $(CLDFLAGS) $(CCDLFLAGS) $(perlmain_objs) $(LLIBPERL) $(static_ext) `cat ext.libs` $(libs)
!NO!SUBS!
        ;;

	darwin)
	    case "$useshrplib$osvers" in
	    true1[5-9]*|true[2-9]*) $spitshell >>$Makefile <<'!NO!SUBS!'
	$(SHRPENV) $(CC) -o perl $(PERL_EXE_LDFLAGS) $(CLDFLAGS) $(CCDLFLAGS) $(perlmain_objs) $(static_ext) $(LLIBPERL) `cat ext.libs` $(libs)
!NO!SUBS!
	       ;;
	    *) $spitshell >>$Makefile <<'!NO!SUBS!'
	$(SHRPENV) $(CC) -o perl $(CLDFLAGS) $(CCDLFLAGS) $(perlmain_objs) $(static_ext) $(LLIBPERL) `cat ext.libs` $(libs)
!NO!SUBS!
	       ;;
	    esac
        ;;

        *) $spitshell >>$Makefile <<'!NO!SUBS!'
	$(SHRPENV) $(CC) -o perl $(CLDFLAGS) $(CCDLFLAGS) $(perlmain_objs) $(static_ext) $(LLIBPERL) `cat ext.libs` $(libs)
!NO!SUBS!
        ;;
        esac

	$spitshell >>$Makefile <<'!NO!SUBS!'

# Microperl.  This is just a convenience thing if one happens to
# build also the full Perl and therefore the real big Makefile:
# usually one should manually explicitly issue the below command.

.PHONY: microperl
microperl:
	$(MAKE) -f Makefile.micro

!NO!SUBS!

fi

# Some environment have no system(), which mkpport uses.
# Let's try running the commands with shell.
case "${osname}" in
catamount)
$spitshell >>$Makefile <<!GROK!THIS!
.PHONY: makeppport
makeppport: \$(MINIPERL_EXE) \$(CONFIGPM)
	-@for f in Makefile.PL PPPort_pm.PL RealPPPort_xs.PL ppport_h.PL; do \
	(cd ext/Devel-PPPort && `pwd`/run.sh ../../$(MINIPERL_EXE) -I../../lib \$\$f); \
	done

!GROK!THIS!
;;
*)
$spitshell >>$Makefile <<'!NO!SUBS!'
.PHONY: makeppport
makeppport: $(MINIPERL_EXE) $(CONFIGPM) $(nonxs_ext)
	$(MINIPERL) mkppport

!NO!SUBS!
;;
esac

$spitshell >>$Makefile <<'!NO!SUBS!'

.PHONY: preplibrary common_build
preplibrary: $(MINIPERL_EXE) $(CONFIGPM) $(PREPLIBRARY_LIBPERL)

$(CONFIGPM_FROM_CONFIG_SH): $(CONFIGPOD)

$(CONFIGPOD): config.sh $(MINIPERL_EXE) configpm Porting/Glossary lib/Config_git.pl
	$(MINIPERL) configpm

unidatafiles $(unidatafiles) pod/perluniprops.pod: uni.data

uni.data: $(MINIPERL_EXE) $(CONFIGPM) lib/unicore/mktables $(nonxs_ext)
	$(MINIPERL) lib/unicore/mktables -C lib/unicore -P pod -maketest -makenormtest -makelist -p
#	Commented out so always runs, mktables looks at far more files than we
#	can in this makefile to decide if needs to run or not
#	touch uni.data

# $(PERL_EXE) and ext because pod_lib.pl needs Digest::MD5
# But also this ensures that all extensions are built before we try to scan
# them, which picks up Devel::PPPort's documentation.
pod/perltoc.pod: $(perltoc_pod_prereqs) $(PERL_EXE) $(ext) pod/buildtoc
	$(RUN_PERL) -f pod/buildtoc -q

pod/perlapi.pod: pod/perlintern.pod

pod/perlintern.pod: $(MINIPERL_EXE) autodoc.pl embed.fnc
	$(MINIPERL) autodoc.pl

pod/perlmodlib.pod: $(MINIPERL_EXE) pod/perlmodlib.PL MANIFEST
	$(MINIPERL) pod/perlmodlib.PL -q

pod/perl5382delta.pod: pod/perldelta.pod
	$(RMS) pod/perl5382delta.pod
	$(LNS) perldelta.pod pod/perl5382delta.pod

extra.pods: $(MINIPERL_EXE)
	-@test ! -f extra.pods || rm -f `cat extra.pods`
	-@rm -f extra.pods
	-@for x in `grep -l '^=[a-z]' README.* | grep -v README.vms` ; do \
	    nx=`echo $$x | sed -e "s/README\.//"`; \
	    $(LNS) ../$$x "pod/perl"$$nx".pod" ; \
	    echo "pod/perl"$$nx".pod" >> extra.pods ; \
	done

extras.make: $(PERL_EXE)
	-@test ! -s extras.lst || PATH="`pwd`:${PATH}" PERL5LIB="`pwd`/lib" $(RUN_PERL) -MCPAN -e '@ARGV&&make(@ARGV)' `cat extras.lst`

extras.test: $(PERL_EXE)
	-@test ! -s extras.lst || PATH="`pwd`:${PATH}" PERL5LIB="`pwd`/lib" $(RUN_PERL) -MCPAN -e '@ARGV&&test(@ARGV)' `cat extras.lst`

extras.install: $(PERL_EXE)
	-@test ! -s extras.lst || PATH="`pwd`:${PATH}" PERL5LIB="`pwd`/lib" $(RUN_PERL) -MCPAN -e '@ARGV&&install(@ARGV)' `cat extras.lst`

.PHONY: install install-strip install-all install-verbose install-silent \
	no-install install.perl install.man install.html

# Set this to an empty string to avoid an attempt of rebuild before install
INSTALL_DEPENDENCE = all

no_install no-install: install-notify

install: install-all
!NO!SUBS!

for name in all notify silent strip verbose; do
    flags="--$name";
    flags=`echo $flags | sed -e 's/--all//'`
    if test "X$hostperl" != X; then
        $spitshell >>$Makefile <<EOT

install_$name install-$name: \$(INSTALL_DEPENDENCE) installperl all installman
	\$(HOST_PERL) installperl --destdir=\$(DESTDIR) $flags \$(INSTALLFLAGS) \$(STRIPFLAGS)
	\$(HOST_PERL) installman --destdir=\$(DESTDIR) $flags
EOT
    else
        $spitshell >>$Makefile <<EOT

install_$name install-$name: \$(INSTALL_DEPENDENCE) installperl all installman
	\$(RUN_PERL) installperl --destdir=\$(DESTDIR) $flags \$(INSTALLFLAGS) \$(STRIPFLAGS)
	\$(RUN_PERL) installman --destdir=\$(DESTDIR) $flags
EOT
    fi

    test $name = notify || echo '	-@test ! -s extras.lst || PATH="`pwd`:${PATH}" PERL5LIB="`pwd`/lib" $(RUN_PERL) -Ilib -MCPAN -e '\''@ARGV&&install(@ARGV)'\'' `cat extras.lst`' >>$Makefile
done

if test "X$hostperl" != X; then
	LOCAL_PERL='$(HOST_PERL)'
	$spitshell >>$Makefile <<'!NO!SUBS!'
install.perl:	$(INSTALL_DEPENDENCE) installperl
	$(HOST_PERL) installperl --destdir=$(DESTDIR) $(INSTALLFLAGS) $(STRIPFLAGS)
	-@test ! -s extras.lst || $(MAKE) extras.install
!NO!SUBS!
else
	LOCAL_PERL='$(RUN_PERL)'
	$spitshell >>$Makefile <<'!NO!SUBS!'
install.perl:	$(INSTALL_DEPENDENCE) installperl
	$(RUN_PERL) installperl --destdir=$(DESTDIR) $(INSTALLFLAGS) $(STRIPFLAGS)
	-@test ! -s extras.lst || PATH="`pwd`:\${PATH}" PERL5LIB="`pwd`/lib" \$(RUN_PERL) -Ilib -MCPAN -e '@ARGV&&install(@ARGV)' `cat extras.lst`

!NO!SUBS!
fi

$spitshell >>$Makefile <<!GROK!THIS!
install.man:	all installman
	${LOCAL_PERL} installman --destdir=\$(DESTDIR) \$(INSTALLFLAGS)

# XXX Experimental. Hardwired values, but useful for testing.
# Eventually Configure could ask for some of these values.
install.html: all installhtml
	-@test -f README.vms && cd vms && \$(LNS) ../README.vms README_vms.pod && cd ..
	${LOCAL_PERL} installhtml   \\
      --podroot=. --podpath=. --recurse  \\
      --htmldir=\$(privlib)/html   \\
      --splithead=pod/perlipc     \\
      --splititem=pod/perlfunc    \\
      --ignore=Porting/Maintainers.pm,Porting/pumpkin.pod,Porting/repository.pod \\
      --verbose
!GROK!THIS!

$spitshell >>$Makefile <<'!NO!SUBS!'

# I now supply perly.c with the kits, so the following section is
# used only if you force bison to run by saying
# 	make regen_perly
# You normally shouldn't remake perly.[ch].

.PHONY: regen_perly

run_byacc run-byacc:
	@echo "run_byacc is obsolete; try 'make regen_perly' instead"

# this outputs perly.h, perly.act and perly.tab
regen_perly regen-perly:
	perl regen_perly.pl

# We don't want to regenerate perly.c and perly.h, but they might
# appear out-of-date after a patch is applied or a new distribution is
# made.
perly.c: perly.y
	-@sh -c true

perly.h: perly.y
	-@sh -c true

SYM  = globvar.sym perlio.sym

SYMH = perlvars.h intrpvar.h

CHMOD_W = chmod +w

# The following files are generated automatically
#	embed.pl:	proto.h embed.h embedvar.h
#	opcode.pl:	opcode.h opnames.h pp_proto.h
#	regcomp.pl:	regnodes.h
#	warnings.pl:	warnings.h lib/warnings.pm
#	feature.pl:	feature.h lib/feature.pm
# The correct versions should be already supplied with the perl kit,
# in case you don't have perl available.
# To force them to be regenerated, run
#       perl regen.pl
# with your existing copy of perl
# (make regen_headers is kept for backwards compatibility)

.PHONY: regen_headers regen_all

regen:	FORCE
	-perl regen.pl
	-perl regen/uconfig_h.pl

regen_headers regen-headers:	FORCE
	-perl regen.pl -v
	-perl regen/uconfig_h.pl -v

regen_meta regen-meta:  META.yml META.json

META.yml:   FORCE
	PATH="`pwd`:${PATH}" PERL5LIB="`pwd`/lib" $(RUN_PERL) Porting/makemeta -y

META.json:   FORCE
	PATH="`pwd`:${PATH}" PERL5LIB="`pwd`/lib" $(RUN_PERL) Porting/makemeta -j


regen_all regen-all: regen regen_meta

.PHONY:	manisort manicheck

manisort:	FORCE
	@perl Porting/manisort -q || (echo "WARNING: re-sorting MANIFEST"; \
		perl Porting/manisort -q -o MANIFEST; sh -c true)

manicheck:	FORCE
	perl Porting/manicheck

# Extensions:
# Names added to $(dynamic_ext) or $(static_ext) or $(nonxs_ext) will
# automatically get built.  There should ordinarily be no need to change
# any of this part of makefile.
#
# The dummy dependency is a place holder in case $(dynamic_ext) or
# $(static_ext) is empty.
#
# DynaLoader may be needed for extensions that use Makefile.PL.

$(DYNALOADER):	$(MINIPERL_EXE) lib/buildcustomize.pl preplibrary FORCE $(nonxs_ext)
	$(MINIPERL) make_ext.pl $@ $(MAKE_EXT_ARGS) MAKE="$(MAKE)" LIBPERL_A=$(LIBPERL) LINKTYPE=static $(STATIC_LDFLAGS)

d_dummy $(dynamic_ext):	$(MINIPERL_EXE) lib/buildcustomize.pl preplibrary makeppport $(DYNALOADER) FORCE $(PERLEXPORT) $(LIBPERL)
	$(MINIPERL) make_ext.pl $@ $(MAKE_EXT_ARGS) MAKE="$(MAKE)" LIBPERL_A=$(LIBPERL) LINKTYPE=dynamic

s_dummy $(static_ext):	$(MINIPERL_EXE) lib/buildcustomize.pl preplibrary makeppport $(DYNALOADER) FORCE
	$(MINIPERL) make_ext.pl $@ $(MAKE_EXT_ARGS) MAKE="$(MAKE)" LIBPERL_A=$(LIBPERL) LINKTYPE=static $(STATIC_LDFLAGS)

n_dummy $(nonxs_ext):	$(MINIPERL_EXE) lib/buildcustomize.pl preplibrary FORCE
	$(MINIPERL) make_ext.pl $@ $(MAKE_EXT_ARGS) MAKE="$(MAKE)" LIBPERL_A=$(LIBPERL)
!NO!SUBS!

$spitshell >>$Makefile <<EOF
$extra_dep
EOF

$spitshell >>$Makefile <<'!NO!SUBS!'

.PHONY: printconfig
printconfig:
	@eval `$(RUN_PERL) -V:$(CONFIGVAR)`; echo $$$(CONFIGVAR)

.PHONY: clean _tidy _mopup _cleaner1 _cleaner2 \
	realclean _realcleaner clobber _clobber \
	distclean veryclean _verycleaner

clean:		realclean

realclean:	_realcleaner _mopup
	@echo "Note that '$(MAKE) realclean' does not delete config.sh or Policy.sh"

_clobber:
	-@rm -f Cross/run-* Cross/to-* Cross/from-* Cross/mkdir
	-rm -rf host
	rm -f t/test_state
	rm -f config.sh cppstdin Policy.sh extras.lst
	rm -f $(MANIFEST_SRT)

clobber:	_realcleaner _mopup _clobber

distclean:	clobber

# Like distclean but also removes emacs backups and *.orig.
veryclean:	_verycleaner _mopup _clobber
	-@rm -f Obsolete Wanted

# Do not 'make _mopup' directly.
_mopup:
	rm -f *$(OBJ_EXT) *$(LIB_EXT) all perlmain.c $(mini_only_src) generate_uudmap$(EXE_EXT) $(generated_headers)
	-rmdir .depending
	-rm *.depends makedepend_file
	-@test -f extra.pods && rm -f `cat extra.pods`
	-@test -f vms/README_vms.pod && rm -f vms/README_vms.pod
	-rm -f perl.exp ext.libs $(generated_pods) uni.data $(mini_only_objs) pod/roffitall
	-rm -f perl.export perl.dll perl.libexp perl.map perl.def
	-rm -f *perl.xok
	-rm -f cygwin.c libperl*.def libperl*.dll cygperl*.dll *.exe.stackdump
	-rm -f $(PERL_EXE) $(MINIPERL_EXE) $(LIBPERL) libperl.* microperl
	-rm -f config.arch config.over $(DTRACE_H)

_cleaner1:
	-cd os2; rm -f Makefile
	-cd pod; $(LDLIBPTH) $(MAKE) $(CLEAN)
	-cd utils; $(LDLIBPTH) $(MAKE) $(CLEAN)
	-@if test -f $(MINIPERL_EXE) ; then \
	for x in $(DYNALOADER) $(dynamic_ext) $(static_ext) $(nonxs_ext) ; do \
	$(MINIPERL) make_ext.pl --target=$(CLEAN) $$x MAKE="$(MAKE)" ; \
	done ; \
	else \
	sh $(CLEAN).sh ; \
	fi
	rm -f realclean.sh veryclean.sh
	-rm -f `grep -v ^# mkppport.lst | grep . | sed -e 's/$$/\/ppport.h/'`

# Dear POSIX, thanks for making the default to xargs to be
# run once if nothing is passed in. It is such a great help.

# Some systems do not support "?", so keep these files separate.

# The Time::HiRes Makefile *should* remove dist/Time-HiRes/xdefine for 'clean',
# but it's possible to get a rebuilt checkout into a state where it can't run
# distclean, and can't recover until the file is gone.
# There's no harm in adding it to the top level 'distclean' target - it should
# anyway by then, and miniperl is long gone.
# Add new rules before that line - the next line (rm -f so_locations ...) is
# used as a placeholder by a regen script.
_cleaner2:
	-rm -f core.*perl.*.? t/core.perl.*.? .?*.c
	rm -f core *perl.core t/core t/*perl.core core.* t/core.*
	rm -f t/$(PERL_EXE) t/rantests
	rm -rf t/tmp*
	rm -rf $(addedbyconf)
	rm -f $(FIRSTMAKEFILE) $(FIRSTMAKEFILE).old makefile.old utils/Makefile
	rm -f $(private)
	rm -rf $(unidatafiles) $(unidatadirs)
	rm -rf lib/auto
	rm -f lib/.exists lib/*/.exists lib/*/*/.exists
	rm -f h2ph.man
	rm -rf .config
	rm -f preload
	rm -f pod2htmd.tmp
	rm -rf pod/perlfunc pod/perlipc
	-rmdir ext/B/lib
	-rm -f dist/Time-HiRes/xdefine
	rm -f so_locations $(LIBPERL_NONSHR)
	-rmdir lib/version lib/threads lib/inc/ExtUtils lib/inc lib/encoding
	-rmdir lib/autodie/exception lib/autodie/Scope lib/autodie lib/XS
	-rmdir lib/Win32API lib/VMS lib/Unicode/Collate/Locale
	-rmdir lib/Unicode/Collate/CJK lib/Unicode/Collate lib/Tie/Hash
	-rmdir lib/Thread lib/Text lib/Test2/Util lib/Test2/Tools
	-rmdir lib/Test2/IPC/Driver lib/Test2/IPC lib/Test2/Hub/Interceptor
	-rmdir lib/Test2/Hub lib/Test2/Formatter lib/Test2/EventFacet/Info
	-rmdir lib/Test2/EventFacet lib/Test2/Event/TAP lib/Test2/Event
	-rmdir lib/Test2/API/InterceptResult lib/Test2/API lib/Test2
	-rmdir lib/Test/use lib/Test/Tester lib/Test/Builder/Tester
	-rmdir lib/Test/Builder/IO lib/Test/Builder lib/Test lib/Term
	-rmdir lib/TAP/Parser/YAMLish lib/TAP/Parser/SourceHandler
	-rmdir lib/TAP/Parser/Scheduler lib/TAP/Parser/Result
	-rmdir lib/TAP/Parser/Iterator lib/TAP/Parser lib/TAP/Harness
	-rmdir lib/TAP/Formatter/File lib/TAP/Formatter/Console
	-rmdir lib/TAP/Formatter lib/TAP lib/Sys/Syslog lib/Sys lib/Sub
	-rmdir lib/Search lib/Scalar lib/Pod/Text lib/Pod/Simple
	-rmdir lib/Pod/Perldoc lib/Pod/Html lib/PerlIO/via lib/PerlIO lib/Perl
	-rmdir lib/Parse/CPAN lib/Parse lib/Params lib/Net/FTP lib/Module/Load
	-rmdir lib/Module/CoreList lib/Module lib/Memoize lib/Math/BigRat
	-rmdir lib/Math/BigInt lib/Math/BigFloat lib/Math lib/MIME
	-rmdir lib/Locale/Maketext lib/Locale lib/List/Util lib/List
	-rmdir lib/JSON/PP lib/JSON lib/IPC lib/IO/Uncompress/Adapter
	-rmdir lib/IO/Uncompress lib/IO/Socket lib/IO/Compress/Zlib
	-rmdir lib/IO/Compress/Zip lib/IO/Compress/Gzip lib/IO/Compress/Base
	-rmdir lib/IO/Compress/Adapter lib/IO/Compress lib/IO
	-rmdir lib/I18N/LangTags lib/I18N lib/Hash/Util lib/Hash lib/HTTP
	-rmdir lib/Filter/Util lib/Filter lib/File/Spec lib/ExtUtils/Typemaps
	-rmdir lib/ExtUtils/ParseXS lib/ExtUtils/MakeMaker/version
	-rmdir lib/ExtUtils/MakeMaker lib/ExtUtils/Liblist
	-rmdir lib/ExtUtils/Constant lib/ExtUtils/Command
	-rmdir lib/ExtUtils/CBuilder/Platform/Windows
	-rmdir lib/ExtUtils/CBuilder/Platform lib/ExtUtils/CBuilder
	-rmdir lib/Exporter lib/Encode/Unicode lib/Encode/MIME/Header
	-rmdir lib/Encode/MIME lib/Encode/KR lib/Encode/JP lib/Encode/CN
	-rmdir lib/Encode lib/Digest lib/Devel lib/Data lib/Config/Perl
	-rmdir lib/Compress/Raw lib/Compress lib/Carp lib/CPAN/Plugin
	-rmdir lib/CPAN/Meta/History lib/CPAN/Meta lib/CPAN/LWP
	-rmdir lib/CPAN/Kwalify lib/CPAN/HTTP lib/CPAN/FTP lib/CPAN/Exception
	-rmdir lib/CPAN/API lib/CPAN lib/Attribute lib/Archive/Tar lib/Archive
	-rmdir lib/App/Prove/State/Result lib/App/Prove/State lib/App/Prove
	-rmdir lib/App lib/Amiga

_realcleaner:
	@$(LDLIBPTH) $(MAKE) _cleaner1 CLEAN=distclean
	@$(LDLIBPTH) $(MAKE) _cleaner2

_verycleaner:
	@$(LDLIBPTH) $(MAKE) _cleaner1 CLEAN=veryclean
	@$(LDLIBPTH) $(MAKE) _cleaner2
	-rm -f *~ *.orig */*~ */*.orig */*/*~ */*/*.orig

.PHONY: lint
lint: $(c)
	rm -f *.ln
	lint $(lintflags) -DPERL_CORE -D_REENTRANT -DDEBUGGING -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 $(c)

cscopeflags = -Rb  # Recursive, build-only.

.PHONY:	cscope
# To query the cscope.out "cscope -dLnsymbol" where n = 0 means uses,
# 1 = definitions, 2 = callees, 3 = callers, for example
# "cscope -dL1Perl_mg_set" or run cscope interactively (no arguments).
cscope.out cscope: $(c) $(h)
	cscope $(cscopeflags)

# Need to unset during recursion to go out of loop.
# The README below ensures that the dependency list is never empty and
# that when MAKEDEPEND is empty $(FIRSTMAKEFILE) doesn't need rebuilding.

MAKEDEPEND = Makefile makedepend_file makedepend

$(FIRSTMAKEFILE):	README $(MAKEDEPEND)
	$(MAKE) depend MAKEDEPEND=
!NO!SUBS!

for f in $SH; do
    file=`SH_to_target $f`
    $spitshell >>$Makefile <<!GROK!THIS!

$file: $f config.sh
	\$(SHELL) $f
!GROK!THIS!
done

$spitshell >>$Makefile <<'!NO!SUBS!'

.PHONY: depend
depend: makedepend $(DTRACE_H) $(generated_headers)
	sh ./makedepend MAKE="$(MAKE)" cflags

.c.c.depends:
	sh ./makedepend_file $< $@ cflags

.PHONY: test check test_prep test_prep_nodll test_prep_pre \
	test_prep_reonly test_tty test-tty test_notty test-notty \
	test_harness test_harness_notty minitest test-reonly _test

# The _test target is there just for Test::Smoke, which does a make test_prep
# always before invoking this target, thus preventing checking again
_test:
	$(RUN_TESTS) choose

# Cannot delegate rebuilding of t/perl to make
# to allow interlaced test and minitest

# Architecture-neutral stuff:

test_prep_pre: preplibrary utilities $(nonxs_ext)
!NO!SUBS!

case "$targethost" in
'') $spitshell >>$Makefile <<'!NO!SUBS!'
test_prep test-prep: test_prep_pre $(MINIPERL_EXE) $(unidatafiles) $(PERL_EXE) \
	$(dynamic_ext) $(TEST_PERL_DLL) runtests $(generated_pods) common_build
	cd t && (rm -f $(PERL_EXE); $(LNS) ../$(PERL_EXE) $(PERL_EXE))

!NO!SUBS!
;;
*) $spitshell >>$Makefile <<!GROK!THIS!
test_prep test-prep: test_prep_pre \$(MINIPERL_EXE) \$(unidatafiles) \$(PERL_EXE) \
	\$(dynamic_ext) \$(TEST_PERL_DLL) runtests \
	\$(generated_pods) common_build
	$to libperl.*
	$to t/*
	$to lib/*
	$to TestInit.pm
	$to win32/FindExt.pm
	$to regen/*
	$to pod/*
	$to MANIFEST
	$to Porting/*
	$to cpan/*/t
	$to dist/*/t
	$to ext/*/t
	$to cpan/Archive-Tar/*
	$to cpan/Module-Metadata/*
	$to cpan/Term-Cap/test.pl
	$to cpan/Pod-Usage/*
	$to cpan/Pod-Parser/*
	$to cpan/IO-Compress/*
	$to cpan/HTTP-Tiny/lib/*
	$to cpan/Filter-Util-Call/filter-util.pl
	$to cpan/Digest-MD5/*
	$to cpan/Digest-SHA/*
	$to cpan/Test-Simple/*
	$to cpan/Pod-Parser/lib/*
	$to cpan/Test-Harness/*
	$to cpan/Scalar-List-Utils/*
	$to ext/IPC-Open3/*
	$to ext/Tie-Memoize/*
	$to ext/POSIX/*
	$to dist/IO/*
# --- These three are needed by ExtUtils::MM_Unix to verify PERL_CORE
	$to config_h.SH
	$to perl.h
	$to cflags
	$to *.h
# --- For t/porting/customized.t
	$to vutil.c
	$to vxs.inc
# --- For t/TEST
	$to config.sh
# --- For lib/diagnostics.t with -Duseshrplib
	$to \$(PERL_EXE)
	cd t && (rm -f \$(PERL_EXE); \$(LNS) ../\$(PERL_EXE) \$(PERL_EXE)) && cd ..
	$to t/\$(PERL_EXE)

!GROK!THIS!
;;
esac

$spitshell >>$Makefile <<'!NO!SUBS!'
test_prep_reonly: $(MINIPERL_EXE) $(PERL_EXE) $(dynamic_ext_re) $(TEST_PERL_DLL)
	$(MINIPERL) make_ext.pl $(dynamic_ext_re) MAKE="$(MAKE)" LIBPERL_A=$(LIBPERL) LINKTYPE=dynamic
	cd t && (rm -f $(PERL_EXE); $(LNS) ../$(PERL_EXE) $(PERL_EXE))
!NO!SUBS!

case "$targethost" in
'') $spitshell >>$Makefile <<'!NO!SUBS!'

test check: test_prep
	$(RUN_TESTS) choose

!NO!SUBS!
;;
*) $spitshell >>$Makefile <<!GROK!THIS!

test check: test_prep
	$run -cwd $targetdir/t -env "$targetenv LD_LIBRARY_PATH=$targetdir:$targetdir/lib:$targetdir/lib/auto:\$\$LD_LIBRARY_PATH" ./TEST

!GROK!THIS!
;;
esac

$spitshell >>$Makefile <<'!NO!SUBS!'
test_tty test-tty: test_prep
	$(RUN_TESTS) tty

test_notty test-notty: test_prep
	$(RUN_TESTS) no-tty

!NO!SUBS!

case "${osname}${osvers}" in
linux*|darwin*)
	$spitshell >>$Makefile <<'!NO!SUBS!'
# Targets for valgrind testing:

test.valgrind check.valgrind:	test_prep
	@echo "Checking usemymalloc='n' in config.sh..."
	@grep "^usemymalloc="    config.sh
	@grep "^usemymalloc='n'" config.sh >/dev/null || exit 1
	@echo "And of course you have to have valgrind..."
	$(VALGRIND) $(VG_TEST) || exit 1
	PERL_VALGRIND=1 VALGRIND='$(VALGRIND)' TESTFILE=harness $(RUN_TESTS) choose
!NO!SUBS!
	;;
esac

$spitshell >>$Makefile <<'!NO!SUBS!'

# minitest can't depend on lib/Config.pm because that might be where miniperl
# is crashing.
minitest_prep: $(MINIPERL_EXE)
	-@test -f lib/Config.pm || $(MAKE) lib/Config.pm $(unidatafiles)
	@echo " "
	@echo "You may see some irrelevant test failures if you have been unable"
	@echo "to build lib/Config.pm, or the Unicode data files."
	@echo " "
	cd t && (rm -f $(PERL_EXE); $(LNS) ../$(MINIPERL_EXE) $(PERL_EXE))

MINITEST_TESTS = base/*.t comp/*.t cmd/*.t run/*.t io/*.t re/*.t opbasic/*.t op/*.t uni/*.t perf/*.t

minitest: minitest_prep
	cd t && $(RUN_PERL) TEST $(MINITEST_TESTS) <$(devtty)

minitest-notty minitest_notty: minitest_prep
	cd t && PERL_SKIP_TTY_TEST=1 $(RUN_PERL) TEST $(MINITEST_TESTS)

# Test via harness

test_harness test-harness: test_prep
	TESTFILE=harness $(RUN_TESTS) choose

test_harness_notty: test_prep
	HARNESS_NOTTY=1 TESTFILE=harness $(RUN_TESTS) choose

test_reonly test-reonly: test_prep_reonly
	TEST_ARGS='re/*.t ../ext/re/t/*.t' PERL_TEST_HARNESS_ASAP=1 TESTFILE=harness $(RUN_TESTS) choose


# Porting tests (well-formedness of pod, manifest, etc)

test_porting test-porting: test_prep
	TEST_ARGS='porting/*.t lib/diagnostics.t' TESTFILE=harness $(RUN_TESTS) choose

!NO!SUBS!

$spitshell>>$Makefile <<!GROK!THIS!

# intended as a common way to add dependencies to test-prep and all
common_build: $common_build_deps

!GROK!THIS!

$spitshell >>$Makefile <<'!NO!SUBS!'

# Handy way to run perlbug -ok without having to install and run the
# installed perlbug. We don't re-run the tests here - we trust the user.
# Please *don't* use this unless all tests pass.
# If you want to report test failures, use "make nok" instead.

.PHONY: ok okfile oknack okfilenack nok nokfile noknack nokfilenack

ok:	utilities
	$(RUN_PERL) utils/perlbug -ok -s '(UNINSTALLED)'

okfile:	utilities
	$(RUN_PERL) utils/perlbug -ok -s '(UNINSTALLED)' -F perl.ok

oknack:	utilities
	$(RUN_PERL) utils/perlbug -ok -s '(UNINSTALLED)' -A

okfilenack:	utilities
	$(RUN_PERL) utils/perlbug -ok -s '(UNINSTALLED)' -F perl.ok -A

nok:	utilities
	$(RUN_PERL) utils/perlbug -nok -s '(UNINSTALLED)'

nokfile:	utilities
	$(RUN_PERL) utils/perlbug -nok -s '(UNINSTALLED)' -F perl.nok

noknack:	utilities
	$(RUN_PERL) utils/perlbug -nok -s '(UNINSTALLED)' -A

nokfilenack:	utilities
	$(RUN_PERL) utils/perlbug -nok -s '(UNINSTALLED)' -F perl.nok -A

.PHONY: clist hlist

clist:	$(c)
	echo $(c) | tr ' ' $(TRNL) >.clist

hlist:  $(h)
	echo $(h) | tr ' ' $(TRNL) >.hlist

.PHONY: distcheck
distcheck: FORCE
	perl '-MExtUtils::Manifest=&fullcheck' -e 'fullcheck()'

.PHONY: ctags

TAGS: $(c_base) $(h)
	etags $(c_base) $(h)
!NO!SUBS!

$spitshell >>$Makefile <<!GROK!THIS!

ctags:
	ctags -f Tags -N --totals --languages=c --langmap=c:+.h $ctags_exclude *.c *.h

# AUTOMATICALLY GENERATED MAKE DEPENDENCIES--PUT NOTHING BELOW THIS LINE
# If this runs make out of memory, delete /usr/include lines.
!GROK!THIS!

$eunicefix Makefile
$rm -f $firstmakefile

# Now do any special processing required before building.

case "$ebcdic" in
define)
    xxx=''
    echo "This is an EBCDIC system, checking if any parser files need regenerating." >&2
case "$osname" in
os390|posix-bc)
    ;;
*)
    echo "'$osname' is an EBCDIC system I don't know that well." >&4
    ;;
esac
    case "$xxx" in
    '') echo "No parser files were regenerated.  That's okay." >&2 ;;
    esac
    ;;
esac

# ex: set ts=8 sts=4 sw=4 noet:
