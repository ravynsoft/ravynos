#!/usr/bin/perl -w
use strict;
use warnings;
use Pod::Usage;

my $h2 = shift @ARGV || '.*';

Pod::Usage::pod2usage(
  '-verbose' => 99,
  '-exitval' => 0,
  '-sections' => "Name/$h2/!.+",
);

=head1 Name

Testing

=head2 Foo

This is foo

=head3 Foo bar

This is foo bar.

=head2 Bar

This is bar.

=head3 Bar baz

This is bar baz.

=cut

