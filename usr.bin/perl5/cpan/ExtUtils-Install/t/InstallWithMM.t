#!/usr/bin/perl -w
use strict;

# Make sure EUI works with MakeMaker

BEGIN {
    unshift @INC, 't/lib';
}

use Config;
use ExtUtils::MakeMaker;

use Test::More;
use MakeMaker::Test::Utils;

my $make;
BEGIN {
    $make = make_run();
    if (!$make) {
	plan skip_all => "make isn't available";
    }
    else {
	plan tests => 15;
    }
}

use MakeMaker::Test::Setup::BFD;
use File::Find;
use File::Spec;
use File::Path;
use File::Temp qw[tempdir];

# Environment variables which interfere with our testing.
delete @ENV{qw(PREFIX LIB MAKEFLAGS)};

# Run Makefile.PL
{
    my $perl = which_perl();
    my $Is_VMS = $^O eq 'VMS';

    perl_lib;

    my $tmpdir = tempdir( DIR => 't', CLEANUP => 1 );
    chdir $tmpdir;

    my $Touch_Time = calibrate_mtime();

    $| = 1;

    ok( setup_recurs(), 'setup' );
    END {
        ok( chdir File::Spec->updir );
        ok( teardown_recurs(), 'teardown' );
    }

    ok( chdir('Big-Dummy'), "chdir'd to Big-Dummy" ) ||
      diag("chdir failed: $!");

    my @mpl_out = run(qq{"$perl" Makefile.PL "PREFIX=../dummy-install"});
    END { rmtree '../dummy-install'; }

    cmp_ok( $?, '==', 0, 'Makefile.PL exited with zero' ) ||
      diag(@mpl_out);

    END { unlink makefile_name(), makefile_backup() }
}


# make
{
    my $make_out = run($make);
    is( $?, 0, 'make ran ok' ) ||
      diag($make_out);
}


# Test 'make install VERBINST=1'
{
    my $make_install_verbinst = make_macro($make, 'install', VERBINST => 1);
    my $install_out = run($make_install_verbinst);
    is( $?, 0, 'install' ) || diag $install_out;
    like( $install_out, qr/^Installing /m );
    like( $install_out, qr/^Writing /m );

    ok( -r '../dummy-install',     '  install dir created' );
    my %files = ();
    find( sub {
              # do it case-insensitive for non-case preserving OSs
              my $file = lc $_;

              # VMS likes to put dots on the end of things that don't have them.
              $file =~ s/\.$// if $Is_VMS;

              $files{$file} = $File::Find::name;
          }, '../dummy-install' );
    ok( $files{'dummy.pm'},     '  Dummy.pm installed' );
    ok( $files{'liar.pm'},      '  Liar.pm installed'  );
    ok( $files{'program'},      '  program installed'  );
    ok( $files{'.packlist'},    '  packlist created'   );
    ok( $files{'perllocal.pod'},'  perllocal.pod created' );
}
