package Test2::EventFacet::Error;
use strict;
use warnings;

our $VERSION = '1.302194';

sub facet_key { 'errors' }
sub is_list { 1 }

BEGIN { require Test2::EventFacet; our @ISA = qw(Test2::EventFacet) }
use Test2::Util::HashBase qw{ -tag -fail };

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::EventFacet::Error - Facet for errors that need to be shown.

=head1 DESCRIPTION

This facet is used when an event needs to convey errors.

=head1 NOTES

This facet has the hash key C<'errors'>, and is a list of facets instead of a
single item.

=head1 FIELDS

=over 4

=item $string = $error->{details}

=item $string = $error->details()

Explanation of the error, or the error itself (such as an exception). In perl
exceptions may be blessed objects, so this field may contain a blessed object.

=item $short_string = $error->{tag}

=item $short_string = $error->tag()

Short tag to categorize the error. This is usually 10 characters or less,
formatters may truncate longer tags.

=item $bool = $error->{fail}

=item $bool = $error->fail()

Not all errors are fatal, some are displayed having already been handled. Set
this to true if you want the error to cause the test to fail. Without this the
error is simply a diagnostics message that has no effect on the overall
pass/fail result.

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
