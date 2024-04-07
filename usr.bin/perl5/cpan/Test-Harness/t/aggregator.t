#!/usr/bin/perl -wT

use strict;
use warnings;
use lib 't/lib';

use Test::More tests => 81;

use TAP::Parser;
use TAP::Parser::Iterator::Array;
use TAP::Parser::Aggregator;

my $tap = <<'END_TAP';
1..5
ok 1 - input file opened
... this is junk
not ok first line of the input valid # todo some data
# this is a comment
ok 3 - read the rest of the file
not ok 4 - this is a real failure
ok 5 # skip we have no description
END_TAP

my $iterator = TAP::Parser::Iterator::Array->new( [ split /\n/ => $tap ] );
isa_ok $iterator, 'TAP::Parser::Iterator';

my $parser1 = TAP::Parser->new( { iterator => $iterator } );
isa_ok $parser1, 'TAP::Parser';

$parser1->run;

$tap = <<'END_TAP';
1..7
ok 1 - gentlemen, start your engines
not ok first line of the input valid # todo some data
# this is a comment
ok 3 - read the rest of the file
not ok 4 - this is a real failure
ok 5 
ok 6 - you shall not pass! # TODO should have failed
not ok 7 - Gandalf wins.  Game over.  # TODO 'bout time!
END_TAP

my $parser2 = TAP::Parser->new( { tap => $tap } );
isa_ok $parser2, 'TAP::Parser';
$parser2->run;

can_ok 'TAP::Parser::Aggregator', 'new';
my $agg = TAP::Parser::Aggregator->new;
isa_ok $agg, 'TAP::Parser::Aggregator';

can_ok $agg, 'add';
ok $agg->add( 'tap1', $parser1 ), '... and calling it should succeed';
ok $agg->add( 'tap2', $parser2 ), '... even if we add more than one parser';
eval { $agg->add( 'tap1', $parser1 ) };
like $@, qr/^You already have a parser for \Q(tap1)/,
  '... but trying to reuse a description should be fatal';

can_ok $agg, 'parsers';
is scalar $agg->parsers, 2,
  '... and it should report how many parsers it has';
is_deeply [ $agg->parsers ], [ $parser1, $parser2 ],
  '... or which parsers it has';
is_deeply $agg->parsers('tap2'), $parser2, '... or reporting a single parser';
is_deeply [ $agg->parsers(qw(tap2 tap1)) ], [ $parser2, $parser1 ],
  '... or a group';

# test aggregate results

can_ok $agg, 'passed';
is $agg->passed, 10,
  '... and we should have the correct number of passed tests';
is_deeply [ $agg->passed ], [qw(tap1 tap2)],
  '... and be able to get their descriptions';

can_ok $agg, 'failed';
is $agg->failed, 2,
  '... and we should have the correct number of failed tests';
is_deeply [ $agg->failed ], [qw(tap1 tap2)],
  '... and be able to get their descriptions';

can_ok $agg, 'todo';
is $agg->todo, 4, '... and we should have the correct number of todo tests';
is_deeply [ $agg->todo ], [qw(tap1 tap2)],
  '... and be able to get their descriptions';

can_ok $agg, 'skipped';
is $agg->skipped, 1,
  '... and we should have the correct number of skipped tests';
is_deeply [ $agg->skipped ], [qw(tap1)],
  '... and be able to get their descriptions';

can_ok $agg, 'parse_errors';
is $agg->parse_errors, 0, '... and the correct number of parse errors';
is_deeply [ $agg->parse_errors ], [],
  '... and be able to get their descriptions';

can_ok $agg, 'todo_passed';
is $agg->todo_passed, 1,
  '... and the correct number of unexpectedly succeeded tests';
is_deeply [ $agg->todo_passed ], [qw(tap2)],
  '... and be able to get their descriptions';

can_ok $agg, 'total';
is $agg->total, $agg->passed + $agg->failed,
  '... and we should have the correct number of total tests';

can_ok $agg, 'planned';
is $agg->planned, $agg->passed + $agg->failed,
  '... and we should have the correct number of planned tests';

can_ok $agg, 'has_problems';
ok $agg->has_problems, '... and it should report true if there are problems';

can_ok $agg, 'has_errors';
ok $agg->has_errors, '... and it should report true if there are errors';

can_ok $agg, 'get_status';
is $agg->get_status, 'FAIL', '... and it should tell us the tests failed';

can_ok $agg, 'all_passed';
ok !$agg->all_passed, '... and it should tell us not all tests passed';

# coverage testing

# _get_parsers
# bad descriptions
# currently the $agg object has descriptions tap1 and tap2
# call _get_parsers with another description.
# $agg will call  its _croak method
my @die;

eval {
    local $SIG{__DIE__} = sub { push @die, @_ };

    $agg->_get_parsers('no_such_parser_for');
};

is @die, 1,
  'coverage tests for missing parsers... and we caught just one death message';
like pop(@die),
  qr/^A parser for \(no_such_parser_for\) could not be found at /,
  '... and it was the expected death message';

# _get_parsers in scalar context

my $gp = $agg->_get_parsers(qw(tap1 tap2))
  ;    # should return ref to array containing parsers for tap1 and tap2

is @$gp, 2,
  'coverage tests for _get_parser in scalar context... and we got the right number of parsers';
isa_ok( $_, 'TAP::Parser' ) for (@$gp);

# _get_parsers
# todo_failed - this is a deprecated method, so it  (and these tests)
# can be removed eventually.  However, it is showing up in the coverage
# as never tested.
my @warn;

eval {
    local $SIG{__WARN__} = sub { push @warn, @_ };

    $agg->todo_failed();
};

# check the warning, making sure to capture the fullstops correctly (not
# as "any char" matches)
is @warn, 1,
  'coverage tests for deprecated todo_failed... and just one warning caught';
like pop(@warn),
  qr/^"todo_failed" is deprecated[.]  Please use "todo_passed"[.]  See the docs[.] at/,
  '... and it was the expected warning';

# has_problems
# this has a large number of conditions 'OR'd together, so the tests get
# a little complicated here

# currently, we have covered the cases of failed() being true and none
# of the summary methods failing

# we need to set up test cases for
# 1. !failed && todo_passed
# 2. !failed && !todo_passed && parse_errors
# 3. !failed && !todo_passed && !parse_errors && exit
# 4. !failed && !todo_passed && !parse_errors && !exit && wait

# note there is nothing wrong per se with the has_problems logic, these
# are simply coverage tests

# 1. !failed && todo_passed

$agg = TAP::Parser::Aggregator->new();
isa_ok $agg, 'TAP::Parser::Aggregator';

$tap = <<'END_TAP';
1..1
ok 1 - you shall not pass! # TODO should have failed
END_TAP

my $parser3 = TAP::Parser->new( { tap => $tap } );
isa_ok $parser3, 'TAP::Parser';
$parser3->run;

$agg->add( 'tap3', $parser3 );

is $agg->passed, 1,
  'coverage tests for !failed && todo_passed... and we should have the correct number of passed tests';
is $agg->failed, 0,
  '... and we should have the correct number of failed tests';
is $agg->todo_passed, 1,
  '... and the correct number of unexpectedly succeeded tests';
ok $agg->has_problems,
  '... and it should report true that there are problems';
is $agg->get_status, 'PASS', '... and the status should be passing';
ok !$agg->has_errors, '.... but it should not report any errors';
ok $agg->all_passed, '... bonus tests should be passing tests, too';

# 2. !failed && !todo_passed && parse_errors

$agg = TAP::Parser::Aggregator->new();

$tap = <<'END_TAP';
1..-1
END_TAP

my $parser4 = TAP::Parser->new( { tap => $tap } );
isa_ok $parser4, 'TAP::Parser';
$parser4->run;

$agg->add( 'tap4', $parser4 );

is $agg->passed, 0,
  'coverage tests for !failed && !todo_passed && parse_errors... and we should have the correct number of passed tests';
is $agg->failed, 0,
  '... and we should have the correct number of failed tests';
is $agg->todo_passed, 0,
  '... and the correct number of unexpectedly succeeded tests';
is $agg->parse_errors, 1, '... and the correct number of parse errors';
ok $agg->has_problems,
  '... and it should report true that there are problems';

# 3. !failed && !todo_passed && !parse_errors && exit
# now this is a little harder to emulate cleanly through creating tap
# fragments and parsing, as exit and wait collect OS-status codes.
# so we'll get a little funky with $agg and push exit and wait descriptions
# in it - not very friendly to internal rep changes.

$agg = TAP::Parser::Aggregator->new();

$tap = <<'END_TAP';
1..1
ok 1 - you shall not pass!
END_TAP

my $parser5 = TAP::Parser->new( { tap => $tap } );
$parser5->run;

$agg->add( 'tap', $parser5 );

push @{ $agg->{descriptions_for_exit} }, 'one possible reason';
$agg->{exit}++;

is $agg->passed, 1,
  'coverage tests for !failed && !todo_passed && !parse_errors... and we should have the correct number of passed tests';
is $agg->failed, 0,
  '... and we should have the correct number of failed tests';
is $agg->todo_passed, 0,
  '... and the correct number of unexpectedly succeeded tests';
is $agg->parse_errors, 0, '... and the correct number of parse errors';

my @exits = $agg->exit;

is @exits, 1, '... and the correct number of exits';
is pop(@exits), 'one possible reason',
  '... and we collected the right exit reason';

ok $agg->has_problems,
  '... and it should report true that there are problems';

# 4. !failed && !todo_passed && !parse_errors && !exit && wait

$agg = TAP::Parser::Aggregator->new();

$agg->add( 'tap', $parser5 );

push @{ $agg->{descriptions_for_wait} }, 'another possible reason';
$agg->{wait}++;

is $agg->passed, 1,
  'coverage tests for !failed && !todo_passed && !parse_errors && !exit... and we should have the correct number of passed tests';
is $agg->failed, 0,
  '... and we should have the correct number of failed tests';
is $agg->todo_passed, 0,
  '... and the correct number of unexpectedly succeeded tests';
is $agg->parse_errors, 0, '... and the correct number of parse errors';
is $agg->exit,         0, '... and the correct number of exits';

my @waits = $agg->wait;

is @waits, 1, '... and the correct number of waits';
is pop(@waits), 'another possible reason',
  '... and we collected the right wait reason';

ok $agg->has_problems,
  '... and it should report true that there are problems';
