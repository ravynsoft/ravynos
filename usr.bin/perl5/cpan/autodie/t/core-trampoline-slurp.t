#!/usr/bin/perl
use strict;
use warnings;
use Test::More tests => 3;

# Tests for GH #22
#
# Slurpy calls (like open, unlink, chown, etc) could not be
# interpreted properly if they leak into another file which
# doesn't have autodie enabled.

use autodie;
use FindBin qw($Bin);
use lib $Bin;
use autodie_test_module;

# This will throw an error, but it shouldn't throw a leak-guard
# failure.
eval { slurp_leak_open(); };
unlike($@,qr/Leak-guard failure/, "Leak guard failure (open)");

eval { slurp_leak_unlink(); };
is($@,"","No error should be thrown by leaked guards (unlink)");
unlike($@,qr/Leak-guard failure/, "Leak guard failure (unlink)");
