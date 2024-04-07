package Test2::EventFacet::Amnesty;
use strict;
use warnings;

our $VERSION = '1.302194';

sub is_list { 1 }

BEGIN { require Test2::EventFacet; our @ISA = qw(Test2::EventFacet) }
use Test2::Util::HashBase qw{ -tag -inherited };

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::EventFacet::Amnesty - Facet for assertion amnesty.

=head1 DESCRIPTION

This package represents what is expected in units of amnesty.

=head1 NOTES

This facet appears in a list instead of being a single item.

=head1 FIELDS

=over 4

=item $string = $amnesty->{details}

=item $string = $amnesty->details()

Human readable explanation of why amnesty was granted.

Example: I<Not implemented yet, will fix>

=item $short_string = $amnesty->{tag}

=item $short_string = $amnesty->tag()

Short string (usually 10 characters or less, not enforced, but may be truncated
by renderers) categorizing the amnesty.

=item $bool = $amnesty->{inherited}

=item $bool = $amnesty->inherited()

This will be true if the amnesty was granted to a parent event and inherited by
this event, which is a child, such as an assertion within a subtest that is
marked todo.

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
