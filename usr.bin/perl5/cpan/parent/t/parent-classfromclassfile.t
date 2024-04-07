#!/usr/bin/perl -w

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't' if -d 't';
        chdir '../lib/parent';
        @INC = '..';
    }
}

use strict;
use Test::More tests => 3;
use lib 't/lib';

use_ok('parent');

# Tests that a bare (non-double-colon) class still loads
# and does not get treated as a file:
eval q{package Test1; require Dummy; use parent -norequire, 'Dummy::InlineChild'; };
is $@, '', "Loading an unadorned class works";
isnt $INC{"Dummy.pm"}, undef, 'We loaded Dummy.pm';
