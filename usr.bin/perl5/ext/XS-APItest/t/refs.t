#!perl

use strict;
use warnings;

use Test::More tests => 9;

use Tie::Scalar;

use_ok('XS::APItest');

my $a;
my $sr = \$a;
my $ar = [];
my $hr = {};
my $cr = sub{};

is XS::APItest::take_svref($sr), $sr;
is XS::APItest::take_avref($ar), $ar;
is XS::APItest::take_hvref($hr), $hr;
is XS::APItest::take_cvref($cr), $cr;

my $obj = tie my $ref, 'Tie::StdScalar';
${$obj} = $sr;
is XS::APItest::take_svref($sr), $sr;

${$obj} = $ar;
is XS::APItest::take_avref($ar), $ar;

${$obj} = $hr;
is XS::APItest::take_hvref($hr), $hr;

${$obj} = $cr;
is XS::APItest::take_cvref($cr), $cr;
