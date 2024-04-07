use strict;
use warnings;
use Test::More;

$@ = 'foo';
explain { 1 => 1 };
is($@, 'foo', "preserved \$@");

done_testing;
