#!/usr/bin/perl -wT

use strict;
use warnings;
use lib 't/lib';

use Test::More tests => 23;

use TAP::Parser;

my $plan_line = 'TAP::Parser::Result::Plan';
my $test_line = 'TAP::Parser::Result::Test';

sub _parser {
    my $parser = TAP::Parser->new( { tap => shift } );
    $parser->run;
    return $parser;
}

# validate that plan!

my $parser = _parser(<<'END_TAP');
ok 1 - input file opened
not ok 2 - first line of the input valid # todo some data
ok 3 - read the rest of the file
1..3
# comments are allowed after an ending plan
END_TAP

can_ok $parser, 'parse_errors';
ok !$parser->parse_errors,
  '... comments should be allowed after a terminating plan';

$parser = _parser(<<'END_TAP');
ok 1 - input file opened
not ok 2 - first line of the input valid # todo some data
ok 3 - read the rest of the file
1..3
# yeah, yeah, I know.
ok
END_TAP

can_ok $parser, 'parse_errors';
is scalar $parser->parse_errors, 2, '... and we should have two parse errors';

is [ $parser->parse_errors ]->[0],
  'Plan (1..3) must be at the beginning or end of the TAP output',
  '... telling us that our plan was misplaced';
is [ $parser->parse_errors ]->[1],
  'Bad plan.  You planned 3 tests but ran 4.',
  '... and telling us we ran the wrong number of tests.';

$parser = _parser(<<'END_TAP');
ok 1 - input file opened
not ok 2 - first line of the input valid # todo some data
ok 3 - read the rest of the file
#1..3
# yo quiero tests!
1..3
END_TAP
ok !$parser->parse_errors, '... but test plan-like data can be in a comment';

$parser = _parser(<<'END_TAP');
ok 1 - input file opened
not ok 2 - first line of the input valid # todo some data
ok 3 - read the rest of the file 1..5
# yo quiero tests!
1..3
END_TAP
ok !$parser->parse_errors, '... or a description';

$parser = _parser(<<'END_TAP');
ok 1 - input file opened
not ok 2 - first line of the input valid # todo 1..4
ok 3 - read the rest of the file
# yo quiero tests!
1..3
END_TAP
ok !$parser->parse_errors, '... or a directive';

# test numbers included?

$parser = _parser(<<'END_TAP');
1..3
ok 1 - input file opened
not ok 2 - first line of the input valid # todo some data
ok read the rest of the file
# this is ...
END_TAP
eval { $parser->run };
ok !$@, 'We can mix and match the presence of test numbers';

$parser = _parser(<<'END_TAP');
1..3
ok 1 - input file opened
not ok 2 - first line of the input valid # todo some data
ok 2 read the rest of the file
END_TAP

is + ( $parser->parse_errors )[0],
  'Tests out of sequence.  Found (2) but expected (3)',
  '... and if the numbers are there, they cannot be out of sequence';

$parser = _parser(<<'END_TAP');
ok 1 - input file opened
not ok 2 - first line of the input valid # todo some data
ok 2 read the rest of the file
END_TAP

is $parser->parse_errors, 2,
  'Having two errors in the TAP should result in two errors (duh)';
my $expected = [
    'Tests out of sequence.  Found (2) but expected (3)',
    'No plan found in TAP output'
];
is_deeply [ $parser->parse_errors ], $expected,
  '... and they should be the correct errors';

$parser = _parser(<<'END_TAP');
ok 1 - input file opened
not ok 2 - first line of the input valid # todo some data
ok 3 read the rest of the file
END_TAP

is $parser->parse_errors, 1, 'Having no plan should cause an error';
is + ( $parser->parse_errors )[0], 'No plan found in TAP output',
  '... with a correct error message';

$parser = _parser(<<'END_TAP');
1..3
ok 1 - input file opened
not ok 2 - first line of the input valid # todo some data
ok 3 read the rest of the file
1..3
END_TAP

is $parser->parse_errors, 1,
  'Having more than one plan should cause an error';
is + ( $parser->parse_errors )[0], 'More than one plan found in TAP output',
  '... with a correct error message';

can_ok $parser, 'is_good_plan';
$parser = _parser(<<'END_TAP');
1..2
ok 1 - input file opened
not ok 2 - first line of the input valid # todo some data
ok 3 read the rest of the file
END_TAP

is $parser->parse_errors, 1,
  'Having the wrong number of planned tests is a parse error';
is + ( $parser->parse_errors )[0],
  'Bad plan.  You planned 2 tests but ran 3.',
  '... with a correct error message';

# XXX internals:  plan will not set to true if defined
$parser->is_good_plan(undef);
$parser = _parser(<<'END_TAP');
ok 1 - input file opened
1..1
END_TAP

ok $parser->is_good_plan,
  '... and it should return true if the plan is correct';

# TAP::Parser coverage tests
{

    # good_plan coverage

    my @warn;

    eval {
        local $SIG{__WARN__} = sub { push @warn, @_ };

        $parser->good_plan;
    };

    is @warn, 1, 'coverage testing of good_plan';

    like pop @warn,
      qr/good_plan[(][)] is deprecated.  Please use "is_good_plan[(][)]"/,
      '...and it fell-back like we expected';
}
