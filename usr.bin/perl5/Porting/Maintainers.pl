#!perl
# A simple listing of core files that have specific maintainers,
# or at least someone that can be called an "interested party".
# Also, a "module" does not necessarily mean a CPAN module, it
# might mean a file or files or a subdirectory.
# Most (but not all) of the modules have dual lives in the core
# and in CPAN.

package Maintainers;

use utf8;
use File::Glob qw(:case);

# IGNORABLE: files which, if they appear in the root of a CPAN
# distribution, need not appear in core (i.e. core-cpan-diff won't
# complain if it can't find them)

@IGNORABLE = qw(
    .cvsignore .dualLivedDiffConfig .gitignore .github .perlcriticrc .perltidyrc
    .travis.yml ANNOUNCE Announce Artistic AUTHORS BENCHMARK BUGS Build.PL
    CHANGELOG ChangeLog Changelog CHANGES Changes CONTRIBUTING CONTRIBUTING.md
    CONTRIBUTING.mkdn COPYING Copying cpanfile CREDITS dist.ini GOALS HISTORY
    INSTALL INSTALL.SKIP LICENCE LICENSE Makefile.PL MANIFEST MANIFEST.SKIP
    META.json META.yml MYMETA.json MYMETA.yml NEW NEWS NOTES perlcritic.rc
    ppport.h README README.md README.pod README.PATCHING SIGNATURE THANKS TODO
    Todo VERSION WHATSNEW
);

# Each entry in the  %Modules hash roughly represents a distribution,
# except when DISTRIBUTION is set, where it *exactly* represents a single
# CPAN distribution.

# The keys of %Modules are human descriptions of the distributions, and
# may not exactly match a module or distribution name. Distributions
# which have an obvious top-level module associated with them will usually
# have a key named for that module, e.g. 'Archive::Extract' for
# Archive-Extract-N.NN.tar.gz; the remaining keys are likely to be based
# on the name of the distribution, e.g. 'Locale-Codes' for
# Locale-Codes-N.NN.tar.gz'.

# UPSTREAM indicates where patches should go.  This is generally now
# inferred from the FILES: modules with files in dist/, ext/ and lib/
# are understood to have UPSTREAM 'blead', meaning that the copy of the
# module in the blead sources is to be considered canonical, while
# modules with files in cpan/ are understood to have UPSTREAM 'cpan',
# meaning that the module on CPAN is to be patched first.

# MAINTAINER has previously been used to indicate who the current maintainer
# of the module is, but this is no longer stated explicitly. It is now
# understood to be either the Perl 5 Porters if UPSTREAM is 'blead', or else
# the CPAN author whose PAUSE user ID forms the first part of the DISTRIBUTION
# value, e.g. 'BINGOS' in the case of 'BINGOS/Archive-Tar-2.00.tar.gz'.
# (PAUSE's View Permissions page may be consulted to find other authors who
# have owner or co-maint permissions for the module in question.)

# FILES is a list of filenames, glob patterns, and directory
# names to be recursed down, which collectively generate a complete list
# of the files associated with the distribution.

# BUGS is an email or url to post bug reports.  For modules with
# UPSTREAM => 'blead', use perl5-porters@perl.org.  rt.cpan.org
# appears to automatically provide a URL for CPAN modules; any value
# given here overrides the default:
# http://rt.cpan.org/Public/Dist/Display.html?Name=$ModuleName

# DISTRIBUTION names the tarball on CPAN which (allegedly) the files
# included in core are derived from. Note that the file's version may not
# necessarily match the newest version on CPAN.  (For dist/ distributions,
# which are blead-first, a request should be placed with the releaser(s) to
# upload the corresponding cpan release, and the entry in this file should
# only be updated when that release has been done.)

# MAIN_MODULE names the top-level module by which the tarball is indexed on
# CPAN in cases where this differs from the distribution's key in %Modules.
# (If it is equal then MAIN_MODULE is optional and should be omitted.)

# EXCLUDED is a list of files to be excluded from a CPAN tarball before
# comparing the remaining contents with core. Each item can either be a
# full pathname (eg 't/foo.t') or a pattern (e.g. qr{^t/}).
# It defaults to the empty list.

# CUSTOMIZED is a list of files that have been customized within the
# Perl core.  Use this whenever patching a cpan upstream distribution
# or whenever we expect to have a file that differs from the tarball.
# If the file in blead matches the file in the tarball from CPAN,
# Porting/core-cpan-diff will warn about it, as it indicates an expected
# customization might have been lost when updating from upstream.  The
# path should be relative to the distribution directory.  If the upstream
# distribution should be modified to incorporate the change then be sure
# to raise a ticket for it on rt.cpan.org and add a comment alongside the
# list of CUSTOMIZED files noting the ticket number.

# DEPRECATED contains the *first* version of Perl in which the module
# was considered deprecated.  It should only be present if the module is
# actually deprecated.  Such modules should use deprecate.pm to
# issue a warning if used.  E.g.:
#
#     use if $] >= 5.011, 'deprecate';
#

# MAP is a hash that maps CPAN paths to their core equivalents.
# Each key represents a string prefix, with longest prefixes checked
# first. The first match causes that prefix to be replaced with the
# corresponding key. For example, with the following MAP:
#   {
#     'lib/'     => 'lib/',
#     ''     => 'lib/Foo/',
#   },
#
# these files are mapped as shown:
#
#    README     becomes lib/Foo/README
#    lib/Foo.pm becomes lib/Foo.pm
#
# The default is dependent on the type of module.
# For distributions which appear to be stored under ext/, it defaults to:
#
#   { '' => 'ext/Foo-Bar/' }
#
# otherwise, it's
#
#   {
#     'lib/'     => 'lib/',
#     ''     => 'lib/Foo/Bar/',
#   }

%Modules = (

    'Archive::Tar' => {
        'DISTRIBUTION' => 'BINGOS/Archive-Tar-2.40.tar.gz',
        'FILES'        => q[cpan/Archive-Tar],
        'BUGS'         => 'bug-archive-tar@rt.cpan.org',
        'EXCLUDED'     => [
            qw(t/07_ptardiff.t),
            qr{t/src/(long|short)/foo.txz},
        ],
    },

    'Attribute::Handlers' => {
        'DISTRIBUTION' => 'RJBS/Attribute-Handlers-0.99.tar.gz',
        'FILES'        => q[dist/Attribute-Handlers],
    },

    'autodie' => {
        'DISTRIBUTION' => 'TODDR/autodie-2.36.tar.gz',
        'FILES'        => q[cpan/autodie],
        'EXCLUDED'     => [
            qr{benchmarks},
            qr{README\.md},
            qr{^xt/},
            # All these tests depend upon external
            # modules that don't exist when we're
            # building the core.  Hence, they can
            # never run, and should not be merged.
            qw( t/author-critic.t
                t/critic.t
                t/fork.t
                t/kwalitee.t
                t/lex58.t
                t/pod-coverage.t
                t/pod.t
                t/release-pod-coverage.t
                t/release-pod-syntax.t
                t/socket.t
                t/system.t
                t/no-all.t
                )
        ],
    },

    'AutoLoader' => {
        'DISTRIBUTION' => 'SMUELLER/AutoLoader-5.74.tar.gz',
        'FILES'        => q[cpan/AutoLoader],
        'EXCLUDED'     => ['t/00pod.t'],
        'CUSTOMIZED'   => ['t/02AutoSplit.t'],
    },

    'autouse' => {
        'DISTRIBUTION' => 'RJBS/autouse-1.11.tar.gz',
        'FILES'        => q[dist/autouse],
        'EXCLUDED'     => [qr{^t/release-.*\.t}],
    },

    'base' => {
        'DISTRIBUTION' => 'RJBS/base-2.23.tar.gz',
        'FILES'        => q[dist/base],
    },

    'bignum' => {
        'DISTRIBUTION' => 'PJACKLAM/bignum-0.66.tar.gz',
        'FILES'        => q[cpan/bignum],
        'EXCLUDED'     => [
            qr{^xt/},
            qr{^t/author-},
            qr{^t/release-},
            qw( t/00sig.t
                t/01load.t
                ),
        ],
    },

    'Carp' => {
        'DISTRIBUTION' => 'XSAWYERX/Carp-1.50.tar.gz',
        'FILES'        => q[dist/Carp],
    },

    'Compress::Raw::Bzip2' => {
        'DISTRIBUTION' => 'PMQS/Compress-Raw-Bzip2-2.204.tar.gz',
        'FILES'        => q[cpan/Compress-Raw-Bzip2],
        'EXCLUDED'     => [
            qr{^t/Test/},
            qr{^t/meta},
            'bzip2-src/bzip2-const.patch',
            'bzip2-src/bzip2-cpp.patch',
            'bzip2-src/bzip2-unsigned.patch',
        ],
        'CUSTOMIZED'   => [
            # https://github.com/pmqs/Compress-Raw-Bzip2/issues/11
            'Bzip2.xs',
            'lib/Compress/Raw/Bzip.pm'
        ],
    },

    'Compress::Raw::Zlib' => {
        'DISTRIBUTION' => 'PMQS/Compress-Raw-Zlib-2.204.tar.gz',
        'FILES'    => q[cpan/Compress-Raw-Zlib],
        'EXCLUDED' => [
            qr{^examples/},
            qr{^t/Test/},
            qr{^t/meta},
            qw( t/000prereq.t
                t/99pod.t
                ),
        ],
        'CUSTOMIZED' => [
            # https://github.com/pmqs/Compress-Raw-Zlib/issues/23
            'lib/Compress/Raw/Zlib.pm',
            'Zlib.xs'
        ],
    },

    'Config::Perl::V' => {
        'DISTRIBUTION' => 'HMBRAND/Config-Perl-V-0.36.tgz',
        'SYNCINFO'     => 'yorton on Sat Mar  4 10:43:06 2023',
        'FILES'        => q[cpan/Config-Perl-V],
        'EXCLUDED'     => [qw(
		examples/show-v.pl
		)],
    },

    'constant' => {
        'DISTRIBUTION' => 'RJBS/constant-1.33.tar.gz',
        'FILES'        => q[dist/constant],
        'EXCLUDED'     => [
            qw( t/00-load.t
                t/more-tests.t
                t/pod-coverage.t
                t/pod.t
                eg/synopsis.pl
                ),
        ],
    },

    'CPAN' => {
        'DISTRIBUTION' => 'ANDK/CPAN-2.36.tar.gz',
        'FILES'        => q[cpan/CPAN],
        'EXCLUDED'     => [
            qr{^distroprefs/},
            qr{^inc/Test/},
            qr{^t/CPAN/},
            qr{^t/data/},
            qr{^t/97-},
            qw( lib/CPAN/Admin.pm
                scripts/cpan-mirrors
                PAUSE2015.pub
                PAUSE2019.pub
                PAUSE2021.pub
                SlayMakefile
                t/00signature.t
                t/04clean_load.t
                t/12cpan.t
                t/13tarzip.t
                t/14forkbomb.t
                t/30shell.coverage
                t/30shell.t
                t/31sessions.t
                t/41distribution.t
                t/42distroprefs.t
                t/43distroprefspref.t
                t/44cpanmeta.t
                t/50pod.t
                t/51pod.t
                t/52podcover.t
                t/60credentials.t
                t/70_critic.t
                t/71_minimumversion.t
                t/local_utils.pm
                t/perlcriticrc
                t/yaml_code.yml
                ),
        ],
    },

    # Note: When updating CPAN-Meta the META.* files will need to be regenerated
    # perl -Icpan/CPAN-Meta/lib Porting/makemeta
    'CPAN::Meta' => {
        'DISTRIBUTION' => 'DAGOLDEN/CPAN-Meta-2.150010.tar.gz',
        'FILES'        => q[cpan/CPAN-Meta],
        'EXCLUDED'     => [
            qw[t/00-report-prereqs.t
               t/00-report-prereqs.dd
              ],
            qr{^xt},
            qr{^history},
        ],
    },

    'CPAN::Meta::Requirements' => {
        'DISTRIBUTION' => 'DAGOLDEN/CPAN-Meta-Requirements-2.140.tar.gz',
        'FILES'        => q[cpan/CPAN-Meta-Requirements],
        'EXCLUDED'     => [
            qw(t/00-report-prereqs.t),
            qw(t/00-report-prereqs.dd),
            qw(t/version-cleanup.t),
            qr{^xt},
        ],
    },

    'CPAN::Meta::YAML' => {
        'DISTRIBUTION' => 'DAGOLDEN/CPAN-Meta-YAML-0.018.tar.gz',
        'FILES'        => q[cpan/CPAN-Meta-YAML],
        'EXCLUDED'     => [
            't/00-report-prereqs.t',
            't/00-report-prereqs.dd',
            qr{^xt},
        ],
    },

    'Data::Dumper' => {
        'DISTRIBUTION' => 'NWCLARK/Data-Dumper-2.183.tar.gz',
        'FILES'        => q[dist/Data-Dumper],
    },

    'DB_File' => {
        'DISTRIBUTION' => 'PMQS/DB_File-1.858.tar.gz',
        'FILES'        => q[cpan/DB_File],
        'EXCLUDED'     => [
            qr{^patches/},
            qr{^t/meta},
            qw( t/pod.t
                t/000prereq.t
                fallback.h
                fallback.xs
                ),
        ],
    },

    'Devel::PPPort' => {
        'DISTRIBUTION' => 'ATOOMIC/Devel-PPPort-3.68.tar.gz',
        'FILES'        => q[dist/Devel-PPPort],
        'EXCLUDED'     => [
            'PPPort.pm',    # we use PPPort_pm.PL instead
        ],
    },

    'Devel::SelfStubber' => {
        'DISTRIBUTION' => 'FLORA/Devel-SelfStubber-1.05.tar.gz',
        'FILES'        => q[dist/Devel-SelfStubber],
        'EXCLUDED'     => [qr{^t/release-.*\.t}],
    },

    'Digest' => {
        'DISTRIBUTION' => 'TODDR/Digest-1.20.tar.gz',
        'FILES'        => q[cpan/Digest],
        'EXCLUDED'     => ['digest-bench'],
    },

    'Digest::MD5' => {
        'DISTRIBUTION' => 'TODDR/Digest-MD5-2.58.tar.gz',
        'FILES'        => q[cpan/Digest-MD5],
        'EXCLUDED'     => [ 'rfc1321.txt', 'bin/md5sum.pl' ],
        'CUSTOMIZED'   => [
            # https://github.com/Dual-Life/digest-md5/pull/24
            'MD5.pm',
            'MD5.xs'
        ],
    },

    'Digest::SHA' => {
        'DISTRIBUTION' => 'MSHELOR/Digest-SHA-6.04.tar.gz',
        'FILES'        => q[cpan/Digest-SHA],
        'EXCLUDED'     => [
            qw( t/pod.t
                t/podcover.t
                examples/dups
                ),
        ],
    },

    'Dumpvalue' => {
        'DISTRIBUTION' => 'FLORA/Dumpvalue-1.17.tar.gz',
        'FILES'        => q[dist/Dumpvalue],
        'EXCLUDED'     => [qr{^t/release-.*\.t}],
    },

    'Encode' => {
        'DISTRIBUTION' => 'DANKOGAI/Encode-3.19.tar.gz',
        'FILES'        => q[cpan/Encode],
        'EXCLUDED'     => [
            qw( t/whatwg-aliases.json
                t/whatwg-aliases.t
                ),
        ],
    },

    'encoding::warnings' => {
        'DISTRIBUTION' => 'AUDREYT/encoding-warnings-0.11.tar.gz',
        'FILES'        => q[dist/encoding-warnings],
        'EXCLUDED'     => [
            qr{^inc/Module/},
            qw(t/0-signature.t),
        ],
    },

    'Env' => {
        'DISTRIBUTION' => 'FLORA/Env-1.04.tar.gz',
        'FILES'        => q[dist/Env],
        'EXCLUDED'     => [qr{^t/release-.*\.t}],
    },

    'experimental' => {
        'DISTRIBUTION' => 'LEONT/experimental-0.031.tar.gz',
        'FILES'        => q[cpan/experimental],
        'EXCLUDED'     => [qr{^xt/}],
    },

    'Exporter' => {
        'DISTRIBUTION' => 'TODDR/Exporter-5.77.tar.gz',
        'FILES'        => q[dist/Exporter],
        'EXCLUDED' => [
            qw( t/pod.t
                t/use.t
                ),
        ],
    },

    'ExtUtils::CBuilder' => {
        'DISTRIBUTION' => 'AMBS/ExtUtils-CBuilder-0.280236.tar.gz',
        'FILES'        => q[dist/ExtUtils-CBuilder],
        'EXCLUDED'     => [
            qw(README.mkdn),
            qr{^xt},
        ],
    },

    'ExtUtils::Constant' => {

        'DISTRIBUTION' => 'NWCLARK/ExtUtils-Constant-0.25.tar.gz',
        'FILES'    => q[cpan/ExtUtils-Constant],
        'CUSTOMIZED' => [
             # https://rt.cpan.org/Ticket/Display.html?id=142181
            'lib/ExtUtils/Constant/Base.pm',

            # https://rt.cpan.org/Public/Bug/Display.html?id=132995
            # https://rt.cpan.org/Ticket/Display.html?id=142183
            't/Constant.t',
        ],
        'EXCLUDED' => [
            qw( lib/ExtUtils/Constant/Aaargh56Hash.pm
                examples/perl_keyword.pl
                examples/perl_regcomp_posix_keyword.pl
                ),
        ],
    },

    'ExtUtils::Install' => {
        'DISTRIBUTION' => 'BINGOS/ExtUtils-Install-2.22.tar.gz',
        'FILES'        => q[cpan/ExtUtils-Install],
        'EXCLUDED'     => [
            qw( t/lib/Test/Builder.pm
                t/lib/Test/Builder/Module.pm
                t/lib/Test/More.pm
                t/lib/Test/Simple.pm
                t/pod-coverage.t
                t/pod.t
                ),
        ],
    },

    'ExtUtils::MakeMaker' => {
        'DISTRIBUTION' => 'BINGOS/ExtUtils-MakeMaker-7.70.tar.gz',
        'SYNCINFO'     => 'yorton on Sun Mar 26 16:20:23 2023',
        'FILES'        => q[cpan/ExtUtils-MakeMaker],
        'EXCLUDED'     => [
            qr{^t/lib/Test/},
            qr{^(bundled|my)/},
            qr{^t/Liblist_Kid.t},
            qr{^t/liblist/},
            qr{^\.perlcriticrc},
            'PATCHING',
            'README.packaging',
            'lib/ExtUtils/MakeMaker/version/vpp.pm',
        ],
    },

    'ExtUtils::PL2Bat' => {
        'DISTRIBUTION' => 'LEONT/ExtUtils-PL2Bat-0.005.tar.gz',
        'FILES'        => q[cpan/ExtUtils-PL2Bat],
        'EXCLUDED'     => [
            't/00-compile.t',
            'script/pl2bat.pl'
        ],
    },

    'ExtUtils::Manifest' => {
        'DISTRIBUTION' => 'ETHER/ExtUtils-Manifest-1.73.tar.gz',
        'FILES'        => q[cpan/ExtUtils-Manifest],
        'EXCLUDED'     => [
            qr(^t/00-report-prereqs),
            qr(^xt/)
        ],
    },

    'ExtUtils::ParseXS' => {
        'DISTRIBUTION' => 'XSAWYERX/ExtUtils-ParseXS-3.44.tar.gz',
        'FILES'        => q[dist/ExtUtils-ParseXS],
    },

    'File::Fetch' => {
        'DISTRIBUTION' => 'BINGOS/File-Fetch-1.04.tar.gz',
        'FILES'        => q[cpan/File-Fetch],
    },

    'File::Path' => {
        'DISTRIBUTION' => 'JKEENAN/File-Path-2.18.tar.gz',
        'FILES'        => q[cpan/File-Path],
        'EXCLUDED'     => [
            qw(t/Path-Class.t),
            qr{^xt/},
        ],
    },

    'File::Temp' => {
        'DISTRIBUTION' => 'ETHER/File-Temp-0.2311.tar.gz',
        'FILES'        => q[cpan/File-Temp],
        'EXCLUDED'     => [
            qw( README.mkdn
                misc/benchmark.pl
                misc/results.txt
                ),
            qr[^t/00-report-prereqs],
            qr{^xt},
        ],
    },

    'Filter::Simple' => {
        'DISTRIBUTION' => 'SMUELLER/Filter-Simple-0.94.tar.gz',
        'FILES'        => q[dist/Filter-Simple],
        'EXCLUDED'     => [
            qr{^demo/}
        ],
    },

    'Filter::Util::Call' => {
        'DISTRIBUTION' => 'RURBAN/Filter-1.64.tar.gz',
        'FILES'        => q[cpan/Filter-Util-Call
                 pod/perlfilter.pod
                ],
        'EXCLUDED' => [
            qr{^decrypt/},
            qr{^examples/},
            qr{^Exec/},
            qr{^lib/Filter/},
            qr{^tee/},
            qw( .appveyor.yml
                .cirrus.yml
                .github/workflows/testsuite.yml
                .whitesource
                Call/Makefile.PL
                Call/ppport.h
                Call/typemap
                mytest
                t/cpp.t
                t/decrypt.t
                t/exec.t
                t/m4.t
                t/order.t
                t/sh.t
                t/tee.t
                t/z_kwalitee.t
                t/z_manifest.t
                t/z_meta.t
                t/z_perl_minimum_version.t
                t/z_pod-coverage.t
                t/z_pod.t
                ),
        ],
        'MAP' => {
            'Call/'            => 'cpan/Filter-Util-Call/',
            't/filter-util.pl' => 'cpan/Filter-Util-Call/filter-util.pl',
            'perlfilter.pod'   => 'pod/perlfilter.pod',
            ''                 => 'cpan/Filter-Util-Call/',
        },
        'CUSTOMIZED'   => [
            qw(pod/perlfilter.pod)
        ],
    },

    'FindBin' => {
        'DISTRIBUTION' => 'TODDR/FindBin-1.53.tar.gz',
        'FILES'        => q[dist/FindBin],
    },

    'Getopt::Long' => {
        'DISTRIBUTION' => 'JV/Getopt-Long-2.54.tar.gz',
        'FILES'        => q[cpan/Getopt-Long],
        'EXCLUDED'     => [
            qr{^examples/},
            qw( lib/newgetopt.pl
                t/gol-compat.t
                ),
        ],
    },

    'HTTP::Tiny' => {
        'DISTRIBUTION' => 'DAGOLDEN/HTTP-Tiny-0.086.tar.gz',
        'FILES'        => q[cpan/HTTP-Tiny],
        'EXCLUDED'     => [
            'corpus/snake-oil.crt', # only used by 210_live_ssl.t
            't/00-report-prereqs.t',
            't/00-report-prereqs.dd',
            't/200_live.t',
            't/200_live_local_ip.t',
            't/210_live_ssl.t',
            qr/^eg/,
            qr/^xt/
        ],
    },

    'I18N::Collate' => {
        'DISTRIBUTION' => 'FLORA/I18N-Collate-1.02.tar.gz',
        'FILES'        => q[dist/I18N-Collate],
        'EXCLUDED'     => [qr{^t/release-.*\.t}],
    },

    'I18N::LangTags' => {
        'FILES'        => q[dist/I18N-LangTags],
    },

    'if' => {
        'DISTRIBUTION' => 'XSAWYERX/if-0.0608.tar.gz',
        'FILES'        => q[dist/if],
    },

    'IO' => {
        'DISTRIBUTION' => 'TODDR/IO-1.51.tar.gz',
        'FILES'        => q[dist/IO],
        'EXCLUDED'     => ['t/test.pl'],
    },

    'IO-Compress' => {
        'DISTRIBUTION' => 'PMQS/IO-Compress-2.204.tar.gz',
        'MAIN_MODULE'  => 'IO::Compress::Base',
        'FILES'        => q[cpan/IO-Compress],
        'EXCLUDED'     => [
            qr{^examples/},
            qr{^t/Test/},
            qr{^t/999meta-},
            't/010examples-bzip2.t',
            't/010examples-zlib.t',
            't/cz-05examples.t',
        ],
    },

    'IO::Socket::IP' => {
        'DISTRIBUTION' => 'PEVANS/IO-Socket-IP-0.41.tar.gz',
        'FILES'        => q[cpan/IO-Socket-IP],
        'EXCLUDED'     => [
            qr{^examples/},
        ],
        'CUSTOMIZED' => [
            # https://rt.cpan.org/Ticket/Display.html?id=148293
            'lib/IO/Socket/IP.pm'
        ],
    },

    'IO::Zlib' => {
        'DISTRIBUTION' => 'TOMHUGHES/IO-Zlib-1.14.tar.gz',
        'FILES'        => q[cpan/IO-Zlib],
    },

    'IPC::Cmd' => {
        'DISTRIBUTION' => 'BINGOS/IPC-Cmd-1.04.tar.gz',
        'FILES'        => q[cpan/IPC-Cmd],
    },

    'IPC::SysV' => {
        'DISTRIBUTION' => 'MHX/IPC-SysV-2.09.tar.gz',
        'FILES'        => q[cpan/IPC-SysV],
        'EXCLUDED'     => [
            qw( const-c.inc
                const-xs.inc
                ),
        ],
    },

    'JSON::PP' => {
        'DISTRIBUTION' => 'ISHIGAKI/JSON-PP-4.16.tar.gz',
        'FILES'        => q[cpan/JSON-PP],
    },

    'lib' => {
        'DISTRIBUTION' => 'SMUELLER/lib-0.63.tar.gz',
        'FILES'        => q[dist/lib],
        'EXCLUDED'     => [
            qw( forPAUSE/lib.pm
                t/00pod.t
                ),
        ],
    },

    'libnet' => {
        'DISTRIBUTION' => 'SHAY/libnet-3.15.tar.gz',
        'SYNCINFO'     => 'yorton on Mon Mar 20 20:48:38 2023',
        'MAIN_MODULE'  => 'Net::Cmd',
        'FILES'        => q[cpan/libnet],
        'EXCLUDED'     => [
            qw( Configure
                t/changes.t
                t/critic.t
                t/pod.t
                t/pod_coverage.t
                ),
            qr(^demos/),
            qr(^t/external/),
        ],
    },

    'Locale::Maketext' => {
        'DISTRIBUTION' => 'TODDR/Locale-Maketext-1.32.tar.gz',
        'FILES'        => q[dist/Locale-Maketext],
        'EXCLUDED'     => [
            qw(
                perlcriticrc
                t/00_load.t
                t/pod.t
                ),
        ],
    },

    'Locale::Maketext::Simple' => {
        'DISTRIBUTION' => 'JESSE/Locale-Maketext-Simple-0.21.tar.gz',
        'FILES'        => q[cpan/Locale-Maketext-Simple],
        'CUSTOMIZED'   => [
            # CVE-2016-1238
            qw( lib/Locale/Maketext/Simple.pm )
        ],
    },

    'Math::BigInt' => {
        'DISTRIBUTION' => 'PJACKLAM/Math-BigInt-1.999837.tar.gz',
        'FILES'        => q[cpan/Math-BigInt],
        'EXCLUDED'     => [
            qr{^xt/},
            qr{^examples/},
            qr{^t/author-},
            qr{^t/release-},
            qw( t/00sig.t
                t/01load.t
                ),
        ],
    },

    'Math::BigInt::FastCalc' => {
        'DISTRIBUTION' => 'PJACKLAM/Math-BigInt-FastCalc-0.5013.tar.gz',
        'FILES'        => q[cpan/Math-BigInt-FastCalc],
        'EXCLUDED'     => [
            qr{^inc/},
            qr{^xt/},
            qr{^t/author-},
            qr{^t/release-},
            qr{^t/Math/BigInt/Lib/TestUtil.pm},
            qw( t/00sig.t
                t/01load.t
                ),

            # instead we use the versions of these test
            # files that come with Math::BigInt:
            qw( t/bigfltpm.inc
                t/bigfltpm.t
                t/bigintpm.inc
                t/bigintpm.t
                t/mbimbf.inc
                t/mbimbf.t
                ),
        ],
    },

    'Math::BigRat' => {
        'DISTRIBUTION' => 'PJACKLAM/Math-BigRat-0.2624.tar.gz',
        'FILES'        => q[cpan/Math-BigRat],
        'EXCLUDED'     => [
            qr{^xt/},
            qr{^math-bigrat-pod.diff},
            qr{^t/author-},
            qr{^t/release-},
            qw( t/00sig.t
                t/01load.t
                ),
        ],
    },

    'Math::Complex' => {
        'DISTRIBUTION' => 'ZEFRAM/Math-Complex-1.59.tar.gz',
        'FILES'        => q[dist/Math-Complex],
        'EXCLUDED'     => [
            qw( t/pod.t
                t/pod-coverage.t
                ),
        ],
    },

    'Memoize' => {
        'DISTRIBUTION' => 'ARISTOTLE/Memoize-1.16.tar.gz',
        'FILES'        => q[cpan/Memoize],
        'EXCLUDED'     => [ qr{^inc/} ],
    },

    'MIME::Base64' => {
        'DISTRIBUTION' => 'CAPOEIRAB/MIME-Base64-3.16.tar.gz',
        'FILES'        => q[cpan/MIME-Base64],
        'EXCLUDED'     => [ qr{^xt/}, 'benchmark', 'benchmark-qp', qr{^t/00-report-prereqs} ],
        'CUSTOMIZED'   => [
            # https://github.com/Dual-Life/mime-base64/pull/17
            'Base64.xs',
            'lib/MIME/Base64.pm'
        ],
    },

    'Module::CoreList' => {
        'DISTRIBUTION' => 'BINGOS/Module-CoreList-5.20230320.tar.gz',
        'SYNCINFO'     => 'yorton on Tue Mar 21 11:46:02 2023',
        'FILES'        => q[dist/Module-CoreList],
    },

    'Module::Load' => {
        'DISTRIBUTION' => 'BINGOS/Module-Load-0.36.tar.gz',
        'FILES'        => q[cpan/Module-Load],
    },

    'Module::Load::Conditional' => {
        'DISTRIBUTION' => 'BINGOS/Module-Load-Conditional-0.74.tar.gz',
        'FILES'        => q[cpan/Module-Load-Conditional],
    },

    'Module::Loaded' => {
        'DISTRIBUTION' => 'BINGOS/Module-Loaded-0.08.tar.gz',
        'FILES'        => q[cpan/Module-Loaded],
    },

    'Module::Metadata' => {
        'DISTRIBUTION' => 'ETHER/Module-Metadata-1.000037.tar.gz',
        'FILES'        => q[cpan/Module-Metadata],
        'EXCLUDED'     => [
            qw(t/00-report-prereqs.t),
            qw(t/00-report-prereqs.dd),
            qr{weaver.ini},
            qr{^xt},
        ],
    },

    'Net::Ping' => {
        'DISTRIBUTION' => 'RURBAN/Net-Ping-2.75.tar.gz',
        'FILES'        => q[dist/Net-Ping],
        'EXCLUDED'     => [
            qr{^\.[awc]},
            qw(README.md.PL),
            qw(t/020_external.t),
            qw(t/600_pod.t),
            qw(t/601_pod-coverage.t),
            qw(t/602_kwalitee.t),
            qw(t/603_meta.t),
            qw(t/604_manifest.t),
            qw(t/appveyor-test.bat),

        ],
    },

    'NEXT' => {
        'DISTRIBUTION' => 'NEILB/NEXT-0.69.tar.gz',
        'FILES'        => q[cpan/NEXT],
        'EXCLUDED'     => [qr{^demo/}],
    },

    'Params::Check' => {
        'DISTRIBUTION' => 'BINGOS/Params-Check-0.38.tar.gz',
        'FILES'        => q[cpan/Params-Check],
    },

    'parent' => {
        'DISTRIBUTION' => 'CORION/parent-0.241.tar.gz',
        'FILES'        => q[cpan/parent],
        'EXCLUDED'     => [
            qr{^xt}
        ],
    },

    'PathTools' => {
        'DISTRIBUTION' => 'XSAWYERX/PathTools-3.75.tar.gz',
        'MAIN_MODULE'  => 'File::Spec',
        'FILES'        => q[dist/PathTools],
        'EXCLUDED'     => [
            qr{^t/lib/Test/},
            qw( t/rel2abs_vs_symlink.t),
        ],
    },

    'Perl::OSType' => {
        'DISTRIBUTION' => 'DAGOLDEN/Perl-OSType-1.010.tar.gz',
        'FILES'        => q[cpan/Perl-OSType],
        'EXCLUDED'     => [qw(tidyall.ini), qr/^xt/, qr{^t/00-}],
    },

    'perlfaq' => {
        'DISTRIBUTION' => 'ETHER/perlfaq-5.20210520.tar.gz',
        'FILES'        => q[cpan/perlfaq],
        'EXCLUDED'     => [ qr/^inc/, qr/^xt/, qr{^t/00-} ],
    },

    'PerlIO::via::QuotedPrint' => {
        'DISTRIBUTION' => 'SHAY/PerlIO-via-QuotedPrint-0.10.tar.gz',
        'FILES'        => q[cpan/PerlIO-via-QuotedPrint],
    },

    'Pod::Checker' => {
        'DISTRIBUTION' => 'MAREKR/Pod-Checker-1.75.tar.gz',
        'FILES'        => q[cpan/Pod-Checker],
    },

    'Pod::Escapes' => {
        'DISTRIBUTION' => 'NEILB/Pod-Escapes-1.07.tar.gz',
        'FILES'        => q[cpan/Pod-Escapes],
    },

    'Pod::Perldoc' => {
        'DISTRIBUTION' => 'MALLEN/Pod-Perldoc-3.28.tar.gz',
        'FILES'        => q[cpan/Pod-Perldoc],

        # Note that we use the CPAN-provided Makefile.PL, since it
        # contains special handling of the installation of perldoc.pod

        'EXCLUDED' => [
            # In blead, the perldoc executable is generated by perldoc.PL
            # instead
            # XXX We can and should fix this, but clean up the DRY-failure in
            # utils first
            'perldoc',

            # https://rt.cpan.org/Ticket/Display.html?id=116827
            't/02_module_pod_output.t'
        ],

        'CUSTOMIZED'   => [
	    # [rt.cpan.org #88204], [rt.cpan.org #120229]
	    'lib/Pod/Perldoc.pm',
	],
    },

    'Pod::Simple' => {
        'DISTRIBUTION' => 'KHW/Pod-Simple-3.43.tar.gz',
        'FILES'        => q[cpan/Pod-Simple],
        'EXCLUDED' => [
            qw{.ChangeLog.swp},
            qr{^\.github/}
	],
    },

    'Pod::Usage' => {
        'DISTRIBUTION' => 'MAREKR/Pod-Usage-2.03.tar.gz',
        'FILES'        => q[cpan/Pod-Usage],
        'EXCLUDED' => [
            qr{^t/00-},
            qr{^xt/}
	],
    },

    'podlators' => {
        'DISTRIBUTION' => 'RRA/podlators-5.01.tar.gz',
        'MAIN_MODULE'  => 'Pod::Man',
        'FILES'        => q[cpan/podlators pod/perlpodstyle.pod],
        'EXCLUDED'     => [
            qr{^\.github/dependabot\.yml},
            qr{^\.github/workflows/build\.yaml},
        ],

        'MAP' => {
            ''                 => 'cpan/podlators/',
            # this file lives outside the cpan/ directory
            'pod/perlpodstyle.pod' => 'pod/perlpodstyle.pod',
        },
    },

    'Safe' => {
        'DISTRIBUTION' => 'RGARCIA/Safe-2.35.tar.gz',
        'FILES'        => q[dist/Safe],
    },

    'Scalar::Util' => {
        'DISTRIBUTION' => 'PEVANS/Scalar-List-Utils-1.63.tar.gz',
        'FILES'        => q[cpan/Scalar-List-Utils],
    },

    'Search::Dict' => {
        'DISTRIBUTION' => 'DAGOLDEN/Search-Dict-1.07.tar.gz',
        'FILES'        => q[dist/Search-Dict],
    },

    'SelfLoader' => {
        'DISTRIBUTION' => 'SMUELLER/SelfLoader-1.24.tar.gz',
        'FILES'        => q[dist/SelfLoader],
        'EXCLUDED'     => ['t/00pod.t'],
    },

    'Socket' => {
        'DISTRIBUTION' => 'PEVANS/Socket-2.036.tar.gz',
        'FILES'        => q[cpan/Socket],
    },

    'Storable' => {
        'DISTRIBUTION' => 'NWCLARK/Storable-3.25.tar.gz',
        'FILES'        => q[dist/Storable],
        'EXCLUDED'     => [
            qr{^t/compat/},
        ],
    },

    'Sys::Syslog' => {
        'DISTRIBUTION' => 'SAPER/Sys-Syslog-0.36.tar.gz',
        'FILES'        => q[cpan/Sys-Syslog],
        'EXCLUDED'     => [
            qr{^eg/},
            qw( README.win32
                t/data-validation.t
                t/distchk.t
                t/pod.t
                t/podcover.t
                t/podspell.t
                t/portfs.t
                win32/PerlLog.RES
                ),
        ],
    },

    'Term::ANSIColor' => {
        'DISTRIBUTION' => 'RRA/Term-ANSIColor-5.01.tar.gz',
        'FILES'        => q[cpan/Term-ANSIColor],
        'EXCLUDED'     => [
            qr{^docs/},
            qr{^examples/},
            qr{^t/data/},
            qr{^t/docs/},
            qr{^t/style/},
            qw( t/module/aliases-env.t ),
        ],
    },

    'Term::Cap' => {
        'DISTRIBUTION' => 'JSTOWE/Term-Cap-1.18.tar.gz',
        'FILES'        => q[cpan/Term-Cap],
    },

    'Term::Complete' => {
        'DISTRIBUTION' => 'FLORA/Term-Complete-1.402.tar.gz',
        'FILES'        => q[dist/Term-Complete],
        'EXCLUDED'     => [qr{^t/release-.*\.t}],
    },

    'Term::ReadLine' => {
        'DISTRIBUTION' => 'FLORA/Term-ReadLine-1.14.tar.gz',
        'FILES'        => q[dist/Term-ReadLine],
        'EXCLUDED'     => [qr{^t/release-.*\.t}],
    },

    'Test' => {
        'DISTRIBUTION' => 'JESSE/Test-1.26.tar.gz',
        'FILES'        => q[dist/Test],
    },

    'Test::Harness' => {
        'DISTRIBUTION' => 'LEONT/Test-Harness-3.44.tar.gz',
        'FILES'        => q[cpan/Test-Harness],
        'EXCLUDED'     => [
            qr{^examples/},
            qr{^xt/},
            qw( Changes-2.64
                MANIFEST.CUMMULATIVE
                HACKING.pod
                perlcriticrc
                t/000-load.t
                t/lib/if.pm
                ),
        ],
        'CUSTOMIZED' => [
            't/harness.t'
        ]
    },

    'Test::Simple' => {
        'DISTRIBUTION' => 'EXODIST/Test-Simple-1.302194.tar.gz',
        'SYNCINFO'     => 'yorton on Tue Mar 14 13:43:38 2023',
        'SYNCINFO'     => 'yorton on Mon Mar  6 19:16:42 2023',
        'FILES'        => q[cpan/Test-Simple],
        'EXCLUDED'     => [
            qr{^examples/},
            qr{^xt/},
            qw( appveyor.yml
                t/00compile.t
                t/00-report.t
                t/zzz-check-breaks.t
                ),
        ],
    },

    'Text::Abbrev' => {
        'DISTRIBUTION' => 'FLORA/Text-Abbrev-1.02.tar.gz',
        'FILES'        => q[dist/Text-Abbrev],
        'EXCLUDED'     => [qr{^t/release-.*\.t}],
    },

    'Text::Balanced' => {
        'DISTRIBUTION' => 'SHAY/Text-Balanced-2.06.tar.gz',
        'FILES'        => q[cpan/Text-Balanced],
    },

    'Text::ParseWords' => {
        'DISTRIBUTION' => 'NEILB/Text-ParseWords-3.31.tar.gz',
        'FILES'        => q[cpan/Text-ParseWords],
    },

    'Text-Tabs+Wrap' => {
        'DISTRIBUTION' => 'ARISTOTLE/Text-Tabs+Wrap-2021.0814.tar.gz',
        'MAIN_MODULE'  => 'Text::Tabs',
        'FILES'        => q[cpan/Text-Tabs],
        'EXCLUDED'   => [
            qr{^xt},

        ],
        'MAP'          => {
            ''                        => 'cpan/Text-Tabs/',
            'lib.modern/Text/Tabs.pm' => 'cpan/Text-Tabs/lib/Text/Tabs.pm',
            'lib.modern/Text/Wrap.pm' => 'cpan/Text-Tabs/lib/Text/Wrap.pm',
        },
    },

    # Jerry Hedden does take patches that are applied to blead first, even
    # though that can be hard to discern from the Git history; so it's
    # correct for this (and Thread::Semaphore, threads, and threads::shared)
    # to be under dist/ rather than cpan/
    'Thread::Queue' => {
        'DISTRIBUTION' => 'JDHEDDEN/Thread-Queue-3.13.tar.gz',
        'FILES'        => q[dist/Thread-Queue],
        'EXCLUDED'     => [
            qr{^examples/},
            qw( t/00_load.t
                t/99_pod.t
                t/test.pl
                ),
        ],
    },

    'Thread::Semaphore' => {
        'DISTRIBUTION' => 'JDHEDDEN/Thread-Semaphore-2.13.tar.gz',
        'FILES'        => q[dist/Thread-Semaphore],
        'EXCLUDED'     => [
            qw( examples/semaphore.pl
                t/00_load.t
                t/99_pod.t
                t/test.pl
                ),
        ],
    },

    'threads' => {
        'DISTRIBUTION' => 'JDHEDDEN/threads-2.21.tar.gz',
        'FILES'        => q[dist/threads],
        'EXCLUDED'     => [
            qr{^examples/},
            qw( t/pod.t
                t/test.pl
                ),
        ],
    },

    'threads::shared' => {
        'DISTRIBUTION' => 'JDHEDDEN/threads-shared-1.59.tar.gz',
        'FILES'        => q[dist/threads-shared],
        'EXCLUDED'     => [
            qw( examples/class.pl
                t/pod.t
                t/test.pl
                ),
        ],
    },

    'Tie::File' => {
        'DISTRIBUTION' => 'TODDR/Tie-File-1.07.tar.gz',
        'FILES'        => q[dist/Tie-File],
    },

    'Tie::RefHash' => {
        'DISTRIBUTION' => 'ETHER/Tie-RefHash-1.40.tar.gz',
        'FILES'        => q[cpan/Tie-RefHash],
        'EXCLUDED'     => [
            qr{^t/00-},
            qr{^xt/},
        ],
    },

    'Time::HiRes' => {
        'DISTRIBUTION' => 'ATOOMIC/Time-HiRes-1.9764.tar.gz',
        'FILES'        => q[dist/Time-HiRes],
    },

    'Time::Local' => {
        'DISTRIBUTION' => 'DROLSKY/Time-Local-1.30.tar.gz',
        'FILES'        => q[cpan/Time-Local],
        'EXCLUDED'     => [
            qr{^xt/},
            qw( CODE_OF_CONDUCT.md
                azure-pipelines.yml
                perlcriticrc
                perltidyrc
                tidyall.ini
                t/00-report-prereqs.t
                t/00-report-prereqs.dd
                ),
        ],
    },

    'Time::Piece' => {
        'DISTRIBUTION' => 'ESAYM/Time-Piece-1.3401.tar.gz',
        'FILES'        => q[cpan/Time-Piece],
        'EXCLUDED'     => [ qw[reverse_deps.txt] ],
        'CUSTOMIZED'   => [
            # https://github.com/Dual-Life/Time-Piece/pull/64
            'Piece.pm',
            'Piece.xs'
         ],
    },

    'Unicode::Collate' => {
        'DISTRIBUTION' => 'SADAHIRO/Unicode-Collate-1.31.tar.gz',
        'FILES'        => q[cpan/Unicode-Collate],
        'EXCLUDED'     => [
            qr{N$},
            qr{^data/},
            qr{^gendata/},
            qw( disableXS
                enableXS
                mklocale
                ),
        ],
    },

    'Unicode::Normalize' => {
        'DISTRIBUTION' => 'KHW/Unicode-Normalize-1.26.tar.gz',
        'FILES'        => q[dist/Unicode-Normalize],
        'EXCLUDED'     => [
            qw( MANIFEST.N
                Normalize.pmN
                disableXS
                enableXS
                ),
        ],
    },

    'version' => {
        'DISTRIBUTION' => 'LEONT/version-0.9929.tar.gz',
        'FILES'        => q[cpan/version vutil.c vutil.h vxs.inc],
        'EXCLUDED' => [
            qr{^vutil/lib/},
            'vutil/Makefile.PL',
            'vutil/ppport.h',
            'vutil/vxs.xs',
            't/00impl-pp.t',
            't/survey_locales',
            'vperl/vpp.pm',
        ],

        # When adding the CPAN-distributed files for version.pm, it is necessary
        # to delete an entire block out of lib/version.pm, since that code is
        # only necessary with the CPAN release.
        'CUSTOMIZED'   => [
            'lib/version.pm',

            't/07locale.t'
         ],

        'MAP' => {
            'vutil/'         => '',
            ''               => 'cpan/version/',
        },
    },

    'warnings' => {
        'FILES'      => q[
                 lib/warnings
                 lib/warnings.{pm,t}
                 regen/warnings.pl
                 t/lib/warnings
        ],
    },

    'Win32' => {
        'DISTRIBUTION' => "JDB/Win32-0.59.tar.gz",
        'FILES'        => q[cpan/Win32],
    },

    'Win32API::File' => {
        'DISTRIBUTION' => 'CHORNY/Win32API-File-0.1203.tar.gz',
        'FILES'        => q[cpan/Win32API-File],
        'EXCLUDED'     => [
            qr{^ex/},
        ],
        # https://rt.cpan.org/Ticket/Display.html?id=127837
        'CUSTOMIZED'   => [
            qw( File.pm
                File.xs
                ),
        ],
    },

    'XSLoader' => {
        'DISTRIBUTION' => 'SAPER/XSLoader-0.24.tar.gz',
        'FILES'        => q[dist/XSLoader],
        'EXCLUDED'     => [
            qr{^eg/},
            qw( t/00-load.t
                t/01-api.t
                t/distchk.t
                t/pod.t
                t/podcover.t
                t/portfs.t
                ),
            'XSLoader.pm',    # we use XSLoader_pm.PL
        ],
    },

    # this pseudo-module represents all the files under ext/ and lib/
    # that aren't otherwise claimed. This means that the following two
    # commands will check that every file under ext/ and lib/ is
    # accounted for, and that there are no duplicates:
    #
    #    perl Porting/Maintainers --checkmani lib ext
    #    perl Porting/Maintainers --checkmani

    '_PERLLIB' => {
        'FILES'    => q[
                ext/Amiga-ARexx/
                ext/Amiga-Exec/
                ext/B/
                ext/Devel-Peek/
                ext/DynaLoader/
                ext/Errno/
                ext/ExtUtils-Miniperl/
                ext/Fcntl/
                ext/File-DosGlob/
                ext/File-Find/
                ext/File-Glob/
                ext/FileCache/
                ext/GDBM_File/
                ext/Hash-Util-FieldHash/
                ext/Hash-Util/
                ext/I18N-Langinfo/
                ext/IPC-Open3/
                ext/NDBM_File/
                ext/ODBM_File/
                ext/Opcode/
                ext/POSIX/
                ext/PerlIO-encoding/
                ext/PerlIO-mmap/
                ext/PerlIO-scalar/
                ext/PerlIO-via/
                ext/Pod-Functions/
                ext/Pod-Html/
                ext/SDBM_File/
                ext/Sys-Hostname/
                ext/Tie-Hash-NamedCapture/
                ext/Tie-Memoize/
                ext/VMS-DCLsym/
                ext/VMS-Filespec/
                ext/VMS-Stdio/
                ext/Win32CORE/
                ext/XS-APItest/
                ext/XS-Typemap/
                ext/attributes/
                ext/mro/
                ext/re/
                lib/AnyDBM_File.{pm,t}
                lib/Benchmark.{pm,t}
                lib/B/Deparse{.pm,.t,-*.t}
                lib/B/Op_private.pm
                lib/CORE.pod
                lib/Class/Struct.{pm,t}
                lib/Config.t
                lib/Config/Extensions.{pm,t}
                lib/DB.{pm,t}
                lib/DBM_Filter.pm
                lib/DBM_Filter/
                lib/DirHandle.{pm,t}
                lib/English.{pm,t}
                lib/ExtUtils/Embed.pm
                lib/ExtUtils/XSSymSet.pm
                lib/ExtUtils/t/Embed.t
                lib/ExtUtils/typemap
                lib/File/Basename.{pm,t}
                lib/File/Compare.{pm,t}
                lib/File/Copy.{pm,t}
                lib/File/stat{.pm,.t,-7896.t}
                lib/FileHandle.{pm,t}
                lib/Getopt/Std.{pm,t}
                lib/Internals.pod
                lib/Internals.t
                lib/meta_notation.{pm,t}
                lib/Net/hostent.{pm,t}
                lib/Net/netent.{pm,t}
                lib/Net/protoent.{pm,t}
                lib/Net/servent.{pm,t}
                lib/PerlIO.pm
                lib/Pod/t/Usage.t
                lib/SelectSaver.{pm,t}
                lib/Symbol.{pm,t}
                lib/Thread.{pm,t}
                lib/Tie/Array.pm
                lib/Tie/Array/
                lib/Tie/ExtraHash.t
                lib/Tie/Handle.pm
                lib/Tie/Handle/
                lib/Tie/Hash.{pm,t}
                lib/Tie/Scalar.{pm,t}
                lib/Tie/StdHandle.pm
                lib/Tie/SubstrHash.{pm,t}
                lib/Time/gmtime.{pm,t}
                lib/Time/localtime.{pm,t}
                lib/Time/tm.pm
                lib/UNIVERSAL.pm
                lib/Unicode/README
                lib/Unicode/testnorm.t
                lib/Unicode/UCD.{pm,t}
                lib/User/grent.{pm,t}
                lib/User/pwent.{pm,t}
                lib/_charnames.pm
                lib/blib.{pm,t}
                lib/builtin.{pm,t}
                lib/bytes.{pm,t}
                lib/bytes_heavy.pl
                lib/charnames.{pm,t}
                lib/dbm_filter_util.pl
                lib/deprecate.pm
                lib/diagnostics.{pm,t}
                lib/dumpvar.{pl,t}
                lib/feature.{pm,t}
                lib/feature/
                lib/filetest.{pm,t}
                lib/h2ph.t
                lib/h2xs.t
                lib/integer.{pm,t}
                lib/less.{pm,t}
                lib/locale.{pm,t}
                lib/locale_threads.t
                lib/open.{pm,t}
                lib/overload/numbers.pm
                lib/overloading.{pm,t}
                lib/overload{.pm,.t,64.t}
                lib/perl5db.{pl,t}
                lib/perl5db/
                lib/perlbug.t
                lib/sigtrap.{pm,t}
                lib/sort.{pm,t}
                lib/strict.{pm,t}
                lib/subs.{pm,t}
                lib/unicore/
                lib/utf8.{pm,t}
                lib/vars{.pm,.t,_carp.t}
                lib/vmsish.{pm,t}
                ],
    },
);


# legacy CPAN flag
for my $mod_name ( keys %Modules ) {
    my $data = $Modules{$mod_name};
    $data->{CPAN} = !!$data->{DISTRIBUTION};
    my (@files)= split /\s+/, $data->{FILES};
    if (@files and $files[0]=~s!^(cpan|dist)/!!) {
        $DistName{$files[0]} = $mod_name;
        $DistName{"$1/$files[0]"} = $mod_name;
    }
}

# legacy UPSTREAM flag
for ( keys %Modules ) {
    # Keep any existing UPSTREAM flag so that "overrides" can be applied
    next if exists $Modules{$_}{UPSTREAM};

    if ($_ eq '_PERLLIB' or $Modules{$_}{FILES} =~ m{^\s*(?:dist|ext|lib)/}) {
        $Modules{$_}{UPSTREAM} = 'blead';
    }
    elsif ($Modules{$_}{FILES} =~ m{^\s*cpan/}) {
        $Modules{$_}{UPSTREAM} = 'cpan';
    }
    else {
        warn "Unexpected location of FILES for module $_: $Modules{$_}{FILES}";
    }
}

# legacy MAINTAINER field
for ( keys %Modules ) {
    # Keep any existing MAINTAINER flag so that "overrides" can be applied
    next if exists $Modules{$_}{MAINTAINER};

    if ($Modules{$_}{UPSTREAM} eq 'blead') {
        $Modules{$_}{MAINTAINER} = 'P5P';
        $Maintainers{P5P} = 'perl5-porters <perl5-porters@perl.org>';
    }
    elsif (exists $Modules{$_}{DISTRIBUTION}) {
        (my $pause_id = $Modules{$_}{DISTRIBUTION}) =~ s{/.*$}{};
        $Modules{$_}{MAINTAINER} = $pause_id;
        $Maintainers{$pause_id} = "<$pause_id\@cpan.org>";
    }
    else {
        warn "No DISTRIBUTION for non-blead module $_";
    }
}

1;
