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
is ($conf->{build}{stamp}, "Jun  2 2015 00:03:35", "Build time");
is ($conf->{config}{version}, "5.22.0", "reconstructed \$Config{version}");

my $opt = Config::Perl::V::plv2hash ("")->{build}{options};
foreach my $o (sort qw(
	HAS_TIMES HAVE_INTERP_INTERN MULTIPLICITY PERLIO_LAYERS
	PERL_DONT_CREATE_GVSV
	PERL_HASH_FUNC_ONE_AT_A_TIME_HARD
	PERL_IMPLICIT_CONTEXT PERL_IMPLICIT_SYS PERL_MALLOC_WRAP
	PERL_NEW_COPY_ON_WRITE PERL_PRESERVE_IVUV
	USE_64_BIT_INT USE_ITHREADS
	USE_LARGE_FILES USE_LOCALE USE_LOCALE_COLLATE
	USE_LOCALE_CTYPE USE_LOCALE_NUMERIC USE_LOCALE_TIME
	USE_PERLIO USE_PERL_ATOF
	)) {
    is ($conf->{build}{options}{$o}, 1, "Runtime option $o set");
    delete $opt->{$o};
    }
foreach my $o (sort keys %$opt) {
    is ($conf->{build}{options}{$o}, 0, "Runtime option $o unset");
    }

eval { require Digest::MD5; };
my $md5 = $@ ? "0" x 32 : "dfb32b8299b66e8bdb2712934f700d94";
ok (my $sig = Config::Perl::V::signature ($conf), "Get signature");

SKIP: {
    ord "A" == 65 or skip "ASCII-centric test", 1;
    is ($sig, $md5, "MD5");
    }

is_deeply ($conf->{build}{patches}, [], "No local patches");

my %check = (
    alignbytes      => 8,
    api_version     => 22,
    bincompat5005   => "undef",
    byteorder       => 12345678,
    cc              => "gcc",
    cccdlflags      => "",
    ccdlflags       => "",
    config_args     => "undef",
    gccversion      => "4.9.2",
    gnulibc_version => "",
    ivsize          => 8,
    ivtype          => "long long",
    ld              => "g++",
    lddlflags       => q{-mdll -s -L"D:\Strawberry\perl\lib\CORE" -L"D:\Strawberry\c\lib"},
    ldflags         => q{-s -L"D:\Strawberry\perl\lib\CORE" -L"D:\Strawberry\c\lib"},
    libc            => "",
    lseektype       => "long long",
    osvers          => "6.3",
    use64bitall     => "undef",
    use64bitint     => "define",
    );
is ($conf->{config}{$_}, $check{$_}, "reconstructed \$Config{$_}") for sort keys %check;

__END__
Summary of my perl5 (revision 5 version 22 subversion 0) configuration:

  Platform:
    osname=MSWin32, osvers=6.3, archname=MSWin32-x64-multi-thread
    uname='Win32 strawberry-perl 5.22.0.1 #1 Mon Jun  1 23:58:39 2015 x64'
    config_args='undef'
    hint=recommended, useposix=true, d_sigaction=undef
    useithreads=define, usemultiplicity=define
    use64bitint=define, use64bitall=undef, uselongdouble=undef
    usemymalloc=n, bincompat5005=undef
  Compiler:
    cc='gcc', ccflags =' -s -O2 -DWIN32 -DWIN64 -DCONSERVATIVE  -DPERL_TEXTMODE_SCRIPTS -DPERL_IMPLICIT_CONTEXT -DPERL_IMPLICIT_SYS -fwrapv -fno-strict-aliasing -mms-bitfields',
    optimize='-s -O2',
    cppflags='-DWIN32'
    ccversion='', gccversion='4.9.2', gccosandvers=''
    intsize=4, longsize=4, ptrsize=8, doublesize=8, byteorder=12345678, doublekind=3
    d_longlong=define, longlongsize=8, d_longdbl=define, longdblsize=8, longdblkind=3
    ivtype='long long', ivsize=8, nvtype='double', nvsize=8, Off_t='long long', lseeksize=8
    alignbytes=8, prototype=define
  Linker and Libraries:
    ld='g++', ldflags ='-s -L"D:\Strawberry\perl\lib\CORE" -L"D:\Strawberry\c\lib"'
    libpth=D:\Strawberry\c\lib D:\Strawberry\c\x86_64-w64-mingw32\lib D:\Strawberry\c\lib\gcc\x86_64-w64-mingw32\4.9.2
    libs=-lmoldname -lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -lnetapi32 -luuid -lws2_32 -lmpr -lwinmm -lversion -lodbc32 -lodbccp32
 -lcomctl32
    perllibs=-lmoldname -lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -lnetapi32 -luuid -lws2_32 -lmpr -lwinmm -lversion -lodbc32 -lodbc
cp32 -lcomctl32
    libc=, so=dll, useshrplib=true, libperl=libperl522.a
    gnulibc_version=''
  Dynamic Linking:
    dlsrc=dl_win32.xs, dlext=xs.dll, d_dlsymun=undef, ccdlflags=' '
    cccdlflags=' ', lddlflags='-mdll -s -L"D:\Strawberry\perl\lib\CORE" -L"D:\Strawberry\c\lib"'


Characteristics of this binary (from libperl):
  Compile-time options: HAS_TIMES HAVE_INTERP_INTERN MULTIPLICITY
                        PERLIO_LAYERS PERL_DONT_CREATE_GVSV
                        PERL_HASH_FUNC_ONE_AT_A_TIME_HARD
                        PERL_IMPLICIT_CONTEXT PERL_IMPLICIT_SYS
                        PERL_MALLOC_WRAP PERL_NEW_COPY_ON_WRITE
                        PERL_PRESERVE_IVUV USE_64_BIT_INT USE_ITHREADS
                        USE_LARGE_FILES USE_LOCALE USE_LOCALE_COLLATE
                        USE_LOCALE_CTYPE USE_LOCALE_NUMERIC USE_LOCALE_TIME
                        USE_PERLIO USE_PERL_ATOF
  Built under MSWin32
  Compiled at Jun  2 2015 00:03:35
  @INC:
    D:/Strawberry/perl/site/lib
    D:/Strawberry/perl/vendor/lib
    D:/Strawberry/perl/lib
    .
