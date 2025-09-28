#!/usr/bin/perl
#
# Test suite for stringify interaction.
#
# Copyright 2011 Revilo Reegiles
# Copyright 2011, 2014, 2020 Russ Allbery <rra@cpan.org>
#
# SPDX-License-Identifier: GPL-1.0-or-later OR Artistic-1.0-Perl

use 5.008;
use strict;
use warnings;

use Test::More tests => 6;

# Create a dummy class that implements stringification.
## no critic (Modules::ProhibitMultiplePackages)
package Test::Stringify;
use overload '""' => 'stringify';
sub new       { return bless({}, 'Test::Stringify') }
sub stringify { return "Foo Bar\n" }

# Back to the main package.
package main;

# Load the module.
BEGIN {
    delete $ENV{ANSI_COLORS_ALIASES};
    delete $ENV{ANSI_COLORS_DISABLED};
    delete $ENV{NO_COLOR};
    use_ok('Term::ANSIColor', qw(colored));
}

# Some basic tests of colored without stringification.
my $result = colored(['blue', 'bold'], 'testing');
is($result, "\e[34;1mtesting\e[0m", 'colored with an array reference');
$result = colored("ok\n", 'bold blue');
is($result, "\e[1;34mok\n\e[0m", 'colored with a following string');

# Create a stringifiable object and repeat the tests.
my $test = Test::Stringify->new;
$result = colored($test . q{}, 'bold blue');
is($result, "\e[1;34mFoo Bar\n\e[0m", 'colored with forced stringification');
$result = colored($test, 'bold blue');
is($result, "\e[1;34mFoo Bar\n\e[0m", 'colored with a non-array reference');

# Create a hash reference and try stringifying it.
## no critic (RegularExpressions::ProhibitEscapedMetacharacters)
my %foo = (foo => 'bar');
$result = colored(\%foo, 'bold blue');
like(
    $result,
    qr{ \e\[1;34m HASH\(.*\) \e\[0m }xms,
    'colored with a hash reference'
);
