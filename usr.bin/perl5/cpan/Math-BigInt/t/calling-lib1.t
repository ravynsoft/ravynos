# -*- mode: perl; -*-

use strict;
use warnings;
use lib 't';

use Test::More tests => 1;

my ($x, $expected, $try);

my $class = 'Math::BigInt';

# test whether Math::BigInt::Scalar via use works (w/ dff. spellings of calc)

$try = qq|use $class 0, "lib" => "Scalar";|
     . q| $x = 2**10; $x = "$x";|;
$expected = eval $try;
is($expected, "1024", $try);
