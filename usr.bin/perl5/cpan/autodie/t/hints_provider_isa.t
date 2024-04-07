#!/usr/bin/perl -w
use strict;
use warnings;
use autodie;

use Test::More 'no_plan';

use FindBin qw($Bin);
use lib "$Bin/lib";

use Hints_provider_isa qw(always_pass always_fail);
use autodie qw(always_pass always_fail);

eval { my $x = always_pass() };
is("$@", "", "always_pass in scalar context");

eval { my @x = always_pass() };
is("$@", "", "always_pass in list context");

eval { my $x = always_fail() };
isnt("$@", "", "always_fail in scalar context");

eval { my @x = always_fail() };
isnt("$@", "", "always_fail in list context");
