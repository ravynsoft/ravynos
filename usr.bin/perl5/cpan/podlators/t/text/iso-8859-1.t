#!/usr/bin/perl
#
# Test Pod::Text ISO-8859-1 handling
#
# Copyright 2016, 2019, 2022 Russ Allbery <rra@cpan.org>
#
# This program is free software; you may redistribute it and/or modify it
# under the same terms as Perl itself.
#
# SPDX-License-Identifier: GPL-1.0-or-later OR Artistic-1.0-Perl

use 5.008;
use strict;
use warnings;

use lib 't/lib';

use Test::More tests => 13;
use Test::Podlators qw(test_snippet test_snippet_with_io);

# Load the module.
BEGIN {
    use_ok('Pod::Text');
}

# Test the snippet with the proper encoding.
test_snippet('Pod::Text', 'text/iso-8859-1', { encoding => 'iso-8859-1' });

# Test error handling when there are characters that cannot be represented in
# the output character set.
test_snippet('Pod::Text', 'text/iso-8859-1-error-die');
test_snippet('Pod::Text', 'text/iso-8859-1-error-pod');

# Force ISO 8859-1 on all relevant file handles.  Hide this in a string eval
# so that older versions of Perl don't croak and minimum-version tests still
# pass.
#
## no critic (BuiltinFunctions::ProhibitStringyEval)
## no critic (ValuesAndExpressions::RequireInterpolationOfMetachars)
eval 'binmode(\*STDOUT, ":encoding(iso-8859-1)")';
my $builder = Test::More->builder;
eval 'binmode($builder->output, ":encoding(iso-8859-1)")';
eval 'binmode($builder->failure_output, ":encoding(iso-8859-1)")';
## use critic

# Test the snippet with ISO 8859-1 output with a PerlIO layer.
test_snippet_with_io(
    'Pod::Text', 'text/iso-8859-1',
    { encoding => 'iso-8859-1', output => 'iso-8859-1', perlio_iso => 1 },
);

# Test the snippet with ISO 8859-1 input but an encoding forcing output to
# UTF-8.
test_snippet('Pod::Text', 'text/iso-8859-1-utf8');
