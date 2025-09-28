package Test2::EventFacet::Plan;
use strict;
use warnings;

our $VERSION = '1.302194';

BEGIN { require Test2::EventFacet; our @ISA = qw(Test2::EventFacet) }
use Test2::Util::HashBase qw{ -count -skip -none };

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::EventFacet::Plan - Facet for setting the plan

=head1 DESCRIPTION

Events use this facet when they need to set the plan.

=head1 FIELDS

=over 4

=item $string = $plan->{details}

=item $string = $plan->details()

Human readable explanation for the plan being set. This is normally not
rendered by most formatters except when the C<skip> field is also set.

=item $positive_int = $plan->{count}

=item $positive_int = $plan->count()

Set the number of expected assertions. This should usually be set to C<0> when
C<skip> or C<none> are also set.

=item $bool = $plan->{skip}

=item $bool = $plan->skip()

When true the entire test should be skipped. This is usually paired with an
explanation in the C<details> field, and a C<control> facet that has
C<terminate> set to C<0>.

=item $bool = $plan->{none}

=item $bool = $plan->none()

This is mainly used by legacy L<Test::Builder> tests which set the plan to C<no
plan>, a construct that predates the much better C<done_testing()>.

If you are using this in non-legacy code you may need to reconsider the course
of your life, maybe a hermitage would suite you?

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
