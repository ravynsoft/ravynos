#!/usr/bin/perl -w

BEGIN {
    use lib 't/lib';
}

use strict;
use warnings;

use NoFork;
require('./t/multiplexer.t');
