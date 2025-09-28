# -*- mode: perl; -*-

# test for bug #34584: hang in exp(1/2)

use strict;
use warnings;

use Test::More tests => 1;

use Math::BigRat;

my $result = Math::BigRat->new('1/2')->bexp();

is("$result", "824360635350064073424325393907081785827/500000000000000000000000000000000000000",
   "exp(1/2) worked");

##############################################################################
# done

1;
