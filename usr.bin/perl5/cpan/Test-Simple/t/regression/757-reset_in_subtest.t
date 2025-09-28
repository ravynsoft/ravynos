use strict;
use warnings;

use Test::More;

subtest 'subtest' => sub {
    Test::Builder->new->reset;
    ok 1;
};

subtest 'subtest' => sub {
    Test::Builder->new->reset;
    subtest 'subtest' => sub {
        Test::Builder->new->reset;
        ok 1;
    };
    ok 1;
};

done_testing;
