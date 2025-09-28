#!./perl

use strict;
use warnings;

use List::Util qw(reduce min);
use Test::More;
plan tests => 33;

my $v = reduce {};

is( $v, undef, 'no args');

$v = reduce { $a / $b } 756,3,7,4;
is( $v, 9, '4-arg divide');

$v = reduce { $a / $b } 6;
is( $v, 6, 'one arg');

my @a = map { rand } 0 .. 20;
$v = reduce { $a < $b ? $a : $b } @a;
is( $v, min(@a), 'min');

@a = map { pack("C", int(rand(256))) } 0 .. 20;
$v = reduce { $a . $b } @a;
is( $v, join("",@a), 'concat');

sub add {
  my($aa, $bb) = @_;
  return $aa + $bb;
}

$v = reduce { my $t="$a $b\n"; 0+add($a, $b) } 3, 2, 1;
is( $v, 6, 'call sub');

# Check that eval{} inside the block works correctly
$v = reduce { eval { die }; $a + $b } 0,1,2,3,4;
is( $v, 10, 'use eval{}');

$v = !defined eval { reduce { die if $b > 2; $a + $b } 0,1,2,3,4 };
ok($v, 'die');

sub foobar { reduce { (defined(wantarray) && !wantarray) ? $a+1 : 0 } 0,1,2,3 }
($v) = foobar();
is( $v, 3, 'scalar context');

sub add2 { $a + $b }

$v = reduce \&add2, 1,2,3;
is( $v, 6, 'sub reference');

$v = reduce { add2() } 3,4,5;
is( $v, 12, 'call sub');


$v = reduce { eval "$a + $b" } 1,2,3;
is( $v, 6, 'eval string');

$a = 8; $b = 9;
$v = reduce { $a * $b } 1,2,3;
is( $a, 8, 'restore $a');
is( $b, 9, 'restore $b');

# Can we leave the sub with 'return'?
$v = reduce {return $a+$b} 2,4,6;
is($v, 12, 'return');

# ... even in a loop?
$v = reduce {while(1) {return $a+$b} } 2,4,6;
is($v, 12, 'return from loop');

# Does it work from another package?
{ package Foo;
  $a = $b;
  ::is((List::Util::reduce {$a*$b} (1..4)), 24, 'other package');
}

# Can we undefine a reduce sub while it's running?
sub self_immolate {undef &self_immolate; 1}
eval { $v = reduce \&self_immolate, 1,2; };
like($@, qr/^Can't undef active subroutine/, "undef active sub");

# Redefining an active sub should not fail, but whether the
# redefinition takes effect immediately depends on whether we're
# running the Perl or XS implementation.

sub self_updating {
  no warnings 'redefine';
  *self_updating = sub{1};
  1
}
eval { $v = reduce \&self_updating, 1,2; };
is($@, '', 'redefine self');

{ my $failed = 0;

    sub rec { my $n = shift;
        if (!defined($n)) {  # No arg means we're being called by reduce()
            return 1; }
        if ($n<5) { rec($n+1); }
        else { $v = reduce \&rec, 1,2; }
        $failed = 1 if !defined $n;
    }

    rec(1);
    ok(!$failed, 'from active sub');
}

# Calling a sub from reduce should leave its refcount unchanged.
SKIP: {
    skip("No Internals::SvREFCNT", 1) if !defined &Internals::SvREFCNT;
    sub mult {$a*$b}
    my $refcnt = &Internals::SvREFCNT(\&mult);
    $v = reduce \&mult, 1..6;
    is(&Internals::SvREFCNT(\&mult), $refcnt, "Refcount unchanged");
}

{
  my $ok = 'failed';
  local $SIG{__DIE__} = sub { $ok = $_[0] =~ /Not a (subroutine|CODE) reference/ ? '' : $_[0] };
  eval { &reduce('foo',1,2) };
  is($ok, '', 'Not a subroutine reference');
  $ok = 'failed';
  eval { &reduce({},1,2) };
  is($ok, '', 'Not a subroutine reference');
}

# These tests are only relevant for the real multicall implementation. The
# pseudo-multicall implementation behaves differently.
SKIP: {
    $List::Util::REAL_MULTICALL ||= 0; # Avoid use only once
    skip("Poor man's MULTICALL can't cope", 2)
      if !$List::Util::REAL_MULTICALL;

    # Can we goto a label from the reduction sub?
    eval {()=reduce{goto foo} 1,2; foo: 1};
    like($@, qr/^Can't "goto" out of a pseudo block/, "goto label");

    # Can we goto a subroutine?
    eval {()=reduce{goto sub{}} 1,2;};
    like($@, qr/^Can't goto subroutine from a sort sub/, "goto sub");
}

{
  my @ret = reduce { $a + $b } 1 .. 5;
  is_deeply( \@ret, [ 15 ], 'reduce in list context yields only final answer' );
}

# XSUB callback
use constant XSUBC => 42;

is reduce(\&XSUBC, 1, 2, 3), 42, "xsub callbacks";

eval { &reduce(1) };
ok($@ =~ /^Not a subroutine reference/, 'check for code reference');
eval { &reduce(1,2) };
ok($@ =~ /^Not a subroutine reference/, 'check for code reference');
eval { &reduce(qw(a b)) };
ok($@ =~ /^Not a subroutine reference/, 'check for code reference');
eval { &reduce([],1,2,3) };
ok($@ =~ /^Not a subroutine reference/, 'check for code reference');
eval { &reduce(+{},1,2,3) };
ok($@ =~ /^Not a subroutine reference/, 'check for code reference');

my @names = ("a\x{100}c", "d\x{101}efgh", 'ijk');
my $longest = reduce { length($a) > length($b) ? $a : $b } @names;
is( length($longest), 6, 'missing SMG rt#121992');
