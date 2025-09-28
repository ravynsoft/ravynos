use warnings;
use strict;

use Test::More tests => 8;

BEGIN { $^H |= 0x20000; }

my $t;

$t = "";
eval q{
	use XS::APItest qw(postinc);
	$t .= "a";
	my $x = 3;
	$t .= "b(".postinc($x).")";
	$t .= "c(".$x.")";
	$t .= "d";
};
is $@, "";
is $t, "ab(3)c(4)d";

$t = "";
eval q{
	use XS::APItest qw(postinc);
	$t .= "a";
	my $x = 3;
	$t .= "b(".postinc($x+1).")";
	$t .= "c(".$x.")";
	$t .= "d";
};
isnt $@, "";
is $t, "";

$t = "";
eval q{
	use XS::APItest qw(postinc);
	$t .= "a";
	my %x = (z => 3);
	my $z = postinc($x{z});
	$t .= "b(".$z.")";
	$t .= "c(".$x{z}.")";
	$t .= "d";
};
is $@, "";
is $t, "ab(3)c(4)d";

$t = "";
eval q{
	use XS::APItest qw(postinc);
	$t .= "a";
	my %x;
	my $z = postinc($x{z});
	$t .= "b(".$z.")";
	$t .= "c(".$x{z}.")";
	$t .= "d";
};
is $@, "";
is $t, "ab(0)c(1)d";

1;
