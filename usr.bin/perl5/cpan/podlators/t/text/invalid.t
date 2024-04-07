#!/usr/bin/perl
#
# Test Pod::Text with a document that produces only errors.
#
# Documents with only errors were shown as contentless but had a POD ERRORS
# section, which previously led to internal errors because state variables
# weren't properly initialized.  See CPAN RT #88724.
#
# Copyright 2013, 2018, 2020, 2022 Russ Allbery <rra@cpan.org>
#
# This program is free software; you may redistribute it and/or modify it
# under the same terms as Perl itself.
#
# SPDX-License-Identifier: GPL-1.0-or-later OR Artistic-1.0-Perl

use 5.010;
use strict;
use warnings;

use Test::More tests => 8;

BEGIN {
    use_ok('Pod::Text');
}

# Set up Pod::Text to output to a string.
my $parser = Pod::Text->new;
isa_ok($parser, 'Pod::Text');
my $output;
$parser->output_string(\$output);

# Ensure any warnings cause a test failure.
## no critic (ErrorHandling::RequireCarping)
local $SIG{__WARN__} = sub { die $_[0] };

# Parse a document provided as a string, ensure that it doesn't produce any
# warnings or errors, and check that it either contains no content or a POD
# ERRORS section.
#
# $document - Document to parse
# $name     - Name of the test
sub check_document {
    my ($document, $name) = @_;
    my $result = eval { $parser->parse_string_document($document) };
    ok($result, "Parsed $name");
    is($@, q{}, 'No exceptions');
    if ($output eq q{}) {
        # Older Pod::Simple doesn't always produce errors.
        ok(1, 'Output is empty');
    } else {
        like($output, qr{POD [ ] ERRORS}xms, 'Output contains POD ERRORS');
    }
    return;
}

# Document whose only content is an invalid command.
my $invalid_char = chr(utf8::unicode_to_native(0xa0));
check_document("=$invalid_char", 'invalid command');

# Document containing only a =cut.
check_document('=cut', 'document with only =cut');
