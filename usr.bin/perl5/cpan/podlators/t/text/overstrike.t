#!/usr/bin/perl
#
# Test Pod::Text::Overstrike with various snippets.
#
# Copyright 2002, 2004, 2006, 2009, 2012-2013, 2018-2019
#     Russ Allbery <rra@cpan.org>
#
# This program is free software; you may redistribute it and/or modify it
# under the same terms as Perl itself.
#
# SPDX-License-Identifier: GPL-1.0-or-later OR Artistic-1.0-Perl

use 5.008;
use strict;
use warnings;

use lib 't/lib';

use Test::More tests => 5;
use Test::Podlators qw(test_snippet);

BEGIN {
    use_ok('Pod::Text::Overstrike');
}

# List of snippets run by this test.
my @snippets = qw(tag-width wrapping);

# Run all the tests.
for my $snippet (@snippets) {
    test_snippet('Pod::Text::Overstrike', "overstrike/$snippet");
}
