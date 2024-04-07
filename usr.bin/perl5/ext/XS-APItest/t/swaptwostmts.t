use warnings;
use strict;

use Test::More tests => 32;

BEGIN { $^H |= 0x20000; }

my $t;

$t = "";
eval q{
	use XS::APItest ();
	$t .= "a";
	swaptwostmts
	$t .= "b";
	$t .= "c";
	$t .= "d";
};
isnt $@, "";

$t = "";
eval q{
	use XS::APItest qw(swaptwostmts);
	$t .= "a";
	swaptwostmts
	$t .= "b";
	$t .= "c";
	$t .= "d";
};
is $@, "";
is $t, "acbd";

$t = "";
eval q{
	use XS::APItest qw(swaptwostmts);
	$t .= "a";
	swaptwostmts
	if(1) { $t .= "b"; }
	$t .= "c";
	$t .= "d";
};
is $@, "";
is $t, "acbd";

$t = "";
eval q{
	use XS::APItest qw(swaptwostmts);
	$t .= "a";
	swaptwostmts
	$t .= "b";
	if(1) { $t .= "c"; }
	$t .= "d";
};
is $@, "";
is $t, "acbd";

$t = "";
eval q{
	use XS::APItest qw(swaptwostmts);
	$t .= "a";
	swaptwostmts
	$t .= "b";
	foreach(1..3) {
		$t .= "c";
		swaptwostmts
		$t .= "d";
		$t .= "e";
		$t .= "f";
	}
	$t .= "g";
};
is $@, "";
is $t, "acedfcedfcedfbg";

$t = "";
eval q{
	use XS::APItest qw(swaptwostmts);
	$t .= "a";
	swaptwostmts
	$t .= "b";
	$t .= "c";
};
is $@, "";
is $t, "acb";

$t = "";
eval q{
	use XS::APItest qw(swaptwostmts);
	$t .= "a";
	swaptwostmts
	$t .= "b";
	$t .= "c"
};
is $@, "";
is $t, "acb";

$t = "";
eval q{
	use XS::APItest qw(swaptwostmts);
	$t .= "a";
	swaptwostmts
	$t .= "b"
};
isnt $@, "";

$t = "";
eval q{
	use XS::APItest qw(swaptwostmts);
	$_ = $t;
	$_ .= "a";
	swaptwostmts
	if(1) { $_ .= "b"; }
	tr/a-z/A-Z/;
	$_ .= "d";
	$t = $_;
};
is $@, "";
is $t, "Abd";

$t = "";
eval q{
	use XS::APItest qw(swaptwostmts);
	sub add_to_t { $t .= $_[0]; }
	add_to_t "a";
	swaptwostmts
	if(1) { add_to_t "b"; }
	add_to_t "c";
	add_to_t "d";
};
is $@, "";
is $t, "acbd";

$t = "";
eval q{
	use XS::APItest qw(swaptwostmts);
	{ $t .= "a"; }
	swaptwostmts
	if(1) { { $t .= "b"; } }
	{ $t .= "c"; }
	{ $t .= "d"; }
};
is $@, "";
is $t, "acbd";

$t = "";
eval q{
	use XS::APItest qw(swaptwostmts);
	no warnings "void";
	{ $t .= "a"; }
	swaptwostmts
	if(1) { { $t .= "b"; } }
	{};
	{ $t .= "d"; }
};
is $@, "";
is $t, "abd";

$t = "";
eval q{
	use XS::APItest qw(swaptwostmts);
	no warnings "void";
	{ $t .= "a"; }
	swaptwostmts
	if(1) { { $t .= "b"; } }
	[];
	{ $t .= "d"; }
};
is $@, "";
is $t, "abd";

$t = "";
eval q{
	use XS::APItest qw(swaptwostmts);
	no warnings "void";
	"@{[ $t .= 'a' ]}";
	swaptwostmts
	if(1) { "@{[ $t .= 'b' ]}"; }
	"@{[ $t .= 'c' ]}";
	"@{[ $t .= 'd' ]}";
};
is $@, "";
is $t, "acbd";

$t = "";
eval q{
	use XS::APItest qw(swaptwostmts);
	$t .= "a";
	swaptwostmts
	x:
	$t .= "b";
	z:
	$t .= "c";
	$t .= "d";
};
is $@, "";
is $t, "acbd";

$t = "";
eval q{
	use XS::APItest qw(swaptwostmts);
	$t .= "a";
	goto x;
	$t .= "b";
	swaptwostmts
	x:
	$t .= "c";
	$t .= "d";
	$t .= "e";
};
is $@, "";
is $t, "ace";

$t = "";
eval q{
	use XS::APItest qw(swaptwostmts);
	$t .= "a";
	goto x;
	$t .= "b";
	swaptwostmts
	$t .= "c";
	x:
	$t .= "d";
	$t .= "e";
};
is $@, "";
is $t, "adce";

1;
