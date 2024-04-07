use warnings;
use strict;

use Test::More tests => 4;

my($t, $n);
$n = 5;

use XS::APItest qw(rpn);
$t = rpn($n
	 $n 1 +
		* #wibble
#wobble
2
		/
);
is $t, 15;
is __LINE__, 18;

$t = 0;
$t = rpn($n $n 1 + *
#line 100
	2 /);
is $t, 15;
is __LINE__, 102;

1;
