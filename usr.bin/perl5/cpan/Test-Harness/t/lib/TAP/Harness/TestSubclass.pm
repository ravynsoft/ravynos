package TAP::Harness::TestSubclass;
use strict;
use warnings;
use base 'TAP::Harness';

sub aggregate_tests {
    local $ENV{HARNESS_IS_SUBCLASS} = __PACKAGE__;
    $_[0]->SUPER::aggregate_tests;
}

1;
