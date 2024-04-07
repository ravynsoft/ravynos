package App::Prove::State;

use strict;
use warnings;

use File::Find;
use File::Spec;
use Carp;

use App::Prove::State::Result;
use TAP::Parser::YAMLish::Reader ();
use TAP::Parser::YAMLish::Writer ();
use base 'TAP::Base';

BEGIN {
    __PACKAGE__->mk_methods('result_class');
}

use constant IS_WIN32 => ( $^O =~ /^(MS)?Win32$/ );
use constant NEED_GLOB => IS_WIN32;

=head1 NAME

App::Prove::State - State storage for the C<prove> command.

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

=head1 DESCRIPTION

The C<prove> command supports a C<--state> option that instructs it to
store persistent state across runs. This module implements that state
and the operations that may be performed on it.

=head1 SYNOPSIS

    # Re-run failed tests
    $ prove --state=failed,save -rbv

=cut

=head1 METHODS

=head2 Class Methods

=head3 C<new>

Accepts a hashref with the following key/value pairs:

=over 4

=item * C<store>

The filename of the data store holding the data that App::Prove::State reads.

=item * C<extensions> (optional)

The test name extensions.  Defaults to C<.t>.

=item * C<result_class> (optional)

The name of the C<result_class>.  Defaults to C<App::Prove::State::Result>.

=back

=cut

# override TAP::Base::new:
sub new {
    my $class = shift;
    my %args = %{ shift || {} };

    my $self = bless {
        select     => [],
        seq        => 1,
        store      => delete $args{store},
        extensions => ( delete $args{extensions} || ['.t'] ),
        result_class =>
          ( delete $args{result_class} || 'App::Prove::State::Result' ),
    }, $class;

    $self->{_} = $self->result_class->new(
        {   tests      => {},
            generation => 1,
        }
    );
    my $store = $self->{store};
    $self->load($store)
      if defined $store && -f $store;

    return $self;
}

=head2 C<result_class>

Getter/setter for the name of the class used for tracking test results.  This
class should either subclass from C<App::Prove::State::Result> or provide an
identical interface.

=cut

=head2 C<extensions>

Get or set the list of extensions that files must have in order to be
considered tests. Defaults to ['.t'].

=cut

sub extensions {
    my $self = shift;
    $self->{extensions} = shift if @_;
    return $self->{extensions};
}

=head2 C<results>

Get the results of the last test run.  Returns a C<result_class()> instance.

=cut

sub results {
    my $self = shift;
    $self->{_} || $self->result_class->new;
}

=head2 C<commit>

Save the test results. Should be called after all tests have run.

=cut

sub commit {
    my $self = shift;
    if ( $self->{should_save} ) {
        $self->save;
    }
}

=head2 Instance Methods

=head3 C<apply_switch>

 $self->apply_switch('failed,save');

Apply a list of switch options to the state, updating the internal
object state as a result. Nothing is returned.

Diagnostics:
    - "Illegal state option: %s"

=over

=item C<last>

Run in the same order as last time

=item C<failed>

Run only the failed tests from last time

=item C<passed>

Run only the passed tests from last time

=item C<all>

Run all tests in normal order

=item C<hot>

Run the tests that most recently failed first

=item C<todo>

Run the tests ordered by number of todos.

=item C<slow>

Run the tests in slowest to fastest order.

=item C<fast>

Run test tests in fastest to slowest order.

=item C<new>

Run the tests in newest to oldest order.

=item C<old>

Run the tests in oldest to newest order.

=item C<save>

Save the state on exit.

=back

=cut

sub apply_switch {
    my $self = shift;
    my @opts = @_;

    my $last_gen      = $self->results->generation - 1;
    my $last_run_time = $self->results->last_run_time;
    my $now           = $self->get_time;

    my @switches = map { split /,/ } @opts;

    my %handler = (
        last => sub {
            $self->_select(
                limit => shift,
                where => sub { $_->generation >= $last_gen },
                order => sub { $_->sequence }
            );
        },
        failed => sub {
            $self->_select(
                limit => shift,
                where => sub { $_->result != 0 },
                order => sub { -$_->result }
            );
        },
        passed => sub {
            $self->_select(
                limit => shift,
                where => sub { $_->result == 0 }
            );
        },
        all => sub {
            $self->_select( limit => shift );
        },
        todo => sub {
            $self->_select(
                limit => shift,
                where => sub { $_->num_todo != 0 },
                order => sub { -$_->num_todo; }
            );
        },
        hot => sub {
            $self->_select(
                limit => shift,
                where => sub { defined $_->last_fail_time },
                order => sub { $now - $_->last_fail_time }
            );
        },
        slow => sub {
            $self->_select(
                limit => shift,
                order => sub { -$_->elapsed }
            );
        },
        fast => sub {
            $self->_select(
                limit => shift,
                order => sub { $_->elapsed }
            );
        },
        new => sub {
            $self->_select(
                limit => shift,
                order => sub { -$_->mtime }
            );
        },
        old => sub {
            $self->_select(
                limit => shift,
                order => sub { $_->mtime }
            );
        },
        fresh => sub {
            $self->_select(
                limit => shift,
                where => sub { $_->mtime >= $last_run_time }
            );
        },
        save => sub {
            $self->{should_save}++;
        },
        adrian => sub {
            unshift @switches, qw( hot all save );
        },
    );

    while ( defined( my $ele = shift @switches ) ) {
        my ( $opt, $arg )
          = ( $ele =~ /^([^:]+):(.*)/ )
          ? ( $1, $2 )
          : ( $ele, undef );
        my $code = $handler{$opt}
          || croak "Illegal state option: $opt";
        $code->($arg);
    }
    return;
}

sub _select {
    my ( $self, %spec ) = @_;
    push @{ $self->{select} }, \%spec;
}

=head3 C<get_tests>

Given a list of args get the names of tests that should run

=cut

sub get_tests {
    my $self    = shift;
    my $recurse = shift;
    my @argv    = @_;
    my %seen;

    my @selected = $self->_query;

    unless ( @argv || @{ $self->{select} } ) {
        @argv = $recurse ? '.' : 't';
        croak qq{No tests named and '@argv' directory not found}
          unless -d $argv[0];
    }

    push @selected, $self->_get_raw_tests( $recurse, @argv ) if @argv;
    return grep { !$seen{$_}++ } @selected;
}

sub _query {
    my $self = shift;
    if ( my @sel = @{ $self->{select} } ) {
        warn "No saved state, selection will be empty\n"
          unless $self->results->num_tests;
        return map { $self->_query_clause($_) } @sel;
    }
    return;
}

sub _query_clause {
    my ( $self, $clause ) = @_;
    my @got;
    my $results = $self->results;
    my $where = $clause->{where} || sub {1};

    # Select
    for my $name ( $results->test_names ) {
        next unless -f $name;
        local $_ = $results->test($name);
        push @got, $name if $where->();
    }

    # Sort
    if ( my $order = $clause->{order} ) {
        @got = map { $_->[0] }
          sort {
                 ( defined $b->[1] <=> defined $a->[1] )
              || ( ( $a->[1] || 0 ) <=> ( $b->[1] || 0 ) )
          } map {
            [   $_,
                do { local $_ = $results->test($_); $order->() }
            ]
          } @got;
    }

    if ( my $limit = $clause->{limit} ) {
        @got = splice @got, 0, $limit if @got > $limit;
    }

    return @got;
}

sub _get_raw_tests {
    my $self    = shift;
    my $recurse = shift;
    my @argv    = @_;
    my @tests;

    # Do globbing on Win32.
    if (NEED_GLOB) {
        eval "use File::Glob::Windows";    # [49732]
        @argv = map { glob "$_" } @argv;
    }
    my $extensions = $self->{extensions};

    for my $arg (@argv) {
        if ( '-' eq $arg ) {
            push @argv => <STDIN>;
            chomp(@argv);
            next;
        }

        push @tests,
            sort -d $arg
          ? $recurse
              ? $self->_expand_dir_recursive( $arg, $extensions )
              : map { glob( File::Spec->catfile( $arg, "*$_" ) ) }
              @{$extensions}
          : $arg;
    }
    return @tests;
}

sub _expand_dir_recursive {
    my ( $self, $dir, $extensions ) = @_;

    my @tests;
    my $ext_string = join( '|', map {quotemeta} @{$extensions} );

    find(
        {   follow      => 1,      #21938
            follow_skip => 2,
            wanted      => sub {
                -f 
                  && /(?:$ext_string)$/
                  && push @tests => $File::Find::name;
              }
        },
        $dir
    );
    return @tests;
}

=head3 C<observe_test>

Store the results of a test.

=cut

# Store:
#     last fail time
#     last pass time
#     last run time
#     most recent result
#     most recent todos
#     total failures
#     total passes
#     state generation
#     parser

sub observe_test {

    my ( $self, $test_info, $parser ) = @_;
    my $name = $test_info->[0];
    my $fail = scalar( $parser->failed ) + ( $parser->has_problems ? 1 : 0 );
    my $todo = scalar( $parser->todo );
    my $start_time = $parser->start_time;
    my $end_time   = $parser->end_time,

      my $test = $self->results->test($name);

    $test->sequence( $self->{seq}++ );
    $test->generation( $self->results->generation );

    $test->run_time($end_time);
    $test->result($fail);
    $test->num_todo($todo);
    $test->elapsed( $end_time - $start_time );

    $test->parser($parser);

    if ($fail) {
        $test->total_failures( $test->total_failures + 1 );
        $test->last_fail_time($end_time);
    }
    else {
        $test->total_passes( $test->total_passes + 1 );
        $test->last_pass_time($end_time);
    }
}

=head3 C<save>

Write the state to a file.

=cut

sub save {
    my ($self) = @_;

    my $store = $self->{store} or return;
    $self->results->last_run_time( $self->get_time );

    my $writer = TAP::Parser::YAMLish::Writer->new;
    local *FH;
    open FH, ">$store" or croak "Can't write $store ($!)";
    $writer->write( $self->results->raw, \*FH );
    close FH;
}

=head3 C<load>

Load the state from a file

=cut

sub load {
    my ( $self, $name ) = @_;
    my $reader = TAP::Parser::YAMLish::Reader->new;
    local *FH;
    open FH, "<$name" or croak "Can't read $name ($!)";

    # XXX this is temporary
    $self->{_} = $self->result_class->new(
        $reader->read(
            sub {
                my $line = <FH>;
                defined $line && chomp $line;
                return $line;
            }
        )
    );

    # $writer->write( $self->{tests} || {}, \*FH );
    close FH;
    $self->_regen_seq;
    $self->_prune_and_stamp;
    $self->results->generation( $self->results->generation + 1 );
}

sub _prune_and_stamp {
    my $self = shift;

    my $results = $self->results;
    my @tests   = $self->results->tests;
    for my $test (@tests) {
        my $name = $test->name;
        if ( my @stat = stat $name ) {
            $test->mtime( $stat[9] );
        }
        else {
            $results->remove($name);
        }
    }
}

sub _regen_seq {
    my $self = shift;
    for my $test ( $self->results->tests ) {
        $self->{seq} = $test->sequence + 1
          if defined $test->sequence && $test->sequence >= $self->{seq};
    }
}

1;
