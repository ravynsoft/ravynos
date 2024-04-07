use strict;
use warnings;

use Test::More;
use Test2::API qw/intercept/;

my @warnings;

intercept {
    SKIP: {
        local $SIG{__WARN__} = sub { @warnings = @_ };
        skip 'Skipping this test' if 1;
        my $var = 'abc';
        is $var, 'abc';
    }
};

ok(!@warnings, "did not warn when waiting for done_testing");

intercept {
    SKIP: {
        local $SIG{__WARN__} = sub { @warnings = @_ };
        plan 'no_plan';
        skip 'Skipping this test' if 1;
        my $var = 'abc';
        is $var, 'abc';
    }
};

ok(!@warnings, "did not warn with 'no_plan'");

intercept {
    SKIP: {
        local $SIG{__WARN__} = sub { @warnings = @_ };
        plan tests => 1;
        skip 'Skipping this test' if 1;
        my $var = 'abc';
        is $var, 'abc';
    }
};

is(@warnings, 1, "warned with static plan");
like(
    $warnings[0],
    qr/skip\(\) needs to know \$how_many tests are in the block/,
    "Got expected warning"
);

done_testing;
