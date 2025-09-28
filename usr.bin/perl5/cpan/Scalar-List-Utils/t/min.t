#!./perl

use strict;
use warnings;

use Test::More tests => 22;
use List::Util qw(min);

my $v;

ok(defined &min, 'defined');

$v = min(9);
is($v, 9, 'single arg');

$v = min (1,2);
is($v, 1, '2-arg ordered');

$v = min(2,1);
is($v, 1, '2-arg reverse ordered');

my @a = map { rand() } 1 .. 20;
my @b = sort { $a <=> $b } @a;
$v = min(@a);
is($v, $b[0], '20-arg random order');

my $one = Foo->new(1);
my $two = Foo->new(2);
my $thr = Foo->new(3);

$v = min($one,$two,$thr);
is($v, 1, 'overload');

$v = min($thr,$two,$one);
is($v, 1, 'overload');

{ package Foo;

use overload
  '""' => sub { ${$_[0]} },
  '0+' => sub { ${$_[0]} },
  '<'  => sub { ${$_[0]} < ${$_[1]} },
  fallback => 1;
  sub new {
    my $class = shift;
    my $value = shift;
    bless \$value, $class;
  }
}

use Math::BigInt;

my $v1 = Math::BigInt->new(2) ** Math::BigInt->new(65);
my $v2 = $v1 - 1;
my $v3 = $v2 - 1;
$v = min($v1,$v2,$v1,$v3,$v1);
is($v, $v3, 'bigint');

$v = min($v1, 1, 2, 3);
is($v, 1, 'bigint and normal int');

$v = min(1, 2, $v1, 3);
is($v, 1, 'bigint and normal int');

{
    # test that min/max and sum call GETMAGIC properly
    # note, in my tests how this fails depends on exactly
    # which List::Util subs are called and in what order.
    my @list;
    for my $size (10, 20, 10, 30) {
        @list = ( 1 ) x $size;

        my $sum= List::Util::sum( 0, $#list );
        ok( $sum == $size-1, "sum(\$#list, 0) == $size-1");

        my $min= List::Util::min( 15, $#list );
        ok( $min <= 15, "min(15,$size)" );

        my $max= List::Util::max( 0, $#list );
        ok( $max == $size-1, "max(\$#list, 0) == $size-1");
    }
}
