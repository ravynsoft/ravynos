#!/usr/bin/perl -w

# Ensure pm_to_blib runs at the right times.

use strict;
use warnings;
use lib 't/lib';

use File::Temp qw[tempdir];

use ExtUtils::MakeMaker;

use MakeMaker::Test::Utils;
use MakeMaker::Test::Setup::BFD;
use Config;
use ExtUtils::MM;
use Test::More
    !MM->can_run(make()) && $ENV{PERL_CORE} && $Config{'usecrosscompile'}
    ? (skip_all => "cross-compiling and make not available")
    : 'no_plan';

my $perl     = which_perl();
my $makefile = makefile_name();
my $make     = make_run();

local $ENV{PERL_INSTALL_QUIET};

# Setup our test environment
{
    chdir 't';
    perl_lib; # sets $ENV{PERL5LIB} relative to t/

    my $tmpdir = tempdir( DIR => '../t', CLEANUP => 1 );
    use Cwd; my $cwd = getcwd; END { chdir $cwd } # so File::Temp can cleanup
    chdir $tmpdir;

    ok( setup_recurs(), 'setup' );
    END {
        ok( chdir File::Spec->updir );
        ok( teardown_recurs(), 'teardown' );
    }

    ok( chdir('Big-Dummy'), "chdir'd to Big-Dummy" ) ||
      diag("chdir failed: $!");
}


# Run make once
{
    run_ok(qq{$perl Makefile.PL});
    run_ok($make);

    ok( -e "blib/lib/Big/Dummy.pm", "blib copied pm file" );
}


# Change a pm file, it should be copied.
{
    # Wait a couple seconds else our changed file will have the same timestamp
    # as the blib file
    sleep 2;

    ok( open my $fh, ">>", "lib/Big/Dummy.pm" ) or die $!;
    print $fh "Something else\n";
    close $fh;

    run_ok($make);
    like slurp("blib/lib/Big/Dummy.pm"), qr/Something else\n$/;
}


# Rerun the Makefile.PL, pm_to_blib should rerun
{
    # Seems there are occasional race conditions with these tests
    # waiting a couple of seconds appears to resolve these
    sleep 2;
    run_ok(qq{$perl Makefile.PL});

    # XXX This is a fragile way to check that it reran.
    like run_ok($make), qr/^Skip /ms;

    ok( -e "blib/lib/Big/Dummy.pm", "blib copied pm file" );
}
