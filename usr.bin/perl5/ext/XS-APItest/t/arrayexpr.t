use warnings;
use strict;

use Test::More tests => 2*10;

BEGIN { $^H |= 0x20000; }

my @t;

sub mymap(&@) { my $sub = shift; return map { $sub->($_) } @_; }
sub myneg(@) { return map { -$_ } @_; }
package AA { sub listmeth { shift; return map { -$_ } @_; } }

@t = ();
eval q{
	use XS::APItest qw(arrayfullexpr);
	no warnings "void";
	push @t, arrayfullexpr 1+2;
	push @t, arrayfullexpr 0 || 2;
	push @t, arrayfullexpr 1 || 2;
	push @t, arrayfullexpr 0 || 2, 3;
	push @t, arrayfullexpr 1 || 2, 3;
	push @t, arrayfullexpr 1, 2;
	push @t, arrayfullexpr 0 or 2;
	push @t, arrayfullexpr 1 or 2;
	push @t, arrayfullexpr 0 or 2, 3;
	push @t, arrayfullexpr 1 or 2, 3;
	{ push @t, arrayfullexpr 1, 2 }
	push @t, (arrayfullexpr 1, 2), 3;
	push @t, arrayfullexpr do { 1; 1 }, 2;
	push @t, arrayfullexpr 3, 4 if 1;
	push @t, arrayfullexpr 5, 6 if 0;
	push @t, arrayfullexpr (7, 8), 9;
	push @t, arrayfullexpr a => "b";
	push @t, arrayfullexpr 1 ? reverse 2, 3 : 4, 5;
	push @t, arrayfullexpr 0 ? reverse 2, 3 : 4, 5;
	push @t, 1 ? reverse arrayfullexpr 2, 3 : 4, 5;
	push @t, 0 ? reverse arrayfullexpr 2, 3 : 4, 5;
	push @t, arrayfullexpr reverse 1, 2, 3;
	push @t, sub { arrayfullexpr return 1, 2, 3 }->();
	push @t, arrayfullexpr myneg 1, 2, 3;
	push @t, arrayfullexpr map { -$_ } 1, 2, 3;
	push @t, arrayfullexpr mymap { -$_[0] } 1, 2, 3;
	push @t, arrayfullexpr AA->listmeth(1, 2), 3;
	push @t, arrayfullexpr listmeth AA (1, 2), 3;
	push @t, arrayfullexpr listmeth AA 1, 2, 3;
	push @t, arrayfullexpr not 1, 2;
	push @t, arrayfullexpr reverse 6, 7, 8 or 9;
	push @t, arrayfullexpr reverse 6, 7, 8 and 9;
	push @t, arrayfullexpr 1 << 2;
	push @t, arrayfullexpr 7 < 8;
};
is $@, "";
is_deeply \@t, [
	[3],
	[2],
	[1],
	[2,3],
	[1,3],
	[1,2],
	[2],
	[1],
	[2,3],
	[1],
	[1,2],
	[1,2], 3,
	[1,2],
	[3,4],
	[7,8,9],
	["a","b"],
	[3,2,5],
	[4,5],
	[2,3], 5,
	4, 5,
	[3,2,1],
	1, 2, 3,
	[-1,-2,-3],
	[-1,-2,-3],
	[-1,-2,-3],
	[-1,-2,3],
	[-1,-2,3],
	[-1,-2,-3],
	[!1],
	[876],
	[9],
	[4],
	[!!1],
];

@t = ();
eval q{
	use XS::APItest qw(arraylistexpr);
	no warnings "void";
	push @t, arraylistexpr 1+2;
	push @t, arraylistexpr 0 || 2;
	push @t, arraylistexpr 1 || 2;
	push @t, arraylistexpr 0 || 2, 3;
	push @t, arraylistexpr 1 || 2, 3;
	push @t, arraylistexpr 1, 2;
	push @t, arraylistexpr 0 or 2;
	push @t, arraylistexpr 1 or 2;
	push @t, arraylistexpr 0 or 2, 3;
	push @t, arraylistexpr 1 or 2, 3;
	{ push @t, arraylistexpr 1, 2 }
	push @t, (arraylistexpr 1, 2), 3;
	push @t, arraylistexpr do { 1; 1 }, 2;
	push @t, arraylistexpr 3, 4 if 1;
	push @t, arraylistexpr 5, 6 if 0;
	push @t, arraylistexpr (7, 8), 9;
	push @t, arraylistexpr a => "b";
	push @t, arraylistexpr 1 ? reverse 2, 3 : 4, 5;
	push @t, arraylistexpr 0 ? reverse 2, 3 : 4, 5;
	push @t, 1 ? reverse arraylistexpr 2, 3 : 4, 5;
	push @t, 0 ? reverse arraylistexpr 2, 3 : 4, 5;
	push @t, arraylistexpr reverse 1, 2, 3;
	push @t, sub { arraylistexpr return 1, 2, 3 }->();
	push @t, arraylistexpr myneg 1, 2, 3;
	push @t, arraylistexpr map { -$_ } 1, 2, 3;
	push @t, arraylistexpr mymap { -$_[0] } 1, 2, 3;
	push @t, arraylistexpr AA->listmeth(1, 2), 3;
	push @t, arraylistexpr listmeth AA (1, 2), 3;
	push @t, arraylistexpr listmeth AA 1, 2, 3;
	push @t, arraylistexpr not 1, 2;
	push @t, arraylistexpr reverse 6, 7, 8 or 9;
	push @t, arraylistexpr reverse 6, 7, 8 and 9;
	push @t, arraylistexpr 1 << 2;
	push @t, arraylistexpr 7 < 8;
};
is $@, "";
is_deeply \@t, [
	[3],
	[2],
	[1],
	[2,3],
	[1,3],
	[1,2],
	[0],
	[1],
	[0],
	[1],
	[1,2],
	[1,2], 3,
	[1,2],
	[3,4],
	[7,8,9],
	["a","b"],
	[3,2,5],
	[4,5],
	[2,3], 5,
	4, 5,
	[3,2,1],
	1, 2, 3,
	[-1,-2,-3],
	[-1,-2,-3],
	[-1,-2,-3],
	[-1,-2,3],
	[-1,-2,3],
	[-1,-2,-3],
	[!1],
	[8,7,6],
	[8,7,6],
	[4],
	[!!1],
];

@t = ();
eval q{
	use XS::APItest qw(arraytermexpr);
	no warnings "void";
	push @t, arraytermexpr 1+2;
	push @t, arraytermexpr 0 || 2;
	push @t, arraytermexpr 1 || 2;
	push @t, arraytermexpr 0 || 2, 3;
	push @t, arraytermexpr 1 || 2, 3;
	push @t, arraytermexpr 1, 2;
	push @t, arraytermexpr 0 or 2;
	push @t, arraytermexpr 1 or 2;
	push @t, arraytermexpr 0 or 2, 3;
	push @t, arraytermexpr 1 or 2, 3;
	{ push @t, arraytermexpr 1 }
	push @t, (arraytermexpr 1, 2), 3;
	push @t, arraytermexpr do { 1; 1 }, 2;
	push @t, arraytermexpr 3, 4 if 1;
	push @t, arraytermexpr 5, 6 if 0;
	push @t, arraytermexpr (7, 8), 9;
	push @t, arraytermexpr a => "b";
	push @t, arraytermexpr 1 ? reverse 2, 3 : 4, 5;
	push @t, arraytermexpr 0 ? reverse 2, 3 : 4, 5;
	push @t, 1 ? reverse arraytermexpr 2, 3 : 4, 5;
	push @t, 0 ? reverse arraytermexpr 2, 3 : 4, 5;
	push @t, arraytermexpr reverse 1, 2, 3;
	push @t, sub { arraytermexpr return 1, 2, 3 }->();
	push @t, arraytermexpr myneg 1, 2, 3;
	push @t, arraytermexpr map { -$_ } 1, 2, 3;
	push @t, arraytermexpr mymap { -$_[0] } 1, 2, 3;
	push @t, arraytermexpr AA->listmeth(1, 2), 3;
	push @t, arraytermexpr listmeth AA (1, 2), 3;
	push @t, arraytermexpr listmeth AA 1, 2, 3;
	push @t, arraytermexpr not 1, 2;
	push @t, arraytermexpr reverse 6, 7, 8 or 9;
	push @t, arraytermexpr reverse 6, 7, 8 and 9;
	push @t, arraytermexpr 1 << 2;
	push @t, arraytermexpr 7 < 8;
};
is $@, "";
is_deeply \@t, [
	[3],
	[2],
	[1],
	[2], 3,
	[1], 3,
	[1], 2,
	[0],
	[1],
	[0],
	[1],
	[1],
	[1], 2, 3,
	[1], 2,
	[3], 4,
	[7,8], 9,
	["a"], "b",
	[3,2], 5,
	[4], 5,
	3, [2], 5,
	4, 5,
	[3,2,1],
	1, 2, 3,
	[-1,-2,-3],
	[-1,-2,-3],
	[-1,-2,-3],
	[-1,-2], 3,
	[-1,-2], 3,
	[-1,-2,-3],
	[!1],
	[8,7,6],
	[8,7,6],
	[4],
	[!!1],
];

@t = ();
eval q{
	use XS::APItest qw(arrayarithexpr);
	no warnings "void";
	push @t, arrayarithexpr 1+2;
	push @t, arrayarithexpr 0 || 2;
	push @t, arrayarithexpr 1 || 2;
	push @t, arrayarithexpr 0 || 2, 3;
	push @t, arrayarithexpr 1 || 2, 3;
	push @t, arrayarithexpr 1, 2;
	push @t, arrayarithexpr 0 or 2;
	push @t, arrayarithexpr 1 or 2;
	push @t, arrayarithexpr 0 or 2, 3;
	push @t, arrayarithexpr 1 or 2, 3;
	{ push @t, arrayarithexpr 1 }
	push @t, (arrayarithexpr 1, 2), 3;
	push @t, arrayarithexpr do { 1; 1 }, 2;
	push @t, arrayarithexpr 3, 4 if 1;
	push @t, arrayarithexpr 5, 6 if 0;
	push @t, arrayarithexpr (7, 8), 9;
	push @t, arrayarithexpr a => "b";
	push @t, arrayarithexpr 1 ? reverse 2, 3 : 4, 5;
	push @t, arrayarithexpr 0 ? reverse 2, 3 : 4, 5;
	push @t, 1 ? reverse arrayarithexpr 2, 3 : 4, 5;
	push @t, 0 ? reverse arrayarithexpr 2, 3 : 4, 5;
	push @t, arrayarithexpr reverse 1, 2, 3;
	push @t, sub { arrayarithexpr return 1, 2, 3 }->();
	push @t, arrayarithexpr myneg 1, 2, 3;
	push @t, arrayarithexpr map { -$_ } 1, 2, 3;
	push @t, arrayarithexpr mymap { -$_[0] } 1, 2, 3;
	push @t, arrayarithexpr AA->listmeth(1, 2), 3;
	push @t, arrayarithexpr listmeth AA (1, 2), 3;
	push @t, arrayarithexpr listmeth AA 1, 2, 3;
	push @t, arrayarithexpr not 1, 2;
	push @t, arrayarithexpr reverse 6, 7, 8 or 9;
	push @t, arrayarithexpr reverse 6, 7, 8 and 9;
	push @t, arrayarithexpr 1 << 2;
	push @t, arrayarithexpr 7 < 8;
};
is $@, "";
is_deeply \@t, [
	[3],
	[0],
	[1],
	[0], 3,
	[1], 3,
	[1], 2,
	[0],
	[1],
	[0],
	[1],
	[1],
	[1], 2, 3,
	[1], 2,
	[3], 4,
	[7,8], 9,
	["a"], "b",
	3, 2, 5,
	3, 2, 5,
	3, [2], 5,
	4, 5,
	[3,2,1],
	1, 2, 3,
	[-1,-2,-3],
	[-1,-2,-3],
	[-1,-2,-3],
	[-1,-2], 3,
	[-1,-2], 3,
	[-1,-2,-3],
	[!1],
	[8,7,6],
	[8,7,6],
	[4],
	!!0,
];

@t = ();
eval q{
	use XS::APItest qw(arrayexprflags);
	push @t, arrayexprflags! 1, 2;
};
is $@, "";
is_deeply \@t, [ [1,2] ];

@t = ();
eval q{
	use XS::APItest qw(arrayexprflags);
	push @t, arrayexprflags? 1, 2;
};
is $@, "";
is_deeply \@t, [ [1,2] ];

@t = ();
eval q{
	use XS::APItest qw(arrayexprflags);
	push @t, arrayexprflags! [);
};
like $@, qr/\A(?:Parse|syntax) error/;
is_deeply \@t, [];

@t = ();
eval q{
	use XS::APItest qw(arrayexprflags);
	push @t, arrayexprflags? [);
};
like $@, qr/\A(?:Parse|syntax) error/;
is_deeply \@t, [];

@t = ();
eval q{
	use XS::APItest qw(arrayexprflags);
	push @t, arrayexprflags! ;
};
like $@, qr/\A(?:Parse|syntax) error/;
is_deeply \@t, [];

@t = ();
eval q{
	use XS::APItest qw(arrayexprflags);
	push @t, arrayexprflags? ;
};
is $@, "";
is_deeply \@t, [ {} ];

1;
