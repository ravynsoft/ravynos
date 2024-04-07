use warnings;
use strict;

use Test::More tests => 16;

BEGIN { $^H |= 0x20000; }

my $t;

$t = "";
eval q{
	use XS::APItest qw(blockasexpr);
	$t .= "a";
	$t .= "b" . blockasexpr { "c"; } . "d";
	$t .= "e";
};
is $@, "";
is $t, "abcde";

$t = "";
eval q{
	use XS::APItest qw(blockasexpr);
	no warnings "void";
	$t .= "a";
	$t .= "b" . blockasexpr { "z"; "c"; } . "d";
	$t .= "e";
};
is $@, "";
is $t, "abcde";

$t = "";
eval q{
	use XS::APItest qw(blockasexpr);
	$t .= "a";
	$t .= "b" . blockasexpr { if($t eq "a") { "c"; } else { "d"; } } . "e";
	$t .= "f";
};
is $@, "";
is $t, "abcef";

$t = "";
eval q{
	use XS::APItest qw(blockasexpr);
	$t .= "a";
	$t .= "b" . blockasexpr { if($t eq "z") { "c"; } else { "d"; } } . "e";
	$t .= "f";
};
is $@, "";
is $t, "abdef";

$t = "";
eval q{
	use XS::APItest qw(blockasexpr);
	no warnings "void";
	$t .= "a";
	$t .= "b" . blockasexpr { { "z"; "c"; } } . "d";
	$t .= "e";
};
is $@, "";
is $t, "abcde";

$t = "";
eval q{
	use XS::APItest qw(blockasexpr);
	$t .= "a";
	$t .= blockasexpr {
		my $t = "z";
		"b";
	};
	$t .= "c";
};
is $@, "";
is $t, "abc";

$t = "";
eval q{
	use XS::APItest qw(blockasexpr);
	my $f = 1.5;
	$t .= "a(".($f+$f).")";
	$t .= "b(" . blockasexpr {
		use integer;
		$f+$f;
	} . ")";
	$t .= "c(".($f+$f).")";
};
is $@, "";
is $t, "a(3)b(2)c(3)";

$t = "";
eval q{
	use XS::APItest qw(blockasexpr);
	our $z = "z";
	$t .= "a$z";
	$t .= "b" . blockasexpr {
		local $z = "y";
		$z;
	};
	$t .= "c$z";
};
is $@, "";
is $t, "azbycz";

1;
