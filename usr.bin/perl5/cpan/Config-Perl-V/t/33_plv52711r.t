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
is ($conf->{build}{stamp}, "Apr 12 2018 13:37:01", "Build time");
is ($conf->{config}{version}, "5.27.11", "reconstructed \$Config{version}");

my $opt = Config::Perl::V::plv2hash ("")->{build}{options};
foreach my $o (sort qw(
	DEBUGGING HAS_TIMES MULTIPLICITY PERLIO_LAYERS PERL_COPY_ON_WRITE
	PERL_DONT_CREATE_GVSV PERL_TRACK_MEMPOOL PERL_IMPLICIT_CONTEXT
	PERL_MALLOC_WRAP PERL_OP_PARENT PERL_PRESERVE_IVUV PERL_USE_DEVEL
	USE_64_BIT_ALL
	USE_64_BIT_INT USE_ITHREADS USE_LARGE_FILES USE_LOCALE USE_LOCALE_COLLATE
	USE_LOCALE_CTYPE USE_LOCALE_NUMERIC USE_LOCALE_TIME
	USE_LONG_DOUBLE USE_PERLIO USE_PERL_ATOF USE_REENTRANT_API
	)) {
    is ($conf->{build}{options}{$o}, 1, "Runtime option $o set");
    delete $opt->{$o};
    }
foreach my $o (sort keys %$opt) {
    is ($conf->{build}{options}{$o}, 0, "Runtime option $o unset");
    }

eval { require Digest::MD5; };
my $md5 = $@ ? "0" x 32 : "bd9cf7a142ddbb434adea5b08eaefdc8";
ok (my $sig = Config::Perl::V::signature ($conf), "Get signature");

SKIP: {
    ord "A" == 65 or skip "ASCII-centric test", 1;
    is ($sig, $md5, "MD5");
    }

is_deeply ($conf->{build}{patches}, [], "Local patches");

my %check = (
    alignbytes      => 16,
    api_version     => 27,
    bincompat5005   => "undef",
    byteorder       => 12345678,
    cc              => "cc",
    cccdlflags      => "-fPIC",
    ccdlflags       => "-Wl,-E",
    config_args     => "-Dusedevel -Duse64bitall -Dusethreads -Duseithreads -Duselongdouble -des",
    gccversion      => "7.3.1 20180307 [gcc-7-branch revision 258314]",
    gnulibc_version => "2.27",
    ivsize          => 8,
    ivtype          => "long",
    ld              => "cc",
    lddlflags       => "-shared -O2 -L/pro/local/lib -fstack-protector-strong",
    ldflags         => "-L/pro/local/lib -fstack-protector-strong",
    libc            => "libc-2.27.so",
    lseektype       => "off_t",
    osvers          => "4.16.0-1-default",
    use64bitall     => "define",
    use64bitint     => "define",
    usemymalloc     => "n",
    default_inc_excludes_dot
		    => "define",
    );
is ($conf->{config}{$_}, $check{$_}, "reconstructed \$Config{$_}") for sort keys %check;

ok (my $info = summary ($conf), "A summary");
ok (exists $info->{$_}, "Summary has $_") for qw( cc config_args usemymalloc default_inc_excludes_dot );
is ($info->{default_inc_excludes_dot}, "define", "This build does not have . in INC");

__END__
Summary of my perl5 (revision 5 version 27 subversion 11) configuration:
  Snapshot of: 5f6af817add6d2df3603e0e94b6eb27ba5fb3970
  Platform:
    osname=linux
    osvers=4.16.0-1-default
    archname=x86_64-linux-thread-multi-ld
    uname='linux lx09 4.16.0-1-default #1 smp preempt wed apr 4 13:35:56 utc 2018 (e16f96d) x86_64 x86_64 x86_64 gnulinux '
    config_args='-Dusedevel -Duse64bitall -Dusethreads -Duseithreads -Duselongdouble -des'
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
    cc='cc'
    ccflags ='-D_REENTRANT -D_GNU_SOURCE -fPIC -DDEBUGGING -fwrapv -fno-strict-aliasing -pipe -fstack-protector-strong -I/pro/local/include -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_FORTIFY_SOURCE=2'
    optimize='-O2'
    cppflags='-D_REENTRANT -D_GNU_SOURCE -fPIC -DDEBUGGING -fwrapv -fno-strict-aliasing -pipe -fstack-protector-strong -I/pro/local/include'
    ccversion=''
    gccversion='7.3.1 20180307 [gcc-7-branch revision 258314]'
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
    ld='cc'
    ldflags ='-L/pro/local/lib -fstack-protector-strong'
    libpth=/usr/local/lib /usr/lib64/gcc/x86_64-suse-linux/7/include-fixed /usr/lib64/gcc/x86_64-suse-linux/7/../../../../x86_64-suse-linux/lib /usr/lib /pro/local/lib /lib/../lib64 /usr/lib/../lib64 /lib /lib64 /usr/lib64 /usr/local/lib64
    libs=-lpthread -lnsl -lgdbm -ldb -ldl -lm -lcrypt -lutil -lc -lgdbm_compat
    perllibs=-lpthread -lnsl -ldl -lm -lcrypt -lutil -lc
    libc=libc-2.27.so
    so=so
    useshrplib=false
    libperl=libperl.a
    gnulibc_version='2.27'
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
    USE_LONG_DOUBLE
    USE_PERLIO
    USE_PERL_ATOF
    USE_REENTRANT_API
  Built under linux
  Compiled at Apr 12 2018 13:37:01
  @INC:
    lib
    /pro/lib/perl5/site_perl/5.27.11/x86_64-linux-thread-multi-ld
    /pro/lib/perl5/site_perl/5.27.11
    /pro/lib/perl5/5.27.11/x86_64-linux-thread-multi-ld
    /pro/lib/perl5/5.27.11
