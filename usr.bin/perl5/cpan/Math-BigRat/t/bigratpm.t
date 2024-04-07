# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More tests => 899;

use Math::BigRat lib => 'Calc';

our ($CLASS, $LIB);
$CLASS = "Math::BigRat";
$LIB  = "Math::BigInt::Calc";  # backend

require './t/bigratpm.inc';     # all tests here for sharing
