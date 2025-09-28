package Test2::EventFacet::Parent;
use strict;
use warnings;

our $VERSION = '1.302194';

use Carp qw/confess/;

BEGIN { require Test2::EventFacet; our @ISA = qw(Test2::EventFacet) }
use Test2::Util::HashBase qw{-hid -children -buffered -start_stamp -stop_stamp};

sub init {
    confess "Attribute 'hid' must be set"
        unless defined $_[0]->{+HID};

    $_[0]->{+CHILDREN} ||= [];
}

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::EventFacet::Parent - Facet for events contains other events

=head1 DESCRIPTION

This facet is used when an event contains other events, such as a subtest.

=head1 FIELDS

=over 4

=item $string = $parent->{details}

=item $string = $parent->details()

Human readable description of the event.

=item $hid = $parent->{hid}

=item $hid = $parent->hid()

Hub ID of the hub that is represented in the parent-child relationship.

=item $arrayref = $parent->{children}

=item $arrayref = $parent->children()

Arrayref containing the facet-data hashes of events nested under this one.

I<To get the actual events you need to get them from the parent event directly>

=item $bool = $parent->{buffered}

=item $bool = $parent->buffered()

True if the subtest is buffered (meaning the formatter has probably not seen
them yet).

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
