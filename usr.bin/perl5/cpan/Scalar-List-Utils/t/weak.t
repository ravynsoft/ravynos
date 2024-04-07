#!./perl

use strict;
use warnings;

use Config;

use Scalar::Util qw(weaken unweaken isweak);
use Test::More tests => 28;

# two references, one is weakened, the other is then undef'ed.
{
  my ($y,$z);

  {
    my $x = "foo";
    $y = \$x;
    $z = \$x;
  }

  ok(ref($y) and ref($z));

  weaken($y);
  ok(ref($y) and ref($z));

  undef($z);
  ok(not(defined($y) and defined($z)));

  undef($y);
  ok(not(defined($y) and defined($z)));
}

# one reference, which is weakened
{
  my $y;

  {
    my $x = "foo";
    $y = \$x;
  }

  ok(ref($y));

  weaken($y);
  ok(not defined $y);
}

my $flag;

# a circular structure
{
  $flag = 0;

  {
    my $y = bless {}, 'Dest';
    $y->{Self} = $y;
    $y->{Flag} = \$flag;

    weaken($y->{Self});
    ok( ref($y) );
  }

  ok( $flag == 1 );
  undef $flag;
}

# a more complicated circular structure
{
  $flag = 0;

  {
    my $y = bless {}, 'Dest';
    my $x = bless {}, 'Dest';
    $x->{Ref} = $y;
    $y->{Ref} = $x;
    $x->{Flag} = \$flag;
    $y->{Flag} = \$flag;

    weaken($x->{Ref});
  }
  ok( $flag == 2 );
}

# deleting a weakref before the other one
{
  my ($y,$z);
  {
    my $x = "foo";
    $y = \$x;
    $z = \$x;
  }

  weaken($y);
  undef($y);

  ok(not defined $y);
  ok(ref($z) );
}

# isweakref
{
  $a = 5;
  ok(!isweak($a));
  $b = \$a;
  ok(!isweak($b));
  weaken($b);
  ok(isweak($b));
  $b = \$a;
  ok(!isweak($b));

  my $x = {};
  weaken($x->{Y} = \$a);
  ok(isweak($x->{Y}));
  ok(!isweak($x->{Z}));
}

# unweaken
{
  my ($y,$z);
  {
    my $x = "foo";
    $y = \$x;
    $z = \$x;
  }

  weaken($y);

  ok(isweak($y), '$y is weak after weaken()');
  is($$y, "foo", '$y points at \"foo" after weaken()');

  unweaken($y);

  is(ref $y, "SCALAR", '$y is still a SCALAR ref after unweaken()');
  ok(!isweak($y), '$y is not weak after unweaken()');
  is($$y, "foo", '$y points at \"foo" after unweaken()');

  undef $z;
  ok(defined $y, '$y still defined after undef $z');
}

# test weaken on a read only ref
SKIP: {
  # Doesn't work for older perls, see bug [perl #24506]
  skip("Test does not work with perl < 5.8.3", 5) if $] < 5.008003;

  # in a MAD build, constants have refcnt 2, not 1
  skip("Test does not work with MAD", 5) if exists $Config{mad};

  $a = eval '\"hello"';
  ok(ref($a)) or print "# didn't get a ref from eval\n";

  $b = $a;
  eval { weaken($b) };
  # we didn't die
  is($@, "");
  ok(isweak($b));
  is($$b, "hello");

  $a="";
  ok(not $b) or diag("b did not go away");
}

package Dest;

sub DESTROY {
  ${$_[0]{Flag}} ++;
}
