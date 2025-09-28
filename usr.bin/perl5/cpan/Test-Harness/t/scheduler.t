#!/usr/bin/perl -w

use strict;
use warnings;
use lib 't/lib';

use Test::More;
use TAP::Parser::Scheduler;

my $perl_rules = {
    par => [
        { seq => '../ext/DB_File/t/*' },
        { seq => '../ext/IO_Compress_Zlib/t/*' },
        { seq => '../lib/CPANPLUS/*' },
        { seq => '../lib/ExtUtils/t/*' },
        '*'
    ]
};

my $incomplete_rules = { par => [ { seq => [ '*A', '*D' ] } ] };

my $some_tests = [
    '../ext/DB_File/t/A',
    'foo',
    '../ext/DB_File/t/B',
    '../ext/DB_File/t/C',
    '../lib/CPANPLUS/D',
    '../lib/CPANPLUS/E',
    'bar',
    '../lib/CPANPLUS/F',
    '../ext/DB_File/t/D',
    '../ext/DB_File/t/E',
    '../ext/DB_File/t/F',
];

my @schedule = (
    {   name  => 'Sequential, no rules',
        tests => $some_tests,
        jobs  => 1,
    },
    {   name  => 'Sequential, Perl rules',
        rules => $perl_rules,
        tests => $some_tests,
        jobs  => 1,
    },
    {   name  => 'Two in parallel, Perl rules',
        rules => $perl_rules,
        tests => $some_tests,
        jobs  => 2,
    },
    {   name  => 'Massively parallel, Perl rules',
        rules => $perl_rules,
        tests => $some_tests,
        jobs  => 1000,
    },
    {   name  => 'Massively parallel, no rules',
        tests => $some_tests,
        jobs  => 1000,
    },
    {   name  => 'Sequential, incomplete rules',
        rules => $incomplete_rules,
        tests => $some_tests,
        jobs  => 1,
    },
    {   name  => 'Two in parallel, incomplete rules',
        rules => $incomplete_rules,
        tests => $some_tests,
        jobs  => 2,
    },
    {   name  => 'Massively parallel, incomplete rules',
        rules => $incomplete_rules,
        tests => $some_tests,
        jobs  => 1000,
    },
);

plan tests => @schedule * 2 + 266;

for my $test (@schedule) {
    test_scheduler(
        $test->{name},
        $test->{tests},
        $test->{rules},
        $test->{jobs}
    );
}

# An ad-hoc test

{
    my @tests = qw(
      A1 A2 A3 B1 C1 C8 C5 C7 C4 C6 C3 C2 C9 D1 D2 D3 E3 E2 E1
    );

    my $rules = {
        par => [
            { seq => 'A*' },
            { par => 'B*' },
            { seq => [ 'C1', 'C2' ] },
            {   par => [
                    { seq => [ 'C3', 'C4', 'C5' ] },
                    { seq => [ 'C6', 'C7', 'C8' ] }
                ]
            },
            {   seq => [
                    { par => ['D*'] },
                    { par => ['E*'] }
                ]
            },
        ]
    };

    my $scheduler = TAP::Parser::Scheduler->new(
        tests => \@tests,
        rules => $rules
    );

    # diag $scheduler->as_string;

    my $A1 = ok_job( $scheduler, 'A1' );
    my $B1 = ok_job( $scheduler, 'B1' );
    finish($A1);
    my $A2 = ok_job( $scheduler, 'A2' );
    my $C1 = ok_job( $scheduler, 'C1' );
    finish( $A2, $C1 );
    my $A3 = ok_job( $scheduler, 'A3' );
    my $C2 = ok_job( $scheduler, 'C2' );
    finish( $A3, $C2 );
    my $C3 = ok_job( $scheduler, 'C3' );
    my $C6 = ok_job( $scheduler, 'C6' );
    my $D1 = ok_job( $scheduler, 'D1' );
    my $D2 = ok_job( $scheduler, 'D2' );
    finish($C6);
    my $C7 = ok_job( $scheduler, 'C7' );
    my $D3 = ok_job( $scheduler, 'D3' );
    ok_job( $scheduler, '#' );
    ok_job( $scheduler, '#' );
    finish( $D3, $C3, $D1, $B1 );
    my $C4 = ok_job( $scheduler, 'C4' );
    finish( $C4, $C7 );
    my $C5 = ok_job( $scheduler, 'C5' );
    my $C8 = ok_job( $scheduler, 'C8' );
    ok_job( $scheduler, '#' );
    finish($D2);
    my $E3 = ok_job( $scheduler, 'E3' );
    my $E2 = ok_job( $scheduler, 'E2' );
    my $E1 = ok_job( $scheduler, 'E1' );
    finish( $E1, $E2, $E3, $C5, $C8 );
    my $C9 = ok_job( $scheduler, 'C9' );
    ok_job( $scheduler, undef );
}

{
    my @tests = ();
    for my $t ( 'A' .. 'Z' ) {
        push @tests, map {"$t$_"} 1 .. 9;
    }
    my $rules = { par => [ map { { seq => "$_*" } } 'A' .. 'Z' ] };

    my $scheduler = TAP::Parser::Scheduler->new(
        tests => \@tests,
        rules => $rules
    );

    # diag $scheduler->as_string;

    for my $n ( 1 .. 9 ) {
        my @got = ();
        push @got, ok_job( $scheduler, "$_$n" ) for 'A' .. 'Z';
        ok_job( $scheduler, $n == 9 ? undef : '#' );
        finish(@got);
    }
}

sub finish { $_->finish for @_ }

sub ok_job {
    my ( $scheduler, $want ) = @_;
    my $job = $scheduler->get_job;
    if ( !defined $want ) {
        ok !defined $job, 'undef';
    }
    elsif ( $want eq '#' ) {
        ok $job->is_spinner, 'spinner';
    }
    else {
        is $job->filename, $want, $want;
    }
    return $job;
}

sub test_scheduler {
    my ( $name, $tests, $rules, $jobs ) = @_;

    ok my $scheduler = TAP::Parser::Scheduler->new(
        tests => $tests,
        defined $rules ? ( rules => $rules ) : (),
      ),
      "$name: new";

    # diag $scheduler->as_string;

    my @pipeline = ();
    my @got      = ();

    while ( defined( my $job = $scheduler->get_job ) ) {

        # diag $scheduler->as_string;
        if ( $job->is_spinner || @pipeline >= $jobs ) {
            die "Oops! Spinner!" unless @pipeline;
            my $done = shift @pipeline;
            $done->finish;

            # diag "Completed ", $done->filename;
        }
        next if $job->is_spinner;

        # diag "      Got ", $job->filename;
        push @pipeline, $job;

        push @got, $job->filename;
    }

    is_deeply [ sort @got ], [ sort @$tests ], "$name: got all tests";
}

