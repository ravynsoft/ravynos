# -*- mode: perl; -*-

# test the "l", "lib", "try" and "only" options:

use strict;
use warnings;

use Test::More tests => 21;

use bignum;

# Catch warnings.

my $WARNINGS;
local $SIG{__WARN__} = sub {
    $WARNINGS = $_[0];
};

my $rc;

$WARNINGS = "";
$rc = eval { bignum -> import("l" => "foo") };
is($@, '',
   qq|eval { bignum -> import("l" => "foo") }|);
is($WARNINGS, "", "no warnings");

$WARNINGS = "";
$rc = eval { bignum -> import("lib" => "foo") };
is($@, '',
   qq|eval { bignum -> import("lib" => "foo") }|);
is($WARNINGS, "", "no warnings");

$WARNINGS = "";
$rc = eval { bignum -> import("try" => "foo") };
is($@, '',
   qq|eval { bignum -> import("try" => "foo") }|);
is($WARNINGS, "", "no warnings");

$WARNINGS = "";
$rc = eval { bignum -> import("try" => "foo") };
is($@, '',
   qq|eval { bignum -> import("try" => "foo") }|);
is($WARNINGS, "", "no warnings");

$WARNINGS = "";
$rc = eval { bignum -> import("foo" => "bar") };
like($@, qr/^Unknown option/,
     qq|eval { bignum -> import("foo" => "bar") }|);
is($WARNINGS, "", "no warnings");

$WARNINGS = "";
$rc = eval { bignum -> import("only" => "bar") };
is($@, "",
   qq|eval { bignum -> import("only" => "bar") }|);
is($WARNINGS, "", "no warnings");

# test that options are only lowercase (don't see a reason why allow UPPER)

foreach (qw/L LIB Lib T Trace TRACE V Version VERSION/) {
    $rc = eval { bignum -> import($_ => "bar") };
    like($@, qr/^Unknown option/i,   # should die
        qq|eval { bignum -> import($_ => "bar") }|);
}
