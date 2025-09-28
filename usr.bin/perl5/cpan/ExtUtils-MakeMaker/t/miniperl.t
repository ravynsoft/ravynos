#!/usr/bin/perl -w

# Test that we can build modules as miniperl.
# This mostly means no XS modules.

use strict;
use warnings;
use lib 't/lib';

use Test::More;
use Config;

# In a BEGIN block so the END tests aren't registered.
BEGIN {
    plan skip_all => 'miniperl test only necessary for the perl core'
      if !$ENV{PERL_CORE};

    plan $ENV{PERL_CORE} && $Config{'usecrosscompile'}
      ? (skip_all => 'cross-compiling and make not available')
      : 'no_plan';
}

# Disable all XS from here on
use MakeMaker::Test::NoXS;

use ExtUtils::MakeMaker;

use MakeMaker::Test::Utils;
use MakeMaker::Test::Setup::BFD;

my $perl     = which_perl();
my $makefile = makefile_name();
my $make     = make_run();


# Setup our test environment
{
    chdir 't';

    perl_lib;

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
}
