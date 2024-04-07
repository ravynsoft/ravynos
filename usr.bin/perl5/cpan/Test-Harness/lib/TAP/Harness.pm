package TAP::Harness;

use strict;
use warnings;
use Carp;

use File::Spec;
use File::Path;
use IO::Handle;

use base 'TAP::Base';

=head1 NAME

TAP::Harness - Run test scripts with statistics

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

$ENV{HARNESS_ACTIVE}  = 1;
$ENV{HARNESS_VERSION} = $VERSION;

END {

    # For VMS.
    delete $ENV{HARNESS_ACTIVE};
    delete $ENV{HARNESS_VERSION};
}

=head1 DESCRIPTION

This is a simple test harness which allows tests to be run and results
automatically aggregated and output to STDOUT.

=head1 SYNOPSIS

 use TAP::Harness;
 my $harness = TAP::Harness->new( \%args );
 $harness->runtests(@tests);

=cut

my %VALIDATION_FOR;
my @FORMATTER_ARGS;

sub _error {
    my $self = shift;
    return $self->{error} unless @_;
    $self->{error} = shift;
}

BEGIN {

    @FORMATTER_ARGS = qw(
      directives verbosity timer failures comments errors stdout color
      show_count normalize
    );

    %VALIDATION_FOR = (
        lib => sub {
            my ( $self, $libs ) = @_;
            $libs = [$libs] unless 'ARRAY' eq ref $libs;

            return [ map {"-I$_"} @$libs ];
        },
        switches          => sub { shift; shift },
        exec              => sub { shift; shift },
        merge             => sub { shift; shift },
        aggregator_class  => sub { shift; shift },
        formatter_class   => sub { shift; shift },
        multiplexer_class => sub { shift; shift },
        parser_class      => sub { shift; shift },
        scheduler_class   => sub { shift; shift },
        formatter         => sub { shift; shift },
        jobs              => sub { shift; shift },
        test_args         => sub { shift; shift },
        ignore_exit       => sub { shift; shift },
        rules             => sub { shift; shift },
        rulesfile         => sub { shift; shift },
        sources           => sub { shift; shift },
        version           => sub { shift; shift },
        trap              => sub { shift; shift },
    );

    for my $method ( sort keys %VALIDATION_FOR ) {
        no strict 'refs';
        if ( $method eq 'lib' || $method eq 'switches' ) {
            *{$method} = sub {
                my $self = shift;
                unless (@_) {
                    $self->{$method} ||= [];
                    return wantarray
                      ? @{ $self->{$method} }
                      : $self->{$method};
                }
                $self->_croak("Too many arguments to method '$method'")
                  if @_ > 1;
                my $args = shift;
                $args = [$args] unless ref $args;
                $self->{$method} = $args;
                return $self;
            };
        }
        else {
            *{$method} = sub {
                my $self = shift;
                return $self->{$method} unless @_;
                $self->{$method} = shift;
            };
        }
    }

    for my $method (@FORMATTER_ARGS) {
        no strict 'refs';
        *{$method} = sub {
            my $self = shift;
            return $self->formatter->$method(@_);
        };
    }
}

##############################################################################

=head1 METHODS

=head2 Class Methods

=head3 C<new>

 my %args = (
    verbosity => 1,
    lib     => [ 'lib', 'blib/lib', 'blib/arch' ],
 )
 my $harness = TAP::Harness->new( \%args );

The constructor returns a new C<TAP::Harness> object. It accepts an
optional hashref whose allowed keys are:

=over 4

=item * C<verbosity>

Set the verbosity level:

     1   verbose        Print individual test results to STDOUT.
     0   normal
    -1   quiet          Suppress some test output (mostly failures 
                        while tests are running).
    -2   really quiet   Suppress everything but the tests summary.
    -3   silent         Suppress everything.

=item * C<timer>

Append run time for each test to output. Uses L<Time::HiRes> if
available.

=item * C<failures>

Show test failures (this is a no-op if C<verbose> is selected).

=item * C<comments>

Show test comments (this is a no-op if C<verbose> is selected).

=item * C<show_count>

Update the running test count during testing.

=item * C<normalize>

Set to a true value to normalize the TAP that is emitted in verbose modes.

=item * C<lib>

Accepts a scalar value or array ref of scalar values indicating which
paths to allowed libraries should be included if Perl tests are
executed. Naturally, this only makes sense in the context of tests
written in Perl.

=item * C<switches>

Accepts a scalar value or array ref of scalar values indicating which
switches should be included if Perl tests are executed. Naturally, this
only makes sense in the context of tests written in Perl.

=item * C<test_args>

A reference to an C<@INC> style array of arguments to be passed to each
test program.

  test_args => ['foo', 'bar'],

if you want to pass different arguments to each test then you should
pass a hash of arrays, keyed by the alias for each test:

  test_args => {
    my_test    => ['foo', 'bar'],
    other_test => ['baz'],
  }

=item * C<color>

Attempt to produce color output.

=item * C<exec>

Typically, Perl tests are run through this. However, anything which
spits out TAP is fine. You can use this argument to specify the name of
the program (and optional switches) to run your tests with:

  exec => ['/usr/bin/ruby', '-w']

You can also pass a subroutine reference in order to determine and
return the proper program to run based on a given test script. The
subroutine reference should expect the TAP::Harness object itself as the
first argument, and the file name as the second argument. It should
return an array reference containing the command to be run and including
the test file name. It can also simply return C<undef>, in which case
TAP::Harness will fall back on executing the test script in Perl:

    exec => sub {
        my ( $harness, $test_file ) = @_;

        # Let Perl tests run.
        return undef if $test_file =~ /[.]t$/;
        return [ qw( /usr/bin/ruby -w ), $test_file ]
          if $test_file =~ /[.]rb$/;
      }

If the subroutine returns a scalar with a newline or a filehandle, it
will be interpreted as raw TAP or as a TAP stream, respectively.

=item * C<merge>

If C<merge> is true the harness will create parsers that merge STDOUT
and STDERR together for any processes they start.

=item * C<sources>

I<NEW to 3.18>.

If set, C<sources> must be a hashref containing the names of the
L<TAP::Parser::SourceHandler>s to load and/or configure.  The values are a
hash of configuration that will be accessible to the source handlers via
L<TAP::Parser::Source/config_for>.

For example:

  sources => {
    Perl => { exec => '/path/to/custom/perl' },
    File => { extensions => [ '.tap', '.txt' ] },
    MyCustom => { some => 'config' },
  }

The C<sources> parameter affects how C<source>, C<tap> and C<exec> parameters
are handled.

For more details, see the C<sources> parameter in L<TAP::Parser/new>,
L<TAP::Parser::Source>, and L<TAP::Parser::IteratorFactory>.

=item * C<aggregator_class>

The name of the class to use to aggregate test results. The default is
L<TAP::Parser::Aggregator>.

=item * C<version>

I<NEW to 3.22>.

Assume this TAP version for L<TAP::Parser> instead of default TAP
version 12.

=item * C<formatter_class>

The name of the class to use to format output. The default is
L<TAP::Formatter::Console>, or L<TAP::Formatter::File> if the output
isn't a TTY.

=item * C<multiplexer_class>

The name of the class to use to multiplex tests during parallel testing.
The default is L<TAP::Parser::Multiplexer>.

=item * C<parser_class>

The name of the class to use to parse TAP. The default is
L<TAP::Parser>.

=item * C<scheduler_class>

The name of the class to use to schedule test execution. The default is
L<TAP::Parser::Scheduler>.

=item * C<formatter>

If set C<formatter> must be an object that is capable of formatting the
TAP output. See L<TAP::Formatter::Console> for an example.

=item * C<errors>

If parse errors are found in the TAP output, a note of this will be
made in the summary report. To see all of the parse errors, set this
argument to true:

  errors => 1

=item * C<directives>

If set to a true value, only test results with directives will be
displayed. This overrides other settings such as C<verbose> or
C<failures>.

=item * C<ignore_exit>

If set to a true value instruct C<TAP::Parser> to ignore exit and wait
status from test scripts.

=item * C<jobs>

The maximum number of parallel tests to run at any time.  Which tests
can be run in parallel is controlled by C<rules>.  The default is to
run only one test at a time.

=item * C<rules>

A reference to a hash of rules that control which tests may be executed in
parallel. If no rules are declared and L<CPAN::Meta::YAML> is available,
C<TAP::Harness> attempts to load rules from a YAML file specified by the
C<rulesfile> parameter. If no rules file exists, the default is for all
tests to be eligible to be run in parallel.

Here some simple examples. For the full details of the data structure
and the related glob-style pattern matching, see
L<TAP::Parser::Scheduler/"Rules data structure">.

    # Run all tests in sequence, except those starting with "p"
    $harness->rules({
        par => 't/p*.t'
    });

    # Equivalent YAML file
    ---
    par: t/p*.t

    # Run all tests in parallel, except those starting with "p"
    $harness->rules({
        seq => [
                  { seq => 't/p*.t' },
                  { par => '**'     },
               ],
    });

    # Equivalent YAML file
    ---
    seq:
        - seq: t/p*.t
        - par: **

    # Run some  startup tests in sequence, then some parallel tests than some
    # teardown tests in sequence.
    $harness->rules({
        seq => [
            { seq => 't/startup/*.t' },
            { par => ['t/a/*.t','t/b/*.t','t/c/*.t'], }
            { seq => 't/shutdown/*.t' },
        ],

    });

    # Equivalent YAML file
    ---
    seq:
        - seq: t/startup/*.t
        - par:
            - t/a/*.t
            - t/b/*.t
            - t/c/*.t
        - seq: t/shutdown/*.t

This is an experimental feature and the interface may change.

=item * C<rulesfiles>

This specifies where to find a YAML file of test scheduling rules.  If not
provided, it looks for a default file to use.  It first checks for a file given
in the C<HARNESS_RULESFILE> environment variable, then it checks for
F<testrules.yml> and then F<t/testrules.yml>.

=item * C<stdout>

A filehandle for catching standard output.

=item * C<trap>

Attempt to print summary information if run is interrupted by
SIGINT (Ctrl-C).

=back

Any keys for which the value is C<undef> will be ignored.

=cut

# new supplied by TAP::Base

{
    my @legal_callback = qw(
      parser_args
      made_parser
      before_runtests
      after_runtests
      after_test
    );

    my %default_class = (
        aggregator_class  => 'TAP::Parser::Aggregator',
        formatter_class   => 'TAP::Formatter::Console',
        multiplexer_class => 'TAP::Parser::Multiplexer',
        parser_class      => 'TAP::Parser',
        scheduler_class   => 'TAP::Parser::Scheduler',
    );

    sub _initialize {
        my ( $self, $arg_for ) = @_;
        $arg_for ||= {};

        $self->SUPER::_initialize( $arg_for, \@legal_callback );
        my %arg_for = %$arg_for;    # force a shallow copy

        for my $name ( sort keys %VALIDATION_FOR ) {
            my $property = delete $arg_for{$name};
            if ( defined $property ) {
                my $validate = $VALIDATION_FOR{$name};

                my $value = $self->$validate($property);
                if ( $self->_error ) {
                    $self->_croak;
                }
                $self->$name($value);
            }
        }

        $self->jobs(1) unless defined $self->jobs;

        if ( ! defined $self->rules ) {
            $self->_maybe_load_rulesfile;
        }

        local $default_class{formatter_class} = 'TAP::Formatter::File'
          unless -t ( $arg_for{stdout} || \*STDOUT ) && !$ENV{HARNESS_NOTTY};

        while ( my ( $attr, $class ) = each %default_class ) {
            $self->$attr( $self->$attr() || $class );
        }

        unless ( $self->formatter ) {

            # This is a little bodge to preserve legacy behaviour. It's
            # pretty horrible that we know which args are destined for
            # the formatter.
            my %formatter_args = ( jobs => $self->jobs );
            for my $name (@FORMATTER_ARGS) {
                if ( defined( my $property = delete $arg_for{$name} ) ) {
                    $formatter_args{$name} = $property;
                }
            }

            $self->formatter(
                $self->_construct( $self->formatter_class, \%formatter_args )
            );
        }

        if ( my @props = sort keys %arg_for ) {
            $self->_croak("Unknown arguments to TAP::Harness::new (@props)");
        }

        return $self;
    }

    sub _maybe_load_rulesfile {
        my ($self) = @_;

        my ($rulesfile) =   defined $self->rulesfile ? $self->rulesfile :
                            defined($ENV{HARNESS_RULESFILE}) ? $ENV{HARNESS_RULESFILE} :
                            grep { -r } qw(./testrules.yml t/testrules.yml);

        if ( defined $rulesfile && -r $rulesfile ) {
            if ( ! eval { require CPAN::Meta::YAML; 1} ) {
               warn "CPAN::Meta::YAML required to process $rulesfile" ;
               return;
            }
            my $layer = $] lt "5.008" ? "" : ":encoding(UTF-8)";
            open my $fh, "<$layer", $rulesfile
                or die "Couldn't open $rulesfile: $!";
            my $yaml_text = do { local $/; <$fh> };
            my $yaml = CPAN::Meta::YAML->read_string($yaml_text)
                or die CPAN::Meta::YAML->errstr;
            $self->rules( $yaml->[0] );
        }
        return;
    }
}

##############################################################################

=head2 Instance Methods

=head3 C<runtests>

    $harness->runtests(@tests);

Accepts an array of C<@tests> to be run. This should generally be the
names of test files, but this is not required. Each element in C<@tests>
will be passed to C<TAP::Parser::new()> as a C<source>. See
L<TAP::Parser> for more information.

It is possible to provide aliases that will be displayed in place of the
test name by supplying the test as a reference to an array containing
C<< [ $test, $alias ] >>:

    $harness->runtests( [ 't/foo.t', 'Foo Once' ],
                        [ 't/foo.t', 'Foo Twice' ] );

Normally it is an error to attempt to run the same test twice. Aliases
allow you to overcome this limitation by giving each run of the test a
unique name.

Tests will be run in the order found.

If the environment variable C<PERL_TEST_HARNESS_DUMP_TAP> is defined it
should name a directory into which a copy of the raw TAP for each test
will be written. TAP is written to files named for each test.
Subdirectories will be created as needed.

Returns a L<TAP::Parser::Aggregator> containing the test results.

=cut

sub runtests {
    my ( $self, @tests ) = @_;

    my $aggregate = $self->_construct( $self->aggregator_class );

    $self->_make_callback( 'before_runtests', $aggregate );
    $aggregate->start;
    my $finish = sub {
        my $interrupted = shift;
        $aggregate->stop;
        $self->summary( $aggregate, $interrupted );
        $self->_make_callback( 'after_runtests', $aggregate );
    };
    my $run = sub {
        my $bailout;
        eval { $self->aggregate_tests( $aggregate, @tests ); 1 }
            or do { $bailout = $@ || 'unknown_error' };
        $finish->();
        die $bailout if defined $bailout;
    };

    if ( $self->trap ) {
        local $SIG{INT} = sub {
            print "\n";
            $finish->(1);
            exit;
        };
        $run->();
    }
    else {
        $run->();
    }

    return $aggregate;
}

=head3 C<summary>

  $harness->summary( $aggregator );

Output the summary for a L<TAP::Parser::Aggregator>.

=cut

sub summary {
    my ( $self, @args ) = @_;
    $self->formatter->summary(@args);
}

sub _after_test {
    my ( $self, $aggregate, $job, $parser ) = @_;

    $self->_make_callback( 'after_test', $job->as_array_ref, $parser );
    $aggregate->add( $job->description, $parser );
}

sub _bailout {
    my ( $self, $result, $parser, $session, $aggregate, $job ) = @_;

    $self->finish_parser( $parser, $session );
    $self->_after_test( $aggregate, $job, $parser );
    $job->finish;

    my $explanation = $result->explanation;
    die "FAILED--Further testing stopped"
      . ( $explanation ? ": $explanation\n" : ".\n" );
}

sub _aggregate_parallel {
    my ( $self, $aggregate, $scheduler ) = @_;

    my $jobs = $self->jobs;
    my $mux  = $self->_construct( $self->multiplexer_class );

    RESULT: {

        # Keep multiplexer topped up
        FILL:
        while ( $mux->parsers < $jobs ) {
            my $job = $scheduler->get_job;

            # If we hit a spinner stop filling and start running.
            last FILL if !defined $job || $job->is_spinner;

            my ( $parser, $session ) = $self->make_parser($job);
            $mux->add( $parser, [ $session, $job ] );

            # The job has started: begin the timers
            $parser->start_time( $parser->get_time );
            $parser->start_times( $parser->get_times );
        }

        if ( my ( $parser, $stash, $result ) = $mux->next ) {
            my ( $session, $job ) = @$stash;
            if ( defined $result ) {
                $session->result($result);
                $self->_bailout($result, $parser, $session, $aggregate, $job )
                    if $result->is_bailout;
            }
            else {

                # End of parser. Automatically removed from the mux.
                $self->finish_parser( $parser, $session );
                $self->_after_test( $aggregate, $job, $parser );
                $job->finish;
            }
            redo RESULT;
        }
    }

    return;
}

sub _aggregate_single {
    my ( $self, $aggregate, $scheduler ) = @_;

    JOB:
    while ( my $job = $scheduler->get_job ) {
        next JOB if $job->is_spinner;

        my ( $parser, $session ) = $self->make_parser($job);

        while ( defined( my $result = $parser->next ) ) {
            $session->result($result);
            if ( $result->is_bailout ) {

                # Keep reading until input is exhausted in the hope
                # of allowing any pending diagnostics to show up.
                1 while $parser->next;
                $self->_bailout($result, $parser, $session, $aggregate, $job );
            }
        }

        $self->finish_parser( $parser, $session );
        $self->_after_test( $aggregate, $job, $parser );
        $job->finish;
    }

    return;
}

=head3 C<aggregate_tests>

  $harness->aggregate_tests( $aggregate, @tests );

Run the named tests and display a summary of result. Tests will be run
in the order found.

Test results will be added to the supplied L<TAP::Parser::Aggregator>.
C<aggregate_tests> may be called multiple times to run several sets of
tests. Multiple C<Test::Harness> instances may be used to pass results
to a single aggregator so that different parts of a complex test suite
may be run using different C<TAP::Harness> settings. This is useful, for
example, in the case where some tests should run in parallel but others
are unsuitable for parallel execution.

    my $formatter   = TAP::Formatter::Console->new;
    my $ser_harness = TAP::Harness->new( { formatter => $formatter } );
    my $par_harness = TAP::Harness->new(
        {   formatter => $formatter,
            jobs      => 9
        }
    );
    my $aggregator = TAP::Parser::Aggregator->new;

    $aggregator->start();
    $ser_harness->aggregate_tests( $aggregator, @ser_tests );
    $par_harness->aggregate_tests( $aggregator, @par_tests );
    $aggregator->stop();
    $formatter->summary($aggregator);

Note that for simpler testing requirements it will often be possible to
replace the above code with a single call to C<runtests>.

Each element of the C<@tests> array is either:

=over

=item * the source name of a test to run

=item * a reference to a [ source name, display name ] array

=back

In the case of a perl test suite, typically I<source names> are simply the file
names of the test scripts to run.

When you supply a separate display name it becomes possible to run a
test more than once; the display name is effectively the alias by which
the test is known inside the harness. The harness doesn't care if it
runs the same test more than once when each invocation uses a
different name.

=cut

sub aggregate_tests {
    my ( $self, $aggregate, @tests ) = @_;

    my $jobs      = $self->jobs;
    my $scheduler = $self->make_scheduler(@tests);

    # #12458
    local $ENV{HARNESS_IS_VERBOSE} = 1
      if $self->formatter->verbosity > 0;

    # Formatter gets only names.
    $self->formatter->prepare( map { $_->description } $scheduler->get_all );

    if ( $self->jobs > 1 ) {
        $self->_aggregate_parallel( $aggregate, $scheduler );
    }
    else {
        $self->_aggregate_single( $aggregate, $scheduler );
    }

    return;
}

sub _add_descriptions {
    my $self = shift;

    # Turn unwrapped scalars into anonymous arrays and copy the name as
    # the description for tests that have only a name.
    return map { @$_ == 1 ? [ $_->[0], $_->[0] ] : $_ }
      map { 'ARRAY' eq ref $_ ? $_ : [$_] } @_;
}

=head3 C<make_scheduler>

Called by the harness when it needs to create a
L<TAP::Parser::Scheduler>. Override in a subclass to provide an
alternative scheduler. C<make_scheduler> is passed the list of tests
that was passed to C<aggregate_tests>.

=cut

sub make_scheduler {
    my ( $self, @tests ) = @_;
    return $self->_construct(
        $self->scheduler_class,
        tests => [ $self->_add_descriptions(@tests) ],
        rules => $self->rules
    );
}

=head3 C<jobs>

Gets or sets the number of concurrent test runs the harness is
handling.  By default, this value is 1 -- for parallel testing, this
should be set higher.

=cut

##############################################################################

sub _get_parser_args {
    my ( $self, $job ) = @_;
    my $test_prog = $job->filename;
    my %args      = ();

    $args{sources} = $self->sources if $self->sources;

    my @switches;
    @switches = $self->lib if $self->lib;
    push @switches => $self->switches if $self->switches;
    $args{switches}    = \@switches;
    $args{spool}       = $self->_open_spool($test_prog);
    $args{merge}       = $self->merge;
    $args{ignore_exit} = $self->ignore_exit;
    $args{version}     = $self->version if $self->version;

    if ( my $exec = $self->exec ) {
        $args{exec}
          = ref $exec eq 'CODE'
          ? $exec->( $self, $test_prog )
          : [ @$exec, $test_prog ];
        if ( not defined $args{exec} ) {
            $args{source} = $test_prog;
        }
        elsif ( ( ref( $args{exec} ) || "" ) ne "ARRAY" ) {
            $args{source} = delete $args{exec};
        }
    }
    else {
        $args{source} = $test_prog;
    }

    if ( defined( my $test_args = $self->test_args ) ) {

        if ( ref($test_args) eq 'HASH' ) {

            # different args for each test
            if ( exists( $test_args->{ $job->description } ) ) {
                $test_args = $test_args->{ $job->description };
            }
            else {
                $self->_croak( "TAP::Harness Can't find test_args for "
                      . $job->description );
            }
        }

        $args{test_args} = $test_args;
    }

    return \%args;
}

=head3 C<make_parser>

Make a new parser and display formatter session. Typically used and/or
overridden in subclasses.

    my ( $parser, $session ) = $harness->make_parser;

=cut

sub make_parser {
    my ( $self, $job ) = @_;

    my $args = $self->_get_parser_args($job);
    $self->_make_callback( 'parser_args', $args, $job->as_array_ref );
    my $parser = $self->_construct( $self->parser_class, $args );

    $self->_make_callback( 'made_parser', $parser, $job->as_array_ref );
    my $session = $self->formatter->open_test( $job->description, $parser );

    return ( $parser, $session );
}

=head3 C<finish_parser>

Terminate use of a parser. Typically used and/or overridden in
subclasses. The parser isn't destroyed as a result of this.

=cut

sub finish_parser {
    my ( $self, $parser, $session ) = @_;

    $session->close_test;
    $self->_close_spool($parser);

    return $parser;
}

sub _open_spool {
    my $self = shift;
    my $test = shift;

    if ( my $spool_dir = $ENV{PERL_TEST_HARNESS_DUMP_TAP} ) {

        my $spool = File::Spec->catfile( $spool_dir, $test );

        # Make the directory
        my ( $vol, $dir, undef ) = File::Spec->splitpath($spool);
        my $path = File::Spec->catpath( $vol, $dir, '' );
        eval { mkpath($path) };
        $self->_croak($@) if $@;

        my $spool_handle = IO::Handle->new;
        open( $spool_handle, ">$spool" )
          or $self->_croak(" Can't write $spool ( $! ) ");

        return $spool_handle;
    }

    return;
}

sub _close_spool {
    my $self = shift;
    my ($parser) = @_;

    if ( my $spool_handle = $parser->delete_spool ) {
        close($spool_handle)
          or $self->_croak(" Error closing TAP spool file( $! ) \n ");
    }

    return;
}

sub _croak {
    my ( $self, $message ) = @_;
    unless ($message) {
        $message = $self->_error;
    }
    $self->SUPER::_croak($message);

    return;
}

1;

__END__

##############################################################################

=head1 CONFIGURING

C<TAP::Harness> is designed to be easy to configure.

=head2 Plugins

C<TAP::Parser> plugins let you change the way TAP is I<input> to and I<output>
from the parser.

L<TAP::Parser::SourceHandler>s handle TAP I<input>.  You can configure them
and load custom handlers using the C<sources> parameter to L</new>.

L<TAP::Formatter>s handle TAP I<output>.  You can load custom formatters by
using the C<formatter_class> parameter to L</new>.  To configure a formatter,
you currently need to instantiate it outside of L<TAP::Harness> and pass it in
with the C<formatter> parameter to L</new>.  This I<may> be addressed by adding
a I<formatters> parameter to L</new> in the future.

=head2 C<Module::Build>

L<Module::Build> version C<0.30> supports C<TAP::Harness>.

To load C<TAP::Harness> plugins, you'll need to use the C<tap_harness_args>
parameter to C<new>, typically from your C<Build.PL>.  For example:

  Module::Build->new(
      module_name        => 'MyApp',
      test_file_exts     => [qw(.t .tap .txt)],
      use_tap_harness    => 1,
      tap_harness_args   => {
          sources => {
              MyCustom => {},
              File => {
                  extensions => ['.tap', '.txt'],
              },
          },
          formatter_class => 'TAP::Formatter::HTML',
      },
      build_requires     => {
          'Module::Build' => '0.30',
          'TAP::Harness'  => '3.18',
      },
  )->create_build_script;

See L</new>

=head2 C<ExtUtils::MakeMaker>

L<ExtUtils::MakeMaker> does not support L<TAP::Harness> out-of-the-box.

=head2 C<prove>

L<prove> supports C<TAP::Harness> plugins, and has a plugin system of its
own.  See L<prove/FORMATTERS>, L<prove/SOURCE HANDLERS> and L<App::Prove>
for more details.

=head1 WRITING PLUGINS

If you can't configure C<TAP::Harness> to do what you want, and you can't find
an existing plugin, consider writing one.

The two primary use cases supported by L<TAP::Harness> for plugins are I<input>
and I<output>:

=over 2

=item Customize how TAP gets into the parser

To do this, you can either extend an existing L<TAP::Parser::SourceHandler>,
or write your own.  It's a pretty simple API, and they can be loaded and
configured using the C<sources> parameter to L</new>.

=item Customize how TAP results are output from the parser

To do this, you can either extend an existing L<TAP::Formatter>, or write your
own.  Writing formatters are a bit more involved than writing a
I<SourceHandler>, as you'll need to understand the L<TAP::Parser> API.  A
good place to start is by understanding how L</aggregate_tests> works.

Custom formatters can be loaded configured using the C<formatter_class>
parameter to L</new>.

=back

=head1 SUBCLASSING

If you can't configure C<TAP::Harness> to do exactly what you want, and writing
a plugin isn't an option, consider extending it.  It is designed to be (mostly)
easy to subclass, though the cases when sub-classing is necessary should be few
and far between.

=head2 Methods

The following methods are ones you may wish to override if you want to
subclass C<TAP::Harness>.

=over 4

=item L</new>

=item L</runtests>

=item L</summary>

=back

=cut

=head1 REPLACING

If you like the C<prove> utility and L<TAP::Parser> but you want your
own harness, all you need to do is write one and provide C<new> and
C<runtests> methods. Then you can use the C<prove> utility like so:

 prove --harness My::Test::Harness

Note that while C<prove> accepts a list of tests (or things to be
tested), C<new> has a fairly rich set of arguments. You'll probably want
to read over this code carefully to see how all of them are being used.

=head1 SEE ALSO

L<Test::Harness>

=cut

# vim:ts=4:sw=4:et:sta
