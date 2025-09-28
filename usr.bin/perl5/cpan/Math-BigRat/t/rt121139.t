# -*- mode: perl; -*-

# check for cpan rt #121139

use strict;
use warnings;
use Test::More tests => 2;
use Math::BigRat;

my $a = Math::BigRat->new('3/2');
my $x = Math::BigRat->new('2/3');
is("$a", "3/2");

my $y = $a;
$y = $x * $y;
is("$a", "3/2");
