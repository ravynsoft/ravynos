#!/usr/bin/perl
#
# Tests for 256-color support.
#
# Copyright 2012 Kurt Starsinic <kstarsinic@gmail.com>
# Copyright 2012-2013, 2016, 2020 Russ Allbery <rra@cpan.org>
#
# SPDX-License-Identifier: GPL-1.0-or-later OR Artistic-1.0-Perl

use 5.008;
use strict;
use warnings;

use Test::More tests => 100;

# Load the module.
BEGIN {
    delete $ENV{ANSI_COLORS_ALIASES};
    delete $ENV{ANSI_COLORS_DISABLED};
    delete $ENV{NO_COLOR};
    use_ok('Term::ANSIColor', qw(:constants256 color uncolor colorvalid));
}

# Test basic 256-color codes.
is(color('ansi0'),  "\e[38;5;0m",   'ANSI 0');
is(color('ansi15'), "\e[38;5;15m",  'ANSI 15');
is(color('rgb000'), "\e[38;5;16m",  'RGB 000');
is(color('rgb555'), "\e[38;5;231m", 'RGB 555');
is(color('grey0'),  "\e[38;5;232m", 'Grey 0');
is(color('grey23'), "\e[38;5;255m", 'Grey 23');

# Errors at boundary cases.
for my $color (qw(ansi256 rgb600 rgb060 rgb006 rgb666 rgb999 rgb0000 grey24)) {
    my $output = eval { color($color) };
    is($output, undef, 'color on unknown color name fails');
    like(
        $@,
        qr{ \A Invalid [ ] attribute [ ] name [ ] \Q$color\E [ ] at [ ] }xms,
        '...with the right error'
    );
    ok(!colorvalid($color), '...and colorvalid says it is invalid');
}

# Check that various 256-color codes are valid.
for my $color (qw(ansi0 ansi15 rgb000 rgb555 grey0 grey23)) {
    ok(colorvalid($color), "Color $color is valid");
}

# Check uncolor with 256-color codes.
is_deeply([uncolor('38;5;0')],        ['ansi0'],    'uncolor of ansi0');
is_deeply([uncolor("\e[38;5;231m")],  ['rgb555'],   'uncolor of rgb555');
is_deeply([uncolor("\e[48;05;001m")], ['on_ansi1'], 'uncolor with leading 0s');
is_deeply([uncolor("\e[38;5;233")],   ['grey1'],    'uncolor of grey1');

# An invalid 256-color code should report an error on the part that makes it
# invalid.  Check truncated codes (should report on the 38 or 48), codes with
# an invalid second part (likewise), and codes with an invalid third part
# (should report the complete code).
#
# This is a hash of test escape sequences to the invalid sequence that should
# be reported.
my %uncolor_tests = (
    "\e[38m"       => 38,
    "\e[38;5m"     => 38,
    "\e[38;5;256m" => '38;5;256',
    "\e[38;5;777m" => '38;5;777',
    "\e[48m"       => 48,
    "\e[48;5m"     => 48,
    "\e[48;5;256m" => '48;5;256',
    "\e[48;5;777m" => '48;5;777',
);
while (my ($escape, $invalid) = each %uncolor_tests) {
    my $output = eval { uncolor($escape) };
    is($output, undef, "uncolor on unknown color code \Q$escape\E fails");
    like(
        $@,
        qr{ \A No [ ] name [ ] for [ ] escape [ ] sequence [ ] \Q$invalid\E
            [ ] at [ ] }xms,
        '...with the right error'
    );
}

# Test all the variations of a few different constants.
is((ANSI0 't'),   "\e[38;5;0mt",   'Basic constant works for ANSI0');
is((ANSI15 't'),  "\e[38;5;15mt",  '...and for ANSI15');
is((ANSI255 't'), "\e[38;5;255mt", '...and for ANSI255');
is((RGB000 't'),  "\e[38;5;16mt",  '...and for RGB000');
is((RGB555 't'),  "\e[38;5;231mt", '...and for RGB555');
is((GREY0 't'),   "\e[38;5;232mt", '...and for GREY0');
is((GREY23 't'),  "\e[38;5;255mt", '...and for GREY23');

# Do the same for disabled colors.
local $ENV{ANSI_COLORS_DISABLED} = 1;
is(ANSI0,  q{}, 'ANSI_COLORS_DISABLED works for ANSI0');
is(ANSI15, q{}, '...and for ANSI15');
is(RGB000, q{}, '...and for RGB000');
is(RGB555, q{}, '...and for RGB555');
is(GREY0,  q{}, '...and for GREY0');
is(GREY23, q{}, '...and for GREY23');
delete $ENV{ANSI_COLORS_DISABLED};

# Do the same with NO_COLOR.
local $ENV{NO_COLOR} = 0;
is(ANSI0,  q{}, 'NO_COLOR works for ANSI0');
is(ANSI15, q{}, '...and for ANSI15');
is(RGB000, q{}, '...and for RGB000');
is(RGB555, q{}, '...and for RGB555');
is(GREY0,  q{}, '...and for GREY0');
is(GREY23, q{}, '...and for GREY23');
delete $ENV{NO_COLOR};

# Do the same for AUTORESET.
$Term::ANSIColor::AUTORESET = 1;
is((ANSI0 't'),  "\e[38;5;0mt\e[0m",   'AUTORESET works for ANSI0');
is((ANSI15 't'), "\e[38;5;15mt\e[0m",  '...and for ANSI15');
is((RGB000 't'), "\e[38;5;16mt\e[0m",  '...and for RGB000');
is((RGB555 't'), "\e[38;5;231mt\e[0m", '...and for RGB555');
is((GREY0 't'),  "\e[38;5;232mt\e[0m", '...and for GREY0');
is((GREY23 't'), "\e[38;5;255mt\e[0m", '...and for GREY23');
is((ANSI0),      "\e[38;5;0m",         'AUTORESET without text for ANSI0');
is((ANSI15),     "\e[38;5;15m",        '...and for ANSI15');
is((RGB000),     "\e[38;5;16m",        '...and for RGB000');
is((RGB555),     "\e[38;5;231m",       '...and for RGB555');
is((GREY0),      "\e[38;5;232m",       '...and for GREY0');
is((GREY23),     "\e[38;5;255m",       '...and for GREY23');
$Term::ANSIColor::AUTORESET = 0;

# Do the same for AUTOLOCAL.
$Term::ANSIColor::AUTOLOCAL = 1;
is((ANSI0 't'),  "\e[38;5;0mt\e[0m",   'AUTOLOCAL works for ANSI0');
is((ANSI15 't'), "\e[38;5;15mt\e[0m",  '...and for ANSI15');
is((RGB000 't'), "\e[38;5;16mt\e[0m",  '...and for RGB000');
is((RGB555 't'), "\e[38;5;231mt\e[0m", '...and for RGB555');
is((GREY0 't'),  "\e[38;5;232mt\e[0m", '...and for GREY0');
is((GREY23 't'), "\e[38;5;255mt\e[0m", '...and for GREY23');
is((ANSI0),      "\e[38;5;0m",         'AUTOLOCAL without text for ANSI0');
is((ANSI15),     "\e[38;5;15m",        '...and for ANSI15');
is((RGB000),     "\e[38;5;16m",        '...and for RGB000');
is((RGB555),     "\e[38;5;231m",       '...and for RGB555');
is((GREY0),      "\e[38;5;232m",       '...and for GREY0');
is((GREY23),     "\e[38;5;255m",       '...and for GREY23');
$Term::ANSIColor::AUTOLOCAL = 0;
