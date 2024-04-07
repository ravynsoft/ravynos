use Test::More;
use strict;
use warnings;
# HARNESS-NO-STREAM

# See https://github.com/Test-More/test-more/issues/789

BEGIN {
    plan skip_all => 'AUTHOR_TESTING not enabled'
        unless $ENV{AUTHOR_TESTING};

    plan skip_all => "This test requires Test::Class"
        unless eval { require Test::Class; 1 };

    plan skip_all => "This test requires Test::Script"
        unless eval { require Test::Script; 1 };
}

package Test;

use base 'Test::Class';

use Test::More;
use Test::Script;

sub a_compilation_test : Test(startup => 1) {
    script_compiles(__FILE__);
}

sub test : Test(1) {
    ok(1);
}

package main;

use Test::Class;

Test::Class->runtests;
