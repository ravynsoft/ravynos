#! /usr/local/perl -w
# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

use Test::More tests => 1;

BEGIN {
    $SIG{__DIE__}   = sub {
	warn @_;
	BAIL_OUT( q[Couldn't use module; can't continue.] );
    };
}

BEGIN {
    use version 0.9929;
}

pass "Didn't get caught by the wrong DIE handler, which is a good thing";
