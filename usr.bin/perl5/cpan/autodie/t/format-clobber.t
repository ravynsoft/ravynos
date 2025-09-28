#!/usr/bin/env perl
use warnings;
use strict;

use FindBin;
use lib "$FindBin::Bin/lib";
use Test::More tests => 21;

our ($pvio, $pvfm);

use_ok('OtherTypes');

# Since we use use_ok, this is effectively 'compile time'.

ok( defined *OtherTypes::foo{SCALAR},
    "SCALAR slot intact at compile time" );
ok( defined *OtherTypes::foo{ARRAY},
    "ARRAY slot intact at compile time" );
ok( defined *OtherTypes::foo{HASH},
    "HASH slot intact at compile time" );
ok( defined *OtherTypes::foo{IO},
    "IO slot intact at compile time" );
ok( defined *OtherTypes::foo{FORMAT},
    "FORMAT slot intact at compile time" );

is( $OtherTypes::foo, 23,
    "SCALAR slot correct at compile time" );
is( $OtherTypes::foo[0], "bar",
    "ARRAY slot correct at compile time" );
is( $OtherTypes::foo{mouse}, "trap",
    "HASH slot correct at compile time" );
is( *OtherTypes::foo{IO}, $pvio,
    "IO slot correct at compile time" );
is( *OtherTypes::foo{FORMAT}, $pvfm,
    "FORMAT slot correct at compile time" );

eval q{
    ok( defined *OtherTypes::foo{SCALAR},
        "SCALAR slot intact at run time" );
    ok( defined *OtherTypes::foo{ARRAY},
        "ARRAY slot intact at run time" );
    ok( defined *OtherTypes::foo{HASH},
        "HASH slot intact at run time" );
    ok( defined *OtherTypes::foo{IO},
        "IO slot intact at run time" );

    TODO: {
        local $TODO = "Copying formats fails due to a bug in Perl.";
        ok( defined *OtherTypes::foo{FORMAT},
            "FORMAT slot intact at run time" );
    }

    is( $OtherTypes::foo, 23,
        "SCALAR slot correct at run time" );
    is( $OtherTypes::foo[0], "bar",
        "ARRAY slot correct at run time" );
    is( $OtherTypes::foo{mouse}, "trap",
        "HASH slot correct at run time" );
    is( *OtherTypes::foo{IO}, $pvio,
        "IO slot correct at run time" );

    TODO: {
        local $TODO = "Copying formats fails due to a bug in Perl.";
        is( *OtherTypes::foo{FORMAT}, $pvfm,
            "FORMAT slot correct at run time" );
    }
};
