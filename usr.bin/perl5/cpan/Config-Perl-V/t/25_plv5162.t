#!/pro/bin/perl

use strict;
use warnings;

BEGIN {
    use Test::More;
    my $tests = 164;
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
is ($conf->{build}{stamp}, "Aug 25 2013 01:24:40", "Build time");
is ($conf->{config}{version}, "5.16.2", "reconstructed \%Config{version}");
is ($conf->{config}{ccversion}, "", "Using gcc. non-gcc version should not be defined");

my $opt = Config::Perl::V::plv2hash ("")->{build}{options};
foreach my $o (sort qw(
	HAS_TIMES MULTIPLICITY PERLIO_LAYERS
	PERL_DONT_CREATE_GVSV PERL_IMPLICIT_CONTEXT
	PERL_MALLOC_WRAP PERL_PRESERVE_IVUV USE_64_BIT_ALL
	USE_64_BIT_INT USE_ITHREADS USE_LARGE_FILES
	USE_LOCALE USE_LOCALE_COLLATE USE_LOCALE_CTYPE
	USE_LOCALE_NUMERIC USE_PERLIO USE_PERL_ATOF
	USE_REENTRANT_API
	)) {
    is ($conf->{build}{options}{$o}, 1, "Runtime option $o set");
    delete $opt->{$o};
    }
foreach my $o (sort keys %$opt) {
    is ($conf->{build}{options}{$o}, 0, "Runtime option $o unset");
    }

eval { require Digest::MD5; };
my $md5 = $@ ? "0" x 32 : "2917ca2a97b6db1ab8fb08798f53c0bb";
ok (my $sig = Config::Perl::V::signature ($conf), "Get signature");

SKIP: {
    ord "A" == 65 or skip "ASCII-centric test", 1;
    is ($sig, $md5, "MD5");
    }

is_deeply ($conf->{build}{patches}, [
    "/Library/Perl/Updates/<version> comes before system perl directories",
    "installprivlib and installarchlib points to the Updates directory",
    "CVE-2013-1667 hashtable DOS fix",
    ], "Local patches");

my %check = (

    archname        => "darwin-thread-multi-2level",
    bincompat5005   => "undef",
    config_args     => "-ds -e -Dprefix=/usr -Dccflags=-g  -pipe  -Dldflags= -Dman3ext=3pm -Duseithreads -Duseshrplib -Dinc_version_list=none -Dcc=cc",
    d_sfio          => "undef",
    d_sigaction     => "define",
    hint            => "recommended",
    myuname         => "darwin jackson.apple.com 13.0 darwin kernel version 13.0.0: tue jul 30 20:52:22 pdt 2013; root:xnu-2422.1.53~3release_x86_64 x86_64",
    use64bitall     => "define",
    use64bitint     => "define",
    useithreads     => "define",
    uselargefiles   => "define",
    uselongdouble   => "undef",
    usemultiplicity => "define",
    usemymalloc     => "n",
    useperlio       => "define",
    useposix        => "true",
    usesocks        => "undef",

    alignbytes      => 8,
    byteorder       => "12345678",
    cc              => "cc",
    ccflags         => "-arch x86_64 -arch i386 -g -pipe -fno-common -DPERL_DARWIN -fno-strict-aliasing -fstack-protector -I/usr/local/include",
    ccversion       => "",
    cppflags        => "-g -pipe -fno-common -DPERL_DARWIN -fno-strict-aliasing -fstack-protector -I/usr/local/include",
    d_longdbl       => "define",
    d_longlong      => "define",
    doublesize      => 8,
    gccosandvers    => "",
    gccversion      => "4.2.1 Compatible Apple LLVM 5.0 (clang-500.0.68)",
    intsize         => 4,
    ivsize          => 8,
    ivtype          => "long",
    longdblsize     => 16,
    longlongsize    => 8,
    longsize        => 8,
    lseeksize       => 8,
    nvsize          => 8,
    nvtype          => "double",
    lseektype       => "off_t",
    optimize        => "-Os",
    prototype       => "define",
    ptrsize         => 8,

    gnulibc_version => "",
    ld              => "cc -mmacosx-version-min=10.9",
    ldflags         => "-arch x86_64 -arch i386 -fstack-protector -L/usr/local/lib",
    libc            => "",
    libperl         => "libperl.dylib",
    libpth          => "/usr/local/lib /usr/lib",
    libs            => "",
    perllibs        => "",
    so              => "dylib",
    useshrplib      => "true",

    cccdlflags      => "",
    ccdlflags       => "",
    d_dlsymun       => "undef",
    dlext           => "bundle",
    dlsrc           => "dl_dlopen.xs",
    lddlflags       => "-arch x86_64 -arch i386 -bundle -undefined dynamic_lookup -L/usr/local/lib -fstack-protector",
    );
is ($conf->{config}{$_}, $check{$_}, "reconstructed \$Config{$_}") for sort keys %check;


__END__
Summary of my perl5 (revision 5 version 16 subversion 2) configuration:
   
  Platform:
    osname=darwin, osvers=13.0, archname=darwin-thread-multi-2level
    uname='darwin jackson.apple.com 13.0 darwin kernel version 13.0.0: tue jul 30 20:52:22 pdt 2013; root:xnu-2422.1.53~3release_x86_64 x86_64 '
    config_args='-ds -e -Dprefix=/usr -Dccflags=-g  -pipe  -Dldflags= -Dman3ext=3pm -Duseithreads -Duseshrplib -Dinc_version_list=none -Dcc=cc'
    hint=recommended, useposix=true, d_sigaction=define
    useithreads=define, usemultiplicity=define
    useperlio=define, d_sfio=undef, uselargefiles=define, usesocks=undef
    use64bitint=define, use64bitall=define, uselongdouble=undef
    usemymalloc=n, bincompat5005=undef
  Compiler:
    cc='cc', ccflags ='-arch x86_64 -arch i386 -g -pipe -fno-common -DPERL_DARWIN -fno-strict-aliasing -fstack-protector -I/usr/local/include',
    optimize='-Os',
    cppflags='-g -pipe -fno-common -DPERL_DARWIN -fno-strict-aliasing -fstack-protector -I/usr/local/include'
    ccversion='', gccversion='4.2.1 Compatible Apple LLVM 5.0 (clang-500.0.68)', gccosandvers=''
    intsize=4, longsize=8, ptrsize=8, doublesize=8, byteorder=12345678
    d_longlong=define, longlongsize=8, d_longdbl=define, longdblsize=16
    ivtype='long', ivsize=8, nvtype='double', nvsize=8, Off_t='off_t', lseeksize=8
    alignbytes=8, prototype=define
  Linker and Libraries:
    ld='cc -mmacosx-version-min=10.9', ldflags ='-arch x86_64 -arch i386 -fstack-protector -L/usr/local/lib'
    libpth=/usr/local/lib /usr/lib
    libs= 
    perllibs=
    libc=, so=dylib, useshrplib=true, libperl=libperl.dylib
    gnulibc_version=''
  Dynamic Linking:
    dlsrc=dl_dlopen.xs, dlext=bundle, d_dlsymun=undef, ccdlflags=' '
    cccdlflags=' ', lddlflags='-arch x86_64 -arch i386 -bundle -undefined dynamic_lookup -L/usr/local/lib -fstack-protector'


Characteristics of this binary (from libperl): 
  Compile-time options: HAS_TIMES MULTIPLICITY PERLIO_LAYERS
                        PERL_DONT_CREATE_GVSV PERL_IMPLICIT_CONTEXT
                        PERL_MALLOC_WRAP PERL_PRESERVE_IVUV USE_64_BIT_ALL
                        USE_64_BIT_INT USE_ITHREADS USE_LARGE_FILES
                        USE_LOCALE USE_LOCALE_COLLATE USE_LOCALE_CTYPE
                        USE_LOCALE_NUMERIC USE_PERLIO USE_PERL_ATOF
                        USE_REENTRANT_API
  Locally applied patches:
	/Library/Perl/Updates/<version> comes before system perl directories
	installprivlib and installarchlib points to the Updates directory
	CVE-2013-1667 hashtable DOS fix
  Built under darwin
  Compiled at Aug 25 2013 01:24:40
  %ENV:
    PERL5LIB=""
    PERL5OPT=""
    PERL5_CPANPLUS_IS_RUNNING="37393"
    PERL5_CPAN_IS_RUNNING="37393"
  @INC:
    /Library/Perl/5.16/darwin-thread-multi-2level
    /Library/Perl/5.16
    /Network/Library/Perl/5.16/darwin-thread-multi-2level
    /Network/Library/Perl/5.16
    /Library/Perl/Updates/5.16.2/darwin-thread-multi-2level
    /Library/Perl/Updates/5.16.2
    /System/Library/Perl/5.16/darwin-thread-multi-2level
    /System/Library/Perl/5.16
    /System/Library/Perl/Extras/5.16/darwin-thread-multi-2level
    /System/Library/Perl/Extras/5.16
    .

