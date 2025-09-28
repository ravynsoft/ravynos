#!/usr/bin/perl
#
# Test Pod::Text UTF-8 handling, with and without PerlIO.
#
# Copyright 2002, 2004, 2006-2010, 2012, 2014, 2018, 2020, 2022
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

use Test::More tests => 17;
use Test::Podlators qw(test_snippet_with_io);

BEGIN {
    use_ok('Pod::Text');
}

# Force UTF-8 on all relevant file handles.  Hide this in a string eval so
# that older versions of Perl don't croak and minimum-version tests still
# pass.
#
## no critic (BuiltinFunctions::ProhibitStringyEval)
## no critic (ValuesAndExpressions::RequireInterpolationOfMetachars)
eval 'binmode(\*STDOUT, ":encoding(utf-8)")';
my $builder = Test::More->builder;
eval 'binmode($builder->output, ":encoding(utf-8)")';
eval 'binmode($builder->failure_output, ":encoding(utf-8)")';
## use critic

# For each of the UTF-8 snippets, check them with and without PerlIO layers.
for my $snippet (qw(late-encoding s-whitespace utf8)) {
    test_snippet_with_io('Pod::Text', "text/$snippet");
    test_snippet_with_io('Pod::Text', "text/$snippet", { perlio_utf8 => 1 });
}

# Load a snippet in ISO 8859-1 that forces the output to be in UTF-8.
test_snippet_with_io(
    'Pod::Text', 'text/utf8-iso',
    { encoding => 'iso-8859-1' },
);
test_snippet_with_io(
    'Pod::Text', 'text/utf8-iso',
    { encoding => 'iso-8859-1', perlio_utf8 => 1, output => 'utf-8' },
);
