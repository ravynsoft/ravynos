#!/pro/bin/perl

use strict;
use warnings;

BEGIN {
    use Test::More;
    my $tests = 107;
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
is ($conf->{build}{stamp}, "May 11 2012 16:36:53", "Build time");
is ($conf->{config}{version}, "5.14.2", "reconstructed \%Config{version}");
is ($conf->{config}{gccversion}, "", "not built with gcc");
is ($conf->{config}{ccversion}, "11.1.0.8", "xlc version");

my $opt = Config::Perl::V::plv2hash ("")->{build}{options};
foreach my $o (sort qw(
	DEBUGGING PERL_DONT_CREATE_GVSV PERL_MALLOC_WRAP
	PERL_PRESERVE_IVUV PERL_USE_DEVEL USE_64_BIT_ALL
	USE_64_BIT_INT USE_LARGE_FILES USE_PERLIO
	USE_PERL_ATOF
	)) {
    is ($conf->{build}{options}{$o}, 1, "Runtime option $o set");
    delete $opt->{$o};
    }
foreach my $o (sort keys %$opt) {
    is ($conf->{build}{options}{$o}, 0, "Runtime option $o unset");
    }

eval { require Digest::MD5; };
my $md5 = $@ ? "0" x 32 : "778815a670c0c454738aedf0c88930ba";
ok (my $sig = Config::Perl::V::signature ($conf), "Get signature");

SKIP: {
    ord "A" == 65 or skip "ASCII-centric test", 1;
    is ($sig, $md5, "MD5");
    }

__END__
Summary of my perl5 (revision 5 version 14 subversion 2) configuration:

  Platform:
    osname=aix, osvers=5.3.0.0, archname=aix-64all
    uname='aix i3 3 5 0004898ad300 '
    config_args='-Dusedevel -Duse64bitall -Uversiononly -Dinc_version_list=none -des'
    hint=recommended, useposix=true, d_sigaction=define
    useithreads=undef, usemultiplicity=undef
    useperlio=define, d_sfio=undef, uselargefiles=define, usesocks=undef
    use64bitint=define, use64bitall=define, uselongdouble=undef
    usemymalloc=n, bincompat5005=undef
  Compiler:
    cc='xlc -q64', ccflags ='-q64 -DDEBUGGING -qlanglvl=extended -D_ALL_SOURCE -D_ANSI_C_SOURCE -D_POSIX_SOURCE -qmaxmem=-1 -qnoansialias -DUSE_NATIVE_DLOPEN -qlanglvl=extended -I/pro/local/include -q64 -DUSE_64_BIT_ALL -q64',
    optimize='-O',
    cppflags='-DDEBUGGING -D_ALL_SOURCE -D_ANSI_C_SOURCE -D_POSIX_SOURCE -DUSE_NATIVE_DLOPEN -I/pro/local/include'
    ccversion='11.1.0.8', gccversion='', gccosandvers=''
    intsize=4, longsize=8, ptrsize=8, doublesize=8, byteorder=87654321
    d_longlong=define, longlongsize=8, d_longdbl=define, longdblsize=8
    ivtype='long', ivsize=8, nvtype='double', nvsize=8, Off_t='off_t', lseeksize=8
    alignbytes=8, prototype=define
  Linker and Libraries:
    ld='ld', ldflags ='-L/usr/local/ppc64/lib64 -b64 -q64 -L/pro/local/lib -brtl -bdynamic -b64'
    libpth=/usr/local/ppc64/lib64 /lib /usr/lib /usr/ccs/lib /usr/local/lib /usr/lib64
    libs=-lbind -lnsl -ldbm -ldb -ldl -lld -lm -lcrypt -lc
    perllibs=-lbind -lnsl -ldl -lld -lm -lcrypt -lc
    libc=/lib/libc.a, so=a, useshrplib=false, libperl=libperl.a
    gnulibc_version=''
  Dynamic Linking:
    dlsrc=dl_aix.xs, dlext=so, d_dlsymun=undef, ccdlflags='  -bE:/pro/lib/perl5/5.14.2/aix-64all/CORE/perl.exp'
    cccdlflags=' ', lddlflags='-b64  -bhalt:4 -G -bI:$(PERL_INC)/perl.exp -bE:$(BASEEXT).exp -bnoentry -lc -lm -L/usr/local/ppc64/lib64 -L/pro/local/lib'


Characteristics of this binary (from libperl):
  Compile-time options: DEBUGGING PERL_DONT_CREATE_GVSV PERL_MALLOC_WRAP
                        PERL_PRESERVE_IVUV PERL_USE_DEVEL USE_64_BIT_ALL
                        USE_64_BIT_INT USE_LARGE_FILES USE_PERLIO
                        USE_PERL_ATOF
  Built under aix
  Compiled at May 11 2012 16:36:53
  @INC:
    /pro/lib/perl5/site_perl/5.14.2/aix-64all
    /pro/lib/perl5/site_perl/5.14.2
    /pro/lib/perl5/5.14.2/aix-64all
    /pro/lib/perl5/5.14.2
    .
