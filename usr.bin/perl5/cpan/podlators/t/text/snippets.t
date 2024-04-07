#!/usr/bin/perl
#
# Test Pod::Text behavior with various snippets.
#
# Copyright 2002, 2004, 2006-2009, 2012, 2018-2020, 2022
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

use Test::More tests => 67;
use Test::Podlators qw(test_snippet);

# Load the module.
BEGIN {
    use_ok('Pod::Text');
}

# List of snippets run by this test.
my @snippets = qw(
    alt c-with-spaces code cpp empty error-die error-none error-normal
    error-pod error-stderr error-stderr-opt for guesswork-quoting
    guesswork-no-quoting late-encoding link-rt link-url margin naive
    name-quotes name-quotes-none non-latin nonbreaking-space
    nonbreaking-space-l nourls periods quotes-opt s-whitespace
    sentence-spacing utf8 verbatim
);

# Run all the tests.
for my $snippet (@snippets) {
    test_snippet('Pod::Text', "text/$snippet");
}
