#! /bin/sh
# riscos.sh - hints for building perl using the gccsdk cross compiler for RISC OS
#


cc='gcc'
locincpth=""
ccflags="-mpoke-function-name -DDYNAMIC_ENV_FETCH"
prefix='/<Perl$Dir>'
osname='riscos'
libpth=' '
optimize='-O2'
myarchname=''
archname='arm-riscos'
installprefix='~/PerlInst'
mkdir -p $installprefix
startperl="#!/usr/bin/perl"
i_shadow='undef'
ebcdic='undef'

