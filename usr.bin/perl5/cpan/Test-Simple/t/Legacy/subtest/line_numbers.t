#!/usr/bin/perl -w

# Test Test::More::subtest(), focusing on correct line numbers in
# failed test diagnostics.

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = ( '../lib', 'lib' );
    }
    else {
        unshift @INC, 't/lib';
    }
}

use strict;
use warnings;

use Test::More tests => 5;
use Test::Builder;
use Test::Builder::Tester;

# Formatting may change if we're running under Test::Harness.
$ENV{HARNESS_ACTIVE} = 0;

our %line;

{
    test_out("# Subtest: namehere");
    test_out("    1..3");
    test_out("    ok 1");
    test_out("    not ok 2");
    test_err("    #   Failed test at $0 line $line{innerfail1}.");
    test_out("    ok 3");
    test_err("    # Looks like you failed 1 test of 3.");
    test_out("not ok 1 - namehere");
    test_err("#   Failed test 'namehere'");
    test_err("#   at $0 line $line{outerfail1}.");

    subtest namehere => sub {
        plan tests => 3;
        ok 1;
        ok 0; BEGIN{ $line{innerfail1} = __LINE__ }
        ok 1;
    }; BEGIN{ $line{outerfail1} = __LINE__ }
    
    test_test("un-named inner tests");
}
{
    test_out("# Subtest: namehere");
    test_out("    1..3");
    test_out("    ok 1 - first is good");
    test_out("    not ok 2 - second is bad");
    test_err("    #   Failed test 'second is bad'");
    test_err("    #   at $0 line $line{innerfail2}.");
    test_out("    ok 3 - third is good");
    test_err("    # Looks like you failed 1 test of 3.");
    test_out("not ok 1 - namehere");
    test_err("#   Failed test 'namehere'");
    test_err("#   at $0 line $line{outerfail2}.");

    subtest namehere => sub {
        plan tests => 3;
        ok 1, "first is good";
        ok 0, "second is bad"; BEGIN{ $line{innerfail2} = __LINE__ }
        ok 1, "third is good";
    }; BEGIN{ $line{outerfail2} = __LINE__ }
    
    test_test("named inner tests");
}

sub run_the_subtest {
    subtest namehere => sub {
        plan tests => 3;
        ok 1, "first is good";
        ok 0, "second is bad"; BEGIN{ $line{innerfail3} = __LINE__ }
        ok 1, "third is good";
    }; BEGIN{ $line{outerfail3} = __LINE__ }
}
{
    test_out("# Subtest: namehere");
    test_out("    1..3");
    test_out("    ok 1 - first is good");
    test_out("    not ok 2 - second is bad");
    test_err("    #   Failed test 'second is bad'");
    test_err("    #   at $0 line $line{innerfail3}.");
    test_out("    ok 3 - third is good");
    test_err("    # Looks like you failed 1 test of 3.");
    test_out("not ok 1 - namehere");
    test_err("#   Failed test 'namehere'");
    test_err("#   at $0 line $line{outerfail3}.");

    run_the_subtest();
    
    test_test("subtest() called from a sub");
}
{
    test_out( "# Subtest: namehere");
    test_out( "    1..0");
    test_err( "    # No tests run!");
    test_out( 'not ok 1 - No tests run for subtest "namehere"');
    test_err(q{#   Failed test 'No tests run for subtest "namehere"'});
    test_err( "#   at $0 line $line{outerfail4}.");

    subtest namehere => sub {
        done_testing;
    }; BEGIN{ $line{outerfail4} = __LINE__ }

    test_test("lineno in 'No tests run' diagnostic");
}
{
    test_out("# Subtest: namehere");
    test_out("    1..1");
    test_out("    not ok 1 - foo is bar");
    test_err("    #   Failed test 'foo is bar'");
    test_err("    #   at $0 line $line{is_fail}.");
    test_err("    #          got: 'foo'");
    test_err("    #     expected: 'bar'");
    test_err("    # Looks like you failed 1 test of 1.");
    test_out('not ok 1 - namehere');
    test_err("#   Failed test 'namehere'");
    test_err("#   at $0 line $line{is_outer_fail}.");

    subtest namehere => sub {
        plan tests => 1;
        is 'foo', 'bar', 'foo is bar'; BEGIN{ $line{is_fail} = __LINE__ }
    }; BEGIN{ $line{is_outer_fail} = __LINE__ }

    test_test("diag indent for is() in subtest");
}
