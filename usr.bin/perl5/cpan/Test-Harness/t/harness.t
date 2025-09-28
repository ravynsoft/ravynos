#!/usr/bin/perl -w

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;

use Test::More;
use IO::c55Capture;

use Config;
use POSIX;

use TAP::Harness;

# This is done to prevent the colors environment variables from
# interfering.
local $ENV{HARNESS_SUMMARY_COLOR_FAIL};
local $ENV{HARNESS_SUMMARY_COLOR_SUCCESS};
delete $ENV{HARNESS_SUMMARY_COLOR_FAIL};
delete $ENV{HARNESS_SUMMARY_COLOR_SUCCESS};

my $HARNESS = 'TAP::Harness';

my $source_tests = 't/source_tests';
my $sample_tests = 't/sample-tests';

plan tests => 133;

# note that this test will always pass when run through 'prove'
ok $ENV{HARNESS_ACTIVE},  'HARNESS_ACTIVE env variable should be set';
ok $ENV{HARNESS_VERSION}, 'HARNESS_VERSION env variable should be set';

#### For color tests ####

package Colorizer;

sub new { bless {}, shift }
sub can_color {1}

sub set_color {
    my ( $self, $output, $color ) = @_;
    $output->("[[$color]]");
}

package main;

sub colorize {
    my $harness = shift;
    $harness->formatter->_colorizer( Colorizer->new );
}

can_ok $HARNESS, 'new';

eval { $HARNESS->new( { no_such_key => 1 } ) };
like $@, qr/\QUnknown arguments to TAP::Harness::new (no_such_key)/,
  '... and calling it with bad keys should fail';

eval { $HARNESS->new( { lib => 'aint_no_such_lib' } ) };
is $@, '', '... and calling it with a non-existent lib is fine';

eval { $HARNESS->new( { lib => [qw/bad_lib_1 bad_lib_2/] } ) };
is $@, '', '... and calling it with non-existent libs is fine';

ok my $harness = $HARNESS->new,
  'Calling new() without arguments should succeed';

for my $test_args ( get_arg_sets() ) {
    my %args = %$test_args;
    for my $key ( sort keys %args ) {
        $args{$key} = $args{$key}{in};
    }
    ok my $harness = $HARNESS->new( {%args} ),
      'Calling new() with valid arguments should succeed';
    isa_ok $harness, $HARNESS, '... and the object it returns';

    while ( my ( $property, $test ) = each %$test_args ) {
        my $value = $test->{out};
        can_ok $harness, $property;
        is_deeply scalar $harness->$property(), $value, $test->{test_name};
    }
}

{
    my @output;
    no warnings 'redefine';
    local *TAP::Formatter::Base::_output = sub {
        my $self = shift;
        push @output => grep { $_ ne '' }
          map {
            local $_ = $_;
            chomp;
            trim($_)
          } @_;
    };
    my $harness = TAP::Harness->new(
        { verbosity => 1, formatter_class => "TAP::Formatter::Console" } );
    my $harness_whisper = TAP::Harness->new(
        { verbosity => -1, formatter_class => "TAP::Formatter::Console" } );
    my $harness_mute = TAP::Harness->new(
        { verbosity => -2, formatter_class => "TAP::Formatter::Console" } );
    my $harness_directives = TAP::Harness->new(
        { directives => 1, formatter_class => "TAP::Formatter::Console" } );
    my $harness_failures = TAP::Harness->new(
        { failures => 1, formatter_class => "TAP::Formatter::Console" } );

    colorize($harness);

    can_ok $harness, 'runtests';

    # normal tests in verbose mode

    ok my $aggregate = _runtests( $harness, "$source_tests/harness" ),
      '... runtests returns the aggregate';

    isa_ok $aggregate, 'TAP::Parser::Aggregator';

    chomp(@output);

    my @expected = (
        "$source_tests/harness ..",
        '1..1',
        '[[reset]]',
        'ok 1 - this is a test',
        '[[reset]]',
        'ok',
        '[[green]]',
        'All tests successful.',
        '[[reset]]',
    );
    my $status           = pop @output;
    my $expected_status  = qr{^Result: PASS$};
    my $summary          = pop @output;
    my $expected_summary = qr{^Files=1, Tests=1, +\d+ wallclock secs};

    is_deeply \@output, \@expected, '... and the output should be correct';
    like $status, $expected_status,
      '... and the status line should be correct';
    like $summary, $expected_summary,
      '... and the report summary should look correct';

    # use an alias for test name

    @output = ();
    ok $aggregate
      = _runtests( $harness, [ "$source_tests/harness", 'My Nice Test' ] ),
      '... runtests returns the aggregate';

    isa_ok $aggregate, 'TAP::Parser::Aggregator';

    chomp(@output);

    @expected = (
        'My Nice Test ..',
        '1..1',
        '[[reset]]',
        'ok 1 - this is a test',
        '[[reset]]',
        'ok',
        '[[green]]',
        'All tests successful.',
        '[[reset]]',
    );
    $status           = pop @output;
    $expected_status  = qr{^Result: PASS$};
    $summary          = pop @output;
    $expected_summary = qr{^Files=1, Tests=1, +\d+ wallclock secs};

    is_deeply \@output, \@expected, '... and the output should be correct';
    like $status, $expected_status,
      '... and the status line should be correct';
    like $summary, $expected_summary,
      '... and the report summary should look correct';

    # run same test twice

    @output = ();
    ok $aggregate = _runtests(
        $harness, [ "$source_tests/harness", 'My Nice Test' ],
        [ "$source_tests/harness", 'My Nice Test Again' ]
      ),
      '... runtests returns the aggregate';

    isa_ok $aggregate, 'TAP::Parser::Aggregator';

    chomp(@output);

    @expected = (
        'My Nice Test ........',
        '1..1',
        '[[reset]]',
        'ok 1 - this is a test',
        '[[reset]]',
        'ok',
        'My Nice Test Again ..',
        '1..1',
        '[[reset]]',
        'ok 1 - this is a test',
        '[[reset]]',
        'ok',
        '[[green]]',
        'All tests successful.',
        '[[reset]]',
    );
    $status           = pop @output;
    $expected_status  = qr{^Result: PASS$};
    $summary          = pop @output;
    $expected_summary = qr{^Files=2, Tests=2, +\d+ wallclock secs};

    is_deeply \@output, \@expected, '... and the output should be correct';
    like $status, $expected_status,
      '... and the status line should be correct';
    like $summary, $expected_summary,
      '... and the report summary should look correct';

    # normal tests in quiet mode

    @output = ();
    _runtests( $harness_whisper, "$source_tests/harness" );

    chomp(@output);
    @expected = (
        "$source_tests/harness ..",
        'ok',
        'All tests successful.',
    );

    $status           = pop @output;
    $expected_status  = qr{^Result: PASS$};
    $summary          = pop @output;
    $expected_summary = qr/^Files=1, Tests=1, +\d+ wallclock secs/;

    is_deeply \@output, \@expected, '... and the output should be correct';
    like $status, $expected_status,
      '... and the status line should be correct';
    like $summary, $expected_summary,
      '... and the report summary should look correct';

    # normal tests in really_quiet mode

    @output = ();
    _runtests( $harness_mute, "$source_tests/harness" );

    chomp(@output);
    @expected = (
        'All tests successful.',
    );

    $status           = pop @output;
    $expected_status  = qr{^Result: PASS$};
    $summary          = pop @output;
    $expected_summary = qr/^Files=1, Tests=1, +\d+ wallclock secs/;

    is_deeply \@output, \@expected, '... and the output should be correct';
    like $status, $expected_status,
      '... and the status line should be correct';
    like $summary, $expected_summary,
      '... and the report summary should look correct';

    # normal tests with failures

    @output = ();
    _runtests( $harness, "$source_tests/harness_failure" );

    $status  = pop @output;
    $summary = pop @output;

    like $status, qr{^Result: FAIL$},
      '... and the status line should be correct';

    my @summary = @output[ 18 .. $#output ];
    @output = @output[ 0 .. 17 ];

    @expected = (
        "$source_tests/harness_failure ..",
        '1..2',
        '[[reset]]',
        'ok 1 - this is a test',
        '[[reset]]',
        '[[red]]',
        'not ok 2 - this is another test',
        '[[reset]]',
        q{#   Failed test 'this is another test'},
        '[[reset]]',
        '#   in harness_failure.t at line 5.',
        '[[reset]]',
        q{#          got: 'waffle'},
        '[[reset]]',
        q{#     expected: 'yarblokos'},
        '[[reset]]',
        '[[red]]',
        'Failed 1/2 subtests',
    );

    is_deeply \@output, \@expected,
      '... and failing test output should be correct';

    my @expected_summary = (
        '[[reset]]',
        'Test Summary Report',
        '-------------------',
        '[[red]]',
        "$source_tests/harness_failure (Wstat: 0 Tests: 2 Failed: 1)",
        '[[reset]]',
        '[[red]]',
        'Failed test:',
        '[[reset]]',
        '[[red]]',
        '2',
        '[[reset]]',
    );

    is_deeply \@summary, \@expected_summary,
      '... and the failure summary should also be correct';

    # quiet tests with failures

    @output = ();
    _runtests( $harness_whisper, "$source_tests/harness_failure" );

    $status   = pop @output;
    $summary  = pop @output;
    @expected = (
        "$source_tests/harness_failure ..",
        'Failed 1/2 subtests',
        'Test Summary Report',
        '-------------------',
        "$source_tests/harness_failure (Wstat: 0 Tests: 2 Failed: 1)",
        'Failed test:',
        '2',
    );

    like $status, qr{^Result: FAIL$},
      '... and the status line should be correct';

    is_deeply \@output, \@expected,
      '... and failing test output should be correct';

    # really quiet tests with failures

    @output = ();
    _runtests( $harness_mute, "$source_tests/harness_failure" );

    $status   = pop @output;
    $summary  = pop @output;
    @expected = (
        'Test Summary Report',
        '-------------------',
        "$source_tests/harness_failure (Wstat: 0 Tests: 2 Failed: 1)",
        'Failed test:',
        '2',
    );

    like $status, qr{^Result: FAIL$},
      '... and the status line should be correct';

    is_deeply \@output, \@expected,
      '... and failing test output should be correct';

    # only show directives

    @output = ();
    _runtests(
        $harness_directives,
        "$source_tests/harness_directives"
    );

    chomp(@output);

    @expected = (
        "$source_tests/harness_directives ..",
        'not ok 2 - we have a something # TODO some output',
        "ok 3 houston, we don't have liftoff # SKIP no funding",
        'ok',
        'All tests successful.',

        # ~TODO {{{ this should be an option
        #'Test Summary Report',
        #'-------------------',
        #"$source_tests/harness_directives (Wstat: 0 Tests: 3 Failed: 0)",
        #'Tests skipped:',
        #'3',
        # }}}
    );

    $status           = pop @output;
    $summary          = pop @output;
    $expected_summary = qr/^Files=1, Tests=3, +\d+ wallclock secs/;

    is_deeply \@output, \@expected, '... and the output should be correct';
    like $summary, $expected_summary,
      '... and the report summary should look correct';

    like $status, qr{^Result: PASS$},
      '... and the status line should be correct';

    # normal tests with bad tap

    # install callback handler
    my $parser;
    my $callback_count = 0;

    my @callback_log = ();

    for my $evt (qw(parser_args made_parser before_runtests after_runtests)) {
        $harness->callback(
            $evt => sub {
                push @callback_log, $evt;
            }
        );
    }

    $harness->callback(
        made_parser => sub {
            $parser = shift;
            $callback_count++;
        }
    );

    @output = ();
    _runtests( $harness, "$source_tests/harness_badtap" );
    chomp(@output);

    @output   = map { trim($_) } @output;
    $status   = pop @output;
    @summary  = @output[ 12 .. ( $#output - 1 ) ];
    @output   = @output[ 0 .. 11 ];
    @expected = (
        "$source_tests/harness_badtap ..",
        '1..2',
        '[[reset]]',
        'ok 1 - this is a test',
        '[[reset]]',
        '[[red]]',
        'not ok 2 - this is another test',
        '[[reset]]',
        '1..2',
        '[[reset]]',
        '[[red]]',
        'Failed 1/2 subtests',
    );
    is_deeply \@output, \@expected,
      '... and failing test output should be correct';
    like $status, qr{^Result: FAIL$},
      '... and the status line should be correct';
    @expected_summary = (
        '[[reset]]',
        'Test Summary Report',
        '-------------------',
        '[[red]]',
        "$source_tests/harness_badtap (Wstat: 0 Tests: 2 Failed: 1)",
        '[[reset]]',
        '[[red]]',
        'Failed test:',
        '[[reset]]',
        '[[red]]',
        '2',
        '[[reset]]',
        '[[red]]',
        'Parse errors: More than one plan found in TAP output',
        '[[reset]]',
    );
    is_deeply \@summary, \@expected_summary,
      '... and the badtap summary should also be correct';

    cmp_ok( $callback_count, '==', 1, 'callback called once' );
    is_deeply(
        \@callback_log,
        [ 'before_runtests', 'parser_args', 'made_parser', 'after_runtests' ],
        'callback log matches'
    );
    isa_ok $parser, 'TAP::Parser';

    # coverage testing for _should_show_failures
    # only show failures

    @output = ();
    _runtests( $harness_failures, "$source_tests/harness_failure" );

    chomp(@output);

    @expected = (
        "$source_tests/harness_failure ..",
        'not ok 2 - this is another test',
        'Failed 1/2 subtests',
        'Test Summary Report',
        '-------------------',
        "$source_tests/harness_failure (Wstat: 0 Tests: 2 Failed: 1)",
        'Failed test:',
        '2',
    );

    $status  = pop @output;
    $summary = pop @output;

    like $status, qr{^Result: FAIL$},
      '... and the status line should be correct';
    $expected_summary = qr/^Files=1, Tests=2, +\d+ wallclock secs/;
    is_deeply \@output, \@expected, '... and the output should be correct';

    # check the status output for no tests

    @output = ();
    _runtests( $harness_failures, "$sample_tests/no_output" );

    chomp(@output);

    @expected = (
        "$sample_tests/no_output ..",
        'No subtests run',
        'Test Summary Report',
        '-------------------',
        "$sample_tests/no_output (Wstat: 0 Tests: 0 Failed: 0)",
        'Parse errors: No plan found in TAP output',
    );

    $status  = pop @output;
    $summary = pop @output;

    like $status, qr{^Result: FAIL$},
      '... and the status line should be correct';
    $expected_summary = qr/^Files=1, Tests=2, +\d+ wallclock secs/;
    is_deeply \@output, \@expected, '... and the output should be correct';

    SKIP: {
        skip "Skipping for now because of ASAN failures", 1; # Core-only modification
        skip "No SIGSEGV on $^O", 1 if $^O eq 'MSWin32' or $Config::Config{'sig_name'} !~ m/SEGV/;

        @output = ();
        _runtests( $harness_failures, "$sample_tests/segfault" );

        my $out_str = join q<>, @output;

        like( $out_str, qr<SEGV>, 'SIGSEGV is parsed out' );
    }

    #XXXX
}

# make sure we can exec something ... anything!
SKIP: {

    my $cat = '/bin/cat';

    # TODO: use TYPE on win32?
    unless ( -e $cat ) {
        skip "no '$cat'", 2;
    }

    my $capture = IO::c55Capture->new_handle;
    my $harness = TAP::Harness->new(
        {   verbosity => -2,
            stdout    => $capture,
            exec      => [$cat],
        }
    );

    eval { _runtests( $harness, 't/data/catme.1' ); };

    my @output = tied($$capture)->dump;
    my $status = pop @output;
    like $status, qr{^Result: PASS$},
      '... and the status line should be correct';
    pop @output;    # get rid of summary line
    my $answer = pop @output;
    is( $answer, "All tests successful.\n", 'cat meows' );
}

# make sure that we can exec with a code ref.
{
    my $capture = IO::c55Capture->new_handle;
    my $harness = TAP::Harness->new(
        {   verbosity => -2,
            stdout    => $capture,
            exec      => sub {undef},
        }
    );

    _runtests( $harness, "$source_tests/harness" );

    my @output = tied($$capture)->dump;
    my $status = pop @output;
    like $status, qr{^Result: PASS$},
      '... and the status line should be correct';
    pop @output;    # get rid of summary line
    my $answer = pop @output;
    is( $answer, "All tests successful.\n", 'cat meows' );
}

# Exec with a coderef that returns an arrayref
SKIP: {
    my $cat = '/bin/cat';
    unless ( -e $cat ) {
        skip "no '$cat'", 2;
    }

    my $capture = IO::c55Capture->new_handle;
    my $harness = TAP::Harness->new(
        {   verbosity => -2,
            stdout    => $capture,
            exec      => sub {
                return [
                    $cat,
                    't/data/catme.1'
                ];
            },
        }
    );

    _runtests( $harness, "$source_tests/harness" );

    my @output = tied($$capture)->dump;
    my $status = pop @output;
    like $status, qr{^Result: PASS$},
      '... and the status line should be correct';
    pop @output;    # get rid of summary line
    my $answer = pop @output;
    is( $answer, "All tests successful.\n", 'cat meows' );
}

# Exec with a coderef that returns raw TAP
{
    my $capture = IO::c55Capture->new_handle;
    my $harness = TAP::Harness->new(
        {   verbosity => -2,
            stdout    => $capture,
            exec      => sub {
                return "1..1\nok 1 - raw TAP\n";
            },
        }
    );

    _runtests( $harness, "$source_tests/harness" );

    my @output = tied($$capture)->dump;
    my $status = pop @output;
    like $status, qr{^Result: PASS$},
      '... and the status line should be correct';
    pop @output;    # get rid of summary line
    my $answer = pop @output;
    is( $answer, "All tests successful.\n", 'cat meows' );
}

# Exec with a coderef that returns a filehandle
{
    my $capture = IO::c55Capture->new_handle;
    my $harness = TAP::Harness->new(
        {   verbosity => -2,
            stdout    => $capture,
            exec      => sub {
                open my $fh, 't/data/catme.1';
                return $fh;
            },
        }
    );

    _runtests( $harness, "$source_tests/harness" );

    my @output = tied($$capture)->dump;
    my $status = pop @output;
    like $status, qr{^Result: PASS$},
      '... and the status line should be correct';
    pop @output;    # get rid of summary line
    my $answer = pop @output;
    is( $answer, "All tests successful.\n", 'cat meows' );
}

# catches "exec accumulates arguments" issue (r77)
{
    my $capture = IO::c55Capture->new_handle;
    my $harness = TAP::Harness->new(
        {   verbosity => -2,
            stdout    => $capture,
            exec      => [$^X]
        }
    );

    _runtests(
        $harness,
        "$source_tests/harness_complain"
        ,    # will get mad if run with args
        "$source_tests/harness",
    );

    my @output = tied($$capture)->dump;
    my $status = pop @output;
    like $status, qr{^Result: PASS$},
      '... and the status line should be correct';
    pop @output;    # get rid of summary line
    is( $output[-1], "All tests successful.\n",
        'No exec accumulation'
    );
}

# customize default File source
{
    my $capture = IO::c55Capture->new_handle;
    my $harness = TAP::Harness->new(
        {   verbosity => -2,
            stdout    => $capture,
            sources   => {
                File => { extensions => ['.1'] },
            },
        }
    );

    _runtests( $harness, "$source_tests/source.1" );

    my @output = tied($$capture)->dump;
    my $status = pop @output;
    like $status, qr{^Result: PASS$},
      'customized File source has correct status line';
    pop @output;    # get rid of summary line
    my $answer = pop @output;
    is( $answer, "All tests successful.\n", '... all tests passed' );
}

# load a custom source
{
    my $capture = IO::c55Capture->new_handle;
    my $harness = TAP::Harness->new(
        {   verbosity => -2,
            stdout    => $capture,
            sources   => {
                MyFileSourceHandler => { extensions => ['.1'] },
            },
        }
    );

    my $source_test = "$source_tests/source.1";
    eval { _runtests( $harness, "$source_tests/source.1" ); };
    my $e = $@;
    ok( !$e, 'no error on load custom source' ) || diag($e);

    no warnings 'once';
    can_ok( 'MyFileSourceHandler', 'make_iterator' );
    ok( $MyFileSourceHandler::CAN_HANDLE,
        '... MyFileSourceHandler->can_handle was called'
    );
    ok( $MyFileSourceHandler::MAKE_ITER,
        '... MyFileSourceHandler->make_iterator was called'
    );

    my $raw_source = eval { ${ $MyFileSourceHandler::LAST_SOURCE->raw } };
    is( $raw_source, $source_test, '... used the right source' );

    my @output = tied($$capture)->dump;
    my $status = pop(@output) || '';
    like $status, qr{^Result: PASS$}, '... and test has correct status line';
    pop @output;    # get rid of summary line
    my $answer = pop @output;
    is( $answer, "All tests successful.\n", '... all tests passed' );
}

sub trim {
    $_[0] =~ s/^\s+|\s+$//g;
    return $_[0];
}

sub liblist {
    return [ map {"-I$_"} @_ ];
}

sub get_arg_sets {

    # keys are keys to new()
    return {
        lib => {
            in        => 'lib',
            out       => liblist('lib'),
            test_name => '... a single lib switch should be correct'
        },
        verbosity => {
            in        => 1,
            out       => 1,
            test_name => '... and we should be able to set verbosity to 1'
        },

        # verbose => {
        #     in        => 1,
        #     out       => 1,
        #     test_name => '... and we should be able to set verbose to true'
        # },
      },
      { lib => {
            in        => [ 'lib',        't' ],
            out       => liblist( 'lib', 't' ),
            test_name => '... multiple lib dirs should be correct'
        },
        verbosity => {
            in        => 0,
            out       => 0,
            test_name => '... and we should be able to set verbosity to 0'
        },

        # verbose => {
        #     in        => 0,
        #     out       => 0,
        #     test_name => '... and we should be able to set verbose to false'
        # },
      },
      { switches => {
            in        => [ '-T', '-w', '-T' ],
            out       => [ '-T', '-w', '-T' ],
            test_name => '... duplicate switches should remain',
        },
        failures => {
            in  => 1,
            out => 1,
            test_name =>
              '... and we should be able to set failures to true',
        },
        verbosity => {
            in        => -1,
            out       => -1,
            test_name => '... and we should be able to set verbosity to -1'
        },

        # quiet => {
        #     in        => 1,
        #     out       => 1,
        #     test_name => '... and we should be able to set quiet to false'
        # },
      },

      { verbosity => {
            in        => -2,
            out       => -2,
            test_name => '... and we should be able to set verbosity to -2'
        },

        # really_quiet => {
        #     in  => 1,
        #     out => 1,
        #     test_name =>
        #       '... and we should be able to set really_quiet to true',
        # },
        exec => {
            in  => $^X,
            out => $^X,
            test_name =>
              '... and we should be able to set the executable',
        },
      },
      { switches => {
            in  => 'T',
            out => ['T'],
            test_name =>
              '... leading dashes (-) on switches are not optional',
        },
      },
      { switches => {
            in        => '-T',
            out       => ['-T'],
            test_name => '... we should be able to set switches',
        },
        failures => {
            in        => 1,
            out       => 1,
            test_name => '... and we should be able to set failures to true'
        },
      };
}

sub _runtests {
    my ( $harness, @tests ) = @_;
    local $ENV{PERL_TEST_HARNESS_DUMP_TAP} = 0;
    my $aggregate = $harness->runtests(@tests);
    return $aggregate;
}

{

    # coverage tests for ctor

    my $harness = TAP::Harness->new(
        {   timer  => 0,
            errors => 1,
            merge  => 2,

            # formatter => 3,
        }
    );

    is $harness->timer(), 0, 'timer getter';
    is $harness->timer(10), 10, 'timer setter';
    is $harness->errors(), 1, 'errors getter';
    is $harness->errors(10), 10, 'errors setter';
    is $harness->merge(), 2, 'merge getter';
    is $harness->merge(10), 10, 'merge setter';

    # jobs accessor
    is $harness->jobs(), 1, 'jobs';
}

{

# coverage tests for the stdout key of VALIDATON_FOR, used by _initialize() in the ctor

    {

        # ref $ref => false
        my @die;

        eval {
            local $SIG{__DIE__} = sub { push @die, @_ };

            my $harness = TAP::Harness->new(
                {   stdout => bless {}, '0',    # how evil is THAT !!!
                }
            );
        };

        is @die, 1, 'bad filehandle to stdout';
        like pop @die, qr/option 'stdout' needs a filehandle/,
          '... and we died as expected';
    }

    {

        # ref => ! GLOB and ref->can(print)

        package Printable;

        sub new { return bless {}, shift }

        sub print {return}

        package main;

        my $harness = TAP::Harness->new(
            {   stdout => Printable->new(),
            }
        );

        isa_ok $harness, 'TAP::Harness';
    }

    {

        # ref $ref => GLOB

        my $harness = TAP::Harness->new(
            {   stdout => bless {}, 'GLOB',    # again with the evil
            }
        );

        isa_ok $harness, 'TAP::Harness';
    }

    {

        # bare glob

        my $harness = TAP::Harness->new( { stdout => *STDOUT } );

        isa_ok $harness, 'TAP::Harness';
    }

    {

        # string filehandle

        my $string = '';
        open my $fh, ">", \$string or die $!;
        my $harness = TAP::Harness->new( { stdout => $fh } );

        isa_ok $harness, 'TAP::Harness';
    }

    {

        # lexical filehandle reference

        my $string = '';
        open my $fh, ">", \$string or die $!;
        ok !eval { TAP::Harness->new( { stdout => \$fh } ); };
        like $@, qr/^option 'stdout' needs a filehandle /;
    }
}

{

    # coverage testing of lib/switches accessor
    my $harness = TAP::Harness->new;

    my @die;

    eval {
        local $SIG{__DIE__} = sub { push @die, @_ };

        $harness->switches(qw( too many arguments));
    };

    is @die, 1, 'too many arguments to accessor';

    like pop @die, qr/Too many arguments to method 'switches'/,
      '...and we died as expected';

    $harness->switches('simple scalar');

    my $arrref = $harness->switches;
    is_deeply $arrref, ['simple scalar'], 'scalar wrapped in arr ref';
}

{

    # coverage tests for the basically untested T::H::_open_spool

    my @spool = ( 't', 'spool' );
    $ENV{PERL_TEST_HARNESS_DUMP_TAP} = File::Spec->catfile(@spool);

# now given that we're going to be writing stuff to the file system, make sure we have
# a cleanup hook

    END {
        use File::Path;

        # remove the tree if we made it this far
        rmtree( $ENV{PERL_TEST_HARNESS_DUMP_TAP} )
          if $ENV{PERL_TEST_HARNESS_DUMP_TAP};
    }

    my $harness = TAP::Harness->new( { verbosity => -2 } );

    can_ok $harness, 'runtests';

    # normal tests in verbose mode

    my $parser
      = $harness->runtests( File::Spec->catfile( $source_tests, 'harness' ) );

    isa_ok $parser, 'TAP::Parser::Aggregator',
      '... runtests returns the aggregate';

    ok -e File::Spec->catfile(
        $ENV{PERL_TEST_HARNESS_DUMP_TAP},
        $source_tests, 'harness'
    );
}

{

    # test name munging
    my @cases = (
        {   name   => 'all the same',
            input  => [ 'foo.t', 'bar.t', 'fletz.t' ],
            output => [
                [ 'foo.t', 'foo.t' ], [ 'bar.t', 'bar.t' ],
                [ 'fletz.t', 'fletz.t' ]
            ],
        },
        {   name   => 'all the same, already cooked',
            input  => [ 'foo.t', [ 'bar.t', 'brip' ], 'fletz.t' ],
            output => [
                [ 'foo.t', 'foo.t' ], [ 'bar.t', 'brip' ],
                [ 'fletz.t', 'fletz.t' ]
            ],
        },
        {   name   => 'different exts',
            input  => [ 'foo.t', 'bar.u', 'fletz.v' ],
            output => [
                [ 'foo.t', 'foo.t' ], [ 'bar.u', 'bar.u' ],
                [ 'fletz.v', 'fletz.v' ]
            ],
        },
        {   name   => 'different exts, one already cooked',
            input  => [ 'foo.t', [ 'bar.u', 'bam' ], 'fletz.v' ],
            output => [
                [ 'foo.t', 'foo.t' ], [ 'bar.u', 'bam' ],
                [ 'fletz.v', 'fletz.v' ]
            ],
        },
        {   name   => 'different exts, two already cooked',
            input  => [ 'foo.t', [ 'bar.u', 'bam.q' ], [ 'fletz.v', 'boo' ] ],
            output => [
                [ 'foo.t', 'foo.t' ], [ 'bar.u', 'bam.q' ],
                [ 'fletz.v', 'boo' ]
            ],
        },
    );

    for my $case (@cases) {
        is_deeply [ TAP::Harness->_add_descriptions( @{ $case->{input} } ) ],
          $case->{output}, '_add_descriptions: ' . $case->{name};
    }
}
