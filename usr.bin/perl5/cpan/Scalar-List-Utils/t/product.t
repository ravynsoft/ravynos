#!./perl

use strict;
use warnings;

use Test::More tests => 27;

use Config;
use List::Util qw(product);

my $v = product;
is( $v, 1, 'no args');

$v = product(9);
is( $v, 9, 'one arg');

$v = product(1,2,3,4);
is( $v, 24, '4 args');

$v = product(-1);
is( $v, -1, 'one -1');

$v = product(0, 1, 2);
is( $v, 0, 'first factor zero' );

$v = product(0, 1);
is( $v, 0, '0 * 1');

$v = product(1, 0);
is( $v, 0, '1 * 0');

$v = product(0, 0);
is( $v, 0, 'two 0');

# RT139601 cornercases
{
  # Numify the result because some older perl versions see "-0" as a string
  is( 0+product(-1.0, 0), 0, 'product(-1.0, 0)' );
  is( 0+product(-1, 0), 0, 'product(-1, 0)' );
}

my $x = -3;

$v = product($x, 3);
is( $v, -9, 'variable arg');

$v = product(-3.5,3);
is( $v, -10.5, 'real numbers');

my $one  = Foo->new(1);
my $two  = Foo->new(2);
my $four = Foo->new(4);

$v = product($one,$two,$four);
is($v, 8, 'overload');


{ package Foo;

use overload
  '""' => sub { ${$_[0]} },
  '0+' => sub { ${$_[0]} },
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
$v = product($v1,$v2);
is($v, $v1 * $v2, 'bigint');

$v = product(42, $v1);
is($v, $v1 * 42, 'bigint + builtin int');

$v = product(42, $v1, 2);
is($v, $v1 * 42 * 2, 'bigint + builtin int');

{ package example;

  use overload
    '0+' => sub { $_[0][0] },
    '""' => sub { my $r = "$_[0][0]"; $r = "+$r" unless $r =~ m/^\-/; $r .= " [$_[0][1]]"; $r },
    fallback => 1;

  sub new {
    my $class = shift;

    my $this = bless [@_], $class;

    return $this;
  }
}

{
  my $e1 = example->new(7, "test");
  my $t = product($e1, 7, 7);
  is($t, 343, 'overload returning non-overload');
  $t = product(8, $e1, 8);
  is($t, 448, 'overload returning non-overload');
  $t = product(9, 9, $e1);
  is($t, 567, 'overload returning non-overload');
}

SKIP: {
  skip "IV is not at least 64bit", 8 unless $Config{ivsize} >= 8;

  my $t;
  my $min = -(1<<31);
  my $max = (1<<31)-1;

  $t = product($min, $min);
  is($t,  1<<62, 'min * min');
  $t = product($min, $max);
  is($t, (1<<31) - (1<<62), 'min * max');
  $t = product($max, $min);
  is($t, (1<<31) - (1<<62), 'max * min');

  $t = product($max, $max);
  is($t,  4611686014132420609, 'max * max'); # (1<<62)-(1<<32)+1), but Perl 5.6 does not compute constant correctly

  $t = product($min*8, $min);
  cmp_ok($t, '>',  (1<<61), 'min*8*min'); # may be an NV
  $t = product($min*8, $max);
  cmp_ok($t, '<', -(1<<61), 'min*8*max'); # may be an NV
  $t = product($max, $min*8);
  cmp_ok($t, '<', -(1<<61), 'min*max*8'); # may be an NV
  $t = product($max, $max*8);
  cmp_ok($t, '>',  (1<<61), 'max*max*8'); # may be an NV

}
