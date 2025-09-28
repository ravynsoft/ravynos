#!/usr/bin/perl -w
# Test todo subtests.
#
# A subtest in a todo context should have all of its diagnostic output
# redirected to the todo output destination, but individual tests
# within the subtest should not become todo tests themselves.

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

use Test::More;
use Test::Builder;
use Test::Builder::Tester;

# Formatting may change if we're running under Test::Harness.
$ENV{HARNESS_ACTIVE} = 0;

our %line;

# Repeat each test for various combinations of the todo reason,
# the mechanism by which it is set and $Level.
our @test_combos;
foreach my $level (1, 2, 3) {
    push @test_combos, ['$TODO',       'Reason',  $level],
                       ['todo_start',  'Reason',  $level],
                       ['todo_start',  '',        $level],
                       ['todo_start',  0,         $level];
}

plan tests => 8 * @test_combos;

sub test_subtest_in_todo {
    my ($name, $code, $want_out, $no_tests_run) = @_;

    my $xxx = $no_tests_run ? 'No tests run for subtest "xxx"' : 'xxx';

    chomp $want_out;
    my @outlines = split /\n/, $want_out;

    foreach my $combo (@test_combos) {
        my ($set_via, $todo_reason, $level) = @$combo;

        test_out( map { my $x = $_; $x =~ s/\s+$//g; $x } 
            "# Subtest: xxx",
            @outlines,
            "not ok 1 - $xxx # TODO $todo_reason",
            "#   Failed (TODO) test '$xxx'",
            "#   at $0 line $line{xxx}.",
            "not ok 2 - regular todo test # TODO $todo_reason",
            "#   Failed (TODO) test 'regular todo test'",
            "#   at $0 line $line{reg}.",
        );

        {
            local $TODO = $set_via eq '$TODO' ? $todo_reason : undef;
            if ($set_via eq 'todo_start') {
                Test::Builder->new->todo_start($todo_reason);
            }

            subtest_at_level(
                        'xxx', $code, $level); BEGIN{ $line{xxx} = __LINE__ }
            ok 0, 'regular todo test';         BEGIN{ $line{reg} = __LINE__ }

            if ($set_via eq 'todo_start') {
                Test::Builder->new->todo_end;
            }
        }

        test_test("$name ($level), todo [$todo_reason] set via $set_via");
    }
}

package Foo; # If several stack frames are in package 'main' then $Level
             # could be wrong and $main::TODO might still be found.  Using
             # another package makes the tests more sensitive.
             
sub main::subtest_at_level {
    my ($name, $code, $level) = @_;

    if ($level > 1) {
        local $Test::Builder::Level = $Test::Builder::Level + 1;
        main::subtest_at_level($name, $code, $level-1);
    }
    else {
        Test::Builder->new->subtest($name => $code);
    }
}

package main;

test_subtest_in_todo("plan, no tests run", sub {
    plan tests => 2;
}, <<END, 1);
    1..2
    # No tests run!
END

test_subtest_in_todo("noplan, no tests run", sub {
    plan 'no_plan';
}, <<END, 1);
    # No tests run!
END

test_subtest_in_todo("missingplan, no tests run", sub {
    1;
}, <<END, 1);
    1..0
    # No tests run!
END

test_subtest_in_todo("donetesting, no tests run", sub {
    done_testing;
}, <<END, 1);
    1..0
    # No tests run!
END

test_subtest_in_todo("1 failed test", sub {
    ok 0, 'failme'; BEGIN { $line{fail1} = __LINE__ }
}, <<END);
    not ok 1 - failme
    #   Failed test 'failme'
    #   at $0 line $line{fail1}.
    1..1
    # Looks like you failed 1 test of 1.
END

test_subtest_in_todo("1fail, wrongplan", sub {
    plan tests => 17;
    ok 0, 'failme'; BEGIN { $line{fail2} = __LINE__ }
}, <<END);
    1..17
    not ok 1 - failme
    #   Failed test 'failme'
    #   at $0 line $line{fail2}.
    # Looks like you planned 17 tests but ran 1.
    # Looks like you failed 1 test of 1 run.
END

test_subtest_in_todo("1fail, 1pass", sub {
    ok 0, 'failme'; BEGIN { $line{fail3} = __LINE__ }
    ok 1, 'passme';
}, <<END);
    not ok 1 - failme
    #   Failed test 'failme'
    #   at $0 line $line{fail3}.
    ok 2 - passme
    1..2
    # Looks like you failed 1 test of 2.
END

test_subtest_in_todo("todo tests in the subtest", sub {
    ok 0, 'inner test 1';             BEGIN{ $line{in1} = __LINE__ }

    TODO: {
        local $TODO = 'Inner1';
        ok 0, 'failing TODO a';       BEGIN{ $line{fta} = __LINE__ }
        ok 1, 'unexpected pass a';
    }

    ok 0, 'inner test 2';             BEGIN{ $line{in2} = __LINE__ }

    Test::Builder->new->todo_start('Inner2');
    ok 0, 'failing TODO b';           BEGIN{ $line{ftb} = __LINE__ }
    ok 1, 'unexpected pass b';
    Test::Builder->new->todo_end;

    ok 0, 'inner test 3';             BEGIN{ $line{in3} = __LINE__ }
}, <<END);
    not ok 1 - inner test 1
    #   Failed test 'inner test 1'
    #   at $0 line $line{in1}.
    not ok 2 - failing TODO a # TODO Inner1
    #   Failed (TODO) test 'failing TODO a'
    #   at $0 line $line{fta}.
    ok 3 - unexpected pass a # TODO Inner1
    not ok 4 - inner test 2
    #   Failed test 'inner test 2'
    #   at $0 line $line{in2}.
    not ok 5 - failing TODO b # TODO Inner2
    #   Failed (TODO) test 'failing TODO b'
    #   at $0 line $line{ftb}.
    ok 6 - unexpected pass b # TODO Inner2
    not ok 7 - inner test 3
    #   Failed test 'inner test 3'
    #   at $0 line $line{in3}.
    1..7
    # Looks like you failed 3 tests of 7.
END
