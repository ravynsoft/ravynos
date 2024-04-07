#!./perl

use strict;
use warnings;

use Test::More tests => 18;

use Config;
use List::Util qw(sum);

my $v = sum;
is( $v, undef, 'no args');

$v = sum(9);
is( $v, 9, 'one arg');

$v = sum(1,2,3,4);
is( $v, 10, '4 args');

$v = sum(-1);
is( $v, -1, 'one -1');

my $x = -3;

$v = sum($x, 3);
is( $v, 0, 'variable arg');

$v = sum(-3.5,3);
is( $v, -0.5, 'real numbers');

$v = sum(3,-3.5);
is( $v, -0.5, 'initial integer, then real');

my $one = Foo->new(1);
my $two = Foo->new(2);
my $thr = Foo->new(3);

$v = sum($one,$two,$thr);
is($v, 6, 'overload');


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
$v = sum($v1,$v2);
is($v, $v1 + $v2, 'bigint');

$v = sum(42, $v1);
is($v, $v1 + 42, 'bigint + builtin int');

$v = sum(42, $v1, 2);
is($v, $v1 + 42 + 2, 'bigint + builtin int');

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
  my $t = sum($e1, 7, 7);
  is($t, 21, 'overload returning non-overload');
  $t = sum(8, $e1, 8);
  is($t, 23, 'overload returning non-overload');
  $t = sum(9, 9, $e1);
  is($t, 25, 'overload returning non-overload');
}

SKIP: {
  skip "IV is not at least 64bit", 4 unless $Config{ivsize} >= 8;

  # Sum using NV will only preserve 53 bits of integer precision
  my $t = sum(1152921504606846976, 1); # 1<<60, but Perl 5.6 does not compute constant correctly
  cmp_ok($t, 'gt', 1152921504606846976, 'sum uses IV where it can'); # string comparison because Perl 5.6 does not compare it numerically correctly

  SKIP: {
    skip "known to fail on $]", 1 if $] le "5.006002";
    $t = sum(1<<60, 1);
    cmp_ok($t, '>', 1<<60, 'sum uses IV where it can');
  }

  my $min = -(1<<63);
  my $max = 9223372036854775807; # (1<<63)-1, but Perl 5.6 does not compute constant correctly

  $t = sum($min, $max);
  is($t, -1, 'min + max');
  $t = sum($max, $min);
  is($t, -1, 'max + min');
}
