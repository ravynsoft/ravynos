# -*- mode: perl; -*-

# test that the new alias names work

use strict;
use warnings;

use Test::More tests => 6;

use lib 't';

use Math::BigInt::Subclass;

our $CLASS;
$CLASS = 'Math::BigInt::Subclass';

require './t/alias.inc';
