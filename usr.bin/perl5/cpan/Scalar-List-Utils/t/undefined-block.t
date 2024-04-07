#!./perl

use strict;
use warnings;

my @subs;
BEGIN { @subs = qw(reduce first none all any notall pairfirst pairgrep pairmap) };
use List::Util @subs;
use Test::More;
plan tests => @subs * 2;

for my $sub (@subs) {
    eval { no strict 'refs';  no warnings 'uninitialized'; &{$sub}(undef, 1, 2) };
    like($@, qr{^Not a subroutine reference}, "$sub(undef, ...) croaks");

    eval { no strict 'refs'; &{$sub}(\&undefined, 1, 2) };
    like($@, qr{^Undefined subroutine in $sub}, "$sub(\&undefined, ...) croaks");
}
