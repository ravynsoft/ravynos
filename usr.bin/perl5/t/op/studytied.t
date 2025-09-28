#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use strict;
use warnings;

plan tests => 14;

{
    package J;
    my $c = 0;
    sub reset { $c = 0 }
    sub TIESCALAR { bless [] }
    sub FETCH { $c++ ? "next" : "first" }
}

# This test makes sure that we can't pull a fast one on study().  If we
# study() a tied variable, perl should know that the studying isn't
# valid on subsequent references, and should account for it.

for my $do_study (0,1) {
    J::reset();
    my $x;
    tie $x, "J";

    if ($do_study) {
        study $x;
        pass( "Studying..." );
    } else {
        my $first_fetch = $x;
        pass( "Not studying..." );
    }

    # When it was studied (or first_fetched), $x was "first", but is now "next", so
    # should not match /f/.
    ok( $x !~ /f/,              qq{"next" doesn't match /f/} );
    is( index( $x, 'f' ), -1,   qq{"next" doesn't contain "f"} );

    # Subsequent references to $x are "next", so should match /n/
    ok( $x =~ /n/,              qq{"next" matches /n/} );
    is( index( $x, 'n' ), 0,    qq{"next" contains "n" at pos 0} );

    # The letter "t" is in both, but in different positions
    ok( $x =~ /t/,              qq{"next" matches /t/} );
    is( index( $x, 't' ), 3,    qq{"next" contains "t" at pos 3} );
}
