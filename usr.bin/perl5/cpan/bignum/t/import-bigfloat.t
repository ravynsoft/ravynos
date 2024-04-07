# -*- mode: perl; -*-

# test the "l", "lib", "try" and "only" options:

use strict;
use warnings;

use Test::More tests => 21;

use bigfloat;

# Catch warnings.

my $WARNINGS;
local $SIG{__WARN__} = sub {
    $WARNINGS = $_[0];
};

my $rc;

$WARNINGS = "";
$rc = eval { bigfloat -> import("l" => "foo") };
is($@, '',
   qq|eval { bigfloat -> import("l" => "foo") }|);
is($WARNINGS, "", "no warnings");

$WARNINGS = "";
$rc = eval { bigfloat -> import("lib" => "foo") };
is($@, '',
   qq|eval { bigfloat -> import("lib" => "foo") }|);
is($WARNINGS, "", "no warnings");

$WARNINGS = "";
$rc = eval { bigfloat -> import("try" => "foo") };
is($@, '',
   qq|eval { bigfloat -> import("try" => "foo") }|);
is($WARNINGS, "", "no warnings");

$WARNINGS = "";
$rc = eval { bigfloat -> import("try" => "foo") };
is($@, '',
   qq|eval { bigfloat -> import("try" => "foo") }|);
is($WARNINGS, "", "no warnings");

$WARNINGS = "";
$rc = eval { bigfloat -> import("foo" => "bar") };
like($@, qr/^Unknown option/,
     qq|eval { bigfloat -> import("foo" => "bar") }|);
is($WARNINGS, "", "no warnings");

$WARNINGS = "";
$rc = eval { bigfloat -> import("only" => "bar") };
is($@, "",
   qq|eval { bigfloat -> import("only" => "bar") }|);
is($WARNINGS, "", "no warnings");

# test that options are only lowercase (don't see a reason why allow UPPER)

foreach (qw/L LIB Lib T Trace TRACE V Version VERSION/) {
    $rc = eval { bigfloat -> import($_ => "bar") };
    like($@, qr/^Unknown option/i,   # should die
        qq|eval { bigfloat -> import($_ => "bar") }|);
}
