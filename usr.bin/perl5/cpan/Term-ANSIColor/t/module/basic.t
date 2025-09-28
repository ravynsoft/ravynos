#!/usr/bin/perl
#
# Basic test suite for the Term::ANSIColor Perl module.
#
# Copyright 1997-1998, 2000-2002, 2005-2006, 2009-2010, 2012, 2014, 2020
#     Russ Allbery <rra@cpan.org>
#
# SPDX-License-Identifier: GPL-1.0-or-later OR Artistic-1.0-Perl

use 5.008;
use strict;
use warnings;

use Test::More tests => 169;

# Load the module.
BEGIN {
    delete $ENV{ANSI_COLORS_ALIASES};
    delete $ENV{ANSI_COLORS_DISABLED};
    delete $ENV{NO_COLOR};
    use_ok('Term::ANSIColor',
        qw(:pushpop color colored uncolor colorstrip colorvalid));
}

# Various basic tests.
is(color('blue on_green', 'bold'), "\e[34;42;1m", 'Simple attributes');
is(colored('testing', 'blue', 'bold'), "\e[34;1mtesting\e[0m", 'colored');
is((BLUE BOLD 'testing'),              "\e[34m\e[1mtesting",   'Constants');
is(join(q{}, BLUE, BOLD, 'testing'),
    "\e[34m\e[1mtesting", 'Constants with commas');
is((BLUE 'test', 'ing'), "\e[34mtesting", 'Constants with multiple strings');

# Test case variations on attributes.
is(color('Blue BOLD', 'on_GReeN'), "\e[34;1;42m", 'Attribute case');

# color should return undef if there were no attributes.
is(color(), undef, 'color returns undef with no attributes');

# Autoreset after the end of a command string.
$Term::ANSIColor::AUTORESET = 1;
is((BLUE BOLD 'testing'), "\e[34m\e[1mtesting\e[0m\e[0m", 'AUTORESET');
is((BLUE BOLD, 'te', 'st'), "\e[34m\e[1mtest\e[0m", 'AUTORESET with commas');
$Term::ANSIColor::AUTORESET = 0;

# Reset after each line terminator.
$Term::ANSIColor::EACHLINE = "\n";
is(colored("test\n\ntest", 'bold'),
    "\e[1mtest\e[0m\n\n\e[1mtest\e[0m", 'EACHLINE');
$Term::ANSIColor::EACHLINE = "\r\n";
is(
    colored("test\ntest\r\r\n\r\n", 'bold'),
    "\e[1mtest\ntest\r\e[0m\r\n\r\n",
    'EACHLINE with multiple delimiters'
);
$Term::ANSIColor::EACHLINE = "\n";
is(
    colored(['bold', 'on_green'], "test\n", "\n", 'test'),
    "\e[1;42mtest\e[0m\n\n\e[1;42mtest\e[0m",
    'colored with reference to array'
);

# Basic tests for uncolor.
is_deeply([uncolor('1;42', "\e[m", q{}, "\e[0m")],
    [qw(bold on_green clear)], 'uncolor');
is_deeply([uncolor("\e[01m")], ['bold'], 'uncolor("\\e[01m")');
is_deeply([uncolor("\e[m")],   [],       'uncolor("\\e[m")');
is_deeply([uncolor(q{})],      [],       'uncolor("")');

# Several tests for ANSI_COLORS_DISABLED.
local $ENV{ANSI_COLORS_DISABLED} = 1;
is(color('blue'), q{}, 'color support for ANSI_COLORS_DISABLED');
is(colored('testing', 'blue', 'on_red'),
    'testing', 'colored support for ANSI_COLORS_DISABLED');
is((GREEN 'testing'), 'testing', 'Constant support for ANSI_COLORS_DISABLED');
delete $ENV{ANSI_COLORS_DISABLED};

# Earlier versions of Term::ANSIColor didn't support ANSI_COLORS_DISABLED if
# the constant had been created before the environment variable was set.  Test
# all the ones we're going to use to get full test coverage.
local $ENV{ANSI_COLORS_DISABLED} = 1;
is((BLUE 'testing'), 'testing', 'ANSI_COLORS_DISABLED with existing constant');
delete $ENV{ANSI_COLORS_DISABLED};

# If ANSI_COLORS_DISABLED is set to a false value or the empty string, it
# should not take effect.
local $ENV{ANSI_COLORS_DISABLED} = 0;
is(color('bold'), "\e[1m", 'ANSI_COLORS_DISABLED must be true');
is((BOLD),        "\e[1m", '...likewise for constants');
local $ENV{ANSI_COLORS_DISABLED} = q{};
is(color('bold'), "\e[1m", '...likewise when set to an empty string');
is((BOLD),        "\e[1m", '...likewise for constants');
delete $ENV{ANSI_COLORS_DISABLED};

# Similar tests for NO_COLOR, although NO_COLOR may be set to any value.
local $ENV{NO_COLOR} = 1;
is(color('blue'), q{}, 'color support for NO_COLOR');
is(colored('testing', 'blue', 'on_red'),
    'testing', 'colored support for NO_COLOR');
is((BLUE 'testing'), 'testing', 'Constant support for NO_COLOR');
local $ENV{NO_COLOR} = q{};
is(color('blue'), q{}, 'color support for NO_COLOR with empty string');
is((RED 'testing'),
    'testing', 'Constant support for NO_COLOR with empty string');
delete $ENV{NO_COLOR};

# Make sure DARK is exported.  This was omitted in versions prior to 1.07.
is((DARK 'testing'), "\e[2mtesting", 'DARK');

# Check faint as a synonym for dark.
is(colored('test', 'faint'), "\e[2mtest\e[0m", 'colored supports faint');
is((FAINT 'test'), "\e[2mtest", '...and the FAINT constant works');

# Test bright color support.
is(color('bright_red'),    "\e[91m",      'Bright red is supported');
is((BRIGHT_RED 'test'),    "\e[91mtest",  '...and as a constant');
is(color('on_bright_red'), "\e[101m",     '...as is on bright red');
is((ON_BRIGHT_RED 'test'), "\e[101mtest", '...and as a constant');

# Test italic, which was added in 3.02.
is(color('italic'), "\e[3m",     'Italic is supported');
is((ITALIC 'test'), "\e[3mtest", '...and as a constant');

# Test colored with 0 and EACHLINE.  Regression test for an incorrect use of a
# truth check.
$Term::ANSIColor::EACHLINE = "\n";
is(colored('0', 'blue', 'bold'),
    "\e[34;1m0\e[0m", 'colored with 0 and EACHLINE');
is(
    colored("0\n0\n\n", 'blue', 'bold'),
    "\e[34;1m0\e[0m\n\e[34;1m0\e[0m\n\n",
    'colored with 0, EACHLINE, and multiple lines'
);

# Test colored with the empty string and EACHLINE.
is(colored(q{}, 'blue', 'bold'), q{}, 'colored w/empty string and EACHLINE');

# Test push and pop support.
is((PUSHCOLOR RED ON_GREEN 'text'),
    "\e[31m\e[42mtext", 'PUSHCOLOR does not break constants');
is((PUSHCOLOR BLUE 'text'), "\e[34mtext",       '...and adding another level');
is((RESET BLUE 'text'),     "\e[0m\e[34mtext",  '...and using reset');
is((POPCOLOR 'text'),       "\e[31m\e[42mtext", '...and POPCOLOR works');
is((LOCALCOLOR GREEN ON_BLUE 'text'),
    "\e[32m\e[44mtext\e[31m\e[42m", 'LOCALCOLOR');
$Term::ANSIColor::AUTOLOCAL = 1;
is((BLUE 'text'),     "\e[34mtext\e[31m\e[42m", 'AUTOLOCAL');
is((BLUE 'te', 'xt'), "\e[34mtext\e[31m\e[42m", 'AUTOLOCAL with commas');
$Term::ANSIColor::AUTOLOCAL = 0;
is((POPCOLOR 'text'), "\e[0mtext", 'POPCOLOR with empty stack');

# If AUTOLOCAL and AUTORESET are both set, the former takes precedence.
is((PUSHCOLOR RED ON_GREEN 'text'),
    "\e[31m\e[42mtext", 'Push some colors onto the stack');
$Term::ANSIColor::AUTOLOCAL = 1;
$Term::ANSIColor::AUTORESET = 1;
is((BLUE 'text'), "\e[34mtext\e[31m\e[42m", 'AUTOLOCAL overrides AUTORESET');
$Term::ANSIColor::AUTOLOCAL = 0;
is((BLUE 'text'), "\e[34mtext\e[0m", 'AUTORESET works with stacked colors');
is((POPCOLOR 'text'), "\e[0mtext\e[0m", 'POPCOLOR with empty stack');
$Term::ANSIColor::AUTORESET = 0;

# Test push and pop support with the syntax from the original openmethods.com
# submission, which uses a different coding style.
is(PUSHCOLOR(RED ON_GREEN), "\e[31m\e[42m", 'PUSHCOLOR with explict argument');
is(PUSHCOLOR(BLUE), "\e[34m", '...and another explicit argument');
is(
    RESET . BLUE . 'text',
    "\e[0m\e[34mtext",
    '...and constants with concatenation'
);
is(
    POPCOLOR . 'text',
    "\e[31m\e[42mtext",
    '...and POPCOLOR works without an argument'
);
is(
    LOCALCOLOR(GREEN . ON_BLUE . 'text'),
    "\e[32m\e[44mtext\e[31m\e[42m",
    'LOCALCOLOR with two arguments'
);
is(POPCOLOR . 'text', "\e[0mtext", 'POPCOLOR with no arguments');

# Prior to Term::ANSIColor, PUSHCOLOR, unlike all other constants, didn't take
# an array, so it could lose colors in some syntax.
is(PUSHCOLOR(RED, ON_GREEN), "\e[31m\e[42m", 'PUSHCOLOR with two arguments');
is(
    LOCALCOLOR(GREEN, 'text'),
    "\e[32mtext\e[31m\e[42m",
    'LOCALCOLOR with two arguments'
);
is(POPCOLOR(BOLD, 'text'), "\e[0m\e[1mtext", 'POPCOLOR with two arguments');

# Test colorstrip.
is(
    colorstrip("\e[1mBold \e[31;42mon green\e[0m\e[m"),
    'Bold on green',
    'Basic color stripping'
);
is(colorstrip("\e[1m", 'bold', "\e[0m"),
    'bold', 'Color stripping across multiple strings');
is_deeply(
    [colorstrip("\e[1m", 'bold', "\e[0m")],
    [q{}, 'bold', q{}],
    '...and in an array context'
);
is(colorstrip("foo\e[1m", 'bar', "baz\e[0m"),
    'foobarbaz', '...and proper joining in scalar context');
is(
    colorstrip("\e[2cSome other code\e and stray [0m stuff"),
    "\e[2cSome other code\e and stray [0m stuff",
    'colorstrip does not remove non-color stuff'
);

# Test colorvalid.
ok(
    colorvalid('blue bold dark', 'blink on_green'),
    'colorvalid returns true for valid attributes'
);
ok(!colorvalid('green orange'), '...and false for invalid attributes');

# Test error handling in color.
my $output = eval { color('chartreuse') };
is($output, undef, 'color on unknown color name fails');
like(
    $@,
    qr{ \A Invalid [ ] attribute [ ] name [ ] chartreuse [ ] at [ ] }xms,
    '...with the right error'
);

# Test error handling in colored.
$output = eval { colored('Stuff', 'chartreuse') };
is($output, undef, 'colored on unknown color name fails');
like(
    $@,
    qr{ \A Invalid [ ] attribute [ ] name [ ] chartreuse [ ] at [ ] }xms,
    '...with the right error'
);

# Test error handling in uncolor.
$output = eval { uncolor "\e[28m" };
is($output, undef, 'uncolor on unknown color code fails');
like(
    $@,
    qr{ \A No [ ] name [ ] for [ ] escape [ ] sequence [ ] 28 [ ] at [ ] }xms,
    '...with the right error'
);
$output = eval { uncolor "\e[foom" };
is($output, undef, 'uncolor on bad escape sequence fails');
like(
    $@,
    qr{ \A Bad [ ] escape [ ] sequence [ ] foo [ ] at [ ] }xms,
    '...with the right error'
);

# Test error reporting when calling unrecognized Term::ANSIColor subs that go
# through AUTOLOAD.
ok(!eval { Term::ANSIColor::RSET() }, 'Running invalid constant');
like(
    $@,
    qr{ \A undefined [ ] subroutine [ ] \&Term::ANSIColor::RSET [ ] called
        [ ] at [ ] }xms,
    'Correct error from an attribute that is not defined'
);
ok(!eval { Term::ANSIColor::reset() }, 'Running invalid sub');
like(
    $@,
    qr{ \A undefined [ ] subroutine [ ] \&Term::ANSIColor::reset [ ] called
        [ ] at [ ] }xms,
    'Correct error from a lowercase attribute'
);

# Ensure that we still get proper error reporting for unknown constants when
# when colors are disabled.
local $ENV{ANSI_COLORS_DISABLED} = 1;
eval { Term::ANSIColor::RSET() };
like(
    $@,
    qr{ \A undefined [ ] subroutine [ ] \&Term::ANSIColor::RSET [ ] called
        [ ] at [ ] }xms,
    'Correct error from undefined attribute with disabled colors'
);
delete $ENV{ANSI_COLORS_DISABLED};

# These are somewhat redundant, but they ensure we test all the branches in
# our generated constant subs so that we can use Test::Strict to check test
# suite coverage.
is((BOLD 't'),          "\e[1mt",   'Basic constant works for BOLD');
is((BLUE 't'),          "\e[34mt",  '...and for BLUE');
is((GREEN 't'),         "\e[32mt",  '...and for GREEN');
is((DARK 't'),          "\e[2mt",   '...and for DARK');
is((FAINT 't'),         "\e[2mt",   '...and for FAINT');
is((BRIGHT_RED 't'),    "\e[91mt",  '...and for BRIGHT_RED');
is((ON_BRIGHT_RED 't'), "\e[101mt", '...and for ON_BRIGHT_RED');
is((ITALIC 't'),        "\e[3mt",   '...and for ITALIC');
is((RED 't'),           "\e[31mt",  '...and for RED');
is((ON_GREEN 't'),      "\e[42mt",  '...and for ON_GREEN');
is((ON_BLUE 't'),       "\e[44mt",  '...and for ON_BLUE');
is((RESET 't'),         "\e[0mt",   '...and for RESET');

# Do the same for disabled colors.
local $ENV{ANSI_COLORS_DISABLED} = 1;
is(BOLD,          q{}, 'ANSI_COLORS_DISABLED works for BOLD');
is(BLUE,          q{}, '...and for BLUE');
is(GREEN,         q{}, '...and for GREEN');
is(DARK,          q{}, '...and for DARK');
is(FAINT,         q{}, '...and for FAINT');
is(BRIGHT_RED,    q{}, '...and for BRIGHT_RED');
is(ON_BRIGHT_RED, q{}, '...and for ON_BRIGHT_RED');
is(ITALIC,        q{}, '...and for ITALIC');
is(RED,           q{}, '...and for RED');
is(ON_GREEN,      q{}, '...and for ON_GREEN');
is(ON_BLUE,       q{}, '...and for ON_BLUE');
is(RESET,         q{}, '...and for RESET');
delete $ENV{ANSI_COLORS_DISABLED};

# Do the same for disabled colors with NO_COLOR.
local $ENV{NO_COLOR} = 1;
is(BOLD,          q{}, 'NO_COLOR works for BOLD');
is(BLUE,          q{}, '...and for BLUE');
is(GREEN,         q{}, '...and for GREEN');
is(DARK,          q{}, '...and for DARK');
is(FAINT,         q{}, '...and for FAINT');
is(BRIGHT_RED,    q{}, '...and for BRIGHT_RED');
is(ON_BRIGHT_RED, q{}, '...and for ON_BRIGHT_RED');
is(ITALIC,        q{}, '...and for ITALIC');
is(RED,           q{}, '...and for RED');
is(ON_GREEN,      q{}, '...and for ON_GREEN');
is(ON_BLUE,       q{}, '...and for ON_BLUE');
is(RESET,         q{}, '...and for RESET');
delete $ENV{NO_COLOR};

# Do the same for AUTORESET.
$Term::ANSIColor::AUTORESET = 1;
is((BOLD 't'),          "\e[1mt\e[0m",   'AUTORESET works for BOLD');
is((BLUE 't'),          "\e[34mt\e[0m",  '...and for BLUE');
is((GREEN 't'),         "\e[32mt\e[0m",  '...and for GREEN');
is((DARK 't'),          "\e[2mt\e[0m",   '...and for DARK');
is((FAINT 't'),         "\e[2mt\e[0m",   '...and for FAINT');
is((BRIGHT_RED 't'),    "\e[91mt\e[0m",  '...and for BRIGHT_RED');
is((ON_BRIGHT_RED 't'), "\e[101mt\e[0m", '...and for ON_BRIGHT_RED');
is((ITALIC 't'),        "\e[3mt\e[0m",   '...and for ITALIC');
is((RED 't'),           "\e[31mt\e[0m",  '...and for RED');
is((ON_GREEN 't'),      "\e[42mt\e[0m",  '...and for ON_GREEN');
is((ON_BLUE 't'),       "\e[44mt\e[0m",  '...and for ON_BLUE');
is((RESET 't'),         "\e[0mt\e[0m",   '...and for RESET');
is((BOLD),              "\e[1m",         'AUTORESET without text for BOLD');
is((BLUE),              "\e[34m",        '...and for BLUE');
is((GREEN),             "\e[32m",        '...and for GREEN');
is((DARK),              "\e[2m",         '...and for DARK');
is((FAINT),             "\e[2m",         '...and for FAINT');
is((BRIGHT_RED),        "\e[91m",        '...and for BRIGHT_RED');
is((ON_BRIGHT_RED),     "\e[101m",       '...and for ON_BRIGHT_RED');
is((ITALIC),            "\e[3m",         '...and for ITALIC');
is((RED),               "\e[31m",        '...and for RED');
is((ON_GREEN),          "\e[42m",        '...and for ON_GREEN');
is((ON_BLUE),           "\e[44m",        '...and for ON_BLUE');
is((RESET),             "\e[0m",         '...and for RESET');
$Term::ANSIColor::AUTORESET = 0;

# Do the same for AUTOLOCAL.
$Term::ANSIColor::AUTOLOCAL = 1;
is((BOLD 't'),          "\e[1mt\e[0m",   'AUTOLOCAL works for BOLD');
is((BLUE 't'),          "\e[34mt\e[0m",  '...and for BLUE');
is((GREEN 't'),         "\e[32mt\e[0m",  '...and for GREEN');
is((DARK 't'),          "\e[2mt\e[0m",   '...and for DARK');
is((FAINT 't'),         "\e[2mt\e[0m",   '...and for FAINT');
is((BRIGHT_RED 't'),    "\e[91mt\e[0m",  '...and for BRIGHT_RED');
is((ON_BRIGHT_RED 't'), "\e[101mt\e[0m", '...and for ON_BRIGHT_RED');
is((ITALIC 't'),        "\e[3mt\e[0m",   '...and for ITALIC');
is((RED 't'),           "\e[31mt\e[0m",  '...and for RED');
is((ON_GREEN 't'),      "\e[42mt\e[0m",  '...and for ON_GREEN');
is((ON_BLUE 't'),       "\e[44mt\e[0m",  '...and for ON_BLUE');
is((RESET 't'),         "\e[0mt\e[0m",   '...and for RESET');
is((BOLD),              "\e[1m",         'AUTOLOCAL without text for BOLD');
is((BLUE),              "\e[34m",        '...and for BLUE');
is((GREEN),             "\e[32m",        '...and for GREEN');
is((DARK),              "\e[2m",         '...and for DARK');
is((FAINT),             "\e[2m",         '...and for FAINT');
is((BRIGHT_RED),        "\e[91m",        '...and for BRIGHT_RED');
is((ON_BRIGHT_RED),     "\e[101m",       '...and for ON_BRIGHT_RED');
is((ITALIC),            "\e[3m",         '...and for ITALIC');
is((RED),               "\e[31m",        '...and for RED');
is((ON_GREEN),          "\e[42m",        '...and for ON_GREEN');
is((ON_BLUE),           "\e[44m",        '...and for ON_BLUE');
is((RESET),             "\e[0m",         '...and for RESET');
$Term::ANSIColor::AUTOLOCAL = 0;

# Force an internal error inside the AUTOLOAD stub by creating an attribute
# that will generate a syntax error.  This is just for coverage purposes.
# Disable warnings since our syntax error will spew otherwise.
local $SIG{__WARN__} = sub { };
$Term::ANSIColor::ATTRIBUTES{yellow} = q{'ERROR'};
ok(!eval { YELLOW 't' }, 'Caught internal AUTOLOAD error');
like(
    $@,
    qr{ \A failed [ ] to [ ] generate [ ] constant [ ] YELLOW: [ ] }xms,
    '...with correct error message'
);
