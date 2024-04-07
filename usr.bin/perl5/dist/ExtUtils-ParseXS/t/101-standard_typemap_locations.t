#!/usr/bin/perl
use strict;
use warnings;
use Test::More tests =>  3;
use ExtUtils::ParseXS::Utilities qw(
  standard_typemap_locations
);

{
    local @INC = @INC;
    my @stl = standard_typemap_locations( \@INC );
    ok( @stl >= 9, "At least 9 entries in typemap locations list" );
    is( $stl[$#stl], 'typemap',
        "Last element is typemap in current directory");
    SKIP: {
        skip "No lib/ExtUtils/ directories under directories in \@INC",
        1
        unless @stl > 9;

        # We check only as many location entries from the start of the array
        # (where the @INC-related entries are) as there are entries from @INC.
        # We manage to do that by stopping when we find the "updir" related
        # entries, which we assume is never contained in a default @INC entry.
        my $updir = File::Spec->updir;
        my $max = $#INC;
        $max = $#stl if $#stl < $max;
        foreach my $i (0.. $max) {
          $max = $i, last if $stl[$i] =~ /\Q$updir\E/;
        }

        ok(
            ( 0 < (grep -f $_, @stl[0..$max]) ),
            "At least one typemap file exists underneath \@INC directories"
        );
    }
}

