#!/usr/bin/perl -wT

use strict;
use warnings;
use lib 't/lib';

use Test::More tests => 33;

use TAP::Parser;

my $tap = <<'END_TAP';
1..4
ok 1 - input file opened
... this is junk
not ok first line of the input valid # todo some data
# this is a comment
ok 3 - read the rest of the file
not ok 4 - this is a real failure
Bail out!  We ran out of foobar.
END_TAP
my $parser = TAP::Parser->new( { tap => $tap } );
isa_ok $parser, 'TAP::Parser',
  '... we should be able to parse bailed out tests';

my @results;
while ( my $result = $parser->next ) {
    push @results => $result;
}

can_ok $parser, 'passed';
is $parser->passed, 3,
  '... and we shold have the correct number of passed tests';
is_deeply [ $parser->passed ], [ 1, 2, 3 ],
  '... and get a list of the passed tests';

can_ok $parser, 'failed';
is $parser->failed, 1, '... and the correct number of failed tests';
is_deeply [ $parser->failed ], [4], '... and get a list of the failed tests';

can_ok $parser, 'actual_passed';
is $parser->actual_passed, 2,
  '... and we shold have the correct number of actually passed tests';
is_deeply [ $parser->actual_passed ], [ 1, 3 ],
  '... and get a list of the actually passed tests';

can_ok $parser, 'actual_failed';
is $parser->actual_failed, 2,
  '... and the correct number of actually failed tests';
is_deeply [ $parser->actual_failed ], [ 2, 4 ],
  '... or get a list of the actually failed tests';

can_ok $parser, 'todo';
is $parser->todo, 1,
  '... and we should have the correct number of TODO tests';
is_deeply [ $parser->todo ], [2], '... and get a list of the TODO tests';

ok !$parser->skipped,
  '... and we should have the correct number of skipped tests';

# check the plan

can_ok $parser, 'plan';
is $parser->plan,          '1..4', '... and we should have the correct plan';
is $parser->tests_planned, 4,      '... and the correct number of tests';

# results() is sane?

ok @results, 'The parser should return results';
is scalar @results, 8, '... and there should be one for each line';

# check the test plan

my $result = shift @results;
ok $result->is_plan, 'We should have a plan';

# a normal, passing test

my $test = shift @results;
ok $test->is_test, '... and a test';

# junk lines should be preserved

my $unknown = shift @results;
ok $unknown->is_unknown, '... and an unknown line';

# a failing test, which also happens to have a directive

my $failed = shift @results;
ok $failed->is_test, '... and another test';

# comments

my $comment = shift @results;
ok $comment->is_comment, '... and a comment';

# another normal, passing test

$test = shift @results;
ok $test->is_test, '... and another test';

# a failing test

$failed = shift @results;
ok $failed->is_test, '... and yet another test';

# ok 5 # skip we have no description
# skipped test
my $bailout = shift @results;
ok $bailout->is_bailout, 'And finally we should have a bailout';
is $bailout->as_string,  'We ran out of foobar.',
  '... and as_string() should return the explanation';
is $bailout->raw, 'Bail out!  We ran out of foobar.',
  '... and raw() should return the explanation';
is $bailout->explanation, 'We ran out of foobar.',
  '... and it should have the correct explanation';
