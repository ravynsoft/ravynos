# -*- mode: perl; -*-

use strict;
use warnings;

# Test 2 levels of upgrade classes. This used to cause a segv.

use Test::More tests => 1;

use Math::BigInt upgrade => 'Math::BigFloat';
use Math::BigFloat upgrade => 'Math::BigMouse';

no warnings 'once';
@Math::BigMouse::ISA = 'Math::BigFloat';
sub Math::BigMouse::bsqrt {};

() = sqrt Math::BigInt->new(2);
pass('sqrt on a big int does not segv if there are 2 upgrade levels');
