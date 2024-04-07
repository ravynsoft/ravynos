#!./perl

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc( qw(. ../lib) );
}

plan( tests => 56 );

# delete() on hash elements

$foo{1} = 'a';
$foo{2} = 'b';
$foo{3} = 'c';
$foo{4} = 'd';
$foo{5} = 'e';
$foo{6} = 'f';
$foo{7} = 'g';

$foo = delete $foo{2};

cmp_ok($foo,'eq','b','delete 2');
ok(!(exists $foo{2}),'b absent');
cmp_ok($foo{1},'eq','a','a exists');
cmp_ok($foo{3},'eq','c','c exists');
cmp_ok($foo{4},'eq','d','d exists');
cmp_ok($foo{5},'eq','e','e exists');

@foo = delete @foo{4, 5};

cmp_ok(scalar(@foo),'==',2,'deleted slice');
cmp_ok($foo[0],'eq','d','slice 1');
cmp_ok($foo[1],'eq','e','slice 2');
ok(!(exists $foo{4}),'d absent');
ok(!(exists $foo{5}),'e absent');
cmp_ok($foo{1},'eq','a','a still exists');
cmp_ok($foo{3},'eq','c','c still exists');

@foo = delete %foo{6,7};

cmp_ok(scalar(@foo),'==',4,'deleted kvslice');
cmp_ok($foo[0],'eq','6','slice k1');
cmp_ok($foo[1],'eq','f','slice v1');
cmp_ok($foo[2],'eq','7','slice k2');
cmp_ok($foo[3],'eq','g','slice v2');
ok(!(exists $foo{5}),'f absent');
ok(!(exists $foo{6}),'g absent');
cmp_ok($foo{1},'eq','a','a still exists');
cmp_ok($foo{3},'eq','c','c still exists');

$foo = join('',values(%foo));
ok($foo eq 'ac' || $foo eq 'ca','remaining keys');

foreach $key (keys %foo) {
    delete $foo{$key};
}

$foo{'foo'} = 'x';
$foo{'bar'} = 'y';

$foo = join('',values(%foo));
ok($foo eq 'xy' || $foo eq 'yx','fresh keys');

$refhash{"top"}->{"foo"} = "FOO";
$refhash{"top"}->{"bar"} = "BAR";

delete $refhash{"top"}->{"bar"};
@list = keys %{$refhash{"top"}};

cmp_ok("@list",'eq',"foo", 'autoviv and delete hashref');

{
    my %a = ('bar', 33);
    my($a) = \(values %a);
    my $b = \$a{bar};
    my $c = \delete $a{bar};

    ok($a == $b && $b == $c,'a b c equivalent');
}

# delete() on array elements

@foo = ();
$foo[1] = 'a';
$foo[2] = 'b';
$foo[3] = 'c';
$foo[4] = 'd';
$foo[5] = 'e';
$foo[6] = 'f';
$foo[7] = 'g';

$foo = delete $foo[2];

cmp_ok($foo,'eq','b','ary delete 2');
ok(!(exists $foo[2]),'ary b absent');
cmp_ok($foo[1],'eq','a','ary a exists');
cmp_ok($foo[3],'eq','c','ary c exists');
cmp_ok($foo[4],'eq','d','ary d exists');
cmp_ok($foo[5],'eq','e','ary e exists');

@bar = delete @foo[4,5];

cmp_ok(scalar(@bar),'==',2,'ary deleted slice');
cmp_ok($bar[0],'eq','d','ary slice 1');
cmp_ok($bar[1],'eq','e','ary slice 2');
ok(!(exists $foo[4]),'ary d absent');
ok(!(exists $foo[5]),'ary e absent');
cmp_ok($foo[1],'eq','a','ary a still exists');
cmp_ok($foo[3],'eq','c','ary c still exists');

@bar = delete %foo[6,7];

cmp_ok(scalar(@bar),'==',4,'deleted kvslice');
cmp_ok($bar[0],'eq','6','slice k1');
cmp_ok($bar[1],'eq','f','slice v1');
cmp_ok($bar[2],'eq','7','slice k2');
cmp_ok($bar[3],'eq','g','slice v2');
ok(!(exists $bar[5]),'f absent');
ok(!(exists $bar[6]),'g absent');
cmp_ok($foo[1],'eq','a','a still exists');
cmp_ok($foo[3],'eq','c','c still exists');

$foo = join('',@foo);
cmp_ok($foo,'eq','ac','ary elems');
cmp_ok(scalar(@foo),'==',4,'four is the number thou shalt count');

foreach $key (0 .. $#foo) {
    delete $foo[$key];
}

cmp_ok(scalar(@foo),'==',0,'and then there were none');

$foo[0] = 'x';
$foo[1] = 'y';

$foo = "@foo";
cmp_ok($foo,'eq','x y','two fresh');

$refary[0]->[0] = "FOO";
$refary[0]->[3] = "BAR";

delete $refary[0]->[3];

cmp_ok( scalar(@{$refary[0]}),'==',1,'one down');

{
    my @a = 33;
    my($a) = \(@a);
    my $b = \$a[0];
    my $c = \delete $a[bar];

    ok($a == $b && $b == $c,'a b c also equivalent');
}

{
    my %h;
    my ($x,$y) = (1, scalar delete @h{()});
    ok(!defined($y),q([perl #29127] scalar delete of empty slice returned garbage));
}

{
    my $x = 0;
    sub X::DESTROY { $x++ }
    {
	my @a;
	$a[0] = bless [], 'X';
	my $y = delete $a[0];
    }
    cmp_ok($x,'==',1,q([perl #30733] array delete didn't free returned element));
}
