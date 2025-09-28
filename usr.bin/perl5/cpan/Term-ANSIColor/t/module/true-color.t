#!/usr/bin/perl
#
# Tests for true color support (24-bit color).
#
# Copyright 2020 Russ Allbery <rra@cpan.org>
#
# SPDX-License-Identifier: GPL-1.0-or-later OR Artistic-1.0-Perl

use 5.008;
use strict;
use warnings;

use Test::More tests => 82;

# Load the module.
BEGIN {
    delete $ENV{ANSI_COLORS_ALIASES};
    delete $ENV{ANSI_COLORS_DISABLED};
    delete $ENV{NO_COLOR};
    use_ok('Term::ANSIColor', qw(color uncolor colorvalid));
}

# Test basic true color codes.
is(color('r0g0b0'),          "\e[38;2;0;0;0m",       'foreground 0 0 0');
is(color('r000g000b000'),    "\e[38;2;0;0;0m",       'foreground 000 000 000');
is(color('r255g0b0'),        "\e[38;2;255;0;0m",     'foreground 255 0 0');
is(color('r0g255b0'),        "\e[38;2;0;255;0m",     'foreground 255 0 0');
is(color('r0g0b255'),        "\e[38;2;0;0;255m",     'foreground 255 0 0');
is(color('r255g255b255'),    "\e[38;2;255;255;255m", 'foreground 255 255 255');
is(color('r1g02b003'),       "\e[38;2;1;2;3m",       'foreground 1 02 003');
is(color('on_r0g0b0'),       "\e[48;2;0;0;0m",       'background 0 0 0');
is(color('on_r000g000b000'), "\e[48;2;0;0;0m",       'background 000 000 000');
is(color('on_r255g0b0'),     "\e[48;2;255;0;0m",     'background 255 0 0');
is(color('on_r0g255b0'),     "\e[48;2;0;255;0m",     'background 255 0 0');
is(color('on_r0g0b255'),     "\e[48;2;0;0;255m",     'background 255 0 0');
is(color('on_r255g255b255'), "\e[48;2;255;255;255m", 'background 255 255 255');
is(color('on_r1g02b003'),    "\e[48;2;1;2;3m",       'background 1 02 003');

# Check that various true color codes are valid.
my @valid = qw(
  r0g0b0 r255g255b255 r1g02b003 on_r0g0b0 on_r255g255b255 on_r1g02b003
);
for my $color (@valid) {
    ok(colorvalid($color), "Color $color is valid");
}

# Errors at boundary cases.
my @invalid = qw(
  r0g0 r256g0b0 r0g256b0 r0g0b256 r1000g2b3 rgb r1g2b r1gb2 r1b2g3
);
for my $color (@invalid) {
    my $output = eval { color($color) };
    is($output, undef, 'color on an invalid attribute fails');
    like(
        $@,
        qr{ \A Invalid [ ] attribute [ ] name [ ] \Q$color\E [ ] at [ ] }xms,
        '...with the right error'
    );
    ok(!colorvalid($color), '...and colorvalid says it is invalid');
}

# Check uncolor with true color codes.
is_deeply([uncolor('38;2;0;0;0')],  ['r0g0b0'],    'uncolor of r0g0b0');
is_deeply([uncolor('48;02;0;0;0')], ['on_r0g0b0'], 'uncolor of on_r0g0b0');
is_deeply([uncolor("\e[038;2;255;255;255")],
    ['r255g255b255'], 'uncolor of r255g255b255');
is_deeply([uncolor("\e[48;002;255;255;255")],
    ['on_r255g255b255'], 'uncolor of on_r255g255b255');
is_deeply(
    [uncolor("\e[1;38;2;1;02;003;5;48;2;4;5;6m")],
    [qw(bold r1g2b3 blink on_r4g5b6)],
    'uncolor of a complex escape',
);
is_deeply(
    [uncolor("\e[1;38;2;1;02;003;5;48;5;230m")],
    [qw(bold r1g2b3 blink on_rgb554)],
    'uncolor mixing true-color and 256-color',
);

# An invalid true-color code should report an error on the part that makes it
# invalid.  Check truncated codes (should report on the 38 or 48), codes with
# an invalid second part (likewise), and codes with an invalid third part
# (should report the complete code).
#
# This is a hash of test escape sequences to the invalid sequence that should
# be reported.
my %uncolor_tests = (
    "\e[38;1m"             => 38,
    "\e[38;2m"             => 38,
    "\e[38;2;255;0m"       => 38,
    "\e[38;2;256;0;0m"     => '38;2;256;0;0',
    "\e[38;2;0;256;0m"     => '38;2;0;256;0',
    "\e[38;2;0;0;256m"     => '38;2;0;0;256',
    "\e[38;2;777;777;777m" => '38;2;777;777;777',
    "\e[48;1m"             => 48,
    "\e[48;2m"             => 48,
    "\e[48;2;255;0m"       => 48,
    "\e[48;2;256;0;0m"     => '48;2;256;0;0',
    "\e[48;2;0;256;0m"     => '48;2;0;256;0',
    "\e[48;2;0;0;256m"     => '48;2;0;0;256',
    "\e[48;2;777;777;777m" => '48;2;777;777;777',
);
while (my ($escape, $invalid) = each(%uncolor_tests)) {
    my $output = eval { uncolor($escape) };
    is($output, undef, "uncolor on unknown color code \Q$escape\E fails");
    like(
        $@,
        qr{ \A No [ ] name [ ] for [ ] escape [ ] sequence [ ] \Q$invalid\E
            [ ] at [ ] }xms,
        '...with the right error'
    );
}
