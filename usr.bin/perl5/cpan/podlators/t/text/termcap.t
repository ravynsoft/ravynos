#!/usr/bin/perl
#
# Test Pod::Text::Termcap behavior with various snippets.
#
# Copyright 2002, 2004, 2006, 2009, 2012-2014, 2018-2019, 2022
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

use Test::More tests => 15;
use Test::Podlators qw(test_snippet);

# Load the module.
BEGIN {
    use_ok('Pod::Text::Termcap');
}

# Hard-code a few values to try to get reproducible results.
local $ENV{COLUMNS} = 80;
local $ENV{TERM} = 'xterm';
local $ENV{TERMPATH} = File::Spec->catfile('t', 'data', 'termcap');
local $ENV{TERMCAP} = 'xterm:co=#80:do=^J:md=\\E[1m:us=\\E[4m:me=\\E[m';

# Check the regex that matches a single formatting character.
my $parser = Pod::Text::Termcap->new();
is($parser->format_regex(), "\\\e\\[1m|\\\e\\[4m|\\\e\\[m", 'Character regex');

# List of snippets run by this test.
my @snippets = qw(escape-wrapping tag-width tag-wrapping width wrapping);

# Run all the tests.
for my $snippet (@snippets) {
    test_snippet('Pod::Text::Termcap', "termcap/$snippet");
}

# Now test with an unknown terminal type.
local $ENV{TERM} = 'unknown';
local $ENV{TERMCAP} = 'unknown:co=#80:do=^J';
test_snippet('Pod::Text::Termcap', 'termcap/term-unknown');

# Test the character regex with a fake terminal type that only provides bold
# and normal, not underline.
local $ENV{TERM} = 'fake-test-terminal';
local $ENV{TERMCAP} = 'fake-test-terminal:md=\\E[1m:me=\\E[m';
$parser = Pod::Text::Termcap->new();
is($parser->format_regex(), "\\\e\\[1m|\\\e\\[m", 'Limited character regex');
