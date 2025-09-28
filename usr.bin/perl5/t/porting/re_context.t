#!./perl -w
#
# Check that utf8.pm and its dependencies only use the subset of the
# $1..$n capture vars that Perl_save_re_context() is hard-coded to
# localise, because that function has no efficient way of determining at
# runtime what vars to localise.
#
# Note that this script tests for the existence of symbol table entries in
# %::, so @4 etc would trigger a failure as well as $4.
#
# If tests start to fail, either (in order of descending preference):
#
# * fix utf8.pm or its dependencies so that any recent change no longer
#   uses more special vars (ideally it would use no vars);
#
# * fix Perl_save_re_context() so that it localises more vars, then
#   update this test script with the new relaxed var list.


use warnings;
use strict;

# trigger the dependency loading

my $x = lc "\x{411}";

# determine which relevant vars those dependencies accessed

my @vars =
        grep !/^[0123]$/, # $0, and $1, ..$3 allowed
        grep /^(?:\d+|[`'&])$/,  # numeric and $`, $&, $' vars
        sort keys %::;

# load any other modules *after* calculating @vars

require './test.pl';

plan(1);

is(scalar @vars, 0, "extraneous vars")
    or diag("extra vars seen: " . join(", ", map "*$_", @vars));

exit 0;
