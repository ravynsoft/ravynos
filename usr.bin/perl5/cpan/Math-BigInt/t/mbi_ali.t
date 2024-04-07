# -*- mode: perl; -*-

# test that the new alias names work

use strict;
use warnings;

use Test::More tests => 6;

use Math::BigInt;

our $CLASS;
$CLASS = 'Math::BigInt';

require './t/alias.inc';
