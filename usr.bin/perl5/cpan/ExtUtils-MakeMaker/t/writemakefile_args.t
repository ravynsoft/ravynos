#!/usr/bin/perl -w

# This is a test of the verification of the arguments to
# WriteMakefile.

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;
use Config;
use Test::More tests => 43;

use TieOut;
use MakeMaker::Test::Utils;
use MakeMaker::Test::Setup::BFD;

use ExtUtils::MakeMaker;

chdir 't';
perl_lib; # sets $ENV{PERL5LIB} relative to t/

use File::Temp qw[tempdir];
my $tmpdir = tempdir( DIR => '../t', CLEANUP => 1 );
use Cwd; my $cwd = getcwd; END { chdir $cwd } # so File::Temp can cleanup
chdir $tmpdir;

ok( setup_recurs(), 'setup' );
END {
    ok( chdir File::Spec->updir );
    ok( teardown_recurs(), 'teardown' );
}

ok( chdir 'Big-Dummy', "chdir'd to Big-Dummy" ) ||
  diag("chdir failed: $!");

{
    ok( my $stdout = tie *STDOUT, 'TieOut' );
    my $warnings = '';
    local $SIG{__WARN__} = sub {
        if ( $Config{usecrosscompile} ) {
            # libraries might not be present on the target system
            # when cross-compiling
            return if $_[0] =~ /\A\QWarning (mostly harmless): No library found for \E.+/
        }
        $warnings .= join '', @_;
    };

    my $mm;

    eval {
        $mm = WriteMakefile(
            NAME            => 'Big::Dummy',
            VERSION_FROM    => 'lib/Big/Dummy.pm',
            MAN3PODS        => ' ', # common mistake
        );
    };

    is( $warnings, <<VERIFY );
WARNING: MAN3PODS takes a HASH reference not a string/number.
         Please inform the author.
VERIFY

    $warnings = '';
    eval {
        $mm = WriteMakefile(
            NAME            => 'Big::Dummy',
            VERSION_FROM    => 'lib/Big/Dummy.pm',
            AUTHOR          => sub {},
        );
    };

    is( $warnings, <<VERIFY );
WARNING: AUTHOR takes a ARRAY reference not a CODE reference.
         Please inform the author.
VERIFY

    # LIBS accepts *both* a string or an array ref.  The first cut of
    # our verification did not take this into account.
    $warnings = '';
    $mm = WriteMakefile(
        NAME            => 'Big::Dummy',
        VERSION_FROM    => 'lib/Big/Dummy.pm',
        LIBS            => '-lwibble -lwobble',
    );

    # We'll get warnings about the bogus libs, that's ok.
    unlike( $warnings, qr/WARNING: .* takes/ );
    is_deeply( $mm->{LIBS}, ['-lwibble -lwobble'] );

    $warnings = '';
    $mm = WriteMakefile(
        NAME            => 'Big::Dummy',
        VERSION_FROM    => 'lib/Big/Dummy.pm',
        LIBS            => ['-lwibble', '-lwobble'],
    );

    # We'll get warnings about the bogus libs, that's ok.
    unlike( $warnings, qr/WARNING: .* takes/ );
    is_deeply( $mm->{LIBS}, ['-lwibble', '-lwobble'] );

    $warnings = '';
    eval {
        $mm = WriteMakefile(
            NAME            => 'Big::Dummy',
            VERSION_FROM    => 'lib/Big/Dummy.pm',
            LIBS            => { wibble => "wobble" },
        );
    };

    # We'll get warnings about the bogus libs, that's ok.
    like( $warnings, qr{^WARNING: LIBS takes a ARRAY reference or string/number not a HASH reference}m );


    $warnings = '';
    $mm = WriteMakefile(
        NAME            => 'Big::Dummy',
        WIBBLE          => 'something',
        wump            => { foo => 42 },
    );

    like( $warnings, qr{^WARNING: WIBBLE is not a known parameter.\n}m );
    like( $warnings, qr{^WARNING: wump is not a known parameter.\n}m );

    is( $mm->{WIBBLE}, 'something' );
    is_deeply( $mm->{wump}, { foo => 42 } );


    # Test VERSION
    $warnings = '';
    eval {
        $mm = WriteMakefile(
            NAME       => 'Big::Dummy',
            VERSION    => [1,2,3],
        );
    };
    like( $warnings, qr{^WARNING: VERSION takes a version object or string/number} );

    $warnings = '';
    eval {
        $mm = WriteMakefile(
            NAME       => 'Big::Dummy',
            VERSION    => 1.002_003,
        );
    };
    is( $warnings, '' );
    is( $mm->{VERSION}, '1.002003' );

    $warnings = '';
    eval {
        $mm = WriteMakefile(
            NAME       => 'Big::Dummy',
            VERSION    => '1.002_003',
        );
    };
    is( $warnings, '' );
    is( $mm->{VERSION}, '1.002_003' );


    $warnings = '';
    eval {
        $mm = WriteMakefile(
            NAME       => 'Big::Dummy',
            VERSION    => bless {}, "Some::Class",
        );
    };
    like( $warnings, '/^WARNING: VERSION takes a version object or string/number not a Some::Class object/' );


    SKIP: {
        skip("Can't test version objects", 8) unless eval { require version };
        version->import;

        my $version = version->new("1.2.3");
        $warnings = '';
        ok eval {
            $mm = WriteMakefile(
                NAME       => 'Big::Dummy',
                VERSION    => $version,
            );
        } || diag $@;
        is( $warnings, '' );
        isa_ok( $mm->{VERSION}, 'version' );
        is( $mm->{VERSION}, $version );

        $warnings = '';
        $version = qv('1.2.3');
        ok eval {
            $mm = WriteMakefile(
                NAME       => 'Big::Dummy',
                VERSION    => $version,
            );
        } || diag $@;
        is( $warnings, '' );
        isa_ok( $mm->{VERSION}, 'version' );
        is( $mm->{VERSION}, $version );
    }


    # DISTNAME
    $warnings = '';
    eval {
        $mm = WriteMakefile(
            NAME       => 'Big::Dummy',
            VERSION    => '1.00',
            DISTNAME   => "Hooballa",
        );
    };
    is( $warnings, '' );
    is( $mm->{DISTNAME},  "Hooballa" );
    is( $mm->{DISTVNAME}, $Is_VMS ? "Hooballa-1_00" : "Hooballa-1.00" );


    # DISTVNAME (rt.cpan.org 43217)
    $warnings = '';
    eval {
        $mm = WriteMakefile(
            NAME       => 'Big::Dummy',
            VERSION    => 1.00,
            DISTVNAME  => "Hooballoo",
        );
    };
    is( $warnings, '' );
    is( $mm->{DISTVNAME}, 'Hooballoo' );


    # AUTHOR / scalar
    $warnings = '';
    eval {
        $mm = WriteMakefile(
            NAME       => 'Big::Dummy',
            VERSION    => '1.00',
            AUTHOR     => "test",
        );
    };
    is( $warnings, '' );
    is_deeply( $mm->{AUTHOR},  ["test"] );


    # AUTHOR / array
    $warnings = '';
    eval {
        $mm = WriteMakefile(
            NAME       => 'Big::Dummy',
            VERSION    => '1.00',
            AUTHOR     => ["test1", "test2"],
        );
    };
    is( $warnings, '' );
    is_deeply( $mm->{AUTHOR},  ["test1","test2"] );

    # PERL_MM_OPT
    {
      local $ENV{PERL_MM_OPT} = 'CCFLAGS="-Wl,-rpath -Wl,/foo/bar/lib" LIBS="-lwibble -lwobble"';
      $mm = WriteMakefile(
          NAME            => 'Big::Dummy',
          VERSION    => '1.00',
      );

      like( $mm->{CCFLAGS}, qr{-Wl,-rpath -Wl,/foo/bar/lib}, 'parse_args() splits like shell' );
      is_deeply( $mm->{LIBS}, ['-lwibble -lwobble'], 'parse_args() splits like shell' );
    }

    # PERL_MM_OPT
    {
      local $ENV{PERL_MM_OPT} = 'INSTALL_BASE=/how/we/have/not/broken/local/lib';
      $mm = WriteMakefile(
          NAME            => 'Big::Dummy',
          VERSION    => '1.00',
      );

      is( $mm->{INSTALL_BASE}, "/how/we/have/not/broken/local/lib", 'parse_args() splits like shell' );
    }

    # PERL_MM_OPT
    {
      local $ENV{PERL_MM_OPT} = 'INSTALL_BASE="/Users/miyagawa/tmp/car1  foo/foo bar"';
      $mm = WriteMakefile(
          NAME            => 'Big::Dummy',
          VERSION    => '1.00',
      );

      is( $mm->{INSTALL_BASE}, "/Users/miyagawa/tmp/car1  foo/foo bar", 'parse_args() splits like shell' );
    }

}
