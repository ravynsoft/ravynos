#!perl -w
use strict;

use XS::APItest;
use Test::More;

# Some addresses for testing.
my $a = [];
my $h = {};
my $c = sub {};

my $t1 = XS::APItest::PtrTable->new();
isa_ok($t1, 'XS::APItest::PtrTable');
my $t2 = XS::APItest::PtrTable->new();
isa_ok($t2, 'XS::APItest::PtrTable');
cmp_ok($t1, '!=', $t2, 'Not the same object');

undef $t2;

# Still here? :-)
isa_ok($t1, 'XS::APItest::PtrTable');

is($t1->fetch($a), 0, 'Not found');
is($t1->fetch($h), 0, 'Not found');
is($t1->fetch($c), 0, 'Not found');

$t1->store($a, $h);

cmp_ok($t1->fetch($a), '==', $h, 'Found');
is($t1->fetch($h), 0, 'Not found');
is($t1->fetch($c), 0, 'Not found');

$t1->split();

cmp_ok($t1->fetch($a), '==', $h, 'Found');
is($t1->fetch($h), 0, 'Not found');
is($t1->fetch($c), 0, 'Not found');

done_testing();
