#!/usr/bin/perl -w

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;
use Test::More tests => 1;
use ExtUtils::MakeMaker;

eval q{
    os_unsupported();
};

like( $@, qr/^OS unsupported$/, 'OS Unsupported' );
