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
is ($conf->{build}{stamp}, "Apr 13 2019 00:06:38", "Build time");
is ($conf->{config}{version}, "5.29.10", "reconstructed \$Config{version}");

my $opt = Config::Perl::V::plv2hash ("")->{build}{options};
foreach my $o (sort qw(
	DEBUGGING HAS_TIMES MULTIPLICITY PERL_COPY_ON_WRITE PERL_DONT_CREATE_GVSV
	PERL_IMPLICIT_CONTEXT PERLIO_LAYERS PERL_MALLOC_WRAP PERL_OP_PARENT
	PERL_PRESERVE_IVUV PERL_TRACK_MEMPOOL PERL_USE_DEVEL USE_64_BIT_ALL
	USE_64_BIT_INT USE_ITHREADS USE_LARGE_FILES USE_LOCALE USE_LOCALE_COLLATE
	USE_LOCALE_CTYPE USE_LOCALE_NUMERIC USE_LOCALE_TIME USE_LONG_DOUBLE
	USE_PERL_ATOF USE_PERLIO USE_REENTRANT_API USE_THREAD_SAFE_LOCALE
	)) {
    is ($conf->{build}{options}{$o}, 1, "Runtime option $o set");
    delete $opt->{$o};
    }
foreach my $o (sort keys %$opt) {
    is ($conf->{build}{options}{$o}, 0, "Runtime option $o unset");
    }

eval { require Digest::MD5; };
my $md5 = $@ ? "0" x 32 : "8404b533829bd9752df7f662a710f993";
ok (my $sig = Config::Perl::V::signature ($conf), "Get signature");

SKIP: {
    ord "A" == 65 or skip "ASCII-centric test", 1;
    is ($sig, $md5, "MD5");
    }

is_deeply ($conf->{build}{patches}, [
    "SMOKEdfba4714a9dc4c35123b4df0a5e1721ccb081d97" ], "No local patches");

my %check = (
    alignbytes      => 16,
    api_version     => 29,
    bincompat5005   => "undef",
    byteorder       => 12345678,
    cc              => "g++",
    cccdlflags      => "-fPIC",
    ccdlflags       => "-Wl,-E",
    config_args     => "-des -Dcc=g++ -Dusedevel -Duseithreads -Duse64bitall -Duselongdouble -DDEBUGGING",
    gccversion      => "8.3.1 20190226 [gcc-8-branch revision 269204]",
    gnulibc_version => "2.29",
    ivsize          => 8,
    ivtype          => "long",
    ld              => "g++",
    lddlflags       => "-shared -O2 -g -L/pro/local/lib -fstack-protector-strong",
    ldflags         => "-L/pro/local/lib -fstack-protector-strong",
    libc            => "libc-2.29.so",
    lseektype       => "off_t",
    osvers          => "5.0.7-1-default",
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
Summary of my perl5 (revision 5 version 29 subversion 10) configuration:
  Snapshot of: dfba4714a9dc4c35123b4df0a5e1721ccb081d97
  Platform:
    osname=linux
    osvers=5.0.7-1-default
    archname=x86_64-linux-thread-multi-ld
    uname='linux lx09 5.0.7-1-default #1 smp sat apr 6 14:47:49 utc 2019 (8f18342) x86_64 x86_64 x86_64 gnulinux '
    config_args='-des -Dcc=g++ -Dusedevel -Duseithreads -Duse64bitall -Duselongdouble -DDEBUGGING'
    hint=recommended
    useposix=true
    d_sigaction=define
    useithreads=define
    usemultiplicity=define
    use64bitint=define
    use64bitall=define
    uselongdouble=define
    usemymalloc=n
    default_inc_excludes_dot=define
    bincompat5005=undef
  Compiler:
    cc='g++'
    ccflags ='-D_REENTRANT -D_GNU_SOURCE -fPIC -DDEBUGGING -fwrapv -DDEBUGGING -fno-strict-aliasing -pipe -fstack-protector-strong -I/pro/local/include -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_FORTIFY_SOURCE=2'
    optimize='-O2 -g'
    cppflags='-D_REENTRANT -D_GNU_SOURCE -fPIC -DDEBUGGING -fwrapv -DDEBUGGING -fno-strict-aliasing -pipe -fstack-protector-strong -I/pro/local/include'
    ccversion=''
    gccversion='8.3.1 20190226 [gcc-8-branch revision 269204]'
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
    nvtype='long double'
    nvsize=16
    Off_t='off_t'
    lseeksize=8
    alignbytes=16
    prototype=define
  Linker and Libraries:
    ld='g++'
    ldflags ='-L/pro/local/lib -fstack-protector-strong'
    libpth=/usr/include/c++/8 /usr/include/c++/8/x86_64-suse-linux /usr/include/c++/8/backward /usr/local/lib /usr/lib64/gcc/x86_64-suse-linux/8/include-fixed /usr/lib64/gcc/x86_64-suse-linux/8/../../../../x86_64-suse-linux/lib /usr/lib /pro/local/lib /lib/../lib64 /usr/lib/../lib64 /lib /lib64 /usr/lib64 /usr/local/lib64
    libs=-lpthread -lnsl -lgdbm -ldb -ldl -lm -lcrypt -lutil -lc -lgdbm_compat
    perllibs=-lpthread -lnsl -ldl -lm -lcrypt -lutil -lc
    libc=libc-2.29.so
    so=so
    useshrplib=false
    libperl=libperl.a
    gnulibc_version='2.29'
  Dynamic Linking:
    dlsrc=dl_dlopen.xs
    dlext=so
    d_dlsymun=undef
    ccdlflags='-Wl,-E'
    cccdlflags='-fPIC'
    lddlflags='-shared -O2 -g -L/pro/local/lib -fstack-protector-strong'


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
    USE_LONG_DOUBLE
    USE_PERLIO
    USE_PERL_ATOF
    USE_REENTRANT_API
    USE_THREAD_SAFE_LOCALE
  Locally applied patches:
    SMOKEdfba4714a9dc4c35123b4df0a5e1721ccb081d97
  Built under linux
  Compiled at Apr 13 2019 00:06:38
  %ENV:
    PERL6LIB="inst#/pro/3gl/CPAN/rakudo/install"
  @INC:
    lib
    /opt/perl/lib/site_perl/5.29.10/x86_64-linux-thread-multi-ld
    /opt/perl/lib/site_perl/5.29.10
    /opt/perl/lib/5.29.10/x86_64-linux-thread-multi-ld
    /opt/perl/lib/5.29.10
