# -*- mode: perl; -*-

# test that the new alias names work

use strict;
use warnings;

use Test::More tests => 6;

use Math::BigFloat;

our $CLASS;
$CLASS = 'Math::BigFloat';

require './t/alias.inc';
