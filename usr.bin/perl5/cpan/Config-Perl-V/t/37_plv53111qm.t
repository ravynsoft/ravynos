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
is ($conf->{build}{stamp}, "Apr  9 2020 17:12:07", "Build time");
is ($conf->{config}{version}, "5.31.11", "reconstructed \$Config{version}");

my $opt = Config::Perl::V::plv2hash ("")->{build}{options};
foreach my $o (sort qw(
	DEBUGGING HAS_TIMES MULTIPLICITY PERL_COPY_ON_WRITE
	PERL_DONT_CREATE_GVSV PERL_IMPLICIT_CONTEXT PERLIO_LAYERS
	PERL_MALLOC_WRAP PERL_OP_PARENT PERL_PRESERVE_IVUV PERL_TRACK_MEMPOOL
	PERL_USE_DEVEL USE_64_BIT_ALL USE_64_BIT_INT USE_ITHREADS
	USE_LARGE_FILES USE_LOCALE USE_LOCALE_COLLATE USE_LOCALE_CTYPE
	USE_LOCALE_NUMERIC USE_LOCALE_TIME USE_PERL_ATOF USE_PERLIO
	USE_QUADMATH USE_REENTRANT_API USE_THREAD_SAFE_LOCALE
	)) {
    is ($conf->{build}{options}{$o}, 1, "Runtime option $o set");
    delete $opt->{$o};
    }
foreach my $o (sort keys %$opt) {
    is ($conf->{build}{options}{$o}, 0, "Runtime option $o unset");
    }

eval { require Digest::MD5; };
my $md5 = $@ ? "0" x 32 : "146e648c6239f623b8a8242fc8b5759f";
ok (my $sig = Config::Perl::V::signature ($conf), "Get signature");

SKIP: {
    ord "A" == 65 or skip "ASCII-centric test", 1;
    is ($sig, $md5, "MD5");
    }

is_deeply ($conf->{build}{patches}, [ ], "No patches");

my %check = (
    alignbytes      => 16,
    api_version     => 31,
    bincompat5005   => "undef",
    byteorder       => 12345678,
    cc              => "cc",
    cccdlflags      => "-fPIC",
    ccdlflags       => "-Wl,-E",
    config_args     => "-Dusedevel -Duse64bitall -Dusethreads -Duseithreads -Dusequadmath -des",
    gccversion      => "10.0.1 20200302 (experimental) [revision 778a77357cad11e8dd4c810544330af0fbe843b1]",
    gnulibc_version => "2.31",
    ivsize          => 8,
    ivtype          => "long",
    ld              => "cc",
    lddlflags       => "-shared -O2 -L/pro/local/lib -fstack-protector-strong",
    ldflags         => "-L/pro/local/lib -fstack-protector-strong",
    libc            => "libc-2.31.so",
    lseektype       => "off_t",
    osvers          => "5.6.2-1-default",
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
Summary of my perl5 (revision 5 version 31 subversion 11) configuration:
  Snapshot of: +0300
  Platform:
    osname=linux
    osvers=5.6.2-1-default
    archname=x86_64-linux-thread-multi-quadmath
    uname='linux lx09 5.6.2-1-default #1 smp thu apr 2 06:31:32 utc 2020 (c8170d6) x86_64 x86_64 x86_64 gnulinux '
    config_args='-Dusedevel -Duse64bitall -Dusethreads -Duseithreads -Dusequadmath -des'
    hint=recommended
    useposix=true
    d_sigaction=define
    useithreads=define
    usemultiplicity=define
    use64bitint=define
    use64bitall=define
    uselongdouble=undef
    usemymalloc=n
    default_inc_excludes_dot=define
    bincompat5005=undef
  Compiler:
    cc='cc'
    ccflags ='-D_REENTRANT -D_GNU_SOURCE -fPIC -DDEBUGGING -fwrapv -fno-strict-aliasing -pipe -fstack-protector-strong -I/pro/local/include -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_FORTIFY_SOURCE=2'
    optimize='-O2'
    cppflags='-D_REENTRANT -D_GNU_SOURCE -fPIC -DDEBUGGING -fwrapv -fno-strict-aliasing -pipe -fstack-protector-strong -I/pro/local/include'
    ccversion=''
    gccversion='10.0.1 20200302 (experimental) [revision 778a77357cad11e8dd4c810544330af0fbe843b1]'
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
    libpth=/usr/local/lib /usr/lib64/gcc/x86_64-suse-linux/10/include-fixed /usr/lib64/gcc/x86_64-suse-linux/10/../../../../x86_64-suse-linux/lib /usr/lib /pro/local/lib /lib/../lib64 /usr/lib/../lib64 /lib /lib64 /usr/lib64 /usr/local/lib64
    libs=-lpthread -lnsl -lgdbm -ldb -ldl -lm -lcrypt -lutil -lc -lgdbm_compat -lquadmath
    perllibs=-lpthread -lnsl -ldl -lm -lcrypt -lutil -lc -lquadmath
    libc=libc-2.31.so
    so=so
    useshrplib=false
    libperl=libperl.a
    gnulibc_version='2.31'
  Dynamic Linking:
    dlsrc=dl_dlopen.xs
    dlext=so
    d_dlsymun=undef
    ccdlflags='-Wl,-E'
    cccdlflags='-fPIC'
    lddlflags='-shared -O2 -L/pro/local/lib -fstack-protector-strong'


Characteristics of this binary (from libperl):
  Compile-time options:
    DEBUGGING
    HAS_TIMES
    MULTIPLICITY
    PERLIO_LAYERS
    PERL_COPY_ON_WRITE
    PERL_DONT_CREATE_GVSV
    PERL_IMPLICIT_CONTEXT
    PERL_MALLOC_WRAP
    PERL_OP_PARENT
    PERL_PRESERVE_IVUV
    PERL_TRACK_MEMPOOL
    PERL_USE_DEVEL
    USE_64_BIT_ALL
    USE_64_BIT_INT
    USE_ITHREADS
    USE_LARGE_FILES
    USE_LOCALE
    USE_LOCALE_COLLATE
    USE_LOCALE_CTYPE
    USE_LOCALE_NUMERIC
    USE_LOCALE_TIME
    USE_PERLIO
    USE_PERL_ATOF
    USE_QUADMATH
    USE_REENTRANT_API
    USE_THREAD_SAFE_LOCALE
  Built under linux
  Compiled at Apr  9 2020 17:12:07
  %ENV:
    PERL6LIB="inst#/pro/3gl/CPAN/rakudo/install"
  @INC:
    lib
    /pro/lib/perl5/site_perl/5.31.11/x86_64-linux-thread-multi-quadmath
    /pro/lib/perl5/site_perl/5.31.11
    /pro/lib/perl5/5.31.11/x86_64-linux-thread-multi-quadmath
    /pro/lib/perl5/5.31.11
