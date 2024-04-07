#!/usr/bin/perl
#
# Test for graceful degradation to non-UTF-8 output without Encode module.
#
# Copyright 2016 Niko Tyni <ntyni@iki.fi>
# Copyright 2016, 2018-2019, 2022 Russ Allbery <rra@cpan.org>
#
# This program is free software; you may redistribute it and/or modify it
# under the same terms as Perl itself.
#
# SPDX-License-Identifier: GPL-1.0-or-later OR Artistic-1.0-Perl

use 5.010;
use strict;
use warnings;

use Carp qw(croak);

use Test::More tests => 8;

# Remove the record of the Encode module being loaded if it already was (it
# may have been loaded before the test suite runs), and then make it
# impossible to load it.  This should be enough to trigger the fallback code
# in Pod::Man.
BEGIN {
    delete $INC{'Encode.pm'};
    my $reject_encode = sub {
        if ($_[1] eq 'Encode.pm') {
            croak('refusing to load Encode');
        }
    };
    unshift(@INC, $reject_encode);
    ok(!eval { require Encode }, 'Cannot load Encode any more');
}

# Load the module.
BEGIN {
    use_ok('Pod::Man');
}

# Ensure we don't get warnings by throwing an exception if we see any.  This
# is overridden below when we enable utf8 and do expect a warning.
local $SIG{__WARN__} = sub {
    my @warnings = @_;
    croak(join("\n", 'No warnings expected; instead got:', @warnings));
};

# First, check that everything works properly if an encoding of roff is set.
# We expect to get accent-mangled ASCII output.  Don't use Test::Podlators,
# since it wants to import Encode.
my $invalid_char = chr(utf8::unicode_to_native(0xE9));
my $pod = "=encoding latin1\n\n=head1 NAME\n\nBeyonc${invalid_char}!";
my $parser = Pod::Man->new(name => 'test', encoding => 'roff');
my $output;
$parser->output_string(\$output);
$parser->parse_string_document($pod);
like(
    $output,
    qr{ Beyonce\\[*]\' }xms,
    'Works without Encode for roff encoding',
);

# Likewise for an encoding of groff.
$parser = Pod::Man->new(name => 'test', encoding => 'groff');
my $output_groff = q{};
$parser->output_string(\$output_groff);
$parser->parse_string_document($pod);
like(
    $output_groff,
    qr{ Beyonc\\\[u00E9\] }xms,
    'Works without Encode for groff encoding',
);

# The default output format is UTF-8, so it should produce an error message
# and then degrade to groff.
{
    local $SIG{__WARN__} = sub {
        like(
            $_[0],
            qr{ falling [ ] back [ ] to [ ] groff [ ] escapes }xms,
            'Pod::Man warns with no Encode module',
        );
    };
    $parser = Pod::Man->new(name => 'test');
}
$output = q{};
$parser->output_string(\$output);
$parser->parse_string_document($pod);
is($output, $output_groff, 'Degraded gracefull to groff output');

# Now try with an explicit output encoding, which should produce an error
# message and then degrade to groff.
{
    local $SIG{__WARN__} = sub {
        like(
            $_[0],
            qr{ falling [ ] back [ ] to [ ] groff [ ] escapes }xms,
            'Pod::Man warns with no Encode module',
        );
    };
    $parser = Pod::Man->new(name => 'test', encoding => 'iso-8859-1');
}
$output = q{};
$parser->output_string(\$output);
$parser->parse_string_document($pod);
is(
    $output, $output_groff,
    'Explicit degraded gracefully to groff output',
);
