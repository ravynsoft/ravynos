#!/usr/bin/perl -w

use strict;
use warnings;

BEGIN {
    unshift @INC, 't/lib/';
}

use Test::More tests => 3;

use ExtUtils::MakeMaker;

my $MM = bless { MAKE => "nmake6" }, "MM";
is $MM->make, 'nmake';

$MM->{MAKE} = 'GNUmake';
is $MM->make, 'gmake';

$MM->{MAKE} = 'MMS';
is $MM->make, 'mms';
