#!/usr/bin/perl -wT

use strict;
use warnings;
use lib 't/lib';

use Test::More tests => 14;

use TAP::Parser;
use TAP::Parser::Iterator::Array;

sub tap_to_lines {
    my $string = shift;
    my @lines = ( $string =~ /.*\n/g );
    return \@lines;
}

my $tap = <<'END_TAP';
1..4
ok 1 - input file opened
... this is junk
not ok first line of the input valid # todo some data
# this is a comment
ok 3 - read the rest of the file
not ok 4 - this is a real failure
Bail out!  We ran out of foobar.
not ok 5
END_TAP

my $parser = TAP::Parser->new(
    {   iterator => TAP::Parser::Iterator::Array->new( tap_to_lines($tap) ),
    }
);

# results() is sane?

# check the test plan
my $result = $parser->next();

# TEST
ok $result->is_plan, 'We should have a plan';

# a normal, passing test

my $test = $parser->next();

# TEST
ok $test->is_test, '... and a test';

# junk lines should be preserved

my $unknown = $parser->next();

# TEST
ok $unknown->is_unknown, '... and an unknown line';

# a failing test, which also happens to have a directive

my $failed = $parser->next();

# TEST
ok $failed->is_test, '... and another test';

# comments

my $comment = $parser->next();

# TEST
ok $comment->is_comment, '... and a comment';

# another normal, passing test

$test = $parser->next();

# TEST
ok $test->is_test, '... and another test';

# a failing test

$failed = $parser->next();

# TEST
ok $failed->is_test, '... and yet another test';

# ok 5 # skip we have no description
# skipped test
my $bailout = $parser->next();

# TEST
ok $bailout->is_bailout, 'And finally we should have a bailout';

# TEST
is $bailout->as_string, 'We ran out of foobar.',
  '... and as_string() should return the explanation';

# TEST
is( $bailout->raw, 'Bail out!  We ran out of foobar.',
    '... and raw() should return the explanation'
);

# TEST
is( $bailout->explanation, 'We ran out of foobar.',
    '... and it should have the correct explanation'
);

my $more_tap = "1..1\nok 1 - input file opened\n";

my $second_parser = TAP::Parser->new(
    {   iterator =>
          TAP::Parser::Iterator::Array->new( [ split( /\n/, $more_tap ) ] ),
    }
);

$result = $second_parser->next();

# TEST
ok $result->is_plan(), "Result is not the leftover line";

$result = $second_parser->next();

# TEST
ok $result->is_test(), "Result is a test";

# TEST
ok $result->is_ok(), "The event has passed";

