use strict;
use warnings;

use Test2::Tools::Tiny;

ok(1, "");

tests foo => sub {
    ok(1, "name");
    ok(1, "");
};

done_testing;
