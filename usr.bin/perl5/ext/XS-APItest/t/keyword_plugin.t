use warnings;
use strict;

use Test::More tests => 13;

BEGIN { $^H |= 0x20000; }
no warnings;

my($triangle, $num);
$num = 5;

$triangle = undef;
eval q{
	use XS::APItest ();
	$triangle = rpn($num $num 1 + * 2 /);
};
isnt $@, "";

$triangle = undef;
eval q{
	use XS::APItest qw(rpn);
	$triangle = rpn($num $num 1 + * 2 /);
};
is $@, "";
is $triangle, 15;

$triangle = undef;
eval q{
	use XS::APItest qw(rpn);
	$triangle = join(":", "x", rpn($num $num 1 + * 2 /), "y");
};
is $@, "";
is $triangle, "x:15:y";

$triangle = undef;
eval q{
	use XS::APItest qw(rpn);
	$triangle = 1 + rpn($num $num 1 + * 2 /) * 10;
};
is $@, "";
is $triangle, 151;

$triangle = undef;
eval q{
	use XS::APItest qw(rpn);
	$triangle = rpn($num $num 1 + * 2 /);
	$triangle++;
};
is $@, "";
is $triangle, 16;

$triangle = undef;
eval q{
	use XS::APItest qw(rpn);
	$triangle = rpn($num $num 1 + * 2 /)
	$triangle++;
};
isnt $@, "";

$triangle = undef;
eval q{
	use XS::APItest qw(calcrpn);
	calcrpn $triangle { $num $num 1 + * 2 / }
	$triangle++;
};
is $@, "";
is $triangle, 16;

$triangle = undef;
eval q{
	use XS::APItest qw(calcrpn);
	123 + calcrpn $triangle { $num $num 1 + * 2 / } ;
};
isnt $@, "";

1;
