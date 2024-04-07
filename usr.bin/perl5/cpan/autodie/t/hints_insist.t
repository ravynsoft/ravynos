#!/usr/bin/perl -w
use strict;
use warnings;
use autodie;

use Test::More tests => 5;

use FindBin qw($Bin);
use lib "$Bin/lib";

use Hints_provider_does qw(always_pass always_fail no_hints);

eval "use autodie qw( ! always_pass always_fail); ";
is("$@", "", "Insisting on good hints (distributed insist)");

is(always_pass(), "foo", "Always_pass() should still work");
is(always_fail(), "foo", "Always_pass() should still work");

eval "use autodie qw(!always_pass !always_fail); ";
is("$@", "", "Insisting on good hints (individual insist)");

my $ret = eval "use autodie qw(!no_hints); 1;";
isnt("$@", "", "Asking for non-existent hints");
