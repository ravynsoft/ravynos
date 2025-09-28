#!/usr/bin/perl -w

# Test that autodie doesn't pollute the caller with carp and croak.

use strict;

use Test::More tests => 2;

use autodie;

ok !defined &main::carp;
ok !defined &main::croak;
