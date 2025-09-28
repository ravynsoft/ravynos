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
is ($conf->{build}{stamp}, "Oct 21 2010 14:50:53", "Build time");
is ($conf->{config}{version}, "5.8.9", "reconstructed \%Config{version}");
is ($conf->{config}{usethreads}, "define", "This was a threaded perl");

my $opt = Config::Perl::V::plv2hash ("")->{build}{options};
foreach my $o (sort qw(
	DEBUGGING MULTIPLICITY PERL_IMPLICIT_CONTEXT
	PERL_MALLOC_WRAP THREADS_HAVE_PIDS USE_64_BIT_INT
	USE_FAST_STDIO USE_ITHREADS USE_LARGE_FILES
	USE_LONG_DOUBLE USE_PERLIO USE_REENTRANT_API
	)) {
    is ($conf->{build}{options}{$o}, 1, "Runtime option $o set");
    delete $opt->{$o};
    }
foreach my $o (sort keys %$opt) {
    is ($conf->{build}{options}{$o}, 0, "Runtime option $o unset");
    }

eval { require Digest::MD5; };
my $md5 = $@ ? "0" x 32 : "df48dce1adaaf63855d8acd455c51818";
ok (my $sig = Config::Perl::V::signature ($conf), "Get signature");

SKIP: {
    ord "A" == 65 or skip "ASCII-centric test", 1;
    is ($sig, $md5, "MD5");
    }

__END__
Summary of my perl5 (revision 5 version 8 subversion 9) configuration:
  Platform:
    osname=linux, osvers=2.6.34.7-0.4-desktop, archname=i686-linux-thread-multi-64int-ld
    uname='linux tux09.procura.nl 2.6.34.7-0.4-desktop #1 smp preempt 2010-10-07 19:07:51 +0200 i686 i686 i386 gnulinux '
    config_args='-Dusedevel -Dusethreads -Duseithreads -Duse64bitint -Duselongdouble -Duseperlio -des -Dusedevel -Uinstallusrbinperl -Dprefix=/media/Tux/perls-t'
    hint=recommended, useposix=true, d_sigaction=define
    usethreads=define use5005threads=undef useithreads=define usemultiplicity=define
    useperlio=define d_sfio=undef uselargefiles=define usesocks=undef
    use64bitint=define use64bitall=undef uselongdouble=define
    usemymalloc=n, bincompat5005=undef
  Compiler:
    cc='cc', ccflags ='-D_REENTRANT -D_GNU_SOURCE -DTHREADS_HAVE_PIDS -DDEBUGGING -fno-strict-aliasing -pipe -I/pro/local/include -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64',
    optimize='-O2',
    cppflags='-D_REENTRANT -D_GNU_SOURCE -DTHREADS_HAVE_PIDS -DDEBUGGING -fno-strict-aliasing -pipe -I/pro/local/include'
    ccversion='', gccversion='4.5.0 20100604 [gcc-4_5-branch revision 160292]', gccosandvers=''
    intsize=4, longsize=4, ptrsize=4, doublesize=8, byteorder=12345678
    d_longlong=define, longlongsize=8, d_longdbl=define, longdblsize=12
    ivtype='long long', ivsize=8, nvtype='long double', nvsize=12, Off_t='off_t', lseeksize=8
    alignbytes=4, prototype=define
  Linker and Libraries:
    ld='cc', ldflags ='-L/pro/local/lib'
    libpth=/pro/local/lib /lib /usr/lib /usr/local/lib
    libs=-lnsl -lgdbm -ldb -ldl -lm -lcrypt -lutil -lpthread -lc
    perllibs=-lnsl -ldl -lm -lcrypt -lutil -lpthread -lc
    libc=/lib/libc-2.11.2.so, so=so, useshrplib=false, libperl=libperl.a
    gnulibc_version='2.11.2'
  Dynamic Linking:
    dlsrc=dl_dlopen.xs, dlext=so, d_dlsymun=undef, ccdlflags='-Wl,-E'
    cccdlflags='-fPIC', lddlflags='-shared -O2 -L/pro/local/lib'


Characteristics of this binary (from libperl): 
  Compile-time options: DEBUGGING MULTIPLICITY PERL_IMPLICIT_CONTEXT
                        PERL_MALLOC_WRAP THREADS_HAVE_PIDS USE_64_BIT_INT
                        USE_FAST_STDIO USE_ITHREADS USE_LARGE_FILES
                        USE_LONG_DOUBLE USE_PERLIO USE_REENTRANT_API
  Built under linux
  Compiled at Oct 21 2010 14:50:53
  @INC:
    /media/Tux/perls-t/lib/5.8.9/i686-linux-thread-multi-64int-ld
    /media/Tux/perls-t/lib/5.8.9
    /media/Tux/perls-t/lib/site_perl/5.8.9/i686-linux-thread-multi-64int-ld
    /media/Tux/perls-t/lib/site_perl/5.8.9
    .
