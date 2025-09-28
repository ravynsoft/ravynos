# -*- mode: perl; -*-

# test rounding, accuracy, precision and fallback, round_mode and mixing
# of classes under Math::BigInt::BareCalc

use strict;
use warnings;

use Test::More tests => 712             # tests in require'd file
                        + 1;            # tests in this file

use lib 't';

use Math::BigInt   lib => 'BareCalc';
use Math::BigFloat lib => 'BareCalc';

our ($mbi, $mbf);
$mbi = 'Math::BigInt';
$mbf = 'Math::BigFloat';

is(Math::BigInt->config('lib'), 'Math::BigInt::BareCalc',
   "Math::BigInt->config('lib')");

require './t/mbimbf.inc';
