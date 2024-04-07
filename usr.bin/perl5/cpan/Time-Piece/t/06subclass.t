#!perl
use strict;
use warnings;

# This test file exists to show that Time::Piece can be subclassed and that its
# methods will return objects of the class on which they're called.

use Test::More 'no_plan';

use lib "t/lib";
use Time::Piece::Twin;

BEGIN { use_ok('Time::Piece'); }

my $class = 'Time::Piece::Twin';

for my $method (qw(new localtime gmtime)) {
  my $piece = $class->$method;
  isa_ok($piece, $class, "timepiece made via $method");
}

{
  my $piece = $class->strptime("2005-01-01", "%Y-%m-%d");
  isa_ok($piece, $class, "timepiece made via strptime");
}

{
  my $piece = $class->new;
  isa_ok($piece, $class, "timepiece made via new (again)");

  my $sum = $piece + 86_400;
  isa_ok($sum, $class, "tomorrow via addition operator");

  my $diff = $piece - 86_400;
  isa_ok($diff, $class, "yesterday via subtraction operator");
}

{
  my $g = $class->gmtime;
  my $l = $class->localtime;

  #via clone
  my $l_clone = $class->new($l);
  isa_ok($l_clone, $class, 'custom localtime via clone');
  cmp_ok("$l_clone", 'eq', "$l", 'Clones match');

  #via clone with gmtime
  my $g_clone = $class->new($g);
  isa_ok($g_clone, $class, 'custom gmtime via clone');
  cmp_ok("$g_clone", 'eq', "$g", 'Clones match');
}

{
  # let's verify that we can use gmtime from T::P without the export magic
  my $piece = Time::Piece::gmtime;
  isa_ok($piece, "Time::Piece", "object created via full-qualified gmtime");
  isnt(ref $piece, 'Time::Piece::Twin', "it's not a Twin");
}



{
  my $class = "Time::Piece::NumString";
  my $piece = $class->strptime ("2006", "%Y");
  is (2007 - $piece, 1,
      "subtract attempts stringify for unrecognized objects.");
}

## Below is a package which only changes the stringify function.
{
  package Time::Piece::NumString;
  use base qw(Time::Piece);
  use overload '""' => \&_stringify;
  sub _stringify
  {
    my $self = shift;
    return $self->strftime ("%Y");
  }
}
