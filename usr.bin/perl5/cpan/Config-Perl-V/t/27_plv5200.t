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
is ($conf->{build}{stamp}, "Jun 30 2014 15:37:09", "Build time");
is ($conf->{config}{version}, "5.20.0", "reconstructed \$Config{version}");

my $opt = Config::Perl::V::plv2hash ("")->{build}{options};
foreach my $o (sort qw(
	HAS_TIMES MULTIPLICITY PERLIO_LAYERS
	PERL_DONT_CREATE_GVSV
	PERL_HASH_FUNC_ONE_AT_A_TIME_HARD
	PERL_IMPLICIT_CONTEXT PERL_MALLOC_WRAP
	PERL_NEW_COPY_ON_WRITE PERL_PRESERVE_IVUV
	PERL_USE_DEVEL USE_64_BIT_INT USE_ITHREADS
	USE_LARGE_FILES USE_LOCALE USE_LOCALE_COLLATE
	USE_LOCALE_CTYPE USE_LOCALE_NUMERIC USE_LONG_DOUBLE
	USE_PERLIO USE_PERL_ATOF USE_REENTRANT_API
	)) {
    is ($conf->{build}{options}{$o}, 1, "Runtime option $o set");
    delete $opt->{$o};
    }
foreach my $o (sort keys %$opt) {
    is ($conf->{build}{options}{$o}, 0, "Runtime option $o unset");
    }

eval { require Digest::MD5; };
my $md5 = $@ ? "0" x 32 : "3e7b4513cd80c6ef00fcd77e5e16f8b4";
ok (my $sig = Config::Perl::V::signature ($conf), "Get signature");

SKIP: {
    ord "A" == 65 or skip "ASCII-centric test", 1;
    is ($sig, $md5, "MD5");
    }

is_deeply ($conf->{build}{patches}, [], "No local patches");

my %check = (
    alignbytes      => 4,
    api_version     => 20,
    bincompat5005   => "undef",
    byteorder       => 12345678,
    cc              => "cc",
    cccdlflags      => "-fPIC",
    ccdlflags       => "-Wl,-E",
    config_args     => "-Dusedevel -Uversiononly -Dinc_version_list=none -Duse64bitint -Dusethreads -Duseithreads -Duselongdouble -des",
    gccversion      => "4.8.1 20130909 [gcc-4_8-branch revision 202388]",
    gnulibc_version => "2.18",
    ivsize          => 8,
    ivtype          => "long long",
    ld              => "cc",
    lddlflags       => "-shared -O2 -L/pro/local/lib -fstack-protector",
    ldflags         => "-L/pro/local/lib -fstack-protector",
    libc            => "libc-2.18.so",
    lseektype       => "off_t",
    osvers          => "3.11.10-17-desktop",
    use64bitint     => "define",
    );
is ($conf->{config}{$_}, $check{$_}, "reconstructed \$Config{$_}") for sort keys %check;

__END__
Summary of my perl5 (revision 5 version 20 subversion 0) configuration:
   
  Platform:
    osname=linux, osvers=3.11.10-17-desktop, archname=i686-linux-thread-multi-64int-ld
    uname='linux lx09 3.11.10-17-desktop #1 smp preempt mon jun 16 15:28:13 utc 2014 (fba7c1f) i686 i686 i386 gnulinux '
    config_args='-Dusedevel -Uversiononly -Dinc_version_list=none -Duse64bitint -Dusethreads -Duseithreads -Duselongdouble -des'
    hint=recommended, useposix=true, d_sigaction=define
    useithreads=define, usemultiplicity=define
    use64bitint=define, use64bitall=undef, uselongdouble=define
    usemymalloc=n, bincompat5005=undef
  Compiler:
    cc='cc', ccflags ='-D_REENTRANT -D_GNU_SOURCE -fwrapv -fno-strict-aliasing -pipe -fstack-protector -I/pro/local/include -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64',
    optimize='-O2',
    cppflags='-D_REENTRANT -D_GNU_SOURCE -fwrapv -fno-strict-aliasing -pipe -fstack-protector -I/pro/local/include'
    ccversion='', gccversion='4.8.1 20130909 [gcc-4_8-branch revision 202388]', gccosandvers=''
    intsize=4, longsize=4, ptrsize=4, doublesize=8, byteorder=12345678
    d_longlong=define, longlongsize=8, d_longdbl=define, longdblsize=12
    ivtype='long long', ivsize=8, nvtype='long double', nvsize=12, Off_t='off_t', lseeksize=8
    alignbytes=4, prototype=define
  Linker and Libraries:
    ld='cc', ldflags ='-L/pro/local/lib -fstack-protector'
    libpth=/usr/local/lib /usr/lib/gcc/i586-suse-linux/4.8/include-fixed /usr/lib/gcc/i586-suse-linux/4.8/../../../../i586-suse-linux/lib /usr/lib /pro/local/lib /lib
    libs=-lnsl -lgdbm -ldb -ldl -lm -lcrypt -lutil -lpthread -lc -lgdbm_compat
    perllibs=-lnsl -ldl -lm -lcrypt -lutil -lpthread -lc
    libc=libc-2.18.so, so=so, useshrplib=false, libperl=libperl.a
    gnulibc_version='2.18'
  Dynamic Linking:
    dlsrc=dl_dlopen.xs, dlext=so, d_dlsymun=undef, ccdlflags='-Wl,-E'
    cccdlflags='-fPIC', lddlflags='-shared -O2 -L/pro/local/lib -fstack-protector'


Characteristics of this binary (from libperl): 
  Compile-time options: HAS_TIMES MULTIPLICITY PERLIO_LAYERS
                        PERL_DONT_CREATE_GVSV
                        PERL_HASH_FUNC_ONE_AT_A_TIME_HARD
                        PERL_IMPLICIT_CONTEXT PERL_MALLOC_WRAP
                        PERL_NEW_COPY_ON_WRITE PERL_PRESERVE_IVUV
                        PERL_USE_DEVEL USE_64_BIT_INT USE_ITHREADS
                        USE_LARGE_FILES USE_LOCALE USE_LOCALE_COLLATE
                        USE_LOCALE_CTYPE USE_LOCALE_NUMERIC USE_LONG_DOUBLE
                        USE_PERLIO USE_PERL_ATOF USE_REENTRANT_API
  Built under linux
  Compiled at Jun 30 2014 15:37:09
  @INC:
    /pro/lib/perl5/site_perl/5.20.0/i686-linux-thread-multi-64int-ld
    /pro/lib/perl5/site_perl/5.20.0
    /pro/lib/perl5/5.20.0/i686-linux-thread-multi-64int-ld
    /pro/lib/perl5/5.20.0
    .
