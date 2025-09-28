use warnings;
use strict;

use Test::More tests => 14;

BEGIN { $^H |= 0x20000; }

my $t;

$t = "";
eval q{
	use XS::APItest qw(looprest);
	do {
		$t .= "a";
		looprest
		$t .= "b";
		last unless length($t) < 5;
		$t .= "c";
	};
	$t .= "d";
};
is $@, "";
is $t, "abcbcbd";

$t = "";
eval q{
	use XS::APItest qw(looprest);
	$t .= "a";
	looprest
	$t .= "b";
	last unless length($t) < 5;
	$t .= "c";
};
is $@, "";
is $t, "abcbcb";

$t = "";
eval q[
	use XS::APItest qw(looprest);
	do {
		$t .= "a";
		looprest
		$t .= "b";
		last unless length($t) < 5;
		$t .= "c";
];
isnt $@, "";
is $t, "";

$t = "";
eval q[
	use XS::APItest qw(looprest);
	$t .= "a";
	looprest
	$t .= "b";
	last unless length($t) < 5;
	$t .= "c";
	};
];
isnt $@, "";
is $t, "";

$t = "";
eval q{
	use XS::APItest qw(looprest);
	my $x = "a";
	$t .= $x;
	do {
		no warnings "shadow";
		$t .= $x;
		my $x = "b";
		$t .= $x;
		looprest
		$t .= $x;
		my $x = "c";
		$t .= $x;
		last unless length($t) < 7;
		$t .= $x;
		my $x = "d";
		$t .= $x;
	};
	$t .= $x;
};
is $@, "";
is $t, "aabbccdbca";

$t = "";
eval q{
	use XS::APItest qw(looprest);
	do {
		{ $t .= "a"; }
		looprest
		{ $t .= "b"; }
		last unless length($t) < 5;
		{ $t .= "c"; }
	};
	$t .= "d";
};
is $@, "";
is $t, "abcbcbd";

$t = "";
eval q{
	use XS::APItest qw(looprest);
	{ $t .= "a"; }
	looprest
	{ $t .= "b"; }
	last unless length($t) < 5;
	{ $t .= "c"; }
};
is $@, "";
is $t, "abcbcb";

1;
