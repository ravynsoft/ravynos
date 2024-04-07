#!/pro/bin/perl

use strict;
use warnings;

BEGIN {
    use Test::More;
    my $tests = 106;
    unless ($ENV{PERL_CORE}) {
	require Test::NoWarnings;
	Test::NoWarnings->import ();
	$tests++;
	}

    plan tests => $tests;
    }

use Config::Perl::V;

ok (my $conf = Config::Perl::V::plv2hash (<DATA>), "Read perl -v block");
ok (exists $conf->{$_}, "Has $_ entry") for qw( build environment config inc );

is ($conf->{build}{osname}, $conf->{config}{osname}, "osname");
is ($conf->{build}{stamp}, "Mar 23 2010 17:34:56", "Build time");
is ($conf->{config}{"package"}, "perl5", "reconstructed \%Config{package}");
is ($conf->{config}{version}, "5.6.2", "reconstructed \%Config{version}");

my $opt = Config::Perl::V::plv2hash ("")->{build}{options};
foreach my $o (sort qw(
	DEBUGGING USE_64_BIT_INT USE_LARGE_FILES
	)) {
    is ($conf->{build}{options}{$o}, 1, "Runtime option $o set");
    delete $opt->{$o};
    }
foreach my $o (sort keys %$opt) {
    is ($conf->{build}{options}{$o}, 0, "Runtime option $o unset");
    }

eval { require Digest::MD5; };
my $md5 = $@ ? "0" x 32 : "9dc187182be100c1713f210a8c6d9f45";
ok (my $sig = Config::Perl::V::signature ($conf), "Get signature");

SKIP: {
    ord "A" == 65 or skip "ASCII-centric test", 1;
    is ($sig, $md5, "MD5");
    }

__END__
Summary of my perl5 (revision 5.0 version 6 subversion 2) configuration:
  Platform:
    osname=linux, osvers=2.6.31.12-0.2-default, archname=i686-linux-64int-perlio
    uname='linux nb09 2.6.31.12-0.2-default #1 smp 2010-03-16 21:25:39 +0100 i686 i686 i386 gnulinux '
    config_args='-Dusedevel -Duse64bitint -Duseperlio -des -Dusedevel -Uinstallusrbinperl -Dprefix=/media/Tux/perls'
    hint=recommended, useposix=true, d_sigaction=define
    usethreads=undef use5005threads=undef useithreads=undef usemultiplicity=undef
    useperlio=define d_sfio=undef uselargefiles=define usesocks=undef
    use64bitint=define use64bitall=undef uselongdouble=undef
  Compiler:
    cc='cc', ccflags ='-DDEBUGGING -fno-strict-aliasing -I/pro/local/include -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64',
    optimize='-O2',
    cppflags='-DDEBUGGING -fno-strict-aliasing -I/pro/local/include'
    ccversion='', gccversion='4.4.1 [gcc-4_4-branch revision 150839]', gccosandvers=''
    intsize=4, longsize=4, ptrsize=4, doublesize=8, byteorder=12345678
    d_longlong=define, longlongsize=8, d_longdbl=define, longdblsize=12
    ivtype='long long', ivsize=8, nvtype='double', nvsize=8, Off_t='off_t', lseeksize=8
    alignbytes=4, usemymalloc=n, prototype=define
  Linker and Libraries:
    ld='cc', ldflags ='-L/pro/local/lib'
    libpth=/pro/local/lib /lib /usr/lib /usr/local/lib
    libs=-lnsl -lgdbm -ldb -ldl -lm -lc -lcrypt -lutil
    perllibs=-lnsl -ldl -lm -lc -lcrypt -lutil
    libc=/lib/libc-2.10.1.so, so=so, useshrplib=false, libperl=libperl.a
  Dynamic Linking:
    dlsrc=dl_dlopen.xs, dlext=so, d_dlsymun=undef, ccdlflags='-rdynamic'
    cccdlflags='-fpic', lddlflags='-shared -L/pro/local/lib'


Characteristics of this binary (from libperl): 
  Compile-time options: DEBUGGING USE_64_BIT_INT USE_LARGE_FILES
  Built under linux
  Compiled at Mar 23 2010 17:34:56
  @INC:
    /media/Tux/perls/lib/5.6.2/i686-linux-64int-perlio
    /media/Tux/perls/lib/5.6.2
    /media/Tux/perls/lib/site_perl/5.6.2/i686-linux-64int-perlio
    /media/Tux/perls/lib/site_perl/5.6.2
    /media/Tux/perls/lib/site_perl
    .
