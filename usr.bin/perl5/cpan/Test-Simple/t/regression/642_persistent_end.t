use Test::More;
use strict;
use warnings;

use Test2::API qw{
    test2_set_is_end
    test2_get_is_end
    intercept
};

my %res;
intercept {
    my $tb = Test::Builder->new;
    $res{before} = test2_get_is_end();
    test2_set_is_end();
    $res{isset} = test2_get_is_end();
    $tb->reset;
    $res{reset} = test2_get_is_end();
};

ok(!$res{before}, "Not the end");
ok($res{isset}, "the end");
ok(!$res{reset}, "Not the end");

done_testing;
