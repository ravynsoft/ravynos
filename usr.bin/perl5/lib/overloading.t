#./perl

use Test::More;

no warnings 'experimental::builtin';
use builtin qw(refaddr);

{
    package Stringifies;

    use overload (
	fallback => 1,
	'""' => sub { "foo" },
	'0+' => sub { 42 },
	cos => sub { "far side of overload table" },
    );

    sub new { bless {}, shift };
}

my $x = Stringifies->new;
my $y = qr//;
my $ystr = "$y";

is( "$x", "foo", "stringifies" );
is( "$y", $ystr, "stringifies qr//" );
is( 0 + $x, 42, "numifies" );
is( cos($x), "far side of overload table", "cosinusfies" );

{
    no overloading;
    is( "$x", overload::StrVal($x), "no stringification" );
    is( "$y", overload::StrVal($y), "no stringification of qr//" );
    is( 0 + $x, refaddr($x), "no numification" );
    is( cos($x), cos(refaddr($x)), "no cosinusfication" );

    {
	no overloading '""';
	is( "$x", overload::StrVal($x), "no stringification" );
	is( "$y", overload::StrVal($y), "no stringification of qr//" );
	is( 0 + $x, refaddr($x), "no numification" );
	is( cos($x), cos(refaddr($x)), "no cosinusfication" );
    }
}

{
    no overloading '""';

    is( "$x", overload::StrVal($x), "no stringification" );
    is( "$y", overload::StrVal($y), "no stringification of qr//" );
    is( 0 + $x, 42, "numifies" );
    is( cos($x), "far side of overload table", "cosinusfies" );

    my $q = qr/abc/;
    ok "abc" =~ $q, '=~ qr// with no "" overloading';
    ok "abcd" =~ /${q}d/, '=~ /foo$qr/ with no "" overloading';
    {
	no overloading 'qr';
	my $q = qr/abc/;
	ok "abc" =~ $q, '=~ qr// with no "" or qr overloading';
	ok "abcd" =~ /${q}d/, '=~ /foo$qr/ with no "" or qr overloading';
    }

    {
	no overloading;
	is( "$x", overload::StrVal($x), "no stringification" );
	is( "$y", overload::StrVal($y), "no stringification of qr//" );
	is( 0 + $x, refaddr($x), "no numification" );
	is( cos($x), cos(refaddr($x)), "no cosinusfication" );
    }

    use overloading '""';

    is( "$x", "foo", "stringifies" );
    is( "$y", $ystr, "stringifies qr//" );
    is( 0 + $x, 42, "numifies" );
    is( cos($x), "far side of overload table", "cosinusfies" );

    no overloading '0+';
    is( "$x", "foo", "stringifies" );
    is( "$y", $ystr, "stringifies qr//" );
    is( 0 + $x, refaddr($x), "no numification" );
    is( cos($x), "far side of overload table", "cosinusfies" );

    {
	no overloading '""';
	is( "$x", overload::StrVal($x), "no stringification" );
	is( "$y", overload::StrVal($y), "no stringification of qr//" );
	is( 0 + $x, refaddr($x), "no numification" );
	is( cos($x), "far side of overload table", "cosinusfies" );

	{
	    use overloading;
	    is( "$x", "foo", "stringifies" );
	    is( "$y", $ystr, "stringifies qr//" );
	    is( 0 + $x, 42, "numifies" );
	    is( cos($x), "far side of overload table", "cosinusfies" );
	}
    }

    is( "$x", "foo", "stringifies" );
    is( "$y", $ystr, "stringifies qr//" );
    is( 0 + $x, refaddr($x), "no numification" );
    is( cos($x), "far side of overload table", "cosinusfies" );

    no overloading "cos";
    is( "$x", "foo", "stringifies" );
    is( "$y", $ystr, "stringifies qr//" );
    is( 0 + $x, refaddr($x), "no numification" );
    is( cos($x), cos(refaddr($x)), "no cosinusfication" );

    BEGIN { ok(exists($^H{overloading}), "overloading hint present") }

    use overloading;

    BEGIN { ok(!exists($^H{overloading}), "overloading hint removed") }
}

done_testing();
