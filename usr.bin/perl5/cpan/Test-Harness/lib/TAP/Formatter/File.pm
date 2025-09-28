package TAP::Formatter::File;

use strict;
use warnings;
use TAP::Formatter::File::Session;
use POSIX qw(strftime);

use base 'TAP::Formatter::Base';

=head1 NAME

TAP::Formatter::File - Harness output delegate for file output

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

=head1 DESCRIPTION

This provides file orientated output formatting for TAP::Harness.

=head1 SYNOPSIS

 use TAP::Formatter::File;
 my $harness = TAP::Formatter::File->new( \%args );

=head2 C<< open_test >>

See L<TAP::Formatter::Base>

=cut

sub open_test {
    my ( $self, $test, $parser ) = @_;

    my $session = TAP::Formatter::File::Session->new(
        {   name      => $test,
            formatter => $self,
            parser    => $parser,
        }
    );

    $session->header;

    return $session;
}

sub _should_show_count {
    return 0;
}

1;
