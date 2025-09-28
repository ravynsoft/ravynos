#!perl

use strict;
use warnings;

use Test::More tests => 1;

use XS::APItest qw(join_with_space);

sub foo { 'A' .. 'C' }

my $bar = 42;
my @baz = ('x', 'y');

my $str = join_with_space $bar, foo, @baz;
is $str, "42 A B C x y";
