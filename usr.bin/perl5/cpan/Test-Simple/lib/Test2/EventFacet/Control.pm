package Test2::EventFacet::Control;
use strict;
use warnings;

our $VERSION = '1.302194';

BEGIN { require Test2::EventFacet; our @ISA = qw(Test2::EventFacet) }
use Test2::Util::HashBase qw{ -global -terminate -halt -has_callback -encoding -phase };

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::EventFacet::Control - Facet for hub actions and behaviors.

=head1 DESCRIPTION

This facet is used when the event needs to give instructions to the Test2
internals.

=head1 FIELDS

=over 4

=item $string = $control->{details}

=item $string = $control->details()

Human readable explanation for the special behavior.

=item $bool = $control->{global}

=item $bool = $control->global()

True if the event is global in nature and should be seen by all hubs.

=item $exit = $control->{terminate}

=item $exit = $control->terminate()

Defined if the test should immediately exit, the value is the exit code and may
be C<0>.

=item $bool = $control->{halt}

=item $bool = $control->halt()

True if all testing should be halted immediately.

=item $bool = $control->{has_callback}

=item $bool = $control->has_callback()

True if the C<callback($hub)> method on the event should be called.

=item $encoding = $control->{encoding}

=item $encoding = $control->encoding()

This can be used to change the encoding from this event onward.

=item $phase = $control->{phase}

=item $phase = $control->phase()

Used to signal that a phase change has occurred. Currently only the perl END
phase is signaled.

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
