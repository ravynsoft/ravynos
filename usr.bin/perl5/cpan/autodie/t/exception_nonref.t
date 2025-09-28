#!/usr/bin/perl -w
use strict;
use Test::More tests => 1;
use autodie;

eval { syswrite "xxxxxxxx", \*STDOUT };
my $err = $@;

like( $err, qr<xxxxxxxx>, 'expected failure on attempt to write to non-reference' );

done_testing();
