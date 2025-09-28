#!/pro/bin/perl

use strict;
use warnings;

BEGIN {
    use Test::More;
    my $tests = 134;
    unless ($ENV{PERL_CORE}) {
	require Test::NoWarnings;
	Test::NoWarnings->import ();
	$tests++;
	}

    plan tests => $tests;
    }

use Config::Perl::V qw( summary );

ok (my $conf = Config::Perl::V::plv2hash (<DATA>), "Read perl -v block");
ok (exists $conf->{$_}, "Has $_ entry") for qw( build environment config inc );

is ($conf->{build}{osname}, $conf->{config}{osname}, "osname");
is ($conf->{build}{stamp}, "Jan  1 2022 11:18:27", "Build time");
is ($conf->{config}{version}, "5.35.8", "reconstructed \$Config{version}");

my $opt = Config::Perl::V::plv2hash ("")->{build}{options};
foreach my $o (sort qw(
	HAS_TIMES PERLIO_LAYERS PERL_COPY_ON_WRITE PERL_DONT_CREATE_GVSV
	PERL_MALLOC_WRAP PERL_OP_PARENT PERL_PRESERVE_IVUV PERL_USE_DEVEL
	USE_64_BIT_ALL USE_64_BIT_INT USE_LARGE_FILES USE_LOCALE
	USE_LOCALE_COLLATE USE_LOCALE_CTYPE USE_LOCALE_NUMERIC
	USE_LOCALE_TIME USE_PERLIO USE_PERL_ATOF USE_QUADMATH
	)) {
    is ($conf->{build}{options}{$o}, 1, "Runtime option $o set");
    delete $opt->{$o};
    }
foreach my $o (sort keys %$opt) {
    is ($conf->{build}{options}{$o}, 0, "Runtime option $o unset");
    }

eval { require Digest::MD5; };
my $md5 = $@ ? "0" x 32 : "3a52d65d54ee1032f878b51fb20c8efd";
ok (my $sig = Config::Perl::V::signature ($conf), "Get signature");
is ($sig, $md5, "MD5");

is_deeply ($conf->{build}{patches}, [ ], "No patches");

my %check = (
    alignbytes      => 16,
    api_version     => 35,
    bincompat5005   => undef,
    byteorder       => 12345678,
    cc              => "cc",
    cccdlflags      => "-fPIC",
    ccdlflags       => "-Wl,-E",
    config_args     => "-Dusedevel -Duse64bitall -Dusequadmath -Uuseperlio -des",
    gccversion      => "11.2.1 20211124 [revision 7510c23c1ec53aa4a62705f0384079661342ff7b]",
    gnulibc_version => "2.34",
    ivsize          => 8,
    ivtype          => "long",
    ld              => "cc",
    lddlflags       => "-shared -O2 -L/pro/local/lib -fstack-protector-strong",
    ldflags         => "-L/pro/local/lib -fstack-protector-strong",
    libc            => "/lib/../lib64/libc.so.6",
    lseektype       => "off_t",
    osvers          => "5.15.8-1-default",
    use64bitall     => "define",
    use64bitint     => "define",
    usemymalloc     => "n",
    default_inc_excludes_dot
		    => "define",
    );
is ($conf->{config}{$_}, $check{$_}, "reconstructed \$Config{$_}") for sort keys %check;

ok (my $info = summary ($conf), "A summary");
ok (exists $info->{$_}, "Summary has $_") for qw( cc config_args usemymalloc default_inc_excludes_dot );
is ($info->{default_inc_excludes_dot}, "define", "This build has . NOT in INC");

__END__
Summary of my perl5 (revision 5 version 35 subversion 8) configuration:
  Snapshot of: 0ccfd062e2cfd32efe146d4c16faf3cae9e3cc84
  Platform:
    osname=linux
    osvers=5.15.8-1-default
    archname=x86_64-linux-quadmath
    uname='linux lx09 5.15.8-1-default #1 smp wed dec 15 08:12:54 utc 2021 (0530e5c) x86_64 x86_64 x86_64 gnulinux '
    config_args='-Dusedevel -Duse64bitall -Dusequadmath -Uuseperlio -des'
    hint=recommended
    useposix=true
    d_sigaction=define
    useithreads=undef
    usemultiplicity=undef
    use64bitint=define
    use64bitall=define
    uselongdouble=undef
    usemymalloc=n
    default_inc_excludes_dot=define
  Compiler:
    cc='cc'
    ccflags ='-pie -fPIE -fPIC -fwrapv -fno-strict-aliasing -pipe -fstack-protector-strong -I/pro/local/include -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_FORTIFY_SOURCE=2'
    optimize='-O2'
    cppflags='-pie -fPIE -fPIC -fwrapv -fno-strict-aliasing -pipe -fstack-protector-strong -I/pro/local/include'
    ccversion=''
    gccversion='11.2.1 20211124 [revision 7510c23c1ec53aa4a62705f0384079661342ff7b]'
    gccosandvers=''
    intsize=4
    longsize=8
    ptrsize=8
    doublesize=8
    byteorder=12345678
    doublekind=3
    d_longlong=define
    longlongsize=8
    d_longdbl=define
    longdblsize=16
    longdblkind=3
    ivtype='long'
    ivsize=8
    nvtype='__float128'
    nvsize=16
    Off_t='off_t'
    lseeksize=8
    alignbytes=16
    prototype=define
  Linker and Libraries:
    ld='cc'
    ldflags ='-L/pro/local/lib -fstack-protector-strong'
    libpth=/usr/local/lib /usr/x86_64-suse-linux/lib /usr/lib /data/pro/local/lib /usr/lib64 /usr/local/lib64
    libs=-lpthread -lnsl -lgdbm -ldb -ldl -lm -lcrypt -lutil -lc -lgdbm_compat -lquadmath
    perllibs=-lpthread -lnsl -ldl -lm -lcrypt -lutil -lc -lquadmath
    libc=/lib/../lib64/libc.so.6
    so=so
    useshrplib=false
    libperl=libperl.a
    gnulibc_version='2.34'
  Dynamic Linking:
    dlsrc=dl_dlopen.xs
    dlext=so
    d_dlsymun=undef
    ccdlflags='-Wl,-E'
    cccdlflags='-fPIC'
    lddlflags='-shared -O2 -L/pro/local/lib -fstack-protector-strong'


Characteristics of this binary (from libperl):
  Compile-time options:
    HAS_TIMES
    PERLIO_LAYERS
    PERL_COPY_ON_WRITE
    PERL_DONT_CREATE_GVSV
    PERL_MALLOC_WRAP
    PERL_OP_PARENT
    PERL_PRESERVE_IVUV
    PERL_USE_DEVEL
    USE_64_BIT_ALL
    USE_64_BIT_INT
    USE_LARGE_FILES
    USE_LOCALE
    USE_LOCALE_COLLATE
    USE_LOCALE_CTYPE
    USE_LOCALE_NUMERIC
    USE_LOCALE_TIME
    USE_PERLIO
    USE_PERL_ATOF
    USE_QUADMATH
  Built under linux
  Compiled at Jan  1 2022 11:18:27
  %ENV:
    PERL6LIB="inst#/pro/3gl/CPAN/rakudo/install"
  @INC:
    lib
    /pro/lib/perl5/site_perl/5.35.8/x86_64-linux-quadmath
    /pro/lib/perl5/site_perl/5.35.8
    /pro/lib/perl5/5.35.8/x86_64-linux-quadmath
    /pro/lib/perl5/5.35.8
