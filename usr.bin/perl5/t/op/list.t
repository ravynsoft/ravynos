#!./perl

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc(qw(. ../lib));
}

plan( tests => 73 );

@foo = (1, 2, 3, 4);
cmp_ok($foo[0], '==', 1, 'first elem');
cmp_ok($foo[3], '==', 4, 'last elem');

$_ = join(':',@foo);
cmp_ok($_, 'eq', '1:2:3:4', 'join list');

($a,$b,$c,$d) = (1,2,3,4);
cmp_ok("$a;$b;$c;$d", 'eq', '1;2;3;4', 'list assign');

($c,$b,$a) = split(/ /,"111 222 333");
cmp_ok("$a;$b;$c",'eq','333;222;111','list split on space');

($a,$b,$c) = ($c,$b,$a);
cmp_ok("$a;$b;$c",'eq','111;222;333','trio rotate');

($a, $b) = ($b, $a);
cmp_ok("$a-$b",'eq','222-111','duo swap');

($a, $b) = ($b, $a) = ($a, $b);
cmp_ok("$a-$b",'eq','222-111','duo swap swap');

($a, $b[1], $c{2}, $d) = (1, 2, 3, 4);
cmp_ok($a,'==',1,'assign scalar in list');
cmp_ok($b[1],'==',2,'assign aelem in list');
cmp_ok($c{2},'==',3,'assign helem in list');
cmp_ok($d,'==',4,'assign last scalar in list');

@foo = (1,2,3,4,5,6,7,8);
($a, $b, $c, $d) = @foo;
cmp_ok("$a/$b/$c/$d",'eq','1/2/3/4','long list assign');

@foo = (1,2);
($a, $b, $c, $d) = @foo;
cmp_ok($a,'==',1,'short list 1 defined');
cmp_ok($b,'==',2,'short list 2 defined');
ok(!defined($c),'short list 3 undef');
ok(!defined($d),'short list 4 undef');

@foo = @bar = (1);
cmp_ok(join(':',@foo,@bar),'eq','1:1','list reassign');

@foo = @bar = (2,3);
cmp_ok(join(':',join('+',@foo),join('-',@bar)),'eq','2+3:2-3','long list reassign');

@foo = ();
@foo = 1+2+3;
cmp_ok(join(':',@foo),'eq','6','scalar assign to array');

{
    my ($a, $b, $c);
    for ($x = 0; $x < 3; $x = $x + 1) {
        ($a, $b, $c) = 
              $x == 0 ?  ('a','b','c')
            : $x == 1 ?  ('d','e','f')
            :            ('g','h','i')
        ;
        if ($x == 0) {
            cmp_ok($a,'eq','a','ternary for a 1');
            cmp_ok($b,'eq','b','ternary for b 1');
            cmp_ok($c,'eq','c','ternary for c 1');
        }
        if ($x == 1) {
            cmp_ok($a,'eq','d','ternary for a 2');
            cmp_ok($b,'eq','e','ternary for b 2');
            cmp_ok($c,'eq','f','ternary for c 2');
        }
        if ($x == 2) {
            cmp_ok($a,'eq','g','ternary for a 3');
            cmp_ok($b,'eq','h','ternary for b 3');
            cmp_ok($c,'eq','i','ternary for c 3');
        }
    }
}

{
    my ($a, $b, $c);
    for ($x = 0; $x < 3; $x = $x + 1) {
        ($a, $b, $c) = do {
            if ($x == 0) {
                ('a','b','c');
            }
            elsif ($x == 1) {
                ('d','e','f');
            }
            else {
                ('g','h','i');
            }
        };
        if ($x == 0) {
            cmp_ok($a,'eq','a','block for a 1');
            cmp_ok($b,'eq','b','block for b 1');
            cmp_ok($c,'eq','c','block for c 1');
        }
        if ($x == 1) {
            cmp_ok($a,'eq','d','block for a 2');
            cmp_ok($b,'eq','e','block for b 2');
            cmp_ok($c,'eq','f','block for c 2');
        }
        if ($x == 2) {
            cmp_ok($a,'eq','g','block for a 3');
            cmp_ok($b,'eq','h','block for b 3');
            cmp_ok($c,'eq','i','block for c 3');
        }
    }
}

$x = 666;
@a = ($x == 12345 || (1,2,3));
cmp_ok(join('*',@a),'eq','1*2*3','logical or f');

@a = ($x == $x || (4,5,6));
cmp_ok(join('*',@a),'eq','1','logical or t');

cmp_ok(join('',1,2,(3,4,5)),'eq','12345','list ..(...)');
cmp_ok(join('',(1,2,3,4,5)),'eq','12345','list (.....)');
cmp_ok(join('',(1,2,3,4),5),'eq','12345','list (....).');
cmp_ok(join('',1,(2,3,4),5),'eq','12345','list .(...).');
cmp_ok(join('',1,2,(3,4),5),'eq','12345','list ..(..).');
cmp_ok(join('',1,2,3,(4),5),'eq','12345','list ...(.).');
cmp_ok(join('',(1,2),3,(4,5)),'eq','12345','list (..).(..)');

{
    my @a = (0, undef, undef, 3);
    my @b = @a[1,2];
    my @c = (0, undef, undef, 3)[1, 2];
    cmp_ok(scalar(@b),'==',scalar(@c),'slice and slice');
    cmp_ok(scalar(@c),'==',2,'slice len');

    @b = (29, scalar @c[()]);
    cmp_ok(join(':',@b),'eq','29:','slice ary nil');

    my %h = (a => 1);
    @b = (30, scalar @h{()});
    cmp_ok(join(':',@b),'eq','30:','slice hash nil');

    my $size = scalar(()[1..1]);
    cmp_ok($size,'==','0','size nil');

    $size = scalar(()=((1,2,3,4,5)[()])[2,3,4]);
    is $size, 0, 'slice of empty list from complex expr is empty list';

    @a = (1)[2,3,4];
    is "@{[ map $_//'undef', @a ]}", "undef undef undef",
       'slice beyond the end of non-empty list returns undefs';
}

{
    # perl #39882
    sub test_two_args {
        my $test_name = shift;
        is(scalar(@_), 2, $test_name);
    }
    test_two_args("simple list slice",      (10,11)[2,3]);
    test_two_args("grepped list slice",     grep(1, (10,11)[2,3]));
    test_two_args("sorted list slice",      sort((10,11)[2,3]));
    test_two_args("assigned list slice",    my @tmp = (10,11)[2,3]);
    test_two_args("do-returned list slice", do { (10,11)[2,3]; });
    test_two_args("list literal slice",     qw(a b)[2,3]);
    is (()=qw()[2,3], 0, "empty literal slice");
}

{
    # perl #20321
    is (join('', @{[('abc'=~/./g)[0,1,2,1,0]]}), "abcba");
}

{
    is(join('', qw(a b c)[2,0,1]), "cab");
    my @a = qw(a b c);
    is(join(":", @a), "a:b:c");
    my @b = qw();
    is($#b, -1);
}

{
    # comma operator with lvalue only propagates the lvalue context to
    # the last operand.
    ("const", my $x) ||= 1;
    is( $x, 1 );
}

# [perl #78194] list slice aliasing op return values
sub {
 is(\$_[0], \$_[1],
  '[perl #78194] \$_[0] == \$_[1] when @_ aliases elems repeated by lslice'
 )
}
 ->(("${\''}")[0,0]);

# [perl #122995] Hang when compiling while(1) in a sub-list
# No ok() or is() necessary.
sub foo { () = ($a, my $b, ($c, do { while(1) {} })) }

# List assignment and OPpTARGET_MY
{
    my ($a,$b);
    my $foo = "foo";
    my $bar = "bar";
    ($a,$b) = ($b = $foo."", $a = $bar . "");
    is("$a,$b", "foo,bar", 'common vars check accounts for OPpTARGET_MY');
}

sub TIESCALAR {bless{}}
sub FETCH {$_[0]{fetched}++}
sub empty {}
tie $t, "";
() = (empty(), ($t)x10); # empty() since sub calls usually result in copies
is(tied($t)->{fetched}, undef, 'assignment to empty list makes no copies');

# this was passing a trash SV at the top of the stack to SvIV()
ok(($0[()[()]],1), "[perl #126193] list slice with zero indexes");

# RT #131732: pp_list must extend stack when empty-array arg and not in list
# context
{
    my @x;
    @x;
    pass('no panic'); # panics only under DEBUGGING
}

fresh_perl_is(<<'EOS', "", {}, "[perl #131954] heap use after free in pp_list");
#!./perl
BEGIN {
my $bar = "bar";

sub test_no_error {
    eval $_[0];
}

test_no_error($_) for split /\n/,
q[	x
	definfoo, $bar;
	x
	x
	x
	grep((not $bar, $bar, $bar), $bar);
        x
        x
    x
        x
        x
        x
        x
        x
        x
        x
        x
        x
        x
       x
        x
        x
        x
        x
        x
        x
 ];
}
EOS

# this used to SEGV due to deep recursion in Perl_list()

{
    my $e = "1"; $e = "(1,$e)" for 1..100_000; $e = "() = $e"; eval $e;
    is $@, "", "SEGV in Perl_list";
}
