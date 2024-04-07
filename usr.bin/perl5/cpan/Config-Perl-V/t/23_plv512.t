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
is ($conf->{build}{stamp}, "Dec 20 2010 12:46:00", "Build time");
is ($conf->{config}{version}, "5.12.2", "reconstructed \%Config{version}");
is ($conf->{config}{gccversion}, "", "not built with gcc");
is ($conf->{config}{ccversion}, "B3910B", "built with HP C-ANSI-C");

my $opt = Config::Perl::V::plv2hash ("")->{build}{options};
foreach my $o (sort qw(
	PERL_DONT_CREATE_GVSV PERL_MALLOC_WRAP USE_64_BIT_ALL
	USE_64_BIT_INT USE_LARGE_FILES USE_LONG_DOUBLE
	USE_PERLIO USE_PERL_ATOF
	)) {
    is ($conf->{build}{options}{$o}, 1, "Runtime option $o set");
    delete $opt->{$o};
    }
foreach my $o (sort keys %$opt) {
    is ($conf->{build}{options}{$o}, 0, "Runtime option $o unset");
    }

eval { require Digest::MD5; };
my $md5 = $@ ? "0" x 32 : "a2c38153cc47d340bc140d0bfe294afb";
ok (my $sig = Config::Perl::V::signature ($conf), "Get signature");

SKIP: {
    ord "A" == 65 or skip "ASCII-centric test", 1;
    is ($sig, $md5, "MD5");
    }

__END__
Summary of my perl5 (revision 5 version 12 subversion 2) configuration:

  Platform:
    osname=hpux, osvers=11.31, archname=IA64.ARCHREV_0-LP64-ld
    uname='hp-ux x2 b.11.31 u ia64 2977233888 unlimited-user license '
    config_args='-Duse64bitall -Duselongdouble -des'
    hint=recommended, useposix=true, d_sigaction=define
    useithreads=undef, usemultiplicity=undef
    useperlio=define, d_sfio=undef, uselargefiles=define, usesocks=undef
    use64bitint=define, use64bitall=define, uselongdouble=define
    usemymalloc=n, bincompat5005=undef
  Compiler:
    cc='cc', ccflags =' -Ae -DPERL_DONT_CREATE_GVSV +Z -z -D_HPUX_SOURCE -Wl,+vnocompatwarnings +DD64 -I/pro/local/include -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 ',
    optimize='+O2 +Onolimit',
    cppflags='-Aa -D__STDC_EXT__ -DPERL_DONT_CREATE_GVSV +Z -z -D_HPUX_SOURCE -Ae -DPERL_DONT_CREATE_GVSV +Z -z -D_HPUX_SOURCE -Wl,+vnocompatwarnings +DD64 -I/pro/local/include'
    ccversion='B3910B', gccversion='', gccosandvers=''
    intsize=4, longsize=8, ptrsize=8, doublesize=8, byteorder=87654321
    d_longlong=define, longlongsize=8, d_longdbl=define, longdblsize=16
    ivtype='long', ivsize=8, nvtype='long double', nvsize=16, Off_t='off_t', lseeksize=8
    alignbytes=16, prototype=define
  Linker and Libraries:
    ld='/usr/bin/ld', ldflags ='-L/pro/local/lib +DD64 -L/usr/lib/hpux64'
    libpth=/pro/local/lib /usr/lib/hpux64 /lib /usr/lib /usr/ccs/lib /usr/local/lib
    libs=-lcl -lpthread -lnsl -lnm -ldb -ldl -ldld -lm -lsec -lc
    perllibs=-lcl -lpthread -lnsl -lnm -ldl -ldld -lm -lsec -lc
    libc=/usr/lib/hpux64/libc.so, so=so, useshrplib=false, libperl=libperl.a
    gnulibc_version=''
  Dynamic Linking:
    dlsrc=dl_hpux.xs, dlext=so, d_dlsymun=undef, ccdlflags='-Wl,-E -Wl,-B,deferred '
    cccdlflags='+Z', lddlflags='-b +vnocompatwarnings -L/pro/local/lib -L/usr/lib/hpux64'


Characteristics of this binary (from libperl):
  Compile-time options: PERL_DONT_CREATE_GVSV PERL_MALLOC_WRAP USE_64_BIT_ALL
                        USE_64_BIT_INT USE_LARGE_FILES USE_LONG_DOUBLE
                        USE_PERLIO USE_PERL_ATOF
  Built under hpux
  Compiled at Dec 20 2010 12:46:00
  @INC:
    /pro/lib/perl5/site_perl/5.12.2/IA64.ARCHREV_0-LP64-ld
    /pro/lib/perl5/site_perl/5.12.2
    /pro/lib/perl5/5.12.2/IA64.ARCHREV_0-LP64-ld
    /pro/lib/perl5/5.12.2
    .
