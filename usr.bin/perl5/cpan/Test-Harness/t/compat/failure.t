#!/usr/bin/perl -w

use strict;
use warnings;
use lib 't/lib';

use Test::More tests => 5;

use File::Spec;
use Test::Harness;

{

    #todo_skip 'Harness compatibility incomplete', 5;
    #local $TODO = 'Harness compatibility incomplete';
    my $died;

    sub prepare_for_death {
        $died = 0;
        return sub { $died = 1 }
    }

    my $curdir = File::Spec->curdir;
    my $sample_tests = File::Spec->catdir( $curdir, 't', 'sample-tests' );

    {
        local $SIG{__DIE__} = prepare_for_death();
        eval { _runtests( File::Spec->catfile( $sample_tests, "simple" ) ); };
        ok( !$@, "simple lives" );
        is( $died, 0, "Death never happened" );
    }

    {
        local $SIG{__DIE__} = prepare_for_death();
        eval {
            _runtests( File::Spec->catfile( $sample_tests, "too_many" ) );
        };
        ok( $@, "error OK" );
        ok( $@ =~ m[Failed 1/1], "too_many dies" );
        is( $died, 1, "Death happened" );
    }
}

sub _runtests {
    my (@tests) = @_;

    local $ENV{PERL_TEST_HARNESS_DUMP_TAP} = 0;
    local $ENV{HARNESS_VERBOSE}            = 0;
    local $ENV{HARNESS_DEBUG}              = 0;
    local $ENV{HARNESS_TIMER}              = 0;

    local $Test::Harness::Verbose = -9;

    runtests(@tests);
}

# vim:ts=4:sw=4:et:sta
