package TAP::Formatter::Session;

use strict;
use warnings;

use base 'TAP::Base';

my @ACCESSOR;

BEGIN {

    @ACCESSOR = qw( name formatter parser show_count );

    for my $method (@ACCESSOR) {
        no strict 'refs';
        *$method = sub { shift->{$method} };
    }
}

=head1 NAME

TAP::Formatter::Session - Abstract base class for harness output delegate 

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

=head1 METHODS

=head2 Class Methods

=head3 C<new>

 my %args = (
    formatter => $self,
 )
 my $harness = TAP::Formatter::Console::Session->new( \%args );

The constructor returns a new C<TAP::Formatter::Console::Session> object.

=over 4

=item * C<formatter>

=item * C<parser>

=item * C<name>

=item * C<show_count>

=back

=cut

sub _initialize {
    my ( $self, $arg_for ) = @_;
    $arg_for ||= {};

    $self->SUPER::_initialize($arg_for);
    my %arg_for = %$arg_for;    # force a shallow copy

    for my $name (@ACCESSOR) {
        $self->{$name} = delete $arg_for{$name};
    }

    if ( !defined $self->show_count ) {
        $self->{show_count} = 1;    # defaults to true
    }
    if ( $self->show_count ) {      # but may be a damned lie!
        $self->{show_count} = $self->_should_show_count;
    }

    if ( my @props = sort keys %arg_for ) {
        $self->_croak(
            "Unknown arguments to " . __PACKAGE__ . "::new (@props)" );
    }

    return $self;
}

=head3 C<header>

Output test preamble

=head3 C<result>

Called by the harness for each line of TAP it receives.

=head3 C<close_test>

Called to close a test session.

=head3 C<clear_for_close>

Called by C<close_test> to clear the line showing test progress, or the parallel
test ruler, prior to printing the final test result.

=head3 C<time_report>

Return a formatted string about the elapsed (wall-clock) time
and about the consumed CPU time.

=cut

sub header { }

sub result { }

sub close_test { }

sub clear_for_close { }

sub _should_show_count {
    my $self = shift;
    return
         !$self->formatter->verbose
      && -t $self->formatter->stdout
      && !$ENV{HARNESS_NOTTY};
}

sub _format_for_output {
    my ( $self, $result ) = @_;
    return $self->formatter->normalize ? $result->as_string : $result->raw;
}

sub _output_test_failure {
    my ( $self, $parser ) = @_;
    my $formatter = $self->formatter;
    return if $formatter->really_quiet;

    my $tests_run     = $parser->tests_run;
    my $tests_planned = $parser->tests_planned;

    my $total
      = defined $tests_planned
      ? $tests_planned
      : $tests_run;

    my $passed = $parser->passed;

    # The total number of fails includes any tests that were planned but
    # didn't run
    my $failed = $parser->failed + $total - $tests_run;
    my $exit   = $parser->exit;

    if ( my $exit = $parser->exit ) {
        my $wstat = $parser->wait;
        my $status = sprintf( "%d (wstat %d, 0x%x)", $exit, $wstat, $wstat );
        $formatter->_failure_output("Dubious, test returned $status\n");
    }

    if ( $failed == 0 ) {
        $formatter->_failure_output(
            $total
            ? "All $total subtests passed "
            : 'No subtests run '
        );
    }
    else {
        $formatter->_failure_output("Failed $failed/$total subtests ");
        if ( !$total ) {
            $formatter->_failure_output("\nNo tests run!");
        }
    }

    if ( my $skipped = $parser->skipped ) {
        $passed -= $skipped;
        my $test = 'subtest' . ( $skipped != 1 ? 's' : '' );
        $formatter->_output(
            "\n\t(less $skipped skipped $test: $passed okay)");
    }

    if ( my $failed = $parser->todo_passed ) {
        my $test = $failed > 1 ? 'tests' : 'test';
        $formatter->_output(
            "\n\t($failed TODO $test unexpectedly succeeded)");
    }

    $formatter->_output("\n");
}

sub _make_ok_line {
    my ( $self, $suffix ) = @_;
    return "ok$suffix\n";
}

sub time_report {
    my ( $self, $formatter, $parser ) = @_;

    my @time_report;
    if ( $formatter->timer ) {
        my $start_time = $parser->start_time;
        my $end_time   = $parser->end_time;
        if ( defined $start_time and defined $end_time ) {
            my $elapsed = $end_time - $start_time;
            push @time_report,
              $self->time_is_hires
                ? sprintf( ' %8d ms', $elapsed * 1000 )
                : sprintf( ' %8s s', $elapsed || '<1' );
        }
        my $start_times = $parser->start_times();
        my $end_times   = $parser->end_times();
        my $usr  = $end_times->[0] - $start_times->[0];
        my $sys  = $end_times->[1] - $start_times->[1];
        my $cusr = $end_times->[2] - $start_times->[2];
        my $csys = $end_times->[3] - $start_times->[3];
        push @time_report,
          sprintf('(%5.2f usr %5.2f sys + %5.2f cusr %5.2f csys = %5.2f CPU)',
                  $usr, $sys, $cusr, $csys,
                  $usr + $sys + $cusr + $csys);
    }

    return "@time_report";
}

1;
