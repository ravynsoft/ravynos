package Test2::EventFacet;
use strict;
use warnings;

our $VERSION = '1.302194';

use Test2::Util::HashBase qw/-details/;
use Carp qw/croak/;

my $SUBLEN = length(__PACKAGE__ . '::');
sub facet_key {
    my $key = ref($_[0]) || $_[0];
    substr($key, 0, $SUBLEN, '');
    return lc($key);
}

sub is_list { 0 }

sub clone {
    my $self = shift;
    my $type = ref($self);
    return bless {%$self, @_}, $type;
}

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::EventFacet - Base class for all event facets.

=head1 DESCRIPTION

Base class for all event facets.

=head1 METHODS

=over 4

=item $key = $facet_class->facet_key()

This will return the key for the facet in the facet data hash.

=item $bool = $facet_class->is_list()

This will return true if the facet should be in a list instead of a single
item.

=item $clone = $facet->clone()

=item $clone = $facet->clone(%replace)

This will make a shallow clone of the facet. You may specify fields to override
as arguments.

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
