package App::Prove::State::Result::Test;

use strict;
use warnings;

=head1 NAME

App::Prove::State::Result::Test - Individual test results.

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

=head1 DESCRIPTION

The C<prove> command supports a C<--state> option that instructs it to
store persistent state across runs. This module encapsulates the results for a
single test.

=head1 SYNOPSIS

    # Re-run failed tests
    $ prove --state=failed,save -rbv

=cut

my %methods = (
    name           => { method => 'name' },
    elapsed        => { method => 'elapsed', default => 0 },
    gen            => { method => 'generation', default => 1 },
    last_pass_time => { method => 'last_pass_time', default => undef },
    last_fail_time => { method => 'last_fail_time', default => undef },
    last_result    => { method => 'result', default => 0 },
    last_run_time  => { method => 'run_time', default => undef },
    last_todo      => { method => 'num_todo', default => 0 },
    mtime          => { method => 'mtime', default => undef },
    seq            => { method => 'sequence', default => 1 },
    total_passes   => { method => 'total_passes', default => 0 },
    total_failures => { method => 'total_failures', default => 0 },
    parser         => { method => 'parser' },
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

=head1 METHODS

=head2 Class Methods

=head3 C<new>

=cut

sub new {
    my ( $class, $arg_for ) = @_;
    $arg_for ||= {};
    bless $arg_for => $class;
}

=head2 Instance Methods

=head3 C<name>

The name of the test.  Usually a filename.

=head3 C<elapsed>

The total elapsed times the test took to run, in seconds from the epoch..

=head3 C<generation>

The number for the "generation" of the test run.  The first generation is 1
(one) and subsequent generations are 2, 3, etc.

=head3 C<last_pass_time>

The last time the test program passed, in seconds from the epoch.

Returns C<undef> if the program has never passed.

=head3 C<last_fail_time>

The last time the test suite failed, in seconds from the epoch.

Returns C<undef> if the program has never failed.

=head3 C<mtime>

Returns the mtime of the test, in seconds from the epoch.

=head3 C<raw>

Returns a hashref of raw test data, suitable for serialization by YAML.

=head3 C<result>

Currently, whether or not the test suite passed with no 'problems' (such as
TODO passed).

=head3 C<run_time>

The total time it took for the test to run, in seconds.  If C<Time::HiRes> is
available, it will have finer granularity.

=head3 C<num_todo>

The number of tests with TODO directives.

=head3 C<sequence>

The order in which this test was run for the given test suite result. 

=head3 C<total_passes>

The number of times the test has passed.

=head3 C<total_failures>

The number of times the test has failed.

=head3 C<parser>

The underlying parser object.  This is useful if you need the full
information for the test program.

=cut

sub raw {
    my $self = shift;
    my %raw  = %$self;

    # this is backwards-compatibility hack and is not guaranteed.
    delete $raw{name};
    delete $raw{parser};
    return \%raw;
}

1;
