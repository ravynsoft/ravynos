#!./perl

use strict;
use warnings;

use Test::More tests => 12;
use Scalar::Util qw(blessed);

my $t;

ok(!defined blessed(undef), 'undef is not blessed');
ok(!defined blessed(1),     'Numbers are not blessed');
ok(!defined blessed('A'),   'Strings are not blessed');
ok(!defined blessed({}),    'Unblessed HASH-ref');
ok(!defined blessed([]),    'Unblessed ARRAY-ref');
ok(!defined blessed(\$t),   'Unblessed SCALAR-ref');

my $x;

$x = bless [], "ABC";
is(blessed($x), "ABC", 'blessed ARRAY-ref');

$x = bless {}, "DEF";
is(blessed($x), "DEF", 'blessed HASH-ref');

$x = bless {}, "0";
cmp_ok(blessed($x), "eq", "0", 'blessed HASH-ref');

{
  my $blessed = do {
    my $depth;
    no warnings 'redefine';
    local *UNIVERSAL::can = sub { die "Burp!" if ++$depth > 2; blessed(shift) };
    $x = bless {}, "DEF";
    blessed($x);
  };
  is($blessed, "DEF", 'recursion of UNIVERSAL::can');
}

{
  package Broken;
  sub isa { die };
  sub can { die };

  my $obj = bless [], __PACKAGE__;
  ::is( ::blessed($obj), __PACKAGE__, "blessed on broken isa() and can()" );
}

SKIP: {
  # Unicode package names only supported in perl 5.16 onwards
  skip "Unicode package names are not supported", 1 if $] < 5.016;

  my $utf8_pack= "X\x{100}";
  my $obj= bless {}, $utf8_pack;
  ::is( ::blessed($obj), $utf8_pack, "blessed preserves utf8ness for utf8 class names" );
}
