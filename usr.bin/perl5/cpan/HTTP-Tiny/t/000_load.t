#!perl

use strict;
use warnings;

use Test::More 0.88 tests => 1;

require_ok('HTTP::Tiny');

local $HTTP::Tiny::VERSION = $HTTP::Tiny::VERSION || 'from repo';
note("HTTP::Tiny $HTTP::Tiny::VERSION, Perl $], $^X");

