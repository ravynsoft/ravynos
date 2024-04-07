package Test2::EventFacet::Render;
use strict;
use warnings;

our $VERSION = '1.302194';

sub is_list { 1 }

BEGIN { require Test2::EventFacet; our @ISA = qw(Test2::EventFacet) }
use Test2::Util::HashBase qw{ -tag -facet -mode };

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::EventFacet::Render - Facet that dictates how to render an event.

=head1 DESCRIPTION

This facet is used to dictate how the event should be rendered by the standard
test2 rendering tools. If this facet is present then ONLY what is specified by
it will be rendered. It is assumed that anything important or note-worthy will
be present here, no other facets will be considered for rendering/display.

This facet is a list type, you can add as many items as needed.

=head1 FIELDS

=over 4

=item $string = $render->[#]->{details}

=item $string = $render->[#]->details()

Human readable text for display.

=item $string = $render->[#]->{tag}

=item $string = $render->[#]->tag()

Tag that should prefix/identify the main text.

=item $string = $render->[#]->{facet}

=item $string = $render->[#]->facet()

Optional, if the display text was generated from another facet this should
state what facet it was.

=item $mode = $render->[#]->{mode}

=item $mode = $render->[#]->mode()

=over 4

=item calculated

Calculated means the facet was generated from another facet. Calculated facets
may be cleared and regenerated whenever the event state changes.

=item replace

Replace means the facet is intended to replace the normal rendering of the
event.

=back

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
