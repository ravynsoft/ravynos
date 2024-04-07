package Test2::EventFacet::Hub;
use strict;
use warnings;

our $VERSION = '1.302194';

sub is_list { 1 }
sub facet_key { 'hubs' }

BEGIN { require Test2::EventFacet; our @ISA = qw(Test2::EventFacet) }
use Test2::Util::HashBase qw{-pid -tid -hid -nested -buffered -uuid -ipc};

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::EventFacet::Hub - Facet for the hubs an event passes through.

=head1 DESCRIPTION

These are a record of the hubs an event passes through. Most recent hub is the
first one in the list.

=head1 FACET FIELDS

=over 4

=item $string = $trace->{details}

=item $string = $trace->details()

The hub class or subclass

=item $int = $trace->{pid}

=item $int = $trace->pid()

PID of the hub this event was sent to.

=item $int = $trace->{tid}

=item $int = $trace->tid()

The thread ID of the hub the event was sent to.

=item $hid = $trace->{hid}

=item $hid = $trace->hid()

The ID of the hub that the event was send to.

=item $huuid = $trace->{huuid}

=item $huuid = $trace->huuid()

The UUID of the hub that the event was sent to.

=item $int = $trace->{nested}

=item $int = $trace->nested()

How deeply nested the hub was.

=item $bool = $trace->{buffered}

=item $bool = $trace->buffered()

True if the event was buffered and not sent to the formatter independent of a
parent (This should never be set when nested is C<0> or C<undef>).

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
