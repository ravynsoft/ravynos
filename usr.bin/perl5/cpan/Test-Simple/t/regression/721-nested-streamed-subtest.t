use strict;
use warnings;

use Test2::Tools::Tiny;

# This module's exports interfere with the ones in t/tools.pl
use Test::More ();
use Test::Builder::Formatter();
use Test2::API qw/run_subtest test2_stack/;

{
    test2_stack->top;
    my $temp_hub = test2_stack->new_hub();
    $temp_hub->format(Test::Builder::Formatter->new());

    my $output = capture {
        run_subtest(
            'parent',
            sub {
                run_subtest(
                    'buffered',
                    sub {
                        ok(1, 'b1');
                        ok(1, 'b2');
                    },
                    {buffered => 1},
                );
                run_subtest(
                    'streamed',
                    sub {
                        ok(1, 's1');
                        ok(1, 's2');
                    },
                    {buffered => 0},
                );
            },
            {buffered => 1},
        );
    };

    test2_stack->pop($temp_hub);

    Test::More::subtest(
        'Test2::API::run_subtest',
        sub {
            is($output->{STDERR}, q{}, 'no output on stderr');
            like($output->{STDOUT}, qr/ +ok 1 - b1/, 'got ok output for tests in buffered subtest');
            like($output->{STDOUT}, qr/ +ok 2 - b2/, 'got ok output for tests in buffered subtest');
            like($output->{STDOUT}, qr/ +ok 1 - s1/, 'got ok output for tests in streamed subtest');
            like($output->{STDOUT}, qr/ +ok 2 - s2/, 'got ok output for tests in streamed subtest');
        }
    );
}

{
    test2_stack->top;
    my $temp_hub = test2_stack->new_hub();
    $temp_hub->format(Test::Builder::Formatter->new());

    my $output = capture {
        run_subtest(
            'parent',
            sub {
                run_subtest(
                    'buffered',
                    sub {
                        ok(1, 'b1');
                        ok(1, 'b2');
                    },
                    {buffered => 1},
                );
                Test::More::subtest(
                    'streamed',
                    sub {
                        ok(1, 's1');
                        ok(1, 's2');
                    },
                    {buffered => 0},
                );
            },
            {buffered => 1},
        );
    };

    test2_stack->pop($temp_hub);

    Test::More::subtest(
        'Test::More::subtest and Test2::API::run_subtest',
        sub {
            is($output->{STDERR}, q{}, 'no output on stderr');
            like($output->{STDOUT}, qr/ +ok 1 - b1/, 'got ok output for tests in buffered subtest');
            like($output->{STDOUT}, qr/ +ok 2 - b2/, 'got ok output for tests in buffered subtest');
            like($output->{STDOUT}, qr/ +ok 1 - s1/, 'got ok output for tests in streamed subtest');
            like($output->{STDOUT}, qr/ +ok 2 - s2/, 'got ok output for tests in streamed subtest');
        }
    );
}

done_testing;
