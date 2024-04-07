#!./perl

use strict;
use warnings;

use Test::More;
use XS::APItest;

# basic constants
{
    ok(SvIsBOOL(!!0), 'false is boolean');
    ok(SvIsBOOL(!!1), 'true is boolean');

    ok(!SvIsBOOL(0), '0 is not boolean');
    ok(!SvIsBOOL(1), '1 is not boolean');
    ok(!SvIsBOOL(""), '"" is not boolean');
}

# variables
{
    my $falsevar = !!0;
    my $truevar  = !!1;

    ok(SvIsBOOL($falsevar), 'false var is boolean');
    ok(SvIsBOOL($truevar),  'true var is boolean');

    my $str = "$truevar";
    my $num = $truevar + 0;

    ok(!SvIsBOOL($str), 'stringified true is not boolean');
    ok(!SvIsBOOL($num), 'numified true is not boolean');

    ok(SvIsBOOL($truevar), 'true var remains boolean after stringification and numification');
}

# aggregate members
{
    my %hash = ( false => !!0, true => !!1 );

    ok(SvIsBOOL($hash{false}), 'false HELEM is boolean');
    ok(SvIsBOOL($hash{true}),  'true HELEM is boolean');

    # We won't test AELEM but it's likely to be the same
}


is(test_bool_internals(), 0, "Bulk test internal bool related APIs");


done_testing;
