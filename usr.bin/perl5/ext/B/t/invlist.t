#!./perl

BEGIN {
    unshift @INC, 't';
    require Config;
    if ( ( $Config::Config{'extensions'} !~ /\bB\b/ ) ) {
        print "1..0 # Skip -- Perl configured without B module\n";
        exit 0;
    }
}

use strict;
use warnings;
use Test::More;

if ( $Config::Config{useithreads} ) {
    plan( skip_all => "Perl compiled with ithreads... no invlist in the example");
}

use_ok('B');

# Somewhat minimal tests.

my $found_invlist;

# we are going to walk this sub
sub check {
    "ABCD" !~ tr/\0-\377//c;    # this is using the Latin1_invlist
}

sub B::OP::visit {
    my $op = shift;

    note ref($op) . " ; NAME: ", $op->name, " ; TYPE: ", $op->type;

    return unless ref $op eq 'B::SVOP' && $op->name eq 'trans';

    my $sv = $op->sv;

    note "SV: ", ref $sv, " = " . $sv->LEN . " " . $sv->CUR;
    foreach my $elt ( $sv->ARRAY ) {
        next unless ref $elt eq 'B::INVLIST';
        $found_invlist = 1;
        my $invlist = $elt;

        is $invlist->prev_index, 0, "prev_index=0";
        is $invlist->is_offset,  0, "is_offset = 0 (false)";

        my @array = $invlist->get_invlist_array;
        is scalar @array, 2, "invlist array size is 2" or diag explain \@array;
        is $array[0], 0,   "PL_Latin1 first value in the invlist array is 0"  or diag explain \@array;
        is $array[1], 256, "PL_Latin1 second value in the invlist array is 0" or diag explain \@array;

        is $invlist->array_len(), 2, "PL_Latin1 array length is 2";
    }

    return;
}

my $op = B::svref_2object( \*main::check );
B::walkoptree( $op->CV->ROOT, 'visit' );

ok $found_invlist, "visited one INVLIST";

done_testing();
