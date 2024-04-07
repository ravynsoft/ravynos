#!./perl

use strict;
use warnings;

use Test::More tests => 3;

use List::Util qw( sum0 );

my $v = sum0;
is( $v, 0, 'no args' );

$v = sum0(9);
is( $v, 9, 'one arg' );

$v = sum0(1,2,3,4);
is( $v, 10, '4 args');
