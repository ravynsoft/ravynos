use warnings;
use strict;

use Test::More tests => 2;

use XS::APItest qw(underscore_length);

$_ = "foo";
is underscore_length(), 3;

$_ = "snowman \x{2603}";
is underscore_length(), 9;

1;
