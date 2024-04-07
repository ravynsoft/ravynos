use strict;
use warnings;

use Test2::Tools::Tiny;

use Test2::API qw/run_subtest intercept/;

my $step = 0;
my @callback_calls = ();
Test2::API::test2_add_callback_pre_subtest(
    sub {
        is(
            $step,
            0,
            'pre-subtest callbacks should be invoked before the subtest',
        );
        ++$step;
        push @callback_calls, [@_];
    },
);

run_subtest(
    (my $subtest_name='some subtest'),
    (my $subtest_code=sub {
         is(
             $step,
             1,
             'subtest should be run after the pre-subtest callbacks',
         );
         ++$step;
     }),
    undef,
    (my @subtest_args = (1,2,3)),
);

is_deeply(
    \@callback_calls,
    [[$subtest_name,$subtest_code,@subtest_args]],
    'pre-subtest callbacks should be invoked with the expected arguments',
);

is(
    $step,
    2,
    'the subtest should be run',
);

done_testing;
