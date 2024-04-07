#!./perl

use strict;
use warnings;

use Test::More tests => 5;
use List::Util qw(minstr);

my $v;

ok(defined &minstr, 'defined');

$v = minstr('a');
is($v, 'a', 'single arg');

$v = minstr('a','b');
is($v, 'a', '2-arg ordered');

$v = minstr('B','A');
is($v, 'A', '2-arg reverse ordered');

my @a = map { pack("u", pack("C*",map { int(rand(256))} (0..int(rand(10) + 2)))) } 0 .. 20;
my @b = sort { $a cmp $b } @a;
$v = minstr(@a);
is($v, $b[0], 'random ordered');
