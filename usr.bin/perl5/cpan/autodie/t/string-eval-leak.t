#!/usr/bin/perl -w
use strict;
use warnings;
use Test::More tests => 2;

# Under Perl 5.10.x, a string eval can cause a copy to be taken of
# %^H, which delays stringification of our scope guard objects,
# which in turn causes autodie to leak.  These tests check to see
# if we've successfully worked around this issue.

eval {

    {
        use autodie;
        eval "1";
    }

    open(my $fh, '<', 'this_file_had_better_not_exist');
};

TODO: {
    local $TODO;

    if ( $] >= 5.010 ) {
        $TODO = "Autodie can leak near string evals in 5.10.x";
    }

    is("$@","","Autodie should not leak out of scope");
}

# However, we can plug the leak with 'no autodie'.

no autodie;

eval {
    open(my $fh, '<', 'this_file_had_better_not_exist');
};

is("$@","",'no autodie should be able to workaround this bug');
