package Test2::Event::Note;
use strict;
use warnings;

our $VERSION = '1.302194';


BEGIN { require Test2::Event; our @ISA = qw(Test2::Event) }
use Test2::Util::HashBase qw/message/;

sub init {
    $_[0]->{+MESSAGE} = 'undef' unless defined $_[0]->{+MESSAGE};
}

sub summary { $_[0]->{+MESSAGE} }

sub facet_data {
    my $self = shift;

    my $out = $self->common_facet_data;

    $out->{info} = [
        {
            tag     => 'NOTE',
            debug   => 0,
            details => $self->{+MESSAGE},
        }
    ];

    return $out;
}

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::Event::Note - Note event type

=head1 DESCRIPTION

Notes, typically rendered to STDOUT.

=head1 SYNOPSIS

    use Test2::API qw/context/;
    use Test2::Event::Note;

    my $ctx = context();
    my $event = $ctx->Note($message);

=head1 ACCESSORS

=over 4

=item $note->message

The message for the note.

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
