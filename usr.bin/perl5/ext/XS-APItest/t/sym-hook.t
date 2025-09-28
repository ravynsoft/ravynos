
# Test that PL_check hooks for RV2*V can override symbol lookups.

# So far we only test RV2CV.

use XS::APItest;
use Test::More tests => 4;

BEGIN {
    setup_rv2cv_addunderbar;
    $^H{'XS::APItest/addunder'} = 1; # make foo() actually call foo_()
}

sub foo_ { @_ ? shift . "___" : "phew" }

is(foo(), "phew");

# Make sure subs looked up via rv2cv check hooks are not treated as second-
# class subs.

BEGIN { # If there is a foo symbol, this test will not be testing anything.
    delete $::{foo};
    delete $::{goo};
}
is((foo bar), 'bar___');
$bar = "baz";
is((foo $bar), 'baz___');

# Proto should cause goo() to override Foo->goo interpretation.
{package Foom}
sub goo_ (*) { shift . "===" }
is((goo Foom), "Foom===");
