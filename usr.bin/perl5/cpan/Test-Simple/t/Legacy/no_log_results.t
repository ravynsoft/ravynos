use strict;
use warnings;

use Test::More;

sub it {
    my $tb = Test::Builder->new;
    $tb->no_log_results;

    ok(1, "sample");
    ok(2, "sample");

    is_deeply([$tb->details], [], "no details were logged");
}

it();
subtest it => \&it;

done_testing;
