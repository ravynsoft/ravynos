# -*- mode: perl; -*-

# test the "l", "lib", "try" and "only" options:

use strict;
use warnings;

use Test::More tests => 14;

use bigrat;

# Catch warning.

my $warning;
local $SIG{__WARN__} = sub {
    $warning = $_[0];
};

my $rc;

$warning = "";
$rc = eval { bigrat->import("l" => "foo") };
subtest qq|eval { bigrat->import("l" => "foo") }| => sub {
    plan tests => 2;

    is($@, '', "didn't die");
    is($warning, "", "didn't get a warning");
};

$warning = "";
$rc = eval { bigrat->import("lib" => "foo") };
subtest qq|eval { bigrat->import("lib" => "foo") }| => sub {
    plan tests => 2;

    is($@, '', "didn't die");
    is($warning, "", "didn't get a warning");
};

$warning = "";
$rc = eval { bigrat->import("try" => "foo") };
subtest qq|eval { bigrat->import("try" => "foo") }| => sub {
    plan tests => 2;

    is($@, '', "didn't die");
    is($warning, "", "didn't get a warning");
};

$warning = "";
$rc = eval { bigrat->import("only" => "foo") };
subtest qq|eval { bigrat->import("only" => "foo") }| => sub {
    plan tests => 2;

    is($@, '', "didn't die");
    is($warning, "", "didn't get a warning");
};

$warning = "";
$rc = eval { bigrat->import("foo" => "bar") };
subtest qq|eval { bigrat->import("foo" => "bar") }| => sub {
    plan tests => 2;

    is($@, '', "didn't die");
    is($warning, "", "didn't get a warning");
};

# test that options are only lowercase (don't see a reason why allow UPPER)

foreach (qw/L LIB Lib T Trace TRACE V Version VERSION/) {
    $rc = eval { bigrat->import($_ => "bar") };
    like($@, qr/^Unknown option /i,
         qq|eval { bigrat->import($_ => "bar") }|);
}
