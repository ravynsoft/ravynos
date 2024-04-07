#!/usr/bin/perl -w

use strict;
use warnings;

BEGIN {
    unshift @INC, 't/lib';
}

use Test::More tests => 94;

use EmptyParser;
use TAP::Parser::Grammar;
use TAP::Parser::Iterator::Array;

my $GRAMMAR = 'TAP::Parser::Grammar';

# Array based iterator that we can push items in to
package IT;

sub new {
    my $class = shift;
    return bless [], $class;
}

sub next {
    my $self = shift;
    return shift @$self;
}

sub put {
    my $self = shift;
    unshift @$self, @_;
}

sub handle_unicode { }

package main;

my $iterator = IT->new;
my $parser   = EmptyParser->new;
can_ok $GRAMMAR, 'new';
my $grammar = $GRAMMAR->new( { iterator => $iterator, parser => $parser } );
isa_ok $grammar, $GRAMMAR, '... and the object it returns';

# Note:  all methods are actually class methods.  See the docs for the reason
# why.  We'll still use the instance because that should be forward
# compatible.

my @V12 = sort qw(bailout comment plan simple_test test version);
my @V13 = sort ( @V12, 'pragma', 'yaml' );

can_ok $grammar, 'token_types';
ok my @types = sort( $grammar->token_types ),
  '... and calling it should succeed (v12)';
is_deeply \@types, \@V12, '... and return the correct token types (v12)';

$grammar->set_version(13);
ok @types = sort( $grammar->token_types ),
  '... and calling it should succeed (v13)';
is_deeply \@types, \@V13, '... and return the correct token types (v13)';

can_ok $grammar, 'syntax_for';
can_ok $grammar, 'handler_for';

my ( %syntax_for, %handler_for );
for my $type (@types) {
    ok $syntax_for{$type} = $grammar->syntax_for($type),
      '... and calling syntax_for() with a type name should succeed';
    cmp_ok ref $syntax_for{$type}, 'eq', 'Regexp',
      '... and it should return a regex';

    ok $handler_for{$type} = $grammar->handler_for($type),
      '... and calling handler_for() with a type name should succeed';
    cmp_ok ref $handler_for{$type}, 'eq', 'CODE',
      '... and it should return a code reference';
}

# Test the plan.  Gotta have a plan.
my $plan = '1..1';
like $plan, $syntax_for{'plan'}, 'A basic plan should match its syntax';

my $method = $handler_for{'plan'};
$plan =~ $syntax_for{'plan'};
ok my $plan_token = $grammar->$method($plan),
  '... and the handler should return a token';

my $expected = {
    'explanation'   => '',
    'directive'     => '',
    'type'          => 'plan',
    'tests_planned' => 1,
    'raw'           => '1..1',
    'todo_list'     => [],
};
is_deeply $plan_token, $expected,
  '... and it should contain the correct data';

can_ok $grammar, 'tokenize';
$iterator->put($plan);
ok my $token = $grammar->tokenize,
  '... and calling it with data should return a token';
is_deeply $token, $expected,
  '... and the token should contain the correct data';

# a plan with a skip directive

$plan = '1..0 # SKIP why not?';
like $plan, $syntax_for{'plan'}, 'a basic plan should match its syntax';

$plan =~ $syntax_for{'plan'};
ok $plan_token = $grammar->$method($plan),
  '... and the handler should return a token';

$expected = {
    'explanation'   => 'why not?',
    'directive'     => 'SKIP',
    'type'          => 'plan',
    'tests_planned' => 0,
    'raw'           => '1..0 # SKIP why not?',
    'todo_list'     => [],
};
is_deeply $plan_token, $expected,
  '... and it should contain the correct data';

$iterator->put($plan);
ok $token = $grammar->tokenize,
  '... and calling it with data should return a token';
is_deeply $token, $expected,
  '... and the token should contain the correct data';

# implied skip

$plan = '1..0';
like $plan, $syntax_for{'plan'},
  'A plan  with an implied "skip all" should match its syntax';

$plan =~ $syntax_for{'plan'};
ok $plan_token = $grammar->$method($plan),
  '... and the handler should return a token';

$expected = {
    'explanation'   => '',
    'directive'     => 'SKIP',
    'type'          => 'plan',
    'tests_planned' => 0,
    'raw'           => '1..0',
    'todo_list'     => [],
};
is_deeply $plan_token, $expected,
  '... and it should contain the correct data';

$iterator->put($plan);
ok $token = $grammar->tokenize,
  '... and calling it with data should return a token';
is_deeply $token, $expected,
  '... and the token should contain the correct data';

# bad plan

$plan = '1..0 # TODO 3,4,5';    # old syntax.  No longer supported
unlike $plan, $syntax_for{'plan'},
  'Bad plans should not match the plan syntax';

# Bail out!

my $bailout = 'Bail out!';
like $bailout, $syntax_for{'bailout'},
  'Bail out! should match a bailout syntax';

$iterator->put($bailout);
ok $token = $grammar->tokenize,
  '... and calling it with data should return a token';
$expected = {
    'bailout' => '',
    'type'    => 'bailout',
    'raw'     => 'Bail out!'
};
is_deeply $token, $expected,
  '... and the token should contain the correct data';

$bailout = 'Bail out! some explanation';
like $bailout, $syntax_for{'bailout'},
  'Bail out! should match a bailout syntax';

$iterator->put($bailout);
ok $token = $grammar->tokenize,
  '... and calling it with data should return a token';
$expected = {
    'bailout' => 'some explanation',
    'type'    => 'bailout',
    'raw'     => 'Bail out! some explanation'
};
is_deeply $token, $expected,
  '... and the token should contain the correct data';

# test comment

my $comment = '# this is a comment';
like $comment, $syntax_for{'comment'},
  'Comments should match the comment syntax';

$iterator->put($comment);
ok $token = $grammar->tokenize,
  '... and calling it with data should return a token';
$expected = {
    'comment' => 'this is a comment',
    'type'    => 'comment',
    'raw'     => '# this is a comment'
};
is_deeply $token, $expected,
  '... and the token should contain the correct data';

# test tests :/

my $test = 'ok 1 this is a test';
like $test, $syntax_for{'test'}, 'Tests should match the test syntax';

$iterator->put($test);
ok $token = $grammar->tokenize,
  '... and calling it with data should return a token';

$expected = {
    'ok'          => 'ok',
    'explanation' => '',
    'type'        => 'test',
    'directive'   => '',
    'description' => 'this is a test',
    'test_num'    => '1',
    'raw'         => 'ok 1 this is a test'
};
is_deeply $token, $expected,
  '... and the token should contain the correct data';

# TODO tests

$test = 'not ok 2 this is a test # TODO whee!';
like $test, $syntax_for{'test'}, 'Tests should match the test syntax';

$iterator->put($test);
ok $token = $grammar->tokenize,
  '... and calling it with data should return a token';

$expected = {
    'ok'          => 'not ok',
    'explanation' => 'whee!',
    'type'        => 'test',
    'directive'   => 'TODO',
    'description' => 'this is a test',
    'test_num'    => '2',
    'raw'         => 'not ok 2 this is a test # TODO whee!'
};
is_deeply $token, $expected, '... and the TODO should be parsed';

# false TODO tests

# escaping that hash mark ('#') means this should *not* be a TODO test
$test = 'ok 22 this is a test \# TODO whee!';
like $test, $syntax_for{'test'}, 'Tests should match the test syntax';

$iterator->put($test);
ok $token = $grammar->tokenize,
  '... and calling it with data should return a token';

$expected = {
    'ok'          => 'ok',
    'explanation' => '',
    'type'        => 'test',
    'directive'   => '',
    'description' => 'this is a test \# TODO whee!',
    'test_num'    => '22',
    'raw'         => 'ok 22 this is a test \# TODO whee!'
};
is_deeply $token, $expected,
  '... and the token should contain the correct data';

# pragmas

my $pragma = 'pragma +strict';
like $pragma, $syntax_for{'pragma'}, 'Pragmas should match the pragma syntax';

$iterator->put($pragma);
ok $token = $grammar->tokenize,
  '... and calling it with data should return a token';

$expected = {
    'type'    => 'pragma',
    'raw'     => $pragma,
    'pragmas' => ['+strict'],
};

is_deeply $token, $expected,
  '... and the token should contain the correct data';

$pragma = 'pragma +strict,-foo';
like $pragma, $syntax_for{'pragma'}, 'Pragmas should match the pragma syntax';

$iterator->put($pragma);
ok $token = $grammar->tokenize,
  '... and calling it with data should return a token';

$expected = {
    'type'    => 'pragma',
    'raw'     => $pragma,
    'pragmas' => [ '+strict', '-foo' ],
};

is_deeply $token, $expected,
  '... and the token should contain the correct data';

$pragma = 'pragma  +strict  ,  -foo ';
like $pragma, $syntax_for{'pragma'}, 'Pragmas should match the pragma syntax';

$iterator->put($pragma);
ok $token = $grammar->tokenize,
  '... and calling it with data should return a token';

$expected = {
    'type'    => 'pragma',
    'raw'     => $pragma,
    'pragmas' => [ '+strict', '-foo' ],
};

is_deeply $token, $expected,
  '... and the token should contain the correct data';

# coverage tests

# set_version

{
    my @die;

    eval {
        local $SIG{__DIE__} = sub { push @die, @_ };

        $grammar->set_version('no_such_version');
    };

    unless ( is @die, 1, 'set_version with bad version' ) {
        diag " >>> $_ <<<\n" for @die;
    }

    like pop @die, qr/^Unsupported syntax version: no_such_version at /,
      '... and got expected message';
}

# tokenize
{
    my $iterator = IT->new;
    my $parser   = EmptyParser->new;
    my $grammar
      = $GRAMMAR->new( { iterator => $iterator, parser => $parser } );

    my $plan = '';

    $iterator->put($plan);

    my $result = $grammar->tokenize();

    isa_ok $result, 'TAP::Parser::Result::Unknown';
}

# _make_plan_token

{
    my $parser = EmptyParser->new;
    my $grammar = $GRAMMAR->new( { parser => $parser } );

    my $plan
      = '1..1 # SKIP with explanation';  # trigger warning in _make_plan_token

    my $method = $handler_for{'plan'};

    $plan =~ $syntax_for{'plan'};        # perform regex to populate $1, $2

    my @warn;

    eval {
        local $SIG{__WARN__} = sub { push @warn, @_ };

        $grammar->$method($plan);
    };

    is @warn, 1, 'catch warning on inconsistent plan';

    like pop @warn,
      qr/^Specified SKIP directive in plan but more than 0 tests [(]1\.\.1 # SKIP with explanation[)]/,
      '... and its what we expect';
}

# _make_yaml_token

SKIP: {
	skip 'Test is broken and needs repairs', 2;
    my $iterator = IT->new;
    my $parser   = EmptyParser->new;
    my $grammar
      = $GRAMMAR->new( { iterator => $iterator, parser => $parser } );

    $grammar->set_version(13);

    # now this is badly formed YAML that is missing the
    # leader padding - this is done for coverage testing
    # the $reader code sub in _make_yaml_token, that is
    # passed as the yaml consumer to T::P::YAMLish::Reader.

    # because it isnt valid yaml, the yaml document is
    # not done, and the _peek in the YAMLish::Reader
    # code doesnt find the terminating '...' pattern.
    # but we dont care as this is coverage testing, so
    # if thats what we have to do to exercise that code,
    # so be it.
    my $yaml = [ '  ---  ', '- 2', '  ...  ', ];

    sub iter {
        my $ar = shift;
        return sub {
            return shift @$ar;
        };
    }

    my $iter = iter($yaml);

    while ( my $line = $iter->() ) {
        $iterator->put($line);
    }

    # pad == '   ', marker == '--- '
    # length $pad == 3
    # strip == pad

    my @die;

    eval {
        local $SIG{__DIE__} = sub { push @die, @_ };
        $grammar->tokenize;
    };

    is @die, 1, 'checking badly formed yaml for coverage testing';

    like pop @die, qr/^Missing '[.][.][.]' at end of YAMLish/,
      '...and it died like we expect';
}

{

    # coverage testing for TAP::Parser::Iterator::Array

    my $source = [qw( a b c )];

    my $aiter = TAP::Parser::Iterator::Array->new($source);

    my $first = $aiter->next_raw;

    is $first, 'a', 'access raw iterator';

    is $aiter->exit, undef, '... and note we didnt exhaust the source';
}
