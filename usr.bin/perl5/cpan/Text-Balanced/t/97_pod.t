#!/usr/bin/perl

# Test that the syntax of our POD documentation is valid

use 5.008001;

use strict;
use warnings;

use Test::More;

my @MODULES = (
    'Pod::Simple 3.07',
    'Test::Pod 1.26',
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

all_pod_files_ok();
