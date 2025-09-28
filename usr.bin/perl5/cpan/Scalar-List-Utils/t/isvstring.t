#!./perl

use strict;
use warnings;

$|=1;
use Scalar::Util ();
use Test::More  (grep { /isvstring/ } @Scalar::Util::EXPORT_FAIL)
    ? (skip_all => 'isvstring is not supported on this perl version')
    : (tests => 3);

use Scalar::Util qw(isvstring);

my $vs = ord("A") == 193 ? 241.75.240 : 49.46.48;

ok( $vs == "1.0", 'dotted num');
ok( isvstring($vs), 'isvstring');

my $sv = "1.0";
ok( !isvstring($sv), 'not isvstring');



