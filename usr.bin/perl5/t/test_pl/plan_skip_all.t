#!/usr/bin/env perl -w
use strict;
use warnings;

BEGIN {
    chdir 't' if -d 't';
    push @INC, ".";
    require 'test.pl';
}

plan skip_all => "Test Test::More compatible plan skip_all => \$foo";
