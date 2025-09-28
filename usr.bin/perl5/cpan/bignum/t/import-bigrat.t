# -*- mode: perl; -*-

# test the "l", "lib", "try" and "only" options:

use strict;
use warnings;

use Test::More tests => 21;

use bigrat;

# Catch warnings.

my $WARNINGS;
local $SIG{__WARN__} = sub {
    $WARNINGS = $_[0];
};

my $rc;

$WARNINGS = "";
$rc = eval { bigrat -> import("l" => "foo") };
is($@, '',
   qq|eval { bigrat -> import("l" => "foo") }|);
is($WARNINGS, "", "no warnings");

$WARNINGS = "";
$rc = eval { bigrat -> import("lib" => "foo") };
is($@, '',
   qq|eval { bigrat -> import("lib" => "foo") }|);
is($WARNINGS, "", "no warnings");

$WARNINGS = "";
$rc = eval { bigrat -> import("try" => "foo") };
is($@, '',
   qq|eval { bigrat -> import("try" => "foo") }|);
is($WARNINGS, "", "no warnings");

$WARNINGS = "";
$rc = eval { bigrat -> import("try" => "foo") };
is($@, '',
   qq|eval { bigrat -> import("try" => "foo") }|);
is($WARNINGS, "", "no warnings");

$WARNINGS = "";
$rc = eval { bigrat -> import("foo" => "bar") };
like($@, qr/^Unknown option/,
     qq|eval { bigrat -> import("foo" => "bar") }|);
is($WARNINGS, "", "no warnings");

$WARNINGS = "";
$rc = eval { bigrat -> import("only" => "bar") };
is($@, "",
   qq|eval { bigrat -> import("only" => "bar") }|);
is($WARNINGS, "", "no warnings");

# test that options are only lowercase (don't see a reason why allow UPPER)

foreach (qw/L LIB Lib T Trace TRACE V Version VERSION/) {
    $rc = eval { bigrat -> import($_ => "bar") };
    like($@, qr/^Unknown option/i,   # should die
        qq|eval { bigrat -> import($_ => "bar") }|);
}
