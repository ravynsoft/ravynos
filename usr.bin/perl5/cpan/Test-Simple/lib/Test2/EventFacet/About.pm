package Test2::EventFacet::About;
use strict;
use warnings;

our $VERSION = '1.302194';

BEGIN { require Test2::EventFacet; our @ISA = qw(Test2::EventFacet) }
use Test2::Util::HashBase qw{ -package -no_display -uuid -eid };

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::EventFacet::About - Facet with event details.

=head1 DESCRIPTION

This facet has information about the event, such as event package.

=head1 FIELDS

=over 4

=item $string = $about->{details}

=item $string = $about->details()

Summary about the event.

=item $package = $about->{package}

=item $package = $about->package()

Event package name.

=item $bool = $about->{no_display}

=item $bool = $about->no_display()

True if the event should be skipped by formatters.

=item $uuid = $about->{uuid}

=item $uuid = $about->uuid()

Will be set to a uuid if uuid tagging was enabled.

=item $uuid = $about->{eid}

=item $uuid = $about->eid()

A unique (for the test job) identifier for the event.

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
