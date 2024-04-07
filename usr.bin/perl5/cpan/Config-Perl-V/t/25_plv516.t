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
is ($conf->{build}{stamp}, "Mar 12 2013 08:36:17", "Build time");
is ($conf->{config}{version}, "5.16.3", "reconstructed \%Config{version}");
is ($conf->{config}{ccversion}, "", "Using gcc. non-gcc version should not be defined");

my $opt = Config::Perl::V::plv2hash ("")->{build}{options};
foreach my $o (sort qw(
	HAS_TIMES PERLIO_LAYERS PERL_DONT_CREATE_GVSV
	PERL_MALLOC_WRAP PERL_PRESERVE_IVUV USE_64_BIT_INT
	USE_LARGE_FILES USE_LOCALE USE_LOCALE_COLLATE
	USE_LOCALE_CTYPE USE_LOCALE_NUMERIC USE_PERLIO
	USE_PERL_ATOF
	)) {
    is ($conf->{build}{options}{$o}, 1, "Runtime option $o set");
    delete $opt->{$o};
    }
foreach my $o (sort keys %$opt) {
    is ($conf->{build}{options}{$o}, 0, "Runtime option $o unset");
    }

eval { require Digest::MD5; };
my $md5 = $@ ? "0" x 32 : "7b00cf3b306d96fa802892e6ad4b070f";
ok (my $sig = Config::Perl::V::signature ($conf), "Get signature");

SKIP: {
    ord "A" == 65 or skip "ASCII-centric test", 1;
    is ($sig, $md5, "MD5");
    }

__END__
Summary of my perl5 (revision 5 version 16 subversion 3) configuration:
   
  Platform:
    osname=linux, osvers=3.4.33-2.24-desktop, archname=i686-linux-64int
    uname='linux lx09 3.4.33-2.24-desktop #1 smp preempt tue feb 26 03:34:33 utc 2013 (5f00a32) i686 i686 i386 gnulinux '
    config_args='-Duse64bitint -Duselongdouble -des'
    hint=previous, useposix=true, d_sigaction=define
    useithreads=undef, usemultiplicity=undef
    useperlio=define, d_sfio=undef, uselargefiles=define, usesocks=undef
    use64bitint=define, use64bitall=undef, uselongdouble=undef
    usemymalloc=n, bincompat5005=undef
  Compiler:
    cc='cc', ccflags ='-fno-strict-aliasing -pipe -fstack-protector -I/pro/local/include -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64',
    optimize='-O2',
    cppflags='-fno-strict-aliasing -pipe -fstack-protector -I/pro/local/include -fno-strict-aliasing -pipe -fstack-protector -I/pro/local/include -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64'
    ccversion='', gccversion='4.7.1 20120723 [gcc-4_7-branch revision 189773]', gccosandvers=''
    intsize=4, longsize=4, ptrsize=4, doublesize=8, byteorder=12345678
    d_longlong=define, longlongsize=8, d_longdbl=define, longdblsize=12
    ivtype='long long', ivsize=8, nvtype='double', nvsize=8, Off_t='off_t', lseeksize=8
    alignbytes=4, prototype=define
  Linker and Libraries:
    ld='cc', ldflags ='-L/pro/local/lib -fstack-protector'
    libpth=/pro/local/lib /lib /usr/lib /usr/local/lib
    libs=-lnsl -lgdbm -ldb -ldl -lm -lcrypt -lutil -lc -lgdbm_compat
    perllibs=-lnsl -ldl -lm -lcrypt -lutil -lc
    libc=/lib/libc-2.15.so, so=so, useshrplib=false, libperl=libperl.a
    gnulibc_version='2.15'
  Dynamic Linking:
    dlsrc=dl_dlopen.xs, dlext=so, d_dlsymun=undef, ccdlflags='-Wl,-E'
    cccdlflags='-fPIC', lddlflags='-shared -O2 -L/pro/local/lib -fstack-protector'


Characteristics of this binary (from libperl): 
  Compile-time options: HAS_TIMES PERLIO_LAYERS PERL_DONT_CREATE_GVSV
                        PERL_MALLOC_WRAP PERL_PRESERVE_IVUV USE_64_BIT_INT
                        USE_LARGE_FILES USE_LOCALE USE_LOCALE_COLLATE
                        USE_LOCALE_CTYPE USE_LOCALE_NUMERIC USE_PERLIO
                        USE_PERL_ATOF
  Built under linux
  Compiled at Mar 12 2013 08:36:17
  @INC:
    /pro/lib/perl5/site_perl/5.16.3/i686-linux-64int
    /pro/lib/perl5/site_perl/5.16.3
    /pro/lib/perl5/5.16.3/i686-linux-64int
    /pro/lib/perl5/5.16.3
    .
