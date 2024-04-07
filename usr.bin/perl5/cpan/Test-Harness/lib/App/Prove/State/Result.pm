package App::Prove::State::Result;

use strict;
use warnings;
use Carp 'croak';

use App::Prove::State::Result::Test;

use constant STATE_VERSION => 1;

=head1 NAME

App::Prove::State::Result - Individual test suite results.

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

=head1 DESCRIPTION

The C<prove> command supports a C<--state> option that instructs it to
store persistent state across runs. This module encapsulates the results for a
single test suite run.

=head1 SYNOPSIS

    # Re-run failed tests
    $ prove --state=failed,save -rbv

=cut

=head1 METHODS

=head2 Class Methods

=head3 C<new>

    my $result = App::Prove::State::Result->new({
        generation => $generation,
        tests      => \%tests,
    });

Returns a new C<App::Prove::State::Result> instance.

=cut

sub new {
    my ( $class, $arg_for ) = @_;
    $arg_for ||= {};
    my %instance_data = %$arg_for;    # shallow copy
    $instance_data{version} = $class->state_version;
    my $tests = delete $instance_data{tests} || {};
    my $self = bless \%instance_data => $class;
    $self->_initialize($tests);
    return $self;
}

sub _initialize {
    my ( $self, $tests ) = @_;
    my %tests;
    while ( my ( $name, $test ) = each %$tests ) {
        $tests{$name} = $self->test_class->new(
            {   %$test,
                name => $name
            }
        );
    }
    $self->tests( \%tests );
    return $self;
}

=head2 C<state_version>

Returns the current version of state storage.

=cut

sub state_version {STATE_VERSION}

=head2 C<test_class>

Returns the name of the class used for tracking individual tests.  This class
should either subclass from C<App::Prove::State::Result::Test> or provide an
identical interface.

=cut

sub test_class {
    return 'App::Prove::State::Result::Test';
}

my %methods = (
    generation    => { method => 'generation',    default => 0 },
    last_run_time => { method => 'last_run_time', default => undef },
);

while ( my ( $key, $description ) = each %methods ) {
    my $default = $description->{default};
    no strict 'refs';
    *{ $description->{method} } = sub {
        my $self = shift;
        if (@_) {
            $self->{$key} = shift;
            return $self;
        }
        return $self->{$key} || $default;
    };
}

=head3 C<generation>

Getter/setter for the "generation" of the test suite run. The first
generation is 1 (one) and subsequent generations are 2, 3, etc.

=head3 C<last_run_time>

Getter/setter for the time of the test suite run.

=head3 C<tests>

Returns the tests for a given generation. This is a hashref or a hash,
depending on context called. The keys to the hash are the individual
test names and the value is a hashref with various interesting values.
Each k/v pair might resemble something like this:

 't/foo.t' => {
    elapsed        => '0.0428488254547119',
    gen            => '7',
    last_pass_time => '1219328376.07815',
    last_result    => '0',
    last_run_time  => '1219328376.07815',
    last_todo      => '0',
    mtime          => '1191708862',
    seq            => '192',
    total_passes   => '6',
  }

=cut

sub tests {
    my $self = shift;
    if (@_) {
        $self->{tests} = shift;
        return $self;
    }
    my %tests = %{ $self->{tests} };
    my @tests = sort { $a->sequence <=> $b->sequence } values %tests;
    return wantarray ? @tests : \@tests;
}

=head3 C<test>

 my $test = $result->test('t/customer/create.t');

Returns an individual C<App::Prove::State::Result::Test> instance for the
given test name (usually the filename).  Will return a new
C<App::Prove::State::Result::Test> instance if the name is not found.

=cut

sub test {
    my ( $self, $name ) = @_;
    croak("test() requires a test name") unless defined $name;

    my $tests = $self->{tests} ||= {};
    if ( my $test = $tests->{$name} ) {
        return $test;
    }
    else {
        my $test = $self->test_class->new( { name => $name } );
        $self->{tests}->{$name} = $test;
        return $test;
    }
}

=head3 C<test_names>

Returns an list of test names, sorted by run order.

=cut

sub test_names {
    my $self = shift;
    return map { $_->name } $self->tests;
}

=head3 C<remove>

 $result->remove($test_name);            # remove the test
 my $test = $result->test($test_name);   # fatal error

Removes a given test from results.  This is a no-op if the test name is not
found.

=cut

sub remove {
    my ( $self, $name ) = @_;
    delete $self->{tests}->{$name};
    return $self;
}

=head3 C<num_tests>

Returns the number of tests for a given test suite result.

=cut

sub num_tests { keys %{ shift->{tests} } }

=head3 C<raw>

Returns a hashref of raw results, suitable for serialization by YAML.

=cut

sub raw {
    my $self = shift;
    my %raw  = %$self;

    my %tests;
    for my $test ( $self->tests ) {
        $tests{ $test->name } = $test->raw;
    }
    $raw{tests} = \%tests;
    return \%raw;
}

1;
