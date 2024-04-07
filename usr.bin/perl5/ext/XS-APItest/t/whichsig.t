#!perl

use strict;
use warnings;

use Test::More tests => 9;

use_ok('XS::APItest');

my @types = map { 'whichsig' . $_ } '', qw( _sv _pv _pvn );

sub test { "Sanity check" }

{
    for my $type ( 0..3 ) {
        is XS::APItest::whichsig_type("KILL", $type), 9, "Sanity check, $types[$type] works";
    }
}

is XS::APItest::whichsig_type("KILL\0whoops", 0), 9, "whichsig() is not nul-clean";

is XS::APItest::whichsig_type("KILL\0whoops", 1), -1, "whichsig_sv() is nul-clean";

is XS::APItest::whichsig_type("KILL\0whoops", 2), 9, "whichsig_pv() is not nul-clean";

is XS::APItest::whichsig_type("KILL\0whoops", 3), -1, "whichsig_pvn() is nul-clean";
