package TAP::Parser::Result::Unknown;

use strict;
use warnings;

use base 'TAP::Parser::Result';

=head1 NAME

TAP::Parser::Result::Unknown - Unknown result token.

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

=head1 DESCRIPTION

This is a subclass of L<TAP::Parser::Result>.  A token of this class will be
returned if the parser does not recognize the token line.  For example:

 1..5
 VERSION 7
 ok 1 - woo hooo!
 ... woo hooo! is cool!

In the above "TAP", the second and fourth lines will generate "Unknown"
tokens.

=head1 OVERRIDDEN METHODS

Mainly listed here to shut up the pitiful screams of the pod coverage tests.
They keep me awake at night.

=over 4

=item * C<as_string>

=item * C<raw>

=back

=cut

1;
