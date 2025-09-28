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
is ($conf->{build}{stamp}, "Nov 19 2015 00:18:50", "Build time");
is ($conf->{config}{version}, "5.23.5", "reconstructed \$Config{version}");

my $opt = Config::Perl::V::plv2hash ("")->{build}{options};
foreach my $o (sort qw(
	HAS_TIMES HAVE_INTERP_INTERN MULTIPLICITY PERLIO_LAYERS
	PERL_COPY_ON_WRITE PERL_DONT_CREATE_GVSV
	PERL_HASH_FUNC_ONE_AT_A_TIME_HARD
	PERL_IMPLICIT_CONTEXT PERL_IMPLICIT_SYS PERL_MALLOC_WRAP
	PERL_PRESERVE_IVUV
	USE_ITHREADS
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
my $md5 = $@ ? "0" x 32 : "bccd5d78dfebd48b89faf7f1fe711733";
ok (my $sig = Config::Perl::V::signature ($conf), "Get signature");

SKIP: {
    ord "A" == 65 or skip "ASCII-centric test", 1;
    is ($sig, $md5, "MD5");
    }

is_deeply ($conf->{build}{patches}, [], "No local patches");

my %check = (
    alignbytes      => 8,
    api_version     => 23,
    bincompat5005   => "undef",
    byteorder       => 1234,
    cc              => "cl",
    cccdlflags      => "",
    ccdlflags       => "",
    config_args     => "undef",
    gccversion      => "",
    gnulibc_version => "",
    ivsize          => 4,
    ivtype          => "long",
    ld              => "link",
    lddlflags       => q{-dll -nologo -nodefaultlib -debug -opt:ref,icf -ltcg 		-libpath:"c:\perl\lib\CORE" 		-machine:x86 "/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'" -subsystem:console,"5.01"},
    ldflags         => q{-nologo -nodefaultlib -debug -opt:ref,icf -ltcg 		-libpath:"c:\perl\lib\CORE" 		-machine:x86 "/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'" -subsystem:console,"5.01"},
    libc            => "msvcrt.lib",
    lseektype       => "__int64",
    osvers          => "6.1",
    use64bitall     => "undef",
    use64bitint     => "undef",
    );
is ($conf->{config}{$_}, $check{$_}, "reconstructed \$Config{$_}") for sort keys %check;

__END__
Summary of my perl5 (revision 5 version 23 subversion 5) configuration:
   
  Platform:
    osname=MSWin32, osvers=6.1, archname=MSWin32-x86-multi-thread
    uname=''
    config_args='undef'
    hint=recommended, useposix=true, d_sigaction=undef
    useithreads=define, usemultiplicity=define
    use64bitint=undef, use64bitall=undef, uselongdouble=undef
    usemymalloc=n, bincompat5005=undef
  Compiler:
    cc='cl', ccflags ='-nologo -GF -W3 -O1 -MD -Zi -DNDEBUG -GL -DWIN32 -D_CONSOLE -DNO_STRICT -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE  -DPERL_TEXTMODE_SCRIPTS -DPERL_IMPLICIT_CONTEXT -DPERL_IMPLICIT_SYS',
    optimize='-O1 -MD -Zi -DNDEBUG -GL',
    cppflags='-DWIN32'
    ccversion='18.00.31101', gccversion='', gccosandvers=''
    intsize=4, longsize=4, ptrsize=4, doublesize=8, byteorder=1234, doublekind=3
    d_longlong=undef, longlongsize=8, d_longdbl=define, longdblsize=8, longdblkind=0
    ivtype='long', ivsize=4, nvtype='double', nvsize=8, Off_t='__int64', lseeksize=8
    alignbytes=8, prototype=define
  Linker and Libraries:
    ld='link', ldflags ='-nologo -nodefaultlib -debug -opt:ref,icf -ltcg 		-libpath:"c:\perl\lib\CORE" 		-machine:x86 "/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'" -subsystem:console,"5.01"'
    libpth=\lib
    libs=oldnames.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib netapi32.lib uuid.lib ws2_32.lib mpr.lib winmm.lib version.lib odbc32.lib odbccp32.lib comctl32.lib msvcrt.lib
    perllibs=oldnames.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib netapi32.lib uuid.lib ws2_32.lib mpr.lib winmm.lib version.lib odbc32.lib odbccp32.lib comctl32.lib msvcrt.lib
    libc=msvcrt.lib, so=dll, useshrplib=true, libperl=perl523.lib
    gnulibc_version=''
  Dynamic Linking:
    dlsrc=dl_win32.xs, dlext=dll, d_dlsymun=undef, ccdlflags=' '
    cccdlflags=' ', lddlflags='-dll -nologo -nodefaultlib -debug -opt:ref,icf -ltcg 		-libpath:"c:\perl\lib\CORE" 		-machine:x86 "/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'" -subsystem:console,"5.01"'


Characteristics of this binary (from libperl): 
  Compile-time options: HAS_TIMES HAVE_INTERP_INTERN MULTIPLICITY
                        PERLIO_LAYERS PERL_COPY_ON_WRITE
                        PERL_DONT_CREATE_GVSV
                        PERL_HASH_FUNC_ONE_AT_A_TIME_HARD
                        PERL_IMPLICIT_CONTEXT PERL_IMPLICIT_SYS
                        PERL_MALLOC_WRAP PERL_PRESERVE_IVUV USE_ITHREADS
                        USE_LARGE_FILES USE_LOCALE USE_LOCALE_COLLATE
                        USE_LOCALE_CTYPE USE_LOCALE_NUMERIC USE_LOCALE_TIME
                        USE_PERLIO USE_PERL_ATOF
  Built under MSWin32
  Compiled at Nov 19 2015 00:18:50
  @INC:
    C:/p523/src/lib
    .
