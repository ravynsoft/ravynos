#!/pro/bin/perl

use strict;
use warnings;

BEGIN {
    use Test::More;
    my $tests = 125;
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
is ($conf->{build}{stamp}, "May 18 2013 17:34:20", "Build time");
is ($conf->{config}{version}, "5.18.0", "reconstructed \$Config{version}");

my $opt = Config::Perl::V::plv2hash ("")->{build}{options};
foreach my $o (sort qw(
	HAS_TIMES PERLIO_LAYERS PERL_DONT_CREATE_GVSV
	PERL_HASH_FUNC_ONE_AT_A_TIME_HARD PERL_MALLOC_WRAP
	PERL_PRESERVE_IVUV PERL_SAWAMPERSAND USE_64_BIT_INT
	USE_LARGE_FILES USE_LOCALE USE_LOCALE_COLLATE
	USE_LOCALE_CTYPE USE_LOCALE_NUMERIC USE_LONG_DOUBLE
	USE_PERLIO USE_PERL_ATOF
	)) {
    is ($conf->{build}{options}{$o}, 1, "Runtime option $o set");
    delete $opt->{$o};
    }
foreach my $o (sort keys %$opt) {
    is ($conf->{build}{options}{$o}, 0, "Runtime option $o unset");
    }

eval { require Digest::MD5; };
my $md5 = $@ ? "0" x 32 : "8f46b07a7775e6a92347d4cd564b8f03";
ok (my $sig = Config::Perl::V::signature ($conf), "Get signature");

SKIP: {
    ord "A" == 65 or skip "ASCII-centric test", 1;
    is ($sig, $md5, "MD5");
    }

is_deeply ($conf->{build}{patches}, [], "No local patches");

my %check = (
    alignbytes      => 4,
    api_version     => 18,
    bincompat5005   => "undef",
    byteorder       => 12345678,
    cc              => "cc",
    cccdlflags      => "-fPIC",
    ccdlflags       => "-Wl,-E",
    config_args     => "-Duse64bitint -Duselongdouble -des",
    gccversion      => "4.7.2 20130108 [gcc-4_7-branch revision 195012]",
    gnulibc_version => "2.17",
    ivsize          => 8,
    ivtype          => "long long",
    ld              => "cc",
    lddlflags       => "-shared -O3 -L/pro/local/lib -fstack-protector",
    ldflags         => "-L/pro/local/lib -fstack-protector",
    libc            => "/lib/libc-2.17.so",
    lseektype       => "off_t",
    osvers          => "3.7.10-1.4-desktop",
    use64bitint     => "define",
    );
is ($conf->{config}{$_}, $check{$_}, "reconstructed \$Config{$_}") for sort keys %check;

__END__
Summary of my perl5 (revision 5 version 18 subversion 0) configuration:
   
  Platform:
    osname=linux, osvers=3.7.10-1.4-desktop, archname=i686-linux-64int-ld
    uname='linux test 3.7.10-1.4-desktop #1 smp preempt fri apr 19 12:06:34 utc 2013 (8ef74f8) i686 i686 i386 gnulinux '
    config_args='-Duse64bitint -Duselongdouble -des'
    hint=recommended, useposix=true, d_sigaction=define
    useithreads=undef, usemultiplicity=undef
    useperlio=define, d_sfio=undef, uselargefiles=define, usesocks=undef
    use64bitint=define, use64bitall=undef, uselongdouble=define
    usemymalloc=n, bincompat5005=undef
  Compiler:
    cc='cc', ccflags ='-fno-strict-aliasing -pipe -fstack-protector -I/pro/local/include -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64',
    optimize='-O3',
    cppflags='-fno-strict-aliasing -pipe -fstack-protector -I/pro/local/include'
    ccversion='', gccversion='4.7.2 20130108 [gcc-4_7-branch revision 195012]', gccosandvers=''
    intsize=4, longsize=4, ptrsize=4, doublesize=8, byteorder=12345678
    d_longlong=define, longlongsize=8, d_longdbl=define, longdblsize=12
    ivtype='long long', ivsize=8, nvtype='long double', nvsize=12, Off_t='off_t', lseeksize=8
    alignbytes=4, prototype=define
  Linker and Libraries:
    ld='cc', ldflags ='-L/pro/local/lib -fstack-protector'
    libpth=/pro/local/lib /lib /usr/lib /usr/local/lib
    libs=-lnsl -lgdbm -ldb -ldl -lm -lcrypt -lutil -lc -lgdbm_compat
    perllibs=-lnsl -ldl -lm -lcrypt -lutil -lc
    libc=/lib/libc-2.17.so, so=so, useshrplib=false, libperl=libperl.a
    gnulibc_version='2.17'
  Dynamic Linking:
    dlsrc=dl_dlopen.xs, dlext=so, d_dlsymun=undef, ccdlflags='-Wl,-E'
    cccdlflags='-fPIC', lddlflags='-shared -O3 -L/pro/local/lib -fstack-protector'


Characteristics of this binary (from libperl): 
  Compile-time options: HAS_TIMES PERLIO_LAYERS PERL_DONT_CREATE_GVSV
                        PERL_HASH_FUNC_ONE_AT_A_TIME_HARD PERL_MALLOC_WRAP
                        PERL_PRESERVE_IVUV PERL_SAWAMPERSAND USE_64_BIT_INT
                        USE_LARGE_FILES USE_LOCALE USE_LOCALE_COLLATE
                        USE_LOCALE_CTYPE USE_LOCALE_NUMERIC USE_LONG_DOUBLE
                        USE_PERLIO USE_PERL_ATOF
  Built under linux
  Compiled at May 18 2013 17:34:20
  @INC:
    /pro/lib/perl5/site_perl/5.18.0/i686-linux-64int-ld
    /pro/lib/perl5/site_perl/5.18.0
    /pro/lib/perl5/5.18.0/i686-linux-64int-ld
    /pro/lib/perl5/5.18.0
    .
