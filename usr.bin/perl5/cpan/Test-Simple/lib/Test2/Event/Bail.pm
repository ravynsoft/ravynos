package Test2::Event::Bail;
use strict;
use warnings;

our $VERSION = '1.302194';


BEGIN { require Test2::Event; our @ISA = qw(Test2::Event) }
use Test2::Util::HashBase qw{reason buffered};

# Make sure the tests terminate
sub terminate { 255 };

sub global { 1 };

sub causes_fail { 1 }

sub summary {
    my $self = shift;
    return "Bail out!  " . $self->{+REASON}
        if $self->{+REASON};

    return "Bail out!";
}

sub diagnostics { 1 }

sub facet_data {
    my $self = shift;
    my $out = $self->common_facet_data;

    $out->{control} = {
        global    => 1,
        halt      => 1,
        details   => $self->{+REASON},
        terminate => 255,
    };

    return $out;
}

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::Event::Bail - Bailout!

=head1 DESCRIPTION

The bailout event is generated when things go horribly wrong and you need to
halt all testing in the current file.

=head1 SYNOPSIS

    use Test2::API qw/context/;
    use Test2::Event::Bail;

    my $ctx = context();
    my $event = $ctx->bail('Stuff is broken');

=head1 METHODS

Inherits from L<Test2::Event>. Also defines:

=over 4

=item $reason = $e->reason

The reason for the bailout.

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
