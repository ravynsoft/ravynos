#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use utf8;
use open qw( :utf8 :std );
plan (84);

sub expected {
    my($object, $package, $type) = @_;
    print "# $object $package $type\n";
    is(ref($object), $package);
    my $r = qr/^\Q$package\E=(\w+)\(0x([0-9a-f]+)\)$/u;
    like("$object", $r);
    if ("$object" =~ $r) {
	is($1, $type);
	# in 64-bit platforms hex warns for 32+ -bit values
	cmp_ok(do {no warnings 'portable'; hex($2)}, '==', $object);
    }
    else {
	fail(); fail();
    }
}

# test blessing simple types

$a1 = bless {}, "ዐ";
expected($a1, "ዐ", "HASH");
$b1 = bless [], "Ｂ";
expected($b1, "Ｂ", "ARRAY");
$c1 = bless \(map "$_", "test"), "ᶜ";
expected($c1, "ᶜ", "SCALAR");
$tèst = "foo"; $d1 = bless \*tèst, "ɖ";
expected($d1, "ɖ", "GLOB");
$e1 = bless sub { 1 }, "ಎ";
expected($e1, "ಎ", "CODE");
$f1 = bless \[], "ḟ";
expected($f1, "ḟ", "REF");
$g1 = bless \substr("test", 1, 2), "ㄍ";
expected($g1, "ㄍ", "LVALUE");

# blessing ref to object doesn't modify object

expected(bless(\$a1, "ḟ"), "ḟ", "REF");
expected($a1, "ዐ", "HASH");

# reblessing does modify object

bless $a1, "ዐ2";
expected($a1, "ዐ2", "HASH");

# local and my
{
    local $a1 = bless $a1, "ዐ3";	# should rebless outer $a1
    local $b1 = bless [], "Ｂ3";
    my $c1 = bless $c1, "ᶜ3";		# should rebless outer $c1
    our $test2 = ""; my $d1 = bless \*test2, "ɖ3";
    expected($a1, "ዐ3", "HASH");
    expected($b1, "Ｂ3", "ARRAY");
    expected($c1, "ᶜ3", "SCALAR");
    expected($d1, "ɖ3", "GLOB");
}
expected($a1, "ዐ3", "HASH");
expected($b1, "Ｂ", "ARRAY");
expected($c1, "ᶜ3", "SCALAR");
expected($d1, "ɖ", "GLOB");

# class is magic
"ಎ" =~ /(.)/;
expected(bless({}, $1), "ಎ", "HASH");
{
    local $! = 1;
    my $string = "$!";
    $! = 2;	# attempt to avoid cached string
    $! = 1;
    expected(bless({}, $!), $string, "HASH");

# ref is ref to magic
    {
	{
	    package ḟ;
	    sub test { main::is(${$_[0]}, $string) }
	}
	$! = 2;
	$f1 = bless \$!, "ḟ";
	$! = 1;
	$f1->test;
    }
}

# ref is magic

# class is a ref
$a1 = bless {}, "ዐ4";
$b1 = eval { bless {}, $a1 };
isnt ($@, '', "class is a ref");

# class is an overloaded ref
=begin
$TODO = "Package not yet clean";
{
    package ᚺ4;
    use overload '""' => sub { "ᶜ4" };
}
$h1 = bless {}, "ᚺ4";
$c4 = eval { bless \$test, $h1 };
is ($@, '', "class is an overloaded ref");
expected($c4, 'ᶜ4', "SCALAR");
=cut

{
    my %h = 1..2;
    my($k) = keys %h; 
    my $x=\$k;
    bless $x, 'pàm';
    is(ref $x, 'pàm');

    my $a = bless \(keys %h), 'zàp';
    is(ref $a, 'zàp');
}
