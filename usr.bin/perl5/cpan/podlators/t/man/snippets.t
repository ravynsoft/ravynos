#!/usr/bin/perl
#
# Test Pod::Man behavior with various snippets.
#
# Copyright 2002, 2004, 2006, 2008-2009, 2012-2013, 2015-2016, 2018-2020, 2022
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

use Test::More tests => 113;
use Test::Podlators qw(test_snippet);

# Load the module.
BEGIN {
    use_ok('Pod::Man');
}

# List of snippets run by this test.
my @snippets = qw(
    agrave backslash-man-ref bullet-after-nonbullet bullets c-in-header
    c-in-name dollar-magic error-die error-none error-normal error-pod
    error-stderr error-stderr-opt eth fixed-font fixed-font-in-item for-blocks
    guesswork guesswork-all guesswork-no-quoting guesswork-none
    guesswork-partial guesswork-quoting hyphen-in-s item-fonts language
    link-quoting link-to-url long-quote lquote-and-quote lquote-rquote
    markup-in-name multiline-x naive naive-groff name-guesswork name-quotes
    name-quotes-none nested-lists newlines-in-c non-ascii nonbreaking-space-l
    not-bullet not-numbers nourls periods quote-escaping rquote-none
    soft-hyphens trailing-space true-false x-whitespace x-whitespace-entry
    zero-width-space
);

# Run all the tests.
for my $snippet (@snippets) {
    test_snippet('Pod::Man', "man/$snippet");
}
