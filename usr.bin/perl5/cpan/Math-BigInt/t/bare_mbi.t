# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More tests => 4278;           # tests in require'd file

use lib 't';

use Math::BigInt lib => 'BareCalc';

print "# ", Math::BigInt->config('lib'), "\n";

our ($CLASS, $LIB);
$CLASS = "Math::BigInt";
$LIB   = "Math::BigInt::BareCalc";      # backend

require './t/bigintpm.inc';               # perform same tests as bigintpm.t
