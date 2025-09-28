#!/pro/bin/perl

use strict;
use warnings;

BEGIN {
    use Test::More;
    my $tests = 126;
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
is ($conf->{build}{stamp}, "Apr 19 2015 12:35:54", "Build time");
is ($conf->{config}{version}, "5.20.2", "reconstructed \$Config{version}");

my $opt = Config::Perl::V::plv2hash ("")->{build}{options};
foreach my $o (sort qw(
	HAS_TIMES MULTIPLICITY PERLIO_LAYERS
	PERL_DONT_CREATE_GVSV
	PERL_HASH_FUNC_ONE_AT_A_TIME_HARD
	PERL_IMPLICIT_CONTEXT PERL_MALLOC_WRAP
	PERL_NEW_COPY_ON_WRITE PERL_PRESERVE_IVUV
	PERL_USE_DEVEL USE_64_BIT_ALL USE_64_BIT_INT USE_ITHREADS
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
my $md5 = $@ ? "0" x 32 : "9f954ebc2be7b1d7e151ab28dbdf7062";
ok (my $sig = Config::Perl::V::signature ($conf), "Get signature");

SKIP: {
    ord "A" == 65 or skip "ASCII-centric test", 1;
    is ($sig, $md5, "MD5");
    }

is_deeply ($conf->{build}{patches}, [], "No local patches");

my %check = (
    alignbytes      => 16,
    api_version     => 20,
    bincompat5005   => "undef",
    byteorder       => 12345678,
    cc              => "cc",
    cccdlflags      => "-fPIC",
    ccdlflags       => "-Wl,-E",
    config_args     => "-Dusedevel -Duse64bitall -Dusethreads -Duseithreads -Duselongdouble -des",
    gccversion      => "4.8.3 20140627 [gcc-4_8-branch revision 212064]",
    gnulibc_version => "2.19",
    ivsize          => 8,
    ivtype          => "long",
    ld              => "cc",
    lddlflags       => "-shared -O2 -L/pro/local/lib -fstack-protector",
    ldflags         => "-L/pro/local/lib -fstack-protector",
    libc            => "libc-2.19.so",
    lseektype       => "off_t",
    osvers          => "3.16.7-13-desktop",
    use64bitall     => "define",
    use64bitint     => "define",
    );
is ($conf->{config}{$_}, $check{$_}, "reconstructed \$Config{$_}") for sort keys %check;

__END__
Summary of my perl5 (revision 5 version 20 subversion 2) configuration:
   
  Platform:
    osname=linux, osvers=3.16.7-13-desktop, archname=x86_64-linux-thread-multi-ld
    uname='linux lx09 3.16.7-13-desktop #1 smp preempt wed mar 18 17:31:15 utc 2015 (ba2afab) x86_64 x86_64 x86_64 gnulinux '
    config_args='-Dusedevel -Duse64bitall -Dusethreads -Duseithreads -Duselongdouble -des'
    hint=recommended, useposix=true, d_sigaction=define
    useithreads=define, usemultiplicity=define
    use64bitint=define, use64bitall=define, uselongdouble=define
    usemymalloc=n, bincompat5005=undef
  Compiler:
    cc='cc', ccflags ='-D_REENTRANT -D_GNU_SOURCE -fPIC -fwrapv -fno-strict-aliasing -pipe -fstack-protector -I/pro/local/include -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64',
    optimize='-O2',
    cppflags='-D_REENTRANT -D_GNU_SOURCE -fwrapv -fno-strict-aliasing -pipe -fstack-protector -I/pro/local/include'
    ccversion='', gccversion='4.8.3 20140627 [gcc-4_8-branch revision 212064]', gccosandvers=''
    intsize=4, longsize=8, ptrsize=8, doublesize=8, byteorder=12345678
    d_longlong=define, longlongsize=8, d_longdbl=define, longdblsize=16
    ivtype='long', ivsize=8, nvtype='long double', nvsize=16, Off_t='off_t', lseeksize=8
    alignbytes=16, prototype=define
  Linker and Libraries:
    ld='cc', ldflags ='-L/pro/local/lib -fstack-protector'
    libpth=/usr/local/lib /usr/lib64/gcc/x86_64-suse-linux/4.8/include-fixed /usr/lib64/gcc/x86_64-suse-linux/4.8/../../../../x86_64-suse-linux/lib /usr/lib /pro/local/lib /lib/../lib64 /usr/lib/../lib64 /lib /lib64 /usr/lib64 /usr/local/lib64
    libs=-lnsl -lgdbm -ldb -ldl -lm -lcrypt -lutil -lpthread -lc -lgdbm_compat
    perllibs=-lnsl -ldl -lm -lcrypt -lutil -lpthread -lc
    libc=libc-2.19.so, so=so, useshrplib=false, libperl=libperl.a
    gnulibc_version='2.19'
  Dynamic Linking:
    dlsrc=dl_dlopen.xs, dlext=so, d_dlsymun=undef, ccdlflags='-Wl,-E'
    cccdlflags='-fPIC', lddlflags='-shared -O2 -L/pro/local/lib -fstack-protector'


Characteristics of this binary (from libperl): 
  Compile-time options: HAS_TIMES MULTIPLICITY PERLIO_LAYERS
                        PERL_DONT_CREATE_GVSV
                        PERL_HASH_FUNC_ONE_AT_A_TIME_HARD
                        PERL_IMPLICIT_CONTEXT PERL_MALLOC_WRAP
                        PERL_NEW_COPY_ON_WRITE PERL_PRESERVE_IVUV
                        PERL_USE_DEVEL USE_64_BIT_ALL USE_64_BIT_INT
                        USE_ITHREADS USE_LARGE_FILES USE_LOCALE
                        USE_LOCALE_COLLATE USE_LOCALE_CTYPE
                        USE_LOCALE_NUMERIC USE_LONG_DOUBLE USE_PERLIO
                        USE_PERL_ATOF USE_REENTRANT_API
  Built under linux
  Compiled at Apr 19 2015 12:35:54
  @INC:
    /pro/lib/perl5/site_perl/5.20.2/x86_64-linux-thread-multi-ld
    /pro/lib/perl5/site_perl/5.20.2
    /pro/lib/perl5/5.20.2/x86_64-linux-thread-multi-ld
    /pro/lib/perl5/5.20.2
    .
