#!/usr/bin/perl -w

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;

use Test::More;
use File::Spec;

use App::Prove;
use Getopt::Long;

use Text::ParseWords qw(shellwords);

package FakeProve;

use base qw( App::Prove );

sub new {
    my $class = shift;
    my $self  = $class->SUPER::new(@_);
    $self->{_log} = [];
    return $self;
}

sub _color_default {0}

sub _runtests {
    my $self = shift;
    push @{ $self->{_log} }, [ '_runtests', @_ ];
}

sub get_log {
    my $self = shift;
    my @log  = @{ $self->{_log} };
    $self->{_log} = [];
    return @log;
}

sub _shuffle {
    my $self = shift;
    s/^/xxx/ for @_;
}

package main;

sub mabs {
    my $ar = shift;
    return [ map { File::Spec->rel2abs($_) } @$ar ];
}

{
    my @import_log = ();
    sub test_log_import { push @import_log, [@_] }

    sub get_import_log {
        my @log = @import_log;
        @import_log = ();
        return @log;
    }

    my @plugin_load_log = ();
    sub test_log_plugin_load { push @plugin_load_log, [@_] }

    sub get_plugin_load_log {
        my @log = @plugin_load_log;
        @plugin_load_log = ();
        return @log;
    }
}

my ( @ATTR, %DEFAULT_ASSERTION, @SCHEDULE, $HAS_YAML );

# see the "ACTUAL TEST" section at the bottom

BEGIN {    # START PLAN
    $HAS_YAML = 0;
    eval { require YAML; $HAS_YAML = 1; };

    # list of attributes
    @ATTR = qw(
      archive argv blib color directives exec extensions failures
      formatter harness includes lib merge parse quiet really_quiet
      recurse backwards shuffle taint_fail taint_warn verbose
      warnings_fail warnings_warn
    );

    # what we expect if the 'expect' hash does not define it
    %DEFAULT_ASSERTION = map { $_ => undef } @ATTR;

    $DEFAULT_ASSERTION{includes} = $DEFAULT_ASSERTION{argv}
      = sub { 'ARRAY' eq ref shift };

    my @dummy_tests = map { File::Spec->catdir( 't', 'sample-tests', $_ ) }
      qw(simple simple_yaml);
    my $dummy_test = $dummy_tests[0];

    ########################################################################
 # declarations - this drives all of the subtests.
 # The cheatsheet follows.
 # required: name, expect
 # optional:
 #   args       - arguments to constructor
 #   switches   - command-line switches
 #   runlog     - expected results of internal calls to _runtests, must
 #                match FakeProve's _log attr
 #   run_error  - depends on 'runlog' (if missing, asserts no error)
 #   extra      - follow-up check to handle exceptional cleanup / verification
 #   class      - The App::Prove subclass to test. Defaults to FakeProve
    @SCHEDULE = (
        {   name   => 'Create empty',
            expect => {}
        },
        {   name => 'Set all options via constructor',
            args => {
                archive       => 1,
                argv          => [qw(one two three)],
                blib          => 2,
                color         => 3,
                directives    => 4,
                exec          => 5,
                failures      => 7,
                formatter     => 8,
                harness       => 9,
                includes      => [qw(four five six)],
                lib           => 10,
                merge         => 11,
                parse         => 13,
                quiet         => 14,
                really_quiet  => 15,
                recurse       => 16,
                backwards     => 17,
                shuffle       => 18,
                taint_fail    => 19,
                taint_warn    => 20,
                verbose       => 21,
                warnings_fail => 22,
                warnings_warn => 23,
            },
            expect => {
                archive       => 1,
                argv          => [qw(one two three)],
                blib          => 2,
                color         => 3,
                directives    => 4,
                exec          => 5,
                failures      => 7,
                formatter     => 8,
                harness       => 9,
                includes      => [qw(four five six)],
                lib           => 10,
                merge         => 11,
                parse         => 13,
                quiet         => 14,
                really_quiet  => 15,
                recurse       => 16,
                backwards     => 17,
                shuffle       => 18,
                taint_fail    => 19,
                taint_warn    => 20,
                verbose       => 21,
                warnings_fail => 22,
                warnings_warn => 23,
            }
        },
        {   name   => 'Call with defaults',
            args   => { argv => [qw( one two three )] },
            expect => {},
            runlog => [
                [   '_runtests',
                    {   show_count => 1,
                    },
                    'one', 'two', 'three'
                ]
            ],
        },

        # Test all options individually

        # {   name => 'Just archive',
        #     args => {
        #         argv    => [qw( one two three )],
        #         archive => 1,
        #     },
        #     expect => {
        #         archive => 1,
        #     },
        #     runlog => [
        #         [   {   archive => 1,
        #             },
        #             'one', 'two',
        #             'three'
        #         ]
        #     ],
        # },
        {   name => 'Just argv',
            args => {
                argv => [qw( one two three )],
            },
            expect => {
                argv => [qw( one two three )],
            },
            runlog => [
                [   '_runtests',
                    { show_count => 1 },
                    'one', 'two',
                    'three'
                ]
            ],
        },
        {   name => 'Just blib',
            args => {
                argv => [qw( one two three )],
                blib => 1,
            },
            expect => {
                blib => 1,
            },
            runlog => [
                [   '_runtests',
                    {   lib => mabs( [ 'blib/lib', 'blib/arch' ] ),
                        show_count => 1,
                    },
                    'one', 'two', 'three'
                ]
            ],
        },

        {   name => 'Just color',
            args => {
                argv  => [qw( one two three )],
                color => 1,
            },
            expect => {
                color => 1,
            },
            runlog => [
                [   '_runtests',
                    {   color      => 1,
                        show_count => 1,
                    },
                    'one', 'two', 'three'
                ]
            ],
        },

        {   name => 'Just directives',
            args => {
                argv       => [qw( one two three )],
                directives => 1,
            },
            expect => {
                directives => 1,
            },
            runlog => [
                [   '_runtests',
                    {   directives => 1,
                        show_count => 1,
                    },
                    'one', 'two', 'three'
                ]
            ],
        },
        {   name => 'Just exec',
            args => {
                argv => [qw( one two three )],
                exec => 1,
            },
            expect => {
                exec => 1,
            },
            runlog => [
                [   '_runtests',
                    {   exec       => [1],
                        show_count => 1,
                    },
                    'one', 'two', 'three'
                ]
            ],
        },
        {   name => 'Just failures',
            args => {
                argv     => [qw( one two three )],
                failures => 1,
            },
            expect => {
                failures => 1,
            },
            runlog => [
                [   '_runtests',
                    {   failures   => 1,
                        show_count => 1,
                    },
                    'one', 'two', 'three'
                ]
            ],
        },

        {   name => 'Just formatter',
            args => {
                argv      => [qw( one two three )],
                formatter => 'TAP::Harness',
            },
            expect => {
                formatter => 'TAP::Harness',
            },
            runlog => [
                [   '_runtests',
                    {   formatter_class => 'TAP::Harness',
                        show_count      => 1,
                    },
                    'one', 'two', 'three'
                ]
            ],
        },

        {   name => 'Just includes',
            args => {
                argv     => [qw( one two three )],
                includes => [qw( four five six )],
            },
            expect => {
                includes => [qw( four five six )],
            },
            runlog => [
                [   '_runtests',
                    {   lib => mabs( [qw( four five six )] ),
                        show_count => 1,
                    },
                    'one', 'two', 'three'
                ]
            ],
        },
        {   name => 'Just lib',
            args => {
                argv => [qw( one two three )],
                lib  => 1,
            },
            expect => {
                lib => 1,
            },
            runlog => [
                [   '_runtests',
                    {   lib => mabs( ['lib'] ),
                        show_count => 1,
                    },
                    'one', 'two', 'three'
                ]
            ],
        },
        {   name => 'Just merge',
            args => {
                argv  => [qw( one two three )],
                merge => 1,
            },
            expect => {
                merge => 1,
            },
            runlog => [
                [   '_runtests',
                    {   merge      => 1,
                        show_count => 1,
                    },
                    'one', 'two', 'three'
                ]
            ],
        },
        {   name => 'Just parse',
            args => {
                argv  => [qw( one two three )],
                parse => 1,
            },
            expect => {
                parse => 1,
            },
            runlog => [
                [   '_runtests',
                    {   errors     => 1,
                        show_count => 1,
                    },
                    'one', 'two', 'three'
                ]
            ],
        },
        {   name => 'Just quiet',
            args => {
                argv  => [qw( one two three )],
                quiet => 1,
            },
            expect => {
                quiet => 1,
            },
            runlog => [
                [   '_runtests',
                    {   verbosity  => -1,
                        show_count => 1,
                    },
                    'one', 'two', 'three'
                ]
            ],
        },
        {   name => 'Just really_quiet',
            args => {
                argv         => [qw( one two three )],
                really_quiet => 1,
            },
            expect => {
                really_quiet => 1,
            },
            runlog => [
                [   '_runtests',
                    {   verbosity  => -2,
                        show_count => 1,
                    },
                    'one', 'two', 'three'
                ]
            ],
        },
        {   name => 'Just recurse',
            args => {
                argv    => [qw( one two three )],
                recurse => 1,
            },
            expect => {
                recurse => 1,
            },
            runlog => [
                [   '_runtests',
                    {   show_count => 1,
                    },
                    'one', 'two', 'three'
                ]
            ],
        },
        {   name => 'Just reverse',
            args => {
                argv      => [qw( one two three )],
                backwards => 1,
            },
            expect => {
                backwards => 1,
            },
            runlog => [
                [   '_runtests',
                    {   show_count => 1,
                    },
                    'three', 'two', 'one'
                ]
            ],
        },

        {   name => 'Just shuffle',
            args => {
                argv    => [qw( one two three )],
                shuffle => 1,
            },
            expect => {
                shuffle => 1,
            },
            runlog => [
                [   '_runtests',
                    {   show_count => 1,
                    },
                    'xxxone', 'xxxtwo',
                    'xxxthree'
                ]
            ],
        },
        {   name => 'Just taint_fail',
            args => {
                argv       => [qw( one two three )],
                taint_fail => 1,
            },
            expect => {
                taint_fail => 1,
            },
            runlog => [
                [   '_runtests',
                    {   switches   => ['-T'],
                        show_count => 1,
                    },
                    'one', 'two', 'three'
                ]
            ],
        },
        {   name => 'Just taint_warn',
            args => {
                argv       => [qw( one two three )],
                taint_warn => 1,
            },
            expect => {
                taint_warn => 1,
            },
            runlog => [
                [   '_runtests',
                    {   switches   => ['-t'],
                        show_count => 1,
                    },
                    'one', 'two', 'three'
                ]
            ],
        },
        {   name => 'Just verbose',
            args => {
                argv    => [qw( one two three )],
                verbose => 1,
            },
            expect => {
                verbose => 1,
            },
            runlog => [
                [   '_runtests',
                    {   verbosity  => 1,
                        show_count => 1,
                    },
                    'one', 'two', 'three'
                ]
            ],
        },
        {   name => 'Just warnings_fail',
            args => {
                argv          => [qw( one two three )],
                warnings_fail => 1,
            },
            expect => {
                warnings_fail => 1,
            },
            runlog => [
                [   '_runtests',
                    {   switches   => ['-W'],
                        show_count => 1,
                    },
                    'one', 'two', 'three'
                ]
            ],
        },
        {   name => 'Just warnings_warn',
            args => {
                argv          => [qw( one two three )],
                warnings_warn => 1,
            },
            expect => {
                warnings_warn => 1,
            },
            runlog => [
                [   '_runtests',
                    {   switches   => ['-w'],
                        show_count => 1,
                    },
                    'one', 'two', 'three'
                ]
            ],
        },

        # Command line parsing
        {   name => 'Switch -v',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '-v', $dummy_test ],
            expect   => {
                verbose => 1,
            },
            runlog => [
                [   '_runtests',
                    {   verbosity  => 1,
                        show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        {   name => 'Switch --verbose',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '--verbose', $dummy_test ],
            expect   => {
                verbose => 1,
            },
            runlog => [
                [   '_runtests',
                    {   verbosity  => 1,
                        show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        {   name => 'Switch -f',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '-f', $dummy_test ],
            expect => { failures => 1 },
            runlog => [
                [   '_runtests',
                    {   failures   => 1,
                        show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        {   name => 'Switch --failures',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '--failures', $dummy_test ],
            expect => { failures => 1 },
            runlog => [
                [   '_runtests',
                    {   failures   => 1,
                        show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        {   name => 'Switch -l',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '-l', $dummy_test ],
            expect => { lib => 1 },
            runlog => [
                [   '_runtests',
                    {   lib => mabs( ['lib'] ),
                        show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        {   name => 'Switch --lib',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '--lib', $dummy_test ],
            expect => { lib => 1 },
            runlog => [
                [   '_runtests',
                    {   lib => mabs( ['lib'] ),
                        show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        {   name => 'Switch -b',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '-b', $dummy_test ],
            expect => { blib => 1 },
            runlog => [
                [   '_runtests',
                    {   lib => mabs( [ 'blib/lib', 'blib/arch' ] ),
                        show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        {   name => 'Switch --blib',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '--blib', $dummy_test ],
            expect => { blib => 1 },
            runlog => [
                [   '_runtests',
                    {   lib => mabs( [ 'blib/lib', 'blib/arch' ] ),
                        show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        {   name => 'Switch -s',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '-s', $dummy_test ],
            expect => { shuffle => 1 },
            runlog => [
                [   '_runtests',
                    {   show_count => 1,
                    },
                    "xxx$dummy_test"
                ]
            ],
        },

        {   name => 'Switch --shuffle',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '--shuffle', $dummy_test ],
            expect => { shuffle => 1 },
            runlog => [
                [   '_runtests',
                    {   show_count => 1,
                    },
                    "xxx$dummy_test"
                ]
            ],
        },

        {   name => 'Switch -c',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '-c', $dummy_test ],
            expect => { color => 1 },
            runlog => [
                [   '_runtests',
                    {   color      => 1,
                        show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        {   name => 'Switch -r',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '-r', $dummy_test ],
            expect => { recurse => 1 },
            runlog => [
                [   '_runtests',
                    {   show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        {   name => 'Switch --recurse',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '--recurse', $dummy_test ],
            expect => { recurse => 1 },
            runlog => [
                [   '_runtests',
                    {   show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        {   name => 'Switch --reverse',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '--reverse', @dummy_tests ],
            expect => { backwards => 1 },
            runlog => [
                [   '_runtests',
                    {   show_count => 1,
                    },
                    reverse @dummy_tests
                ]
            ],
        },

        {   name => 'Switch -p',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '-p', $dummy_test ],
            expect   => {
                parse => 1,
            },
            runlog => [
                [   '_runtests',
                    {   errors     => 1,
                        show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        {   name => 'Switch --parse',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '--parse', $dummy_test ],
            expect   => {
                parse => 1,
            },
            runlog => [
                [   '_runtests',
                    {   errors     => 1,
                        show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        {   name => 'Switch -q',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '-q', $dummy_test ],
            expect => { quiet => 1 },
            runlog => [
                [   '_runtests',
                    {   verbosity  => -1,
                        show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        {   name => 'Switch --quiet',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '--quiet', $dummy_test ],
            expect => { quiet => 1 },
            runlog => [
                [   '_runtests',
                    {   verbosity  => -1,
                        show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        {   name => 'Switch -Q',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '-Q', $dummy_test ],
            expect => { really_quiet => 1 },
            runlog => [
                [   '_runtests',
                    {   verbosity  => -2,
                        show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        {   name => 'Switch --QUIET',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '--QUIET', $dummy_test ],
            expect => { really_quiet => 1 },
            runlog => [
                [   '_runtests',
                    {   verbosity  => -2,
                        show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        {   name => 'Switch -m',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '-m', $dummy_test ],
            expect => { merge => 1 },
            runlog => [
                [   '_runtests',
                    {   merge      => 1,
                        show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        {   name => 'Switch --merge',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '--merge', $dummy_test ],
            expect => { merge => 1 },
            runlog => [
                [   '_runtests',
                    {   merge      => 1,
                        show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        {   name => 'Switch --directives',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '--directives', $dummy_test ],
            expect => { directives => 1 },
            runlog => [
                [   '_runtests',
                    {   directives => 1,
                        show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        # .proverc
        {   name => 'Empty exec in .proverc',
            args => {
                argv => [qw( one two three )],
            },
            proverc  => 't/proverc/emptyexec',
            switches => [$dummy_test],
            expect   => { exec => '' },
            runlog   => [
                [   '_runtests',
                    {   exec       => [],
                        show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        # Executing one word (why would it be a -s though?)
        {   name => 'Switch --exec -s',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '--exec', '-s', $dummy_test ],
            expect => { exec => '-s' },
            runlog => [
                [   '_runtests',
                    {   exec       => ['-s'],
                        show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        # multi-part exec
        {   name => 'Switch --exec "/foo/bar/perl -Ilib"',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '--exec', '/foo/bar/perl -Ilib', $dummy_test ],
            expect => { exec => '/foo/bar/perl -Ilib' },
            runlog => [
                [   '_runtests',
                    {   exec       => [qw(/foo/bar/perl -Ilib)],
                        show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        # null exec (run tests as compiled binaries)
        {   name     => 'Switch --exec ""',
            switches => [ '--exec', '', $dummy_test ],
            expect   => {
                exec =>   # ick, must workaround the || default bit with a sub
                  sub { my $val = shift; defined($val) and !length($val) }
            },
            runlog => [
                [   '_runtests',
                    {   exec       => [],
                        show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        # Specify an oddball extension
        {   name     => 'Switch --ext=.wango',
            switches => ['--ext=.wango'],
            expect   => { extensions => ['.wango'] },
            runlog   => [
                [   '_runtests',
                    {   show_count => 1,
                    },
                ]
            ],
        },

        # Handle multiple extensions
        {   name     => 'Switch --ext=.foo --ext=.bar',
            switches => [ '--ext=.foo', '--ext=.bar', ],
            expect   => { extensions => [ '.foo', '.bar' ] },
            runlog   => [
                [   '_runtests',
                    {   show_count => 1,
                    },
                ]
            ],
        },

        # Source handlers
        {   name     => 'Switch --source simple',
            args     => { argv => [qw( one two three )] },
            switches => [ '--source', 'MyCustom', $dummy_test ],
            expect   => {
                sources => {
                    MyCustom => {},
                },
            },
            runlog => [
                [   '_runtests',
                    {   sources => {
                            MyCustom => {},
                        },
                        show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        {   name => 'Switch --sources with config',
            args => { argv => [qw( one two three )] },
            skip => $Getopt::Long::VERSION >= 2.28 && $HAS_YAML ? 0 : 1,
            skip_reason => "YAML not available or Getopt::Long too old",
            switches    => [
                '--source',      'Perl',
                '--perl-option', 'foo=bar baz',
                '--perl-option', 'avg=0.278',
                '--source',      'MyCustom',
                '--source',      'File',
                '--file-option', 'extensions=.txt',
                '--file-option', 'extensions=.tmp',
                '--file-option', 'hash=this=that',
                '--file-option', 'hash=foo=bar',
                '--file-option', 'sep=foo\\=bar',
                $dummy_test
            ],
            expect => {
                sources => {
                    Perl     => { foo => 'bar baz', avg => 0.278 },
                    MyCustom => {},
                    File     => {
                        extensions => [ '.txt', '.tmp' ],
                        hash => { this => 'that', foo => 'bar' },
                        sep  => 'foo=bar',
                    },
                },
            },
            runlog => [
                [   '_runtests',
                    {   sources => {
                            Perl     => { foo => 'bar baz', avg => 0.278 },
                            MyCustom => {},
                            File     => {
                                extensions => [ '.txt', '.tmp' ],
                                hash => { this => 'that', foo => 'bar' },
                                sep  => 'foo=bar',
                            },
                        },
                        show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        # Plugins
        {   name     => 'Load plugin',
            switches => [ '-P', 'Dummy', $dummy_test ],
            args     => {
                argv => [qw( one two three )],
            },
            expect => {
                plugins => ['Dummy'],
            },
            extra => sub {
                my @loaded = get_import_log();
                is_deeply \@loaded, [ ['App::Prove::Plugin::Dummy'] ],
                  "Plugin loaded OK";
            },
            plan   => 1,
            runlog => [
                [   '_runtests',
                    {   show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        {   name     => 'Load plugin (args)',
            switches => [ '-P', 'Dummy=cracking,cheese,gromit', $dummy_test ],
            args     => {
                argv => [qw( one two three )],
            },
            expect => {
                plugins => ['Dummy'],
            },
            extra => sub {
                my @loaded = get_import_log();
                is_deeply \@loaded,
                  [ [   'App::Prove::Plugin::Dummy', 'cracking', 'cheese',
                        'gromit'
                    ]
                  ],
                  "Plugin loaded OK";
            },
            plan   => 1,
            runlog => [
                [   '_runtests',
                    {   show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        {   name     => 'Load plugin (explicit path)',
            switches => [ '-P', 'App::Prove::Plugin::Dummy', $dummy_test ],
            args     => {
                argv => [qw( one two three )],
            },
            expect => {
                plugins => ['Dummy'],
            },
            extra => sub {
                my @loaded = get_import_log();
                is_deeply \@loaded, [ ['App::Prove::Plugin::Dummy'] ],
                  "Plugin loaded OK";
            },
            plan   => 1,
            runlog => [
                [   '_runtests',
                    {   show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        {   name     => 'Load plugin (args + call load method)',
            switches => [ '-P', 'Dummy2=fou,du,fafa', $dummy_test ],
            args     => {
                argv => [qw( one two three )],
            },
            expect => {
                plugins => ['Dummy2'],
            },
            extra => sub {
                my @import = get_import_log();
                is_deeply \@import,
                  [ [ 'App::Prove::Plugin::Dummy2', 'fou', 'du', 'fafa' ] ],
                  "Plugin loaded OK";

                my @loaded = get_plugin_load_log();
                is( scalar @loaded, 1, 'Plugin->load called OK' );
                my ( $plugin_class, $args ) = @{ shift @loaded };
                is( $plugin_class, 'App::Prove::Plugin::Dummy2',
                    'plugin_class passed'
                );
                isa_ok(
                    $args->{app_prove}, 'App::Prove',
                    'app_prove object passed'
                );
                is_deeply(
                    $args->{args}, [qw( fou du fafa )],
                    'expected args passed'
                );
            },
            plan   => 5,
            runlog => [
                [   '_runtests',
                    {   show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        {   name     => 'Load module',
            switches => [ '-M', 'App::Prove::Plugin::Dummy', $dummy_test ],
            args     => {
                argv => [qw( one two three )],
            },
            expect => {
                plugins => ['Dummy'],
            },
            extra => sub {
                my @loaded = get_import_log();
                is_deeply \@loaded, [ ['App::Prove::Plugin::Dummy'] ],
                  "Plugin loaded OK";
            },
            plan   => 1,
            runlog => [
                [   '_runtests',
                    {   show_count => 1,
                    },
                    $dummy_test
                ]
            ],
        },

        # TODO
        # Hmm, that doesn't work...
        # {   name => 'Switch -h',
        #     args => {
        #         argv => [qw( one two three )],
        #     },
        #     switches => [ '-h', $dummy_test ],
        #     expect   => {},
        #     runlog   => [
        #         [   '_runtests',
        #             {},
        #             $dummy_test
        #         ]
        #     ],
        # },

        # {   name => 'Switch --help',
        #     args => {
        #         argv => [qw( one two three )],
        #     },
        #     switches => [ '--help', $dummy_test ],
        #     expect   => {},
        #     runlog   => [
        #         [   {},
        #             $dummy_test
        #         ]
        #     ],
        # },
        # {   name => 'Switch -?',
        #     args => {
        #         argv => [qw( one two three )],
        #     },
        #     switches => [ '-?', $dummy_test ],
        #     expect   => {},
        #     runlog   => [
        #         [   {},
        #             $dummy_test
        #         ]
        #     ],
        # },
        #
        # {   name => 'Switch -H',
        #     args => {
        #         argv => [qw( one two three )],
        #     },
        #     switches => [ '-H', $dummy_test ],
        #     expect   => {},
        #     runlog   => [
        #         [   {},
        #             $dummy_test
        #         ]
        #     ],
        # },
        #
        # {   name => 'Switch --man',
        #     args => {
        #         argv => [qw( one two three )],
        #     },
        #     switches => [ '--man', $dummy_test ],
        #     expect   => {},
        #     runlog   => [
        #         [   {},
        #             $dummy_test
        #         ]
        #     ],
        # },
        #
        # {   name => 'Switch -V',
        #     args => {
        #         argv => [qw( one two three )],
        #     },
        #     switches => [ '-V', $dummy_test ],
        #     expect   => {},
        #     runlog   => [
        #         [   {},
        #             $dummy_test
        #         ]
        #     ],
        # },
        #
        # {   name => 'Switch --version',
        #     args => {
        #         argv => [qw( one two three )],
        #     },
        #     switches => [ '--version', $dummy_test ],
        #     expect   => {},
        #     runlog   => [
        #         [   {},
        #             $dummy_test
        #         ]
        #     ],
        # },
        #
        # {   name => 'Switch --color!',
        #     args => {
        #         argv => [qw( one two three )],
        #     },
        #     switches => [ '--color!', $dummy_test ],
        #     expect   => {},
        #     runlog   => [
        #         [   {},
        #             $dummy_test
        #         ]
        #     ],
        # },
        #
        {   name => 'Switch -I=s@',
            args => {
                argv => [qw( one two three )],
            },
            switches => [ '-Ilib', $dummy_test ],
            expect   => {
                includes => sub {
                    my ( $val, $attr ) = @_;
                    return
                         'ARRAY' eq ref $val
                      && 1 == @$val
                      && $val->[0] =~ /lib$/;
                },
            },
        },

        # {   name => 'Switch -a',
        #     args => {
        #         argv => [qw( one two three )],
        #     },
        #     switches => [ '-a', $dummy_test ],
        #     expect   => {},
        #     runlog   => [
        #         [   {},
        #             $dummy_test
        #         ]
        #     ],
        # },
        #
        # {   name => 'Switch --archive=-s',
        #     args => {
        #         argv => [qw( one two three )],
        #     },
        #     switches => [ '--archive=-s', $dummy_test ],
        #     expect   => {},
        #     runlog   => [
        #         [   {},
        #             $dummy_test
        #         ]
        #     ],
        # },
        #
        # {   name => 'Switch --formatter=-s',
        #     args => {
        #         argv => [qw( one two three )],
        #     },
        #     switches => [ '--formatter=-s', $dummy_test ],
        #     expect   => {},
        #     runlog   => [
        #         [   {},
        #             $dummy_test
        #         ]
        #     ],
        # },
        #
        # {   name => 'Switch -e',
        #     args => {
        #         argv => [qw( one two three )],
        #     },
        #     switches => [ '-e', $dummy_test ],
        #     expect   => {},
        #     runlog   => [
        #         [   {},
        #             $dummy_test
        #         ]
        #     ],
        # },
        #
        # {   name => 'Switch --harness=-s',
        #     args => {
        #         argv => [qw( one two three )],
        #     },
        #     switches => [ '--harness=-s', $dummy_test ],
        #     expect   => {},
        #     runlog   => [
        #         [   {},
        #             $dummy_test
        #         ]
        #     ],
        # },

    );

    # END SCHEDULE
    ########################################################################

    my $extra_plan = 0;
    for my $test (@SCHEDULE) {
        my $plan = 0;
        $plan += $test->{plan} || 0;
        $plan += 2 if $test->{runlog};
        $plan += 1 if $test->{switches};
        $test->{_planned} = $plan + 3 + @ATTR;
        $extra_plan += $plan;
    }

    plan tests => @SCHEDULE * ( 3 + @ATTR ) + $extra_plan;
}    # END PLAN

# ACTUAL TEST
for my $test (@SCHEDULE) {
    my $name = $test->{name};
    my $class = $test->{class} || 'FakeProve';

    SKIP:
    {
        skip $test->{skip_reason}, $test->{_planned} if $test->{skip};

        local $ENV{HARNESS_TIMER};

        ok my $app = $class->new( exists $test->{args} ? $test->{args} : () ),
          "$name: App::Prove created OK";

        isa_ok $app, 'App::Prove';
        isa_ok $app, $class;

        # Optionally parse command args
        if ( my $switches = $test->{switches} ) {
            if ( my $proverc = $test->{proverc} ) {
                $app->add_rc_file(
                    File::Spec->catfile( split /\//, $proverc ) );
            }
            eval { $app->process_args( '--norc', @$switches ) };
            if ( my $err_pattern = $test->{parse_error} ) {
                like $@, $err_pattern, "$name: expected parse error";
            }
            else {
                ok !$@, "$name: no parse error";
            }
        }

        my $expect = $test->{expect} || {};
        for my $attr ( sort @ATTR ) {
            my $val = $app->$attr();
            my $assertion
              = exists $expect->{$attr}
              ? $expect->{$attr}
              : $DEFAULT_ASSERTION{$attr};
            my $is_ok = undef;

            if ( 'CODE' eq ref $assertion ) {
                $is_ok = ok $assertion->( $val, $attr ),
                  "$name: $attr has the expected value";
            }
            elsif ( 'Regexp' eq ref $assertion ) {
                $is_ok = like $val, $assertion,
                  "$name: $attr matches $assertion";
            }
            else {
                $is_ok = is_deeply $val, $assertion,
                  "$name: $attr has the expected value";
            }

            unless ($is_ok) {
                diag "got $val for $attr";
            }
        }

        if ( my $runlog = $test->{runlog} ) {
            eval { $app->run };
            if ( my $err_pattern = $test->{run_error} ) {
                like $@, $err_pattern, "$name: expected error OK";
                pass;
                pass for 1 .. $test->{plan};
            }
            else {
                unless ( ok !$@, "$name: no error OK" ) {
                    diag "$name: error: $@\n";
                }

                my $gotlog = [ $app->get_log ];

                if ( my $extra = $test->{extra} ) {
                    $extra->($gotlog);
                }

                # adapt our expectations if HARNESS_PERL_SWITCHES is set
                push @{ $runlog->[0][1]{switches} },
                  shellwords( $ENV{HARNESS_PERL_SWITCHES} )
                  if $ENV{HARNESS_PERL_SWITCHES};

                unless (
                    is_deeply $gotlog, $runlog,
                    "$name: run results match"
                  )
                {
                    use Data::Dumper;
                    diag Dumper( { wanted => $runlog, got => $gotlog } );
                }
            }
        }

    }    # SKIP
}

