#!./perl

use strict;
use warnings;

use Test::More tests => 32;

use Scalar::Util qw(refaddr);
use vars qw(*F);
use Symbol qw(gensym);

# Ensure we do not trigger and tied methods
tie *F, 'MyTie';

my $i = 1;
foreach my $v (undef, 10, 'string') {
  is(refaddr($v), undef, "not " . (defined($v) ? "'$v'" : "undef"));
}

my $t;
foreach my $r ({}, \$t, [], \*F, sub {}) {
  my $n = "$r";
  $n =~ /0x(\w+)/;
  my $addr = do { no warnings; hex $1 };
  my $before = ref($r);
  is( refaddr($r), $addr, $n);
  is( ref($r), $before, $n);

  my $obj = bless $r, 'FooBar';
  is( refaddr($r), $addr, "blessed with overload $n");
  is( ref($r), 'FooBar', $n);
}

{
  my $z = '77';
  my $y = \$z;
  my $a = '78';
  my $b = \$a;
  tie my %x, 'Hash3', {};
  $x{$y} = 22;
  $x{$b} = 23;
  my $xy = $x{$y};
  my $xb = $x{$b}; 
  ok(ref($x{$y}));
  ok(ref($x{$b}));
  ok(refaddr($xy) == refaddr($y));
  ok(refaddr($xb) == refaddr($b));
  ok(refaddr($x{$y}));
  ok(refaddr($x{$b}));
}
{
  my $z = bless {}, '0';
  ok(refaddr($z));
  {
    no strict 'refs';
    @{"0::ISA"} = qw(FooBar);
  }
  my $a = {};
  my $r = refaddr($a);
  $z = bless $a, '0';
  ok(refaddr($z) > 10);
  is(refaddr($z),$r,"foo");
}

package FooBar;

use overload
    '0+'  => sub { 10 },
    '+'   => sub { 10 + $_[1] },
    '""'  => sub { "10" };

package MyTie;

sub TIEHANDLE { bless {} }
sub DESTROY {}

sub AUTOLOAD {
  our $AUTOLOAD;
  warn "$AUTOLOAD called";
  exit 1; # May be in an eval
}

package Hash3;

use Scalar::Util qw(refaddr);

sub TIEHASH
{
    my $pkg = shift;
    return bless [ @_ ], $pkg;
}
sub FETCH
{
    my $self = shift;
    my $key = shift;
    my ($underlying) = @$self;
    return $underlying->{refaddr($key)};
}
sub STORE
{
    my $self = shift;
    my $key = shift;
    my $value = shift;
    my ($underlying) = @$self;
    return ($underlying->{refaddr($key)} = $key);
}
