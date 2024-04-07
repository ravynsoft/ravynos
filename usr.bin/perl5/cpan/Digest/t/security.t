#!/usr/bin/env perl

# Digest->new() had an exploitable eval

use strict;
use warnings;

use Test::More tests => 1;

use Digest;

$LOL::PWNED = 0;
eval { Digest->new(q[MD;5;$LOL::PWNED = 42]) };
is $LOL::PWNED, 0;
