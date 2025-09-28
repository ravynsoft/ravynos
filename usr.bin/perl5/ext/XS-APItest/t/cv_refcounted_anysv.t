#!./perl

use strict;
use warnings;

use Test::More;
use XS::APItest;

is(test_CvREFCOUNTED_ANYSV(), 0, "Bulk test internal CvREFCOUNTED_ANYSV API");

# TODO: A test of operating via cv_clone()
#   Unfortunately that's very difficult to arrange, because cv_clone() itself
#   requires the CV to have a CvPADLIST, and that macro requires !CvISXSUB.
#   We could instead go via cv_clone_into() but that isn't exposed outside of
#   perl core.
#   I don't know how to unit-test that one.

done_testing;
