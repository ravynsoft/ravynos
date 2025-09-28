# -*- mode: perl; -*-

# test the "l", "lib", "try" and "only" options:

use strict;
use warnings;

use Test::More tests => 14;

use bigfloat;

# Catch warning.

my $warning;
local $SIG{__WARN__} = sub {
    $warning = $_[0];
};

my $rc;

$warning = "";
$rc = eval { bigfloat->import("l" => "foo") };
subtest qq|eval { bigfloat->import("l" => "foo") }| => sub {
    plan tests => 2;

    is($@, '', "didn't die");
    is($warning, "", "didn't get a warning");
};

$warning = "";
$rc = eval { bigfloat->import("lib" => "foo") };
subtest qq|eval { bigfloat->import("lib" => "foo") }| => sub {
    plan tests => 2;

    is($@, '', "didn't die");
    is($warning, "", "didn't get a warning");
};

$warning = "";
$rc = eval { bigfloat->import("try" => "foo") };
subtest qq|eval { bigfloat->import("try" => "foo") }| => sub {
    plan tests => 2;

    is($@, '', "didn't die");
    is($warning, "", "didn't get a warning");
};

$warning = "";
$rc = eval { bigfloat->import("only" => "foo") };
subtest qq|eval { bigfloat->import("only" => "foo") }| => sub {
    plan tests => 2;

    is($@, '', "didn't die");
    is($warning, "", "didn't get a warning");
};

$warning = "";
$rc = eval { bigfloat->import("foo" => "bar") };
subtest qq|eval { bigfloat->import("foo" => "bar") }| => sub {
    plan tests => 2;

    is($@, '', "didn't die");
    is($warning, "", "didn't get a warning");
};

# test that options are only lowercase (don't see a reason why allow UPPER)

foreach (qw/L LIB Lib T Trace TRACE V Version VERSION/) {
    $rc = eval { bigfloat->import($_ => "bar") };
    like($@, qr/^Unknown option /i,
         qq|eval { bigfloat->import($_ => "bar") }|);
}
