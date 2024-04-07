# -*- mode: perl; -*-

# Test use Math::BigFloat with => 'Math::BigInt::SomeSubclass';

use strict;
use warnings;

use Test::More tests => 3070            # tests in require'd file
                         + 1;           # tests in this file

use Math::BigFloat with => 'Math::BigInt::Subclass',
                   lib  => 'Calc';

our ($CLASS, $LIB);
$CLASS = "Math::BigFloat";
$LIB   = "Math::BigInt::Calc";          # backend

# the "with" argument should be ignored
is(Math::BigFloat->config("with"), 'Math::BigInt::Calc',
   qq|Math::BigFloat->config("with")|);

require './t/bigfltpm.inc';     # all tests here for sharing
