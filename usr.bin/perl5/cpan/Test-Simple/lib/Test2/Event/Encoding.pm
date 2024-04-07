package Test2::Event::Encoding;
use strict;
use warnings;

our $VERSION = '1.302194';

use Carp qw/croak/;

BEGIN { require Test2::Event; our @ISA = qw(Test2::Event) }
use Test2::Util::HashBase qw/encoding/;

sub init {
    my $self = shift;
    defined $self->{+ENCODING} or croak "'encoding' is a required attribute";
}

sub summary { 'Encoding set to ' . $_[0]->{+ENCODING} }

sub facet_data {
    my $self = shift;
    my $out = $self->common_facet_data;
    $out->{control}->{encoding} = $self->{+ENCODING};
    $out->{about}->{details} = $self->summary;
    return $out;
}


1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::Event::Encoding - Set the encoding for the output stream

=head1 DESCRIPTION

The encoding event is generated when a test file wants to specify the encoding
to be used when formatting its output. This event is intended to be produced
by formatter classes and used for interpreting test names, message contents,
etc.

=head1 SYNOPSIS

    use Test2::API qw/context/;
    use Test2::Event::Encoding;

    my $ctx = context();
    my $event = $ctx->send_event('Encoding', encoding => 'UTF-8');

=head1 METHODS

Inherits from L<Test2::Event>. Also defines:

=over 4

=item $encoding = $e->encoding

The encoding being specified.

=back

=head1 SOURCE

The source code repository for Test2 can be found at
F<http://github.com/Test-More/test-more/>.

=head1 MAINTAINERS

=over 4

=item Chad Granum E<lt>exodist@cpan.orgE<gt>

=back

=head1 AUTHORS

=over 4

=item Chad Granum E<lt>exodist@cpan.orgE<gt>

=back

=head1 COPYRIGHT

Copyright 2020 Chad Granum E<lt>exodist@cpan.orgE<gt>.

This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

See F<http://dev.perl.org/licenses/>

=cut
