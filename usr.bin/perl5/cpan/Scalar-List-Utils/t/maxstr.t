#!./perl

use strict;
use warnings;

use Test::More tests => 5;
use List::Util qw(maxstr);

my $v;

ok(defined &maxstr, 'defined');

$v = maxstr('a');
is($v, 'a', 'single arg');

$v = maxstr('a','b');
is($v, 'b', '2-arg ordered');

$v = maxstr('B','A');
is($v, 'B', '2-arg reverse ordered');

my @a = map { pack("u", pack("C*",map { int(rand(256))} (0..int(rand(10) + 2)))) } 0 .. 20;
my @b = sort { $a cmp $b } @a;
$v = maxstr(@a);
is($v, $b[-1], 'random ordered');
