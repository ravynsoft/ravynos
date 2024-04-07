#!/usr/bin/perl

# Test that our declared minimum Perl version matches our syntax

use 5.008001;

use strict;
use warnings;

use Test::More;

my @MODULES = (
    'Perl::MinimumVersion 1.20',
    'Test::MinimumVersion 0.101082',
);

# Don't run tests for installs
use Test::More;
unless ( $ENV{AUTHOR_TESTING} ) {
    plan( skip_all => "Author testing only" );
}

# Load the testing modules
foreach my $MODULE ( @MODULES ) {
    ## no critic (BuiltinFunctions::ProhibitStringyEval)
    eval "use $MODULE";
    if ( $@ ) {
        plan( skip_all => "$MODULE not available for testing" );
    }
}

all_minimum_version_from_mymetayml_ok();
