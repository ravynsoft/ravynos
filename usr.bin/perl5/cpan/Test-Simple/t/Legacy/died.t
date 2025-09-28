#!perl -w
# HARNESS-NO-STREAM
# HARNESS-NO-PRELOAD

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = '../lib';
    }
}

# Can't use Test.pm, that's a 5.005 thing.
package My::Test;

# This has to be a require or else the END block below runs before
# Test::Builder's own and the ending diagnostics don't come out right.
require Test::Builder;
my $TB = Test::Builder->create;
$TB->plan(tests => 3);


package main;

require Test::Simple;

chdir 't';
push @INC, '../t/lib/';
require Test::Simple::Catch;
my($out, $err) = Test::Simple::Catch::caught();
local $ENV{HARNESS_ACTIVE} = 0;

Test::Simple->import(tests => 1);
exit 250;

END {
    $TB->is_eq($out->read, <<OUT);
1..1
OUT

    $TB->is_eq($err->read, <<ERR);
# Looks like your test exited with 250 before it could output anything.
ERR

    $TB->is_eq($?, 250, "exit code");

    $? = grep { !$_ } $TB->summary;
}
