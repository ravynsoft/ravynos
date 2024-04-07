#!/usr/bin/perl

use strict;

BEGIN {
    # Get function prototypes
    require './regen/regen_lib.pl';
    unshift @INC, 'ext/ExtUtils-Miniperl/lib';
}

use ExtUtils::Miniperl 1;

my $fh = open_new('miniperlmain.c', undef, {by => "$0 and ExtUtils::Miniperl"});
writemain($fh);
read_only_bottom_close_and_rename($fh);
