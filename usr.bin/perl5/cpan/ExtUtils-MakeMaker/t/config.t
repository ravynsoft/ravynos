#!/usr/bin/perl -w

BEGIN {
    unshift @INC, 't/lib/';
}

use strict;
use warnings;
use Test::More tests => 3;
use Config ();

BEGIN { use_ok 'ExtUtils::MakeMaker::Config'; }

is $Config{path_sep}, $Config::Config{path_sep};

eval {
    $Config{wibble} = 42;
};
is $Config{wibble}, 42;
