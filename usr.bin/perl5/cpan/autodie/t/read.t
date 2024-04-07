#!/usr/bin/perl

use strict;
use warnings;
use autodie;

use Test::More tests => 2;

my $buffer = 'should-not-appear';
eval {
    read('BOFH', $buffer, 1024);
};
like($@, qr/Can't read\(BOFH, <BUFFER>, 1024\)/,
     'read should not show the buffer');
eval {
    read('BOFH', $buffer, 1024, 5);
};
like($@, qr/Can't read\(BOFH, <BUFFER>, 1024, 5\)/,
     'read should not show the buffer');
