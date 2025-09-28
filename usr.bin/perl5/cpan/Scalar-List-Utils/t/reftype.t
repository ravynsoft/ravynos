#!./perl

use strict;
use warnings;

use Test::More tests => 32;

use Scalar::Util qw(reftype);
use vars qw(*F);
use Symbol qw(gensym);

# Ensure we do not trigger and tied methods
tie *F, 'MyTie';
my $RE = $] < 5.011 ? 'SCALAR' : 'REGEXP';

my $s = []; # SvTYPE($s) is SVt_RV, and SvROK($s) is true
$s = undef; # SvTYPE($s) is SVt_RV, but SvROK($s) is false

my $t;
my @test = (
  [ undef, 1,             'number' ],
  [ undef, 'A',           'string' ],
  [ HASH   => {},         'HASH ref' ],
  [ ARRAY  => [],         'ARRAY ref' ],
  [ SCALAR => \$t,        'SCALAR ref' ],
  [ SCALAR => \$s,        'SCALAR ref (but SVt_RV)' ],
  [ REF    => \(\$t),     'REF ref' ],
  [ GLOB   => \*F,        'tied GLOB ref' ],
  [ GLOB   => gensym,     'GLOB ref' ],
  [ CODE   => sub {},     'CODE ref' ],
  [ IO     => *STDIN{IO}, 'IO ref' ],
  [ $RE    => qr/x/,      'REGEEXP' ],
);

foreach my $test (@test) {
  my($type,$what, $n) = @$test;

  is( reftype($what), $type, $n);
  next unless ref($what);

  bless $what, "ABC";
  is( reftype($what), $type, $n);

  bless $what, "0";
  is( reftype($what), $type, $n);
}

package MyTie;

sub TIEHANDLE { bless {} }
sub DESTROY {}

sub AUTOLOAD {
  our $AUTOLOAD;
  warn "$AUTOLOAD called";
  exit 1; # May be in an eval
}
