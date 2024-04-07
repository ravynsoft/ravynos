use Test::More;
use strict;
use warnings;

use Test2::API qw/intercept/;

my @returns;
intercept {
    push @returns => diag('foo');
    push @returns => note('foo');

    my $tb = Test::Builder->new;
    push @returns => $tb->diag('foo');
    push @returns => $tb->note('foo');
};

is(@returns, 4, "4 return values");
is_deeply(\@returns, [0, 0, 0, 0], "All note/diag returns are 0");

done_testing;
