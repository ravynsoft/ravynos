package Test2::Event::Waiting;
use strict;
use warnings;

our $VERSION = '1.302194';


BEGIN { require Test2::Event; our @ISA = qw(Test2::Event) }
use Test2::Util::HashBase;

sub global { 1 };

sub summary { "IPC is waiting for children to finish..." }

sub facet_data {
    my $self = shift;

    my $out = $self->common_facet_data;

    push @{$out->{info}} => {
        tag     => 'INFO',
        debug   => 0,
        details => $self->summary,
    };

    return $out;
}

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::Event::Waiting - Tell all procs/threads it is time to be done

=head1 DESCRIPTION

This event has no data of its own. This event is sent out by the IPC system
when the main process/thread is ready to end.

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
