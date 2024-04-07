#!./perl
#
# This test file is used to bulk check that /\s/ and /[\s]/ 
# test the same and that /\s/ and /\S/ are opposites, and that
# /[\s]/ and /[\S]/ are also opposites, for \s/\S and \d/\D and 
# \w/\W.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc( '../lib', '.' );
}

use strict;
use warnings;
use 5.010;

plan tests => 9;  # Update this when adding/deleting tests.

sub run_tests;

$| = 1;

run_tests() unless caller;

#
# Tests start here.
#
sub run_tests {
    my $upper_bound= 10_000;
    for my $special (qw(\s \w \d)) {
        my $upper= uc($special);
        my @cc_plain_failed;
        my @cc_complement_failed;
        my @plain_complement_failed;
        for my $ord (0 .. $upper_bound) {
            my $ch= chr $ord;
            my $ord = sprintf "U+%04X", $ord;  # For display in Unicode terms
            my $plain= $ch=~/$special/ ? 1 : 0;
            my $plain_u= $ch=~/$upper/ ? 1 : 0;
            push @plain_complement_failed, "$ord-$plain-$plain_u" if $plain == $plain_u;

            my $cc= $ch=~/[$special]/ ? 1 : 0;
            my $cc_u= $ch=~/[$upper]/ ? 1 : 0;
            push @cc_complement_failed, "$ord-$cc-$cc_u" if $cc == $cc_u;

            push @cc_plain_failed, "$ord-$plain-$cc" if $plain != $cc;
        }
        is(join(" | ",@cc_plain_failed),"", "Check that /$special/ and /[$special]/ match same things (ord-plain-cc)");
        is(join(" | ",@plain_complement_failed),"", "Check that /$special/ and /$upper/ are complements (ord-plain-plain_u)");
        is(join(" | ",@cc_complement_failed),"", "Check that /[$special]/ and /[$upper]/ are complements (ord-cc-cc_u)");
    }
} # End of sub run_tests

1;
