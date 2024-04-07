package TAP::Formatter::Console::Session;

use strict;
use warnings;

use base 'TAP::Formatter::Session';

my @ACCESSOR;

BEGIN {
    my @CLOSURE_BINDING = qw( header result clear_for_close close_test );

    for my $method (@CLOSURE_BINDING) {
        no strict 'refs';
        *$method = sub {
            my $self = shift;
            return ( $self->{_closures} ||= $self->_closures )->{$method}
              ->(@_);
        };
    }
}

=head1 NAME

TAP::Formatter::Console::Session - Harness output delegate for default console output

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

=head1 DESCRIPTION

This provides console orientated output formatting for TAP::Harness.

=cut

sub _get_output_result {
    my $self = shift;

    my @color_map = (
        {   test => sub { $_->is_test && !$_->is_ok },
            colors => ['red'],
        },
        {   test => sub { $_->is_test && $_->has_skip },
            colors => [
                'white',
                'on_blue'
            ],
        },
        {   test => sub { $_->is_test && $_->has_todo },
            colors => ['yellow'],
        },
    );

    my $formatter = $self->formatter;
    my $parser    = $self->parser;

    return $formatter->_colorizer
      ? sub {
        my $result = shift;
        for my $col (@color_map) {
            local $_ = $result;
            if ( $col->{test}->() ) {
                $formatter->_set_colors( @{ $col->{colors} } );
                last;
            }
        }
        $formatter->_output( $self->_format_for_output($result) );
        $formatter->_set_colors('reset');
      }
      : sub {
        $formatter->_output( $self->_format_for_output(shift) );
      };
}

sub _closures {
    my $self = shift;

    my $parser     = $self->parser;
    my $formatter  = $self->formatter;
    my $pretty     = $formatter->_format_name( $self->name );
    my $show_count = $self->show_count;

    my $really_quiet = $formatter->really_quiet;
    my $quiet        = $formatter->quiet;
    my $verbose      = $formatter->verbose;
    my $directives   = $formatter->directives;
    my $failures     = $formatter->failures;
    my $comments     = $formatter->comments;

    my $output_result = $self->_get_output_result;

    my $output          = '_output';
    my $plan            = '';
    my $newline_printed = 0;

    my $last_status_printed = 0;

    return {
        header => sub {
            $formatter->_output($pretty)
              unless $really_quiet;
        },

        result => sub {
            my $result = shift;

            if ( $result->is_bailout ) {
                $formatter->_failure_output(
                        "Bailout called.  Further testing stopped:  "
                      . $result->explanation
                      . "\n" );
            }

            return if $really_quiet;

            my $is_test = $result->is_test;

            # These are used in close_test - but only if $really_quiet
            # is false - so it's safe to only set them here unless that
            # relationship changes.

            if ( !$plan ) {
                my $planned = $parser->tests_planned || '?';
                $plan = "/$planned ";
            }
            $output = $formatter->_get_output_method($parser);

            if ( $show_count and $is_test ) {
                my $number = $result->number;
                my $now    = CORE::time;

                # Print status roughly once per second.
                # We will always get the first number as a side effect of
                # $last_status_printed starting with the value 0, which $now
                # will never be. (Unless someone sets their clock to 1970)
                if ( $last_status_printed != $now ) {
                    $formatter->$output("\r$pretty$number$plan");
                    $last_status_printed = $now;
                }
            }

            if (!$quiet
                && (   $verbose
                    || ( $is_test && $failures && !$result->is_ok )
                    || ( $comments   && $result->is_comment )
                    || ( $directives && $result->has_directive ) )
              )
            {
                unless ($newline_printed) {
                    $formatter->_output("\n");
                    $newline_printed = 1;
                }
                $output_result->($result);
                $formatter->_output("\n");
            }
        },

        clear_for_close => sub {
            my $spaces
              = ' ' x length( '.' . $pretty . $plan . $parser->tests_run );
            $formatter->$output("\r$spaces");
        },

        close_test => sub {
            if ( $show_count && !$really_quiet ) {
                $self->clear_for_close;
                $formatter->$output("\r$pretty");
            }

            # Avoid circular references
            $self->parser(undef);
            $self->{_closures} = {};

            return if $really_quiet;

            if ( my $skip_all = $parser->skip_all ) {
                $formatter->_output("skipped: $skip_all\n");
            }
            elsif ( $parser->has_problems ) {
                $self->_output_test_failure($parser);
            }
            else {
                my $time_report = $self->time_report($formatter, $parser);
                $formatter->_output( $self->_make_ok_line($time_report) );
            }
        },
    };
}

=head2 C<< 	clear_for_close >>

=head2 C<< 	close_test >>

=head2 C<< 	header >>

=head2 C<< 	result >>

=cut

1;
