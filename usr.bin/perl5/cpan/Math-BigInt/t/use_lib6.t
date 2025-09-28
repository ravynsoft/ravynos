# -*- mode: perl; -*-

# see if using Math::BigInt and Math::BigFloat works together nicely.
# all use_lib*.t should be equivalent

use strict;
use warnings;
use lib 't';

use Test::More tests => 1;

use Math::BigInt lib => 'BareCalc';             # loads "BareCalc"
eval "use Math::BigFloat only => 'foobar';";    # ignores "foobar"

is(Math::BigInt->config('lib'), 'Math::BigInt::BareCalc',
   "Math::BigInt->config('lib')");
