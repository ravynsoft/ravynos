#!perl -w

# Test that goto &xsub provides the right lexical environment.

use strict;

use Test::More tests => 1;
use XS::APItest;

# This sub must remain outside the ‘use warnings’ scope.
sub no_warnings { goto &stringify }

use warnings;

$SIG{__WARN__} = sub { like shift, qr\^Use of uninitialized\ };

no_warnings(my $x) # undefined variable
