package TAP::Formatter::Color;

use strict;
use warnings;

use constant IS_WIN32 => ( $^O =~ /^(MS)?Win32$/ );

use base 'TAP::Object';

my $NO_COLOR;

BEGIN {
    $NO_COLOR = 0;

    eval 'require Term::ANSIColor';
    if ($@) {
        $NO_COLOR = $@;
    };
    if (IS_WIN32) {
        eval 'use Win32::Console::ANSI';
        if ($@) {
            $NO_COLOR = $@;
        }
    };

    if ($NO_COLOR) {
        *set_color = sub { };
    } else {
        *set_color = sub {
            my ( $self, $output, $color ) = @_;
            $output->( Term::ANSIColor::color($color) );
        };
    }
}

=head1 NAME

TAP::Formatter::Color - Run Perl test scripts with color

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

=head1 DESCRIPTION

Note that this harness is I<experimental>.  You may not like the colors I've
chosen and I haven't yet provided an easy way to override them.

This test harness is the same as L<TAP::Harness>, but test results are output
in color.  Passing tests are printed in green.  Failing tests are in red.
Skipped tests are blue on a white background and TODO tests are printed in
white.

If L<Term::ANSIColor> cannot be found (and L<Win32::Console::ANSI> if running
under Windows) tests will be run without color.

=head1 SYNOPSIS

 use TAP::Formatter::Color;
 my $harness = TAP::Formatter::Color->new( \%args );
 $harness->runtests(@tests);

=head1 METHODS

=head2 Class Methods

=head3 C<new>

The constructor returns a new C<TAP::Formatter::Color> object. If
L<Term::ANSIColor> is not installed, returns undef.

=cut

# new() implementation supplied by TAP::Object

sub _initialize {
    my $self = shift;

    if ($NO_COLOR) {

        # shorten that message a bit
        ( my $error = $NO_COLOR ) =~ s/ in \@INC .*//s;
        warn "Note: Cannot run tests in color: $error\n";
        return;    # abort object construction
    }

    return $self;
}

##############################################################################

=head3 C<can_color>

  Test::Formatter::Color->can_color()

Returns a boolean indicating whether or not this module can actually
generate colored output. This will be false if it could not load the
modules needed for the current platform.

=cut

sub can_color {
    return !$NO_COLOR;
}

=head3 C<set_color>

Set the output color.

=cut

1;
