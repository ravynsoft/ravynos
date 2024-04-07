#!/pro/bin/perl

use strict;
use warnings;

BEGIN {
    use Test::More;
    my $tests = 135;
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
is ($conf->{build}{stamp}, "Feb 28 2023 18:30:03", "Build time");
is ($conf->{config}{version}, "5.37.10", "reconstructed \$Config{version}");

my $opt = Config::Perl::V::plv2hash ("")->{build}{options};
foreach my $o (sort qw(
	DEBUGGING HAS_TIMES MULTIPLICITY PERL_COPY_ON_WRITE PERL_RC_STACK
	PERL_DONT_CREATE_GVSV PERL_HASH_FUNC_SIPHASH13 PERL_HASH_USE_SBOX32
	PERLIO_LAYERS PERL_MALLOC_WRAP PERL_OP_PARENT PERL_PRESERVE_IVUV
	PERL_TRACK_MEMPOOL PERL_USE_DEVEL PERL_USE_SAFE_PUTENV USE_64_BIT_ALL
	USE_64_BIT_INT USE_ITHREADS USE_LARGE_FILES USE_LOCALE
	USE_LOCALE_COLLATE USE_LOCALE_CTYPE USE_LOCALE_NUMERIC USE_LOCALE_TIME
	USE_LONG_DOUBLE USE_PERL_ATOF USE_PERLIO USE_REENTRANT_API
	USE_THREAD_SAFE_LOCALE
	)) {
    is ($conf->{build}{options}{$o}, 1, "Runtime option $o set");
    delete $opt->{$o};
    }
foreach my $o (sort keys %$opt) {
    is ($conf->{build}{options}{$o}, 0, "Runtime option $o unset");
    }

eval { require Digest::MD5; };
my $md5 = $@ ? "0" x 32 : "964776ac5595a8a584dfba7ee063e4b9";
ok (my $sig = Config::Perl::V::signature ($conf), "Get signature");

SKIP: {
    ord "A" == 65 or skip "ASCII-centric test", 1;
    is ($sig, $md5, "MD5");
    }

is_deeply ($conf->{build}{patches}, [ "uncommitted-changes" ], "No patches");

my %check = (
    alignbytes      => 16,
    api_version     => 37,
    bincompat5005   => undef,	# GONE, chainsawed
    byteorder       => 12345678,
    cc              => "cc",
    cccdlflags      => "-fPIC",
    ccdlflags       => "-Wl,-E",
    config_args     => "-Dusedevel -Duse64bitall -Dusethreads -Duseithreads -Duselongdouble -Doptimize='-O0\\ -g' -Accflags=-DPERL_RC_STACK -des",
    gccversion      => "12.2.1 20230124 [revision 193f7e62815b4089dfaed4c2bd34fd4f10209e27]",
    gnulibc_version => "2.37",
    ivsize          => 8,
    ivtype          => "long",
    ld              => "cc",
    lddlflags       => "-shared -O0 -g -L/pro/local/lib -fstack-protector-strong",
    ldflags         => "-L/pro/local/lib -fstack-protector-strong",
    libc            => "/lib/../lib64/libc.so.6",
    lseektype       => "off_t",
    osvers          => "6.1.12-1-default",
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
Summary of my perl5 (revision 5 version 37 subversion 10) configuration:
  Derived from: f0cf813c73daf1ae652b454fc8bc4828aec1f049
  Platform:
    osname=linux
    osvers=6.1.12-1-default
    archname=x86_64-linux-thread-multi-ld
    uname='linux lx09 6.1.12-1-default #1 smp preempt_dynamic wed feb 15 05:31:41 utc 2023 (373f017) x86_64 x86_64 x86_64 gnulinux '
    config_args='-Dusedevel -Duse64bitall -Dusethreads -Duseithreads -Duselongdouble -Doptimize='-O0\ -g' -Accflags=-DPERL_RC_STACK -des'
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
  Compiler:
    cc='cc'
    ccflags ='-D_REENTRANT -D_GNU_SOURCE -pie -fPIE -fPIC -DDEBUGGING -DPERL_RC_STACK -fwrapv -fno-strict-aliasing -pipe -fstack-protector-strong -I/pro/local/include -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_FORTIFY_SOURCE=2'
    optimize='-O0 -g'
    cppflags='-D_REENTRANT -D_GNU_SOURCE -pie -fPIE -fPIC -DDEBUGGING -DPERL_RC_STACK -fwrapv -fno-strict-aliasing -pipe -fstack-protector-strong -I/pro/local/include'
    ccversion=''
    gccversion='12.2.1 20230124 [revision 193f7e62815b4089dfaed4c2bd34fd4f10209e27]'
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
    libpth=/usr/local/lib /usr/x86_64-suse-linux/lib /usr/lib /data/pro/local/lib /usr/lib64 /usr/local/lib64
    libs=-lpthread -lgdbm -ldb -ldl -lm -lcrypt -lutil -lc -lgdbm_compat
    perllibs=-lpthread -ldl -lm -lcrypt -lutil -lc
    libc=/lib/../lib64/libc.so.6
    so=so
    useshrplib=false
    libperl=libperl.a
    gnulibc_version='2.37'
  Dynamic Linking:
    dlsrc=dl_dlopen.xs
    dlext=so
    d_dlsymun=undef
    ccdlflags='-Wl,-E'
    cccdlflags='-fPIC'
    lddlflags='-shared -O0 -g -L/pro/local/lib -fstack-protector-strong'


Characteristics of this binary (from libperl): 
  Compile-time options:
    DEBUGGING
    HAS_TIMES
    MULTIPLICITY
    PERLIO_LAYERS
    PERL_COPY_ON_WRITE
    PERL_DONT_CREATE_GVSV
    PERL_HASH_FUNC_SIPHASH13
    PERL_HASH_USE_SBOX32
    PERL_MALLOC_WRAP
    PERL_OP_PARENT
    PERL_PRESERVE_IVUV
    PERL_RC_STACK
    PERL_TRACK_MEMPOOL
    PERL_USE_DEVEL
    PERL_USE_SAFE_PUTENV
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
    uncommitted-changes
  Built under linux
  Compiled at Feb 28 2023 18:30:03
  %ENV:
    PERL5LIB="/pro/3gl/CPAN/perl-git/lib"
    PERL6LIB="inst#/pro/3gl/CPAN/rakudo/install"
    PERL_ARCHLIB="/pro/3gl/CPAN/perl-git"
    PERL_CORE="1"
    PERL_INC="/pro/3gl/CPAN/perl-git"
    PERL_LIB="/pro/3gl/CPAN/perl-git/lib"
    PERL_SRC="/pro/3gl/CPAN/perl-git"
  @INC:
    /pro/3gl/CPAN/perl-git/lib
    /pro/3gl/CPAN/perl-git/lib
    /pro/lib/perl5/site_perl/5.37.10/x86_64-linux-thread-multi-ld
    /pro/lib/perl5/site_perl/5.37.10
    /pro/lib/perl5/5.37.10/x86_64-linux-thread-multi-ld
    /pro/lib/perl5/5.37.10
