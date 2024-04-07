# -*- mode: perl; -*-

use strict;
use warnings;
use lib 't';

use Test::More tests => 1;

my ($x, $expected, $try);

my $class = 'Math::BigInt';

# test whether :constant works or not

$try = qq|use $class 0, "bgcd", ":constant";|
     . q| $x = 2**150; bgcd($x); $x = "$x";|;
$expected = eval $try;
is($expected, "1427247692705959881058285969449495136382746624", $try);
