use warnings;
use strict;

use Test::More tests => 1;

use XS::APItest qw(stufftest);

my $a = "stufftest+;();";
eval $a;
is $a, "stufftest+;();";

1;
