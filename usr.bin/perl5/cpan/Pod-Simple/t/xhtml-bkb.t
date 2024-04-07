#!/usr/bin/perl -w

# t/xhtml-bkb.t - https://rt.cpan.org/Public/Bug/Display.html?id=77686

use strict;
use warnings;
use Test::More tests => 1;
use Pod::Simple::XHTML;
my $c = <<EOF;
=head1 Documentation

=head2 Changes to Existing Documentation

=head3 L<perldata>
EOF
my $d = Pod::Simple::XHTML->new ();
$d->index (1);
my $e;
$d->output_string (\$e);
$d->parse_string_document ($c);
unlike ($e, qr!<a[^>]+><a[^>]+>!);
