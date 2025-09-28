use warnings;
use strict;

use Test::More tests => 10;

BEGIN { $^H |= 0x20000; }

my $t;

$t = "";
eval q{
	use XS::APItest qw(stmtsasexpr);
	$t .= "a";
	$t .= "b" . stmtsasexpr { "c"; } . "d";
	$t .= "e";
};
is $@, "";
is $t, "abcde";

$t = "";
eval q{
	use XS::APItest qw(stmtsasexpr);
	no warnings "void";
	$t .= "a";
	$t .= "b" . stmtsasexpr { "z"; "c"; } . "d";
	$t .= "e";
};
is $@, "";
is $t, "abcde";

$t = "";
eval q{
	use XS::APItest qw(stmtsasexpr);
	$t .= "a";
	$t .= "b" . stmtsasexpr { if($t eq "a") { "c"; } else { "d"; } } . "e";
	$t .= "f";
};
is $@, "";
is $t, "abcef";

$t = "";
eval q{
	use XS::APItest qw(stmtsasexpr);
	$t .= "a";
	$t .= "b" . stmtsasexpr { if($t eq "z") { "c"; } else { "d"; } } . "e";
	$t .= "f";
};
is $@, "";
is $t, "abdef";

$t = "";
eval q{
	use XS::APItest qw(stmtsasexpr);
	no warnings "void";
	$t .= "a";
	$t .= "b" . stmtsasexpr { { "z"; "c"; } } . "d";
	$t .= "e";
};
is $@, "";
is $t, "abcde";

1;
