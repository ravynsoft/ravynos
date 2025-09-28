#!/bin/sh

case $PERL_CONFIG_SH in
'')
	if test -f config.sh; then TOP=.;
	elif test -f ../config.sh; then TOP=..;
	elif test -f ../../config.sh; then TOP=../..;
	elif test -f ../../../config.sh; then TOP=../../..;
	elif test -f ../../../../config.sh; then TOP=../../../..;
	else
	    echo "Can't find the perl config.sh file produced by Configure"; 
	    exit 1
	fi
	. $TOP/config.sh
	;;
esac
: This forces SH files to create target in same directory as SH file.
: This is so that make depend always knows where to find SH derivatives.
case "$0" in
*/*) cd `expr X$0 : 'X\(.*\)/'` ;;
esac
echo "Extracting myconfig (with variable substitutions)"
$spitshell >myconfig <<!GROK!THIS!
$startsh

# This script is designed to provide a handy summary of the configuration
# information being used to build perl. This is especially useful if you
# are requesting help online or via email.

# Note that the text lines /^Summary of/ .. /^\s*$/ are copied into Config.pm.
cat <<'!NO!SUBS!'
Summary of my $package (revision $revision $version_patchlevel_string) configuration:
  $git_commit_id_title $git_commit_id$git_ancestor_line
  Platform:
    osname=$osname
    osvers=$osvers
    archname=$archname
    uname='$myuname'
    config_args='$config_args'
    hint=$hint
    useposix=$useposix
    d_sigaction=$d_sigaction
    useithreads=$useithreads
    usemultiplicity=$usemultiplicity
    use64bitint=$use64bitint
    use64bitall=$use64bitall
    uselongdouble=$uselongdouble
    usemymalloc=$usemymalloc
    default_inc_excludes_dot=$default_inc_excludes_dot
  Compiler:
    cc='$cc'
    ccflags ='$ccflags'
    optimize='$optimize'
    cppflags='$cppflags'
    ccversion='$ccversion'
    gccversion='$gccversion'
    gccosandvers='$gccosandvers'
    intsize=$intsize
    longsize=$longsize
    ptrsize=$ptrsize
    doublesize=$doublesize
    byteorder=$byteorder
    doublekind=$doublekind
    d_longlong=$d_longlong
    longlongsize=$longlongsize
    d_longdbl=$d_longdbl
    longdblsize=$longdblsize
    longdblkind=$longdblkind
    ivtype='$ivtype'
    ivsize=$ivsize
    nvtype='$nvtype'
    nvsize=$nvsize
    Off_t='$lseektype'
    lseeksize=$lseeksize
    alignbytes=$alignbytes
    prototype=$prototype
  Linker and Libraries:
    ld='$ld'
    ldflags ='$ldflags'
    libpth=$libpth
    libs=$libs
    perllibs=$perllibs
    libc=$libc
    so=$so
    useshrplib=$useshrplib
    libperl=$libperl
    gnulibc_version='$gnulibc_version'
  Dynamic Linking:
    dlsrc=$dlsrc
    dlext=$dlext
    d_dlsymun=$d_dlsymun
    ccdlflags='$ccdlflags'
    cccdlflags='$cccdlflags'
    lddlflags='$lddlflags'

!NO!SUBS!
!GROK!THIS!
chmod 755 myconfig
$eunicefix myconfig
