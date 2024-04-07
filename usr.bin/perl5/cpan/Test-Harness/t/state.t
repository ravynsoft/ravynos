#!/usr/bin/perl -w

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;
use Test::More;
use App::Prove::State;
use App::Prove::State::Result;

sub mn {
    my $pfx = '';
    return map {"$pfx$_"} @_;
}

my @schedule = (
    {   options        => 'all',
        get_tests_args => [],
        expect         => [
            't/compat/env.t',
            't/compat/failure.t',
            't/compat/inc_taint.t',
            't/compat/version.t',
            't/source_handler.t',
            't/yamlish-writer.t',
        ],
    },
    {   options        => 'failed',
        get_tests_args => [],
        expect         => [
            't/compat/inc_taint.t',
            't/compat/version.t',
        ],
    },
    {   options        => 'passed',
        get_tests_args => [],
        expect         => [
            't/compat/env.t',
            't/compat/failure.t',
            't/source_handler.t',
            't/yamlish-writer.t',
        ],
    },
    {   options        => 'last',
        get_tests_args => [],
        expect         => [
            't/compat/env.t',
            't/compat/failure.t',
            't/compat/inc_taint.t',
            't/compat/version.t',
            't/source_handler.t',
        ],
    },
    {   options        => 'todo',
        get_tests_args => [],
        expect         => [
            't/compat/version.t',
            't/compat/failure.t',
        ],

    },
    {   options        => 'hot',
        get_tests_args => [],
        expect         => [
            't/compat/version.t',
            't/yamlish-writer.t',
            't/compat/env.t',
        ],
    },
    {   options        => 'adrian',
        get_tests_args => [],
        expect         => [
            't/compat/version.t',
            't/yamlish-writer.t',
            't/compat/env.t',
            't/compat/failure.t',
            't/compat/inc_taint.t',
            't/source_handler.t',
        ],
    },
    {   options        => 'failed,passed',
        get_tests_args => [],
        expect         => [
            't/compat/inc_taint.t',
            't/compat/version.t',
            't/compat/env.t',
            't/compat/failure.t',
            't/source_handler.t',
            't/yamlish-writer.t',
        ],
    },
    {   options        => [ 'failed', 'passed' ],
        get_tests_args => [],
        expect         => [
            't/compat/inc_taint.t',
            't/compat/version.t',
            't/compat/env.t',
            't/compat/failure.t',
            't/source_handler.t',
            't/yamlish-writer.t',
        ],
    },
    {   options        => 'slow',
        get_tests_args => [],
        expect         => [
            't/yamlish-writer.t',
            't/compat/env.t',
            't/compat/inc_taint.t',
            't/compat/version.t',
            't/compat/failure.t',
            't/source_handler.t',
        ],
    },
    {   options        => 'fast',
        get_tests_args => [],
        expect         => [
            't/source_handler.t',
            't/compat/failure.t',
            't/compat/version.t',
            't/compat/inc_taint.t',
            't/compat/env.t',
            't/yamlish-writer.t',
        ],
    },
    {   options        => 'old',
        get_tests_args => [],
        expect         => [
            't/source_handler.t',
            't/compat/inc_taint.t',
            't/compat/version.t',
            't/yamlish-writer.t',
            't/compat/failure.t',
            't/compat/env.t',
        ],
    },
    {   options        => 'new',
        get_tests_args => [],
        expect         => [
            't/compat/env.t',
            't/compat/failure.t',
            't/yamlish-writer.t',
            't/compat/version.t',
            't/compat/inc_taint.t',
            't/source_handler.t',
        ],
    },
    {   options        => 'fresh',
        get_tests_args => [],
        expect         => [
            't/compat/env.t',
            't/compat/failure.t',
        ],
    },
);

plan tests => @schedule * 2;

for my $test (@schedule) {
    my $state = App::Prove::State->new;
    isa_ok $state, 'App::Prove::State';

    my $desc = $test->{options};

    # Naughty
    $state->{_} = get_state();
    my $options = $test->{options};
    $options = [$options] unless 'ARRAY' eq ref $options;
    $state->apply_switch(@$options);

    my @got    = $state->get_tests( @{ $test->{get_tests_args} } );
    my @expect = mn( @{ $test->{expect} } );
    unless ( is_deeply \@got, \@expect, "$desc: order OK" ) {
        use Data::Dumper;
        diag( Dumper( { got => \@got, want => \@expect } ) );
    }
}

sub get_state {
    return App::Prove::State::Result->new(
        {   generation    => 51,
            last_run_time => 1196285439,
            tests         => {
                mn('t/compat/failure.t') => {
                    last_result    => 0,
                    last_run_time  => 1196371471.57738,
                    last_pass_time => 1196371471.57738,
                    total_passes   => 48,
                    seq            => 1549,
                    gen            => 51,
                    elapsed        => 0.1230,
                    last_todo      => 1,
                    mtime          => 1196285623,
                },
                mn('t/yamlish-writer.t') => {
                    last_result    => 0,
                    last_run_time  => 1196371480.5761,
                    last_pass_time => 1196371480.5761,
                    last_fail_time => 1196368609,
                    total_passes   => 41,
                    seq            => 1578,
                    gen            => 49,
                    elapsed        => 12.2983,
                    last_todo      => 0,
                    mtime          => 1196285400,
                },
                mn('t/compat/env.t') => {
                    last_result    => 0,
                    last_run_time  => 1196371471.42967,
                    last_pass_time => 1196371471.42967,
                    last_fail_time => 1196368608,
                    total_passes   => 48,
                    seq            => 1548,
                    gen            => 52,
                    elapsed        => 3.1290,
                    last_todo      => 0,
                    mtime          => 1196285739,
                },
                mn('t/compat/version.t') => {
                    last_result    => 2,
                    last_run_time  => 1196371472.96476,
                    last_pass_time => 1196371472.96476,
                    last_fail_time => 1196368609,
                    total_passes   => 47,
                    seq            => 1555,
                    gen            => 51,
                    elapsed        => 0.2363,
                    last_todo      => 4,
                    mtime          => 1196285239,
                },
                mn('t/compat/inc_taint.t') => {
                    last_result    => 3,
                    last_run_time  => 1196371471.89682,
                    last_pass_time => 1196371471.89682,
                    total_passes   => 47,
                    seq            => 1551,
                    gen            => 51,
                    elapsed        => 1.6938,
                    last_todo      => 0,
                    mtime          => 1196185639,
                },
                mn('t/source_handler.t') => {
                    last_result    => 0,
                    last_run_time  => 1196371479.72508,
                    last_pass_time => 1196371479.72508,
                    total_passes   => 41,
                    seq            => 1570,
                    gen            => 51,
                    elapsed        => 0.0143,
                    last_todo      => 0,
                    mtime          => 1186285639,
                },
            }
        }
    );
}
