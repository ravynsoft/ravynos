package TAP::Formatter::Console::ParallelSession;

use strict;
use warnings;
use File::Spec;
use File::Path;
use Carp;

use base 'TAP::Formatter::Console::Session';

use constant WIDTH => 72;    # Because Eric says

my %shared;

sub _initialize {
    my ( $self, $arg_for ) = @_;

    $self->SUPER::_initialize($arg_for);
    my $formatter = $self->formatter;

    # Horrid bodge. This creates our shared context per harness. Maybe
    # TAP::Harness should give us this?
    my $context = $shared{$formatter} ||= $self->_create_shared_context;
    push @{ $context->{active} }, $self;

    return $self;
}

sub _create_shared_context {
    my $self = shift;
    return {
        active => [],
        tests  => 0,
        fails  => 0,
    };
}

=head1 NAME

TAP::Formatter::Console::ParallelSession - Harness output delegate for parallel console output

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

=head1 DESCRIPTION

This provides console orientated output formatting for L<TAP::Harness>
when run with multiple L<TAP::Harness/jobs>.

=head1 SYNOPSIS

=cut

=head1 METHODS

=head2 Class Methods

=head3 C<header>

Output test preamble

=cut

sub header {
}

sub _clear_ruler {
    my $self = shift;
    $self->formatter->_output( "\r" . ( ' ' x WIDTH ) . "\r" );
}

my $now = 0;
my $start;

my $trailer     = '... )===';
my $chop_length = WIDTH - length $trailer;

sub _output_ruler {
    my ( $self, $refresh ) = @_;
    my $new_now = time;
    return if $new_now == $now and !$refresh;
    $now = $new_now;
    $start ||= $now;
    my $formatter = $self->formatter;
    return if $formatter->really_quiet;

    my $context = $shared{$formatter};

    my $ruler = sprintf '===( %7d;%d  ', $context->{tests}, $now - $start;

    for my $active ( @{ $context->{active} } ) {
        my $parser  = $active->parser;
        my $tests   = $parser->tests_run;
        my $planned = $parser->tests_planned || '?';

        $ruler .= sprintf '%' . length($planned) . "d/$planned  ", $tests;
    }
    chop $ruler;    # Remove a trailing space
    $ruler .= ')===';

    if ( length $ruler > WIDTH ) {
        $ruler =~ s/(.{$chop_length}).*/$1$trailer/o;
    }
    else {
        $ruler .= '=' x ( WIDTH - length($ruler) );
    }
    $formatter->_output("\r$ruler");
}

=head3 C<result>

  Called by the harness for each line of TAP it receives .

=cut

sub result {
    my ( $self, $result ) = @_;
    my $formatter = $self->formatter;

    # my $really_quiet = $formatter->really_quiet;
    # my $show_count   = $self->_should_show_count;

    if ( $result->is_test ) {
        my $context = $shared{$formatter};
        $context->{tests}++;

        my $active = $context->{active};
        if ( @$active == 1 ) {

            # There is only one test, so use the serial output format.
            return $self->SUPER::result($result);
        }

        $self->_output_ruler( $self->parser->tests_run == 1 );
    }
    elsif ( $result->is_bailout ) {
        $formatter->_failure_output(
                "Bailout called.  Further testing stopped:  "
              . $result->explanation
              . "\n" );
    }
}

=head3 C<clear_for_close>

=cut

sub clear_for_close {
    my $self      = shift;
    my $formatter = $self->formatter;
    return if $formatter->really_quiet;
    my $context = $shared{$formatter};
    if ( @{ $context->{active} } == 1 ) {
        $self->SUPER::clear_for_close;
    }
    else {
        $self->_clear_ruler;
    }
}

=head3 C<close_test>

=cut

sub close_test {
    my $self      = shift;
    my $name      = $self->name;
    my $parser    = $self->parser;
    my $formatter = $self->formatter;
    my $context   = $shared{$formatter};

    $self->SUPER::close_test;

    my $active = $context->{active};

    my @pos = grep { $active->[$_]->name eq $name } 0 .. $#$active;

    die "Can't find myself" unless @pos;
    splice @$active, $pos[0], 1;

    if ( @$active > 1 ) {
        $self->_output_ruler(1);
    }
    elsif ( @$active == 1 ) {

        # Print out "test/name.t ...."
        $active->[0]->SUPER::header;
    }
    else {

        # $self->formatter->_output("\n");
        delete $shared{$formatter};
    }
}

1;
