#!/usr/bin/perl -w

use strict;
use warnings;

BEGIN {
    use lib 't/lib';
}

use Test::More tests => 294;
use IO::c55Capture;

use File::Spec;

use TAP::Parser;
use TAP::Parser::Iterator::Array;

sub _get_results {
    my $parser = shift;
    my @results;
    while ( defined( my $result = $parser->next ) ) {
        push @results => $result;
    }
    return @results;
}

my ( $PARSER, $PLAN, $PRAGMA, $TEST, $COMMENT, $BAILOUT, $UNKNOWN, $YAML, $VERSION ) = qw(
  TAP::Parser
  TAP::Parser::Result::Plan
  TAP::Parser::Result::Pragma
  TAP::Parser::Result::Test
  TAP::Parser::Result::Comment
  TAP::Parser::Result::Bailout
  TAP::Parser::Result::Unknown
  TAP::Parser::Result::YAML
  TAP::Parser::Result::Version
);

my $tap = <<'END_TAP';
TAP version 13
1..7
ok 1 - input file opened
... this is junk
not ok first line of the input valid # todo some data
# this is a comment
ok 3 - read the rest of the file
not ok 4 - this is a real failure
  --- YAML!
  ...
ok 5 # skip we have no description
ok 6 - you shall not pass! # TODO should have failed
not ok 7 - Gandalf wins.  Game over.  # TODO 'bout time!
END_TAP

can_ok $PARSER, 'new';
my $parser = $PARSER->new( { tap => $tap } );
isa_ok $parser, $PARSER, '... and the object it returns';

ok $ENV{TAP_VERSION}, 'TAP_VERSION env variable should be set';

# results() is sane?

my @results = _get_results($parser);
is scalar @results, 12, '... and there should be one for each line';

my $version = shift @results;
isa_ok $version, $VERSION;
is $version->version, '13', '... and the version should be 13';

# check the test plan

my $result = shift @results;
isa_ok $result, $PLAN;
can_ok $result, 'type';
is $result->type, 'plan', '... and it should report the correct type';
ok $result->is_plan, '... and it should identify itself as a plan';
is $result->plan, '1..7', '... and identify the plan';
ok !$result->directive,   '... and this plan should not have a directive';
ok !$result->explanation, '... or a directive explanation';
is $result->as_string, '1..7',
  '... and have the correct string representation';
is $result->raw, '1..7', '... and raw() should return the original line';

# a normal, passing test

my $test = shift @results;
isa_ok $test, $TEST;
is $test->type, 'test', '... and it should report the correct type';
ok $test->is_test, '... and it should identify itself as a test';
is $test->ok,      'ok', '... and it should have the correct ok()';
ok $test->is_ok,   '... and the correct boolean version of is_ok()';
ok $test->is_actual_ok,
  '... and the correct boolean version of is_actual_ok()';
is $test->number, 1, '... and have the correct test number';
is $test->description, '- input file opened',
  '... and the correct description';
ok !$test->directive,   '... and not have a directive';
ok !$test->explanation, '... or a directive explanation';
ok !$test->has_skip,    '... and it is not a SKIPped test';
ok !$test->has_todo,    '... nor a TODO test';
is $test->as_string, 'ok 1 - input file opened',
  '... and its string representation should be correct';
is $test->raw, 'ok 1 - input file opened',
  '... and raw() should return the original line';

# junk lines should be preserved

my $unknown = shift @results;
isa_ok $unknown, $UNKNOWN;
is $unknown->type, 'unknown', '... and it should report the correct type';
ok $unknown->is_unknown, '... and it should identify itself as unknown';
is $unknown->as_string,  '... this is junk',
  '... and its string representation should be returned verbatim';
is $unknown->raw, '... this is junk',
  '... and raw() should return the original line';

# a failing test, which also happens to have a directive

my $failed = shift @results;
isa_ok $failed, $TEST;
is $failed->type, 'test', '... and it should report the correct type';
ok $failed->is_test, '... and it should identify itself as a test';
is $failed->ok,      'not ok', '... and it should have the correct ok()';
ok $failed->is_ok,   '... and TODO tests should always pass';
ok !$failed->is_actual_ok,
  '... and the correct boolean version of is_actual_ok ()';
is $failed->number, 2, '... and have the correct failed number';
is $failed->description, 'first line of the input valid',
  '... and the correct description';
is $failed->directive, 'TODO', '... and should have the correct directive';
is $failed->explanation, 'some data',
  '... and the correct directive explanation';
ok !$failed->has_skip, '... and it is not a SKIPped failed';
ok $failed->has_todo, '... but it is a TODO succeeded';
is $failed->as_string,
  'not ok 2 first line of the input valid # TODO some data',
  '... and its string representation should be correct';
is $failed->raw, 'not ok first line of the input valid # todo some data',
  '... and raw() should return the original line';

# comments

my $comment = shift @results;
isa_ok $comment, $COMMENT;
is $comment->type, 'comment', '... and it should report the correct type';
ok $comment->is_comment, '... and it should identify itself as a comment';
is $comment->comment,    'this is a comment',
  '... and you should be able to fetch the comment';
is $comment->as_string, '# this is a comment',
  '... and have the correct string representation';
is $comment->raw, '# this is a comment',
  '... and raw() should return the original line';

# another normal, passing test

$test = shift @results;
isa_ok $test, $TEST;
is $test->type, 'test', '... and it should report the correct type';
ok $test->is_test, '... and it should identify itself as a test';
is $test->ok,      'ok', '... and it should have the correct ok()';
ok $test->is_ok,   '... and the correct boolean version of is_ok()';
ok $test->is_actual_ok,
  '... and the correct boolean version of is_actual_ok()';
is $test->number, 3, '... and have the correct test number';
is $test->description, '- read the rest of the file',
  '... and the correct description';
ok !$test->directive,   '... and not have a directive';
ok !$test->explanation, '... or a directive explanation';
ok !$test->has_skip,    '... and it is not a SKIPped test';
ok !$test->has_todo,    '... nor a TODO test';
is $test->as_string, 'ok 3 - read the rest of the file',
  '... and its string representation should be correct';
is $test->raw, 'ok 3 - read the rest of the file',
  '... and raw() should return the original line';

# a failing test

$failed = shift @results;
isa_ok $failed, $TEST;
is $failed->type, 'test', '... and it should report the correct type';
ok $failed->is_test, '... and it should identify itself as a test';
is $failed->ok, 'not ok', '... and it should have the correct ok()';
ok !$failed->is_ok, '... and the tests should not have passed';
ok !$failed->is_actual_ok,
  '... and the correct boolean version of is_actual_ok ()';
is $failed->number, 4, '... and have the correct failed number';
is $failed->description, '- this is a real failure',
  '... and the correct description';
ok !$failed->directive,   '... and should have no directive';
ok !$failed->explanation, '... and no directive explanation';
ok !$failed->has_skip,    '... and it is not a SKIPped failed';
ok !$failed->has_todo,    '... and not a TODO test';
is $failed->as_string, 'not ok 4 - this is a real failure',
  '... and its string representation should be correct';
is $failed->raw, 'not ok 4 - this is a real failure',
  '... and raw() should return the original line';

# Some YAML
my $yaml = shift @results;
isa_ok $yaml, $YAML;
is $yaml->type, 'yaml', '... and it should report the correct type';
ok $yaml->is_yaml, '... and it should identify itself as yaml';
is_deeply $yaml->data, 'YAML!', '... and data should be correct';

# ok 5 # skip we have no description
# skipped test

$test = shift @results;
isa_ok $test, $TEST;
is $test->type, 'test', '... and it should report the correct type';
ok $test->is_test, '... and it should identify itself as a test';
is $test->ok,      'ok', '... and it should have the correct ok()';
ok $test->is_ok,   '... and the correct boolean version of is_ok()';
ok $test->is_actual_ok,
  '... and the correct boolean version of is_actual_ok()';
is $test->number, 5, '... and have the correct test number';
ok !$test->description, '... and skipped tests have no description';
is $test->directive, 'SKIP', '... and the correct directive';
is $test->explanation, 'we have no description',
  '... but we should have an explanation';
ok $test->has_skip, '... and it is a SKIPped test';
ok !$test->has_todo, '... but not a TODO test';
is $test->as_string, 'ok 5 # SKIP we have no description',
  '... and its string representation should be correct';
is $test->raw, 'ok 5 # skip we have no description',
  '... and raw() should return the original line';

# a failing test, which also happens to have a directive
# ok 6 - you shall not pass! # TODO should have failed

my $bonus = shift @results;
isa_ok $bonus, $TEST;
can_ok $bonus, 'todo_passed';
is $bonus->type, 'test', 'TODO tests should parse correctly';
ok $bonus->is_test, '... and it should identify itself as a test';
is $bonus->ok,      'ok', '... and it should have the correct ok()';
ok $bonus->is_ok,   '... and TODO tests should not always pass';
ok $bonus->is_actual_ok,
  '... and the correct boolean version of is_actual_ok ()';
is $bonus->number, 6, '... and have the correct failed number';
is $bonus->description, '- you shall not pass!',
  '... and the correct description';
is $bonus->directive, 'TODO', '... and should have the correct directive';
is $bonus->explanation, 'should have failed',
  '... and the correct directive explanation';
ok !$bonus->has_skip, '... and it is not a SKIPped failed';
ok $bonus->has_todo,  '... but it is a TODO succeeded';
is $bonus->as_string, 'ok 6 - you shall not pass! # TODO should have failed',
  '... and its string representation should be correct';
is $bonus->raw, 'ok 6 - you shall not pass! # TODO should have failed',
  '... and raw() should return the original line';
ok $bonus->todo_passed,
  '... todo_bonus() should pass for TODO tests which unexpectedly succeed';

# not ok 7 - Gandalf wins.  Game over.  # TODO 'bout time!

my $passed = shift @results;
isa_ok $passed, $TEST;
can_ok $passed, 'todo_passed';
is $passed->type, 'test', 'TODO tests should parse correctly';
ok $passed->is_test, '... and it should identify itself as a test';
is $passed->ok,      'not ok', '... and it should have the correct ok()';
ok $passed->is_ok,   '... and TODO tests should always pass';
ok !$passed->is_actual_ok,
  '... and the correct boolean version of is_actual_ok ()';
is $passed->number, 7, '... and have the correct passed number';
is $passed->description, '- Gandalf wins.  Game over.',
  '... and the correct description';
is $passed->directive, 'TODO', '... and should have the correct directive';
is $passed->explanation, "'bout time!",
  '... and the correct directive explanation';
ok !$passed->has_skip, '... and it is not a SKIPped passed';
ok $passed->has_todo, '... but it is a TODO succeeded';
is $passed->as_string,
  "not ok 7 - Gandalf wins.  Game over. # TODO 'bout time!",
  '... and its string representation should be correct';
is $passed->raw, "not ok 7 - Gandalf wins.  Game over.  # TODO 'bout time!",
  '... and raw() should return the original line';
ok !$passed->todo_passed,
  '... todo_passed() should not pass for TODO tests which failed';

# test parse results

can_ok $parser, 'passed';
is $parser->passed, 6,
  '... and we should have the correct number of passed tests';
is_deeply [ $parser->passed ], [ 1, 2, 3, 5, 6, 7 ],
  '... and get a list of the passed tests';

can_ok $parser, 'failed';
is $parser->failed, 1, '... and the correct number of failed tests';
is_deeply [ $parser->failed ], [4], '... and get a list of the failed tests';

can_ok $parser, 'actual_passed';
is $parser->actual_passed, 4,
  '... and we should have the correct number of actually passed tests';
is_deeply [ $parser->actual_passed ], [ 1, 3, 5, 6 ],
  '... and get a list of the actually passed tests';

can_ok $parser, 'actual_failed';
is $parser->actual_failed, 3,
  '... and the correct number of actually failed tests';
is_deeply [ $parser->actual_failed ], [ 2, 4, 7 ],
  '... or get a list of the actually failed tests';

can_ok $parser, 'todo';
is $parser->todo, 3,
  '... and we should have the correct number of TODO tests';
is_deeply [ $parser->todo ], [ 2, 6, 7 ],
  '... and get a list of the TODO tests';

can_ok $parser, 'skipped';
is $parser->skipped, 1,
  '... and we should have the correct number of skipped tests';
is_deeply [ $parser->skipped ], [5],
  '... and get a list of the skipped tests';

# check the plan

can_ok $parser, 'plan';
is $parser->plan,          '1..7', '... and we should have the correct plan';
is $parser->tests_planned, 7,      '... and the correct number of tests';

# "Unexpectedly succeeded"
can_ok $parser, 'todo_passed';
is scalar $parser->todo_passed, 1,
  '... and it should report the number of tests which unexpectedly succeeded';
is_deeply [ $parser->todo_passed ], [6],
  '... or *which* tests unexpectedly succeeded';

#
# Bug report from Torsten Schoenfeld
# Makes sure parser can handle blank lines
#

$tap = <<'END_TAP';
1..2
ok 1 - input file opened


ok 2 - read the rest of the file
END_TAP

my $aref = [ split /\n/ => $tap ];

can_ok $PARSER, 'new';
$parser
  = $PARSER->new( { iterator => TAP::Parser::Iterator::Array->new($aref) } );
isa_ok $parser, $PARSER, '... and calling it should succeed';

# results() is sane?

ok @results = _get_results($parser), 'The parser should return results';
is scalar @results, 5, '... and there should be one for each line';

# check the test plan

$result = shift @results;
isa_ok $result, $PLAN;
can_ok $result, 'type';
is $result->type, 'plan', '... and it should report the correct type';
ok $result->is_plan,   '... and it should identify itself as a plan';
is $result->plan,      '1..2', '... and identify the plan';
is $result->as_string, '1..2',
  '... and have the correct string representation';
is $result->raw, '1..2', '... and raw() should return the original line';

# a normal, passing test

$test = shift @results;
isa_ok $test, $TEST;
is $test->type, 'test', '... and it should report the correct type';
ok $test->is_test, '... and it should identify itself as a test';
is $test->ok,      'ok', '... and it should have the correct ok()';
ok $test->is_ok,   '... and the correct boolean version of is_ok()';
ok $test->is_actual_ok,
  '... and the correct boolean version of is_actual_ok()';
is $test->number, 1, '... and have the correct test number';
is $test->description, '- input file opened',
  '... and the correct description';
ok !$test->directive,   '... and not have a directive';
ok !$test->explanation, '... or a directive explanation';
ok !$test->has_skip,    '... and it is not a SKIPped test';
ok !$test->has_todo,    '... nor a TODO test';
is $test->as_string, 'ok 1 - input file opened',
  '... and its string representation should be correct';
is $test->raw, 'ok 1 - input file opened',
  '... and raw() should return the original line';

# junk lines should be preserved

$unknown = shift @results;
isa_ok $unknown, $UNKNOWN;
is $unknown->type, 'unknown', '... and it should report the correct type';
ok $unknown->is_unknown, '... and it should identify itself as unknown';
is $unknown->as_string,  '',
  '... and its string representation should be returned verbatim';
is $unknown->raw, '', '... and raw() should return the original line';

# ... and the second empty line

$unknown = shift @results;
isa_ok $unknown, $UNKNOWN;
is $unknown->type, 'unknown', '... and it should report the correct type';
ok $unknown->is_unknown, '... and it should identify itself as unknown';
is $unknown->as_string,  '',
  '... and its string representation should be returned verbatim';
is $unknown->raw, '', '... and raw() should return the original line';

# a passing test

$test = shift @results;
isa_ok $test, $TEST;
is $test->type, 'test', '... and it should report the correct type';
ok $test->is_test, '... and it should identify itself as a test';
is $test->ok,      'ok', '... and it should have the correct ok()';
ok $test->is_ok,   '... and the correct boolean version of is_ok()';
ok $test->is_actual_ok,
  '... and the correct boolean version of is_actual_ok()';
is $test->number, 2, '... and have the correct test number';
is $test->description, '- read the rest of the file',
  '... and the correct description';
ok !$test->directive,   '... and not have a directive';
ok !$test->explanation, '... or a directive explanation';
ok !$test->has_skip,    '... and it is not a SKIPped test';
ok !$test->has_todo,    '... nor a TODO test';
is $test->as_string, 'ok 2 - read the rest of the file',
  '... and its string representation should be correct';
is $test->raw, 'ok 2 - read the rest of the file',
  '... and raw() should return the original line';

is scalar $parser->passed, 2,
  'Empty junk lines should not affect the correct number of tests passed';

# Check source => "tap content"
can_ok $PARSER, 'new';
$parser = $PARSER->new( { source => "1..1\nok 1\n" } );
isa_ok $parser, $PARSER, '... and calling it should succeed';
ok @results = _get_results($parser), 'The parser should return results';
is( scalar @results, 2, "Got two lines of TAP" );

# Check source => [array]
can_ok $PARSER, 'new';
$parser = $PARSER->new( { source => [ "1..1", "ok 1" ] } );
isa_ok $parser, $PARSER, '... and calling it should succeed';
ok @results = _get_results($parser), 'The parser should return results';
is( scalar @results, 2, "Got two lines of TAP" );

# Check source => $filehandle
can_ok $PARSER, 'new';
open my $fh, 't/data/catme.1';
$parser = $PARSER->new( { source => $fh } );
isa_ok $parser, $PARSER, '... and calling it should succeed';
ok @results = _get_results($parser), 'The parser should return results';
is( scalar @results, 2, "Got two lines of TAP" );

{

    # set a spool to write to
    tie local *SPOOL, 'IO::c55Capture';

    my $tap = <<'END_TAP';
TAP version 13
1..7
ok 1 - input file opened
... this is junk
not ok first line of the input valid # todo some data
# this is a comment
ok 3 - read the rest of the file
not ok 4 - this is a real failure
  --- YAML!
  ...
ok 5 # skip we have no description
ok 6 - you shall not pass! # TODO should have failed
not ok 7 - Gandalf wins.  Game over.  # TODO 'bout time!
END_TAP

    {
        my $parser = $PARSER->new(
            {   tap   => $tap,
                spool => \*SPOOL,
            }
        );

        _get_results($parser);

        my @spooled = tied(*SPOOL)->dump();

        is @spooled, 24, 'coverage testing for spool attribute of parser';
        is join( '', @spooled ), $tap, "spooled tap matches";
    }

    {
        my $parser = $PARSER->new(
            {   tap   => $tap,
                spool => \*SPOOL,
            }
        );

        $parser->callback( 'ALL', sub { } );

        _get_results($parser);

        my @spooled = tied(*SPOOL)->dump();

        is @spooled, 24, 'coverage testing for spool attribute of parser';
        is join( '', @spooled ), $tap, "spooled tap matches";
    }
}

{

    # _initialize coverage

    my $x = bless [], 'kjsfhkjsdhf';

    my @die;

    eval {
        local $SIG{__DIE__} = sub { push @die, @_ };

        $PARSER->new();
    };

    is @die, 1, 'coverage testing for _initialize';

    like pop @die, qr/PANIC:\s+could not determine iterator for input\s*at/,
      '...and it failed as expected';

    @die = ();

    eval {
        local $SIG{__DIE__} = sub { push @die, @_ };

        $PARSER->new(
            {   iterator => 'iterator',
                tap      => 'tap',
                source   => 'source',     # only one of these is allowed
            }
        );
    };

    is @die, 1, 'coverage testing for _initialize';

    like pop @die,
      qr/You may only choose one of 'exec', 'tap', 'source' or 'iterator'/,
      '...and it failed as expected';
}

{

    # coverage of todo_failed

    my $tap = <<'END_TAP';
TAP version 13
1..7
ok 1 - input file opened
... this is junk
not ok first line of the input valid # todo some data
# this is a comment
ok 3 - read the rest of the file
not ok 4 - this is a real failure
  --- YAML!
  ...
ok 5 # skip we have no description
ok 6 - you shall not pass! # TODO should have failed
not ok 7 - Gandalf wins.  Game over.  # TODO 'bout time!
END_TAP

    my $parser = $PARSER->new( { tap => $tap } );

    _get_results($parser);

    my @warn;

    eval {
        local $SIG{__WARN__} = sub { push @warn, @_ };

        $parser->todo_failed;
    };

    is @warn, 1, 'coverage testing of todo_failed';

    like pop @warn,
      qr/"todo_failed" is deprecated.  Please use "todo_passed".  See the docs[.]/,
      '..and failed as expected'
}

{

    # coverage testing for T::P::_initialize

    # coverage of the source argument paths

    # ref argument to source

    my $parser = TAP::Parser->new( { source => [ split /$/, $tap ] } );

    isa_ok $parser, 'TAP::Parser';

    isa_ok $parser->_iterator, 'TAP::Parser::Iterator::Array';

    SKIP: {
        skip 'Segfaults Perl 5.6.0' => 2 if $] <= 5.006000;

        # uncategorisable argument to source
        my @die;

        eval {
            local $SIG{__DIE__} = sub { push @die, @_ };

            $parser = TAP::Parser->new( { source => 'nosuchfile' } );
        };

        is @die, 1, 'uncategorisable source';

        like pop @die, qr/Cannot detect source of 'nosuchfile'/,
          '... and we died as expected';
    }
}

{

    # coverage test of perl source with switches

    my $parser = TAP::Parser->new(
        {   source => File::Spec->catfile(
                't',
                'sample-tests',
                'simple'
            ),
        }
    );

    isa_ok $parser, 'TAP::Parser';

    isa_ok $parser->_iterator, 'TAP::Parser::Iterator::Process';

    # Workaround for Mac OS X problem wrt closing the iterator without
    # reading from it.
    $parser->next;
}

{

    # coverage testing for TAP::Parser::has_problems

    # we're going to need to test lots of fragments of tap
    # to cover all the different boolean tests

    # currently covered are no problems and failed, so let's next test
    # todo_passed

    my $tap = <<'END_TAP';
TAP version 13
1..2
ok 1 - input file opened
ok 2 - Gandalf wins.  Game over.  # TODO 'bout time!
END_TAP

    my $parser = TAP::Parser->new( { tap => $tap } );

    _get_results($parser);

    ok !$parser->failed, 'parser didnt fail';
    ok $parser->todo_passed, '... and todo_passed is true';

    ok !$parser->has_problems, '... and has_problems is false';

    # now parse_errors

    $tap = <<'END_TAP';
TAP version 13
1..2
SMACK
END_TAP

    $parser = TAP::Parser->new( { tap => $tap } );

    _get_results($parser);

    ok !$parser->failed,      'parser didnt fail';
    ok !$parser->todo_passed, '... and todo_passed is false';
    ok $parser->parse_errors, '... and parse_errors is true';

    ok $parser->has_problems, '... and has_problems';

    # Now wait and exit are hard to do in an OS platform-independent way, so
    # we won't even bother

    $tap = <<'END_TAP';
TAP version 13
1..2
ok 1 - input file opened
ok 2 - Gandalf wins
END_TAP

    $parser = TAP::Parser->new( { tap => $tap } );

    _get_results($parser);

    $parser->wait(1);

    ok !$parser->failed,       'parser didnt fail';
    ok !$parser->todo_passed,  '... and todo_passed is false';
    ok !$parser->parse_errors, '... and parse_errors is false';

    ok $parser->wait, '... and wait is set';

    ok $parser->has_problems, '... and has_problems';

    # and use the same for exit

    $parser->wait(0);
    $parser->exit(1);

    ok !$parser->failed,       'parser didnt fail';
    ok !$parser->todo_passed,  '... and todo_passed is false';
    ok !$parser->parse_errors, '... and parse_errors is false';
    ok !$parser->wait,         '... and wait is not set';

    ok $parser->exit, '... and exit is set';

    ok $parser->has_problems, '... and has_problems';
}

{

    # coverage testing of the version states

    my $tap = <<'END_TAP';
TAP version 12
1..2
ok 1 - input file opened
ok 2 - Gandalf wins
END_TAP

    my $parser = TAP::Parser->new( { tap => $tap } );

    _get_results($parser);

    my @errors = $parser->parse_errors;

    is @errors, 1, 'test too low version number';

    like pop @errors,
      qr/Explicit TAP version must be at least 13. Got version 12/,
      '... and trapped expected version error';

    # now too high a version
    $tap = <<'END_TAP';
TAP version 14
1..2
ok 1 - input file opened
ok 2 - Gandalf wins
END_TAP

    $parser = TAP::Parser->new( { tap => $tap } );

    _get_results($parser);

    @errors = $parser->parse_errors;

    is @errors, 1, 'test too high version number';

    like pop @errors,
      qr/TAP specified version 14 but we don't know about versions later than 13/,
      '... and trapped expected version error';
}

{

    # coverage testing of TAP version in the wrong place

    my $tap = <<'END_TAP';
1..2
ok 1 - input file opened
TAP version 12
ok 2 - Gandalf wins
END_TAP

    my $parser = TAP::Parser->new( { tap => $tap } );

    _get_results($parser);

    my @errors = $parser->parse_errors;

    is @errors, 1, 'test TAP version number in wrong place';

    like pop @errors,
      qr/If TAP version is present it must be the first line of output/,
      '... and trapped expected version error';

}

{

    # we're going to bash the internals a bit (but using the API as
    # much as possible) to force grammar->tokenise() to fail

# firstly we'll create a iterator that dies when its next_raw method is called

    package TAP::Parser::Iterator::Dies;

    use strict;

    use base qw(TAP::Parser::Iterator);

    sub next_raw {
        die 'this is the dying iterator';
    }

    # required as part of the TPI interface
    sub exit { }
    sub wait { }

    package main;

    # now build a standard parser

    my $tap = <<'END_TAP';
1..2
ok 1 - input file opened
ok 2 - Gandalf wins
END_TAP

    {
        my $parser = TAP::Parser->new( { tap => $tap } );

        # build a dying iterator
        my $iterator = TAP::Parser::Iterator::Dies->new;

        # now replace the iterator - we're forced to us an T::P intenal
        # method for this
        $parser->_iterator($iterator);

        # build a new grammar
        my $grammar = TAP::Parser::Grammar->new(
            {   iterator => $iterator,
                parser   => $parser
            }
        );

        # replace our grammar with this new one
        $parser->_grammar($grammar);

        # now call next on the parser, and the grammar should die
        my $result = $parser->next;    # will die in iterator

        is $result, undef, 'iterator dies';

        my @errors = $parser->parse_errors;
        is @errors, 2, '...and caught expected errrors';

        like shift @errors, qr/this is the dying iterator/,
          '...and it was what we expected';
    }

    # Do it all again with callbacks to exercise the other code path in
    # the unrolled iterator
    {
        my $parser = TAP::Parser->new( { tap => $tap } );

        $parser->callback( 'ALL', sub { } );

        # build a dying iterator
        my $iterator = TAP::Parser::Iterator::Dies->new;

        # now replace the iterator - we're forced to us an T::P intenal
        # method for this
        $parser->_iterator($iterator);

        # build a new grammar
        my $grammar = TAP::Parser::Grammar->new(
            {   iterator => $iterator,
                parser   => $parser
            }
        );

        # replace our grammar with this new one
        $parser->_grammar($grammar);

        # now call next on the parser, and the grammar should die
        my $result = $parser->next;    # will die in iterator

        is $result, undef, 'iterator dies';

        my @errors = $parser->parse_errors;
        is @errors, 2, '...and caught expected errrors';

        like shift @errors, qr/this is the dying iterator/,
          '...and it was what we expected';
    }
}

{

    # coverage testing of TAP::Parser::_next_state

    package TAP::Parser::WithBrokenState;

    use base qw( TAP::Parser );

    sub _make_state_table {
        return { INIT => { plan => { goto => 'FOO' } } };
    }

    package main;

    my $tap = <<'END_TAP';
1..2
ok 1 - input file opened
ok 2 - Gandalf wins
END_TAP

    my $parser = TAP::Parser::WithBrokenState->new( { tap => $tap } );

    my @die;

    eval {
        local $SIG{__DIE__} = sub { push @die, @_ };

        $parser->next;
        $parser->next;
    };

    is @die, 1, 'detect broken state machine';

    like pop @die, qr/Illegal state: FOO/,
      '...and the message is as we expect';
}

{

    # coverage testing of TAP::Parser::_iter

    package TAP::Parser::WithBrokenIter;

    use base qw( TAP::Parser );

    sub _iter {return}

    package main;

    my $tap = <<'END_TAP';
1..2
ok 1 - input file opened
ok 2 - Gandalf wins
END_TAP

    my $parser = TAP::Parser::WithBrokenIter->new( { tap => $tap } );

    my @die;

    eval {
        local $SIG{__WARN__} = sub { };
        local $SIG{__DIE__} = sub { push @die, @_ };

        $parser->next;
    };

    is @die, 1, 'detect broken iter';

    like pop @die, qr/Can't use/, '...and the message is as we expect';
}

SKIP: {

    # http://markmail.org/message/rkxbo6ft7yorgnzb
    skip "Crashes on older Perls", 2 if $] <= 5.008004 || $] == 5.009;

    # coverage testing of TAP::Parser::_finish

    my $tap = <<'END_TAP';
1..2
ok 1 - input file opened
ok 2 - Gandalf wins
END_TAP

    my $parser = TAP::Parser->new( { tap => $tap } );

    $parser->tests_run(999);

    my @die;

    eval {
        local $SIG{__DIE__} = sub { push @die, @_ };

        _get_results $parser;
    };

    is @die, 1, 'detect broken test counts';

    like pop @die,
      qr/Panic: planned test count [(]1001[)] did not equal sum of passed [(]0[)] and failed [(]2[)] tests!/,
      '...and the message is as we expect';
}

{

    # Sanity check on state table

    my $parser      = TAP::Parser->new( { tap => "1..1\nok 1\n" } );
    my $state_table = $parser->_make_state_table;
    my @states      = sort keys %$state_table;
    my @expect      = sort qw(
      bailout comment plan pragma test unknown version yaml
    );

    my %reachable = ( INIT => 1 );

    for my $name (@states) {
        my $state      = $state_table->{$name};
        my @can_handle = sort keys %$state;
        is_deeply \@can_handle, \@expect, "token types handled in $name";
        for my $type (@can_handle) {
            $reachable{$_}++
              for grep {defined}
              map      { $state->{$type}->{$_} } qw(goto continue);
        }
    }

    is_deeply [ sort keys %reachable ], [@states], "all states reachable";
}

{

    # exit, wait, ignore_exit interactions

    my @truth = (
        [ 0, 0, 0, 0 ],
        [ 0, 0, 1, 0 ],
        [ 1, 0, 0, 1 ],
        [ 1, 0, 1, 0 ],
        [ 1, 1, 0, 1 ],
        [ 1, 1, 1, 0 ],
        [ 0, 1, 0, 1 ],
        [ 0, 1, 1, 0 ],
    );

    for my $t (@truth) {
        my ( $wait, $exit, $ignore_exit, $has_problems ) = @$t;
        my $test_parser = sub {
            my $parser = shift;
            $parser->wait($wait);
            $parser->exit($exit);
            ok $has_problems ? $parser->has_problems : !$parser->has_problems,
              "exit=$exit, wait=$wait, ignore=$ignore_exit";
        };

        my $parser = TAP::Parser->new( { tap => "1..1\nok 1\n" } );
        $parser->ignore_exit($ignore_exit);
        $test_parser->($parser);

        $test_parser->(
            TAP::Parser->new(
                { tap => "1..1\nok 1\n", ignore_exit => $ignore_exit }
            )
        );
    }
}
