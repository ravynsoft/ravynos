package Test2::Event::Exception;
use strict;
use warnings;

our $VERSION = '1.302194';


BEGIN { require Test2::Event; our @ISA = qw(Test2::Event) }
use Test2::Util::HashBase qw{error};

sub init {
    my $self = shift;
    $self->{+ERROR} = "$self->{+ERROR}";
}

sub causes_fail { 1 }

sub summary {
    my $self = shift;
    chomp(my $msg = "Exception: " . $self->{+ERROR});
    return $msg;
}

sub diagnostics { 1 }

sub facet_data {
    my $self = shift;
    my $out = $self->common_facet_data;

    $out->{errors} = [
        {
            tag     => 'ERROR',
            fail    => 1,
            details => $self->{+ERROR},
        }
    ];

    return $out;
}


1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::Event::Exception - Exception event

=head1 DESCRIPTION

An exception event will display to STDERR, and will prevent the overall test
file from passing.

=head1 SYNOPSIS

    use Test2::API qw/context/;
    use Test2::Event::Exception;

    my $ctx = context();
    my $event = $ctx->send_event('Exception', error => 'Stuff is broken');

=head1 METHODS

Inherits from L<Test2::Event>. Also defines:

=over 4

=item $reason = $e->error

The reason for the exception.

=back

=head1 CAVEATS

Be aware that all exceptions are stringified during construction.

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
