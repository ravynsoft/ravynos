#!/usr/bin/perl -w

# This is a test for all the odd little backwards compatible things
# MakeMaker has to support.  And we do mean backwards.

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;
use Test::More tests => 2;

require ExtUtils::MakeMaker;

# CPAN.pm wants MM.
can_ok('MM', 'new');

# Pre 5.8 ExtUtils::Embed wants MY.
can_ok('MY', 'catdir');
