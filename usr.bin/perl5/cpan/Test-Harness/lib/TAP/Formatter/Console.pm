package TAP::Formatter::Console;

use strict;
use warnings;
use base 'TAP::Formatter::Base';
use POSIX qw(strftime);

=head1 NAME

TAP::Formatter::Console - Harness output delegate for default console output

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

=head1 DESCRIPTION

This provides console orientated output formatting for TAP::Harness.

=head1 SYNOPSIS

 use TAP::Formatter::Console;
 my $harness = TAP::Formatter::Console->new( \%args );

=head2 C<< open_test >>

See L<TAP::Formatter::Base>

=cut

sub open_test {
    my ( $self, $test, $parser ) = @_;

    my $class
      = $self->jobs > 1
      ? 'TAP::Formatter::Console::ParallelSession'
      : 'TAP::Formatter::Console::Session';

    eval "require $class";
    $self->_croak($@) if $@;

    my $session = $class->new(
        {   name       => $test,
            formatter  => $self,
            parser     => $parser,
            show_count => $self->show_count,
        }
    );

    $session->header;

    return $session;
}

# Use _colorizer delegate to set output color. NOP if we have no delegate
sub _set_colors {
    my ( $self, @colors ) = @_;
    if ( my $colorizer = $self->_colorizer ) {
        my $output_func = $self->{_output_func} ||= sub {
            $self->_output(@_);
        };
        $colorizer->set_color( $output_func, $_ ) for @colors;
    }
}

sub _failure_color {
    my ($self) = @_;

    return $ENV{'HARNESS_SUMMARY_COLOR_FAIL'} || 'red';
}

sub _success_color {
    my ($self) = @_;

    return $ENV{'HARNESS_SUMMARY_COLOR_SUCCESS'} || 'green';
}

sub _output_success {
    my ( $self, $msg ) = @_;
    $self->_set_colors( $self->_success_color() );
    $self->_output($msg);
    $self->_set_colors('reset');
}

sub _failure_output {
    my $self = shift;
    $self->_set_colors( $self->_failure_color() );
    my $out = join '', @_;
    my $has_newline = chomp $out;
    $self->_output($out);
    $self->_set_colors('reset');
    $self->_output($/)
      if $has_newline;
}

1;
