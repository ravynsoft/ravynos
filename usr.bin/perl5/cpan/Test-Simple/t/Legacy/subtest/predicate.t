#!/usr/bin/perl -w

# Test the use of subtest() to define new test predicates that combine
# multiple existing predicates.

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

# Define a new test predicate with Test::More::subtest(), using
# Test::More predicates as building blocks...

sub foobar_ok ($;$) {
    my ($value, $name) = @_;
    $name ||= "foobar_ok";

    local $Test::Builder::Level = $Test::Builder::Level + 1;
    subtest $name => sub {
        plan tests => 2;
        ok $value =~ /foo/, "foo";
        ok $value =~ /bar/, "bar"; BEGIN{ $line{foobar_ok_bar} = __LINE__ }
    };
}
{
    test_out("# Subtest: namehere");
    test_out("    1..2");
    test_out("    ok 1 - foo");
    test_out("    not ok 2 - bar");
    test_err("    #   Failed test 'bar'");
    test_err("    #   at $0 line $line{foobar_ok_bar}.");
    test_err("    # Looks like you failed 1 test of 2.");
    test_out("not ok 1 - namehere");
    test_err("#   Failed test 'namehere'");
    test_err("#   at $0 line ".(__LINE__+2).".");

    foobar_ok "foot", "namehere";

    test_test("foobar_ok failing line numbers");
}

# Wrap foobar_ok() to make another new predicate...

sub foobar_ok_2 ($;$) {
    my ($value, $name) = @_;

    local $Test::Builder::Level = $Test::Builder::Level + 1;
    foobar_ok($value, $name);
}
{
    test_out("# Subtest: namehere");
    test_out("    1..2");
    test_out("    ok 1 - foo");
    test_out("    not ok 2 - bar");
    test_err("    #   Failed test 'bar'");
    test_err("    #   at $0 line $line{foobar_ok_bar}.");
    test_err("    # Looks like you failed 1 test of 2.");
    test_out("not ok 1 - namehere");
    test_err("#   Failed test 'namehere'");
    test_err("#   at $0 line ".(__LINE__+2).".");

    foobar_ok_2 "foot", "namehere";

    test_test("foobar_ok_2 failing line numbers");
}

# Define another new test predicate, this time using
# Test::Builder::subtest() rather than Test::More::subtest()...

sub barfoo_ok ($;$) {
    my ($value, $name) = @_;
    $name ||= "barfoo_ok";

    Test::Builder->new->subtest($name => sub {
        plan tests => 2;
        ok $value =~ /foo/, "foo";
        ok $value =~ /bar/, "bar"; BEGIN{ $line{barfoo_ok_bar} = __LINE__ }
    });
}
{
    test_out("# Subtest: namehere");
    test_out("    1..2");
    test_out("    ok 1 - foo");
    test_out("    not ok 2 - bar");
    test_err("    #   Failed test 'bar'");
    test_err("    #   at $0 line $line{barfoo_ok_bar}.");
    test_err("    # Looks like you failed 1 test of 2.");
    test_out("not ok 1 - namehere");
    test_err("#   Failed test 'namehere'");
    test_err("#   at $0 line ".(__LINE__+2).".");

    barfoo_ok "foot", "namehere";

    test_test("barfoo_ok failing line numbers");
}

# Wrap barfoo_ok() to make another new predicate...

sub barfoo_ok_2 ($;$) {
    my ($value, $name) = @_;

    local $Test::Builder::Level = $Test::Builder::Level + 1;
    barfoo_ok($value, $name);
}
{
    test_out("# Subtest: namehere");
    test_out("    1..2");
    test_out("    ok 1 - foo");
    test_out("    not ok 2 - bar");
    test_err("    #   Failed test 'bar'");
    test_err("    #   at $0 line $line{barfoo_ok_bar}.");
    test_err("    # Looks like you failed 1 test of 2.");
    test_out("not ok 1 - namehere");
    test_err("#   Failed test 'namehere'");
    test_err("#   at $0 line ".(__LINE__+2).".");

    barfoo_ok_2 "foot", "namehere";

    test_test("barfoo_ok_2 failing line numbers");
}

# A subtest-based predicate called from within a subtest
{
    test_out("# Subtest: outergroup");
    test_out("    1..2");
    test_out("    ok 1 - this passes");
    test_out("    # Subtest: namehere");
    test_out("        1..2");
    test_out("        ok 1 - foo");
    test_out("        not ok 2 - bar");
    test_err("        #   Failed test 'bar'");
    test_err("        #   at $0 line $line{barfoo_ok_bar}.");
    test_err("        # Looks like you failed 1 test of 2.");
    test_out("    not ok 2 - namehere");
    test_err("    #   Failed test 'namehere'");
    test_err("    #   at $0 line $line{ipredcall}.");
    test_err("    # Looks like you failed 1 test of 2.");
    test_out("not ok 1 - outergroup");
    test_err("#   Failed test 'outergroup'");
    test_err("#   at $0 line $line{outercall}.");

    subtest outergroup => sub {
        plan tests => 2;
        ok 1, "this passes";
        barfoo_ok_2 "foot", "namehere"; BEGIN{ $line{ipredcall} = __LINE__ }
    }; BEGIN{ $line{outercall} = __LINE__ }

    test_test("outergroup with internal barfoo_ok_2 failing line numbers");
}
