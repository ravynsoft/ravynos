# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More tests => 3070            # tests in require'd file
                         + 8;           # tests in this file

use lib 't';

use Math::BigFloat::Subclass;

our ($CLASS, $LIB);
$CLASS = "Math::BigFloat::Subclass";
$LIB   = Math::BigFloat->config('lib');         # backend library

require './t/bigfltpm.inc';     # perform same tests as bigfltpm

###############################################################################
# Now do custom tests for Subclass itself

my $ms = $CLASS->new(23);
is($ms->{_custom}, 1, '$ms has custom attribute \$ms->{_custom}');

# Check that subclass is a Math::BigFloat, but not a Math::Bigint
isa_ok($ms, 'Math::BigFloat');
ok(!$ms->isa('Math::BigInt'),
   "An object of class '" . ref($ms) . "' isn't a 'Math::BigInt'");

use Math::BigFloat;

my $bf = Math::BigFloat->new(23);       # same as other
$ms += $bf;
is($ms, 46, '$ms is 46');
is($ms->{_custom}, 1, '$ms has custom attribute $ms->{_custom}');
is(ref($ms), $CLASS, "\$ms is not an object of class '$CLASS'");

cmp_ok(Math::BigFloat::Subclass -> div_scale(), "==", 40,
      "Math::BigFloat::Subclass gets 'div_scale' from parent");

is(Math::BigFloat::Subclass -> round_mode(), "even",
   "Math::BigFloat::Subclass gets 'round_mode' from parent");
