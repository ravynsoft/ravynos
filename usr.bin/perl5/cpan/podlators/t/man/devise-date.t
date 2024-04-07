#!/usr/bin/perl
#
# In order for MakeMaker to build in the core, nothing can use Fcntl which
# includes POSIX.  devise_date()'s use of strftime() was replaced.  This tests
# that it's identical.  It also tests special handling of the POD_MAN_DATE
# and SOURCE_DATE_EPOCH environment variables.
#
# Copyright 2009, 2014-2015, 2018-2019, 2022 Russ Allbery <rra@cpan.org>
#
# This program is free software; you may redistribute it and/or modify it
# under the same terms as Perl itself.
#
# SPDX-License-Identifier: GPL-1.0-or-later OR Artistic-1.0-Perl

use 5.008;
use strict;
use warnings;

use Pod::Man;
use POSIX qw(strftime);

use Test::More tests => 6;

# Start with environment variables affecting the date stripped.
local $ENV{SOURCE_DATE_EPOCH} = undef;
local $ENV{POD_MAN_DATE} = undef;

# Check that the results of device_date matches strftime.  There is no input
# file name, so this will use the current time.
my $parser = Pod::Man->new;
is(
    $parser->devise_date,
    strftime('%Y-%m-%d', gmtime()),
    'devise_date matches strftime',
);

# Set the override environment variable and ensure that it's honored.
local $ENV{POD_MAN_DATE} = '2014-01-01';
is($parser->devise_date, '2014-01-01', 'devise_date honors POD_MAN_DATE');

# Check that an empty environment variable is honored.
local $ENV{POD_MAN_DATE} = q{};
is($parser->devise_date, q{}, 'devise_date honors empty POD_MAN_DATE');

# Set another environment variable and ensure that it's honored.
local $ENV{POD_MAN_DATE} = undef;
local $ENV{SOURCE_DATE_EPOCH} = 1439390140;
is($parser->devise_date, '2015-08-12', 'devise_date honors SOURCE_DATE_EPOCH');

# Check that POD_MAN_DATE overrides SOURCE_DATE_EPOCH
local $ENV{POD_MAN_DATE} = '2013-01-01';
local $ENV{SOURCE_DATE_EPOCH} = 1482676620;
is(
    $parser->devise_date, '2013-01-01',
    'devise_date honors POD_MAN_DATE over SOURCE_DATE_EPOCH',
);

# Check that an invalid SOURCE_DATE_EPOCH is not accepted
local $ENV{POD_MAN_DATE} = undef;
local $ENV{SOURCE_DATE_EPOCH} = '1482676620B';
is(
    $parser->devise_date,
    strftime('%Y-%m-%d', gmtime()),
    'devise_date ignores invalid SOURCE_DATE_EPOCH',
);
