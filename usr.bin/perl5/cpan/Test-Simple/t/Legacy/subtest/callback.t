#!/usr/bin/perl -w

# What happens when a subtest dies?

use lib 't/lib';

use strict;
use Test::More;
use Test::Builder;
use Test2::API;

my $Test = Test::Builder->new;

my $step = 0;
my @callback_calls = ();
Test2::API::test2_add_callback_pre_subtest(
    sub {
        $Test->is_num(
            $step,
            0,
            'pre-subtest callbacks should be invoked before the subtest',
        );
        ++$step;
        push @callback_calls, [@_];
    },
);

$Test->subtest(
    (my $subtest_name='some subtest'),
    (my $subtest_code=sub {
         $Test->is_num(
             $step,
             1,
             'subtest should be run after the pre-subtest callbacks',
         );
         ++$step;
     }),
    (my @subtest_args = (1,2,3)),
);

is_deeply(
    \@callback_calls,
    [[$subtest_name,$subtest_code,@subtest_args]],
    'pre-subtest callbacks should be invoked with the expected arguments',
);

$Test->is_num(
    $step,
    2,
    'the subtest should be run',
);

$Test->done_testing();
