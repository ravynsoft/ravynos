#!/usr/bin/perl
#
# Test Pod::Man with a document that produces only errors.
#
# Copyright 2013, 2016, 2018-2019, 2022 Russ Allbery <rra@cpan.org>
#
# This program is free software; you may redistribute it and/or modify it
# under the same terms as Perl itself.
#
# SPDX-License-Identifier: GPL-1.0-or-later OR Artistic-1.0-Perl

use 5.010;
use strict;
use warnings;

use Test::More tests => 8;

# Load the module.
BEGIN {
    use_ok('Pod::Man');
}

# Set up Pod::Man to output to a string.
my $parser = Pod::Man->new;
isa_ok($parser, 'Pod::Man');
my $output;
$parser->output_string(\$output);

# Ensure there are no warnings by dying on a warning, forcing a test failure.
local $SIG{__WARN__} = sub { croak($_[0]) };

# Try a POD document where the only command is invalid.  Make sure it succeeds
# and doesn't throw an exception.
my $invalid_char = chr(utf8::unicode_to_native(0xa0));
ok(
    eval { $parser->parse_string_document("=$invalid_char") },
    'Parsed invalid document',
);
is($@, q{}, '...with no errors');

# With recent Pod::Simple, there will be a POD ERRORS section.  With older
# versions of Pod::Simple, we have to skip the test since it doesn't trigger
# this problem.
SKIP: {
    if ($output eq q{}) {
        skip('Pod::Simple does not produce errors for invalid commands', 1);
    }
    like(
        $output,
        qr{ [.]SH [ ] "POD [ ] ERRORS" }xms,
        '...and output contains a POD ERRORS section',
    );
}

# Try with a document containing only =cut.
ok(eval { $parser->parse_string_document('=cut') }, 'Parsed =cut document');
is($@, q{}, '...with no errors');

# Same check for a POD ERRORS section.
SKIP: {
    if ($output eq q{}) {
        skip('Pod::Simple does not produce errors for invalid commands', 1);
    }
    like(
        $output,
        qr{ [.]SH [ ] "POD [ ] ERRORS" }xms,
        '...and output contains a POD ERRORS section',
    );
}
