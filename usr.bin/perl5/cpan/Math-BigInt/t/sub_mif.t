# -*- mode: perl; -*-

# test rounding, accuracy, precision and fallback, round_mode and mixing
# of classes

use strict;
use warnings;

use Test::More tests => 712;

use lib 't';

use Math::BigInt::Subclass;
use Math::BigFloat::Subclass;

our ($mbi, $mbf);
$mbi = 'Math::BigInt::Subclass';
$mbf = 'Math::BigFloat::Subclass';

require './t/mbimbf.inc';
