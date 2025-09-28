#!./perl

#
# grep() and map() tests
#

BEGIN {
    chdir 't' if -d 't'; 
    require "./test.pl";
    set_up_inc( qw(. ../lib) );
}

plan( tests => 67 );

{
    my @lol = ([qw(a b c)], [], [qw(1 2 3)]);
    my @mapped = map  {scalar @$_} @lol;
    cmp_ok("@mapped", 'eq', "3 0 3", 'map scalar list of list');

    my @grepped = grep {scalar @$_} @lol;
    cmp_ok("@grepped", 'eq', "$lol[0] $lol[2]", 'grep scalar list of list');
    $test++;

    @grepped = grep { $_ } @mapped;
    cmp_ok( "@grepped", 'eq',  "3 3", 'grep basic');
}

{
    my @res;

    @res = map({$_} ("geronimo"));
    cmp_ok( scalar(@res), '==', 1, 'basic map nr');
    cmp_ok( $res[0], 'eq', 'geronimo', 'basic map is');

    @res = map
             ({$_} ("yoyodyne"));
    cmp_ok( scalar(@res), '==', 1, 'linefeed map nr');
    cmp_ok( $res[0], 'eq', 'yoyodyne', 'linefeed map is');

    @res = (map(
       {a =>$_},
     ("chobb")))[0]->{a};
    cmp_ok( scalar(@res), '==', 1, 'deref map nr');
    cmp_ok( $res[0], 'eq', 'chobb', 'deref map is');

    @res = map {$_} ("geronimo");
    cmp_ok( scalar(@res), '==', 1, 'no paren basic map nr');
    cmp_ok( $res[0], 'eq', 'geronimo', 'no paren basic map is');

    @res = map
             {$_} ("yoyodyne");
    cmp_ok( scalar(@res), '==', 1, 'no paren linefeed map nr');
    cmp_ok( $res[0], 'eq', 'yoyodyne', 'no paren linefeed map is');

    @res = (map
           {a =>$_},
       ("chobb"))[0]->{a};
    cmp_ok( scalar(@res), '==', 1, 'no paren deref map nr');
    cmp_ok( $res[0], 'eq', 'chobb', 'no paren deref map is');

    my $x = "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\n";

    @res = map($_&$x,("sferics\n"));
    cmp_ok( scalar(@res), '==', 1, 'binand map nr 1');
    cmp_ok( $res[0], 'eq', "sferics\n", 'binand map is 1');

    @res = map
            ($_ & $x, ("sferics\n"));
    cmp_ok( scalar(@res), '==', 1, 'binand map nr 2');
    cmp_ok( $res[0], 'eq', "sferics\n", 'binand map is 2');

    @res = map { $_ & $x } ("sferics\n");
    cmp_ok( scalar(@res), '==', 1, 'binand map nr 3');
    cmp_ok( $res[0], 'eq', "sferics\n", 'binand map is 3');

    @res = map
             { $_&$x } ("sferics\n");
    cmp_ok( scalar(@res), '==', 1, 'binand map nr 4');
    cmp_ok( $res[0], 'eq', "sferics\n", 'binand map is 4');

    @res = grep({$_} ("geronimo"));
    cmp_ok( scalar(@res), '==', 1, 'basic grep nr');
    cmp_ok( $res[0], 'eq', 'geronimo', 'basic grep is');

    @res = grep
                ({$_} ("yoyodyne"));
    cmp_ok( scalar(@res), '==', 1, 'linefeed grep nr');
    cmp_ok( $res[0], 'eq', 'yoyodyne', 'linefeed grep is');

    @res = grep
        ({a=>$_}->{a},
        ("chobb"));
    cmp_ok( scalar(@res), '==', 1, 'deref grep nr');
    cmp_ok( $res[0], 'eq', 'chobb', 'deref grep is');

    @res = grep {$_} ("geronimo");
    cmp_ok( scalar(@res), '==', 1, 'no paren basic grep nr');
    cmp_ok( $res[0], 'eq', 'geronimo', 'no paren basic grep is');

    @res = grep
                {$_} ("yoyodyne");
    cmp_ok( scalar(@res), '==', 1, 'no paren linefeed grep nr');
    cmp_ok( $res[0], 'eq', 'yoyodyne', 'no paren linefeed grep is');

    @res = grep {a=>$_}->{a}, ("chobb");
    cmp_ok( scalar(@res), '==', 1, 'no paren deref grep nr');
    cmp_ok( $res[0], 'eq', 'chobb', 'no paren deref grep is');

    @res = grep
         {a=>$_}->{a}, ("chobb");
    cmp_ok( scalar(@res), '==', 1, 'no paren deref linefeed  nr');
    cmp_ok( $res[0], 'eq', 'chobb', 'no paren deref linefeed  is');

    @res = grep($_&"X", ("bodine"));
    cmp_ok( scalar(@res), '==', 1, 'binand X grep nr');
    cmp_ok( $res[0], 'eq', 'bodine', 'binand X grep is');

    @res = grep
           ($_&"X", ("bodine"));
    cmp_ok( scalar(@res), '==', 1, 'binand X linefeed grep nr');
    cmp_ok( $res[0], 'eq', 'bodine', 'binand X linefeed grep is');

    @res = grep {$_&"X"} ("bodine");
    cmp_ok( scalar(@res), '==', 1, 'no paren binand X grep nr');
    cmp_ok( $res[0], 'eq', 'bodine', 'no paren binand X grep is');

    @res = grep
           {$_&"X"} ("bodine");
    cmp_ok( scalar(@res), '==', 1, 'no paren binand X linefeed grep nr');
    cmp_ok( $res[0], 'eq', 'bodine', 'no paren binand X linefeed grep is');
}

{
    # Tests for "for" in "map" and "grep"
    # Used to dump core, bug [perl #17771]

    my @x;
    my $y = '';
    @x = map { $y .= $_ for 1..2; 1 } 3..4;
    cmp_ok( "@x,$y",'eq',"1 1,1212", '[perl #17771] for in map 1');

    $y = '';
    @x = map { $y .= $_ for 1..2; $y .= $_ } 3..4;
    cmp_ok( "@x,$y",'eq',"123 123124,123124", '[perl #17771] for in map 2');

    $y = '';
    @x = map { for (1..2) { $y .= $_ } $y .= $_ } 3..4;
    cmp_ok( "@x,$y",'eq',"123 123124,123124", '[perl #17771] for in map 3');

    $y = '';
    @x = grep { $y .= $_ for 1..2; 1 } 3..4;
    cmp_ok( "@x,$y",'eq',"3 4,1212", '[perl #17771] for in grep 1');

    $y = '';
    @x = grep { for (1..2) { $y .= $_ } 1 } 3..4;
    cmp_ok( "@x,$y",'eq',"3 4,1212", '[perl #17771] for in grep 2');

    # Add also a sample test from [perl #18153].  (The same bug).
    $a = 1; map {if ($a){}} (2);
    pass( '[perl #18153] (not dead yet)' ); # no core dump is all we need
}

{
    sub add_an_x(@){
        map {"${_}x"} @_;
    };
    cmp_ok( join("-",add_an_x(1,2,3,4)), 'eq', "1x-2x-3x-4x", 'add-an-x');
}

{
    my $gimme;

    sub gimme {
        my $want = wantarray();
        if (defined $want) {
            $gimme = $want ? 'list' : 'scalar';
        } else {
            $gimme = 'void';
        }
    }

    my @list = 0..9;

    undef $gimme; gimme for @list;      cmp_ok($gimme, 'eq', 'void',   'gimme a V!');
    undef $gimme; grep { gimme } @list; cmp_ok($gimme, 'eq', 'scalar', 'gimme an S!');
    undef $gimme; map { gimme } @list;  cmp_ok($gimme, 'eq', 'list',   'gimme an L!');
}

{
    # test scalar context return
    my @list = (7, 14, 21);

    my $x = map {$_ *= 2} @list;
    cmp_ok("@list", 'eq', "14 28 42", 'map scalar return');
    cmp_ok($x, '==', 3, 'map scalar count');

    @list = (9, 16, 25, 36);
    $x = grep {$_ % 2} @list;
    cmp_ok($x, '==', 2, 'grep scalar count');

    my @res = grep {$_ % 2} @list;
    cmp_ok("@res", 'eq', "9 25", 'grep extract');
}

{
    # This shouldn't loop indefinitely.
    my @empty = map { while (1) {} } ();
    cmp_ok("@empty", 'eq', '', 'staying alive');
}

{
    my $x;
    eval 'grep $x (1,2,3);';
    like($@, qr/Missing comma after first argument to grep function/,
         "proper error on variable as block. [perl #37314]");
}

# [perl #78194] grep/map aliasing op return values
grep is(\$_, \$_, '[perl #78194] \$_ == \$_ inside grep ..., "$x"'),
     "${\''}", "${\''}";
map is(\$_, \$_, '[perl #78194] \$_ == \$_ inside map ..., "$x"'),
     "${\''}", "${\''}";

# [perl #92254] freeing $_ in gremap block
{
    my $y;
    grep { undef *_ } $y;
    map { undef *_ } $y;
}
pass 'no double frees with grep/map { undef *_ }';

# Don't mortalise PADTMPs.
# This failed while I was messing with leave stuff (but not in a simple
# test, so add one). The '1;' ensures the block is wrapped in ENTER/LEAVE;
# the stringify returns a PADTMP. DAPM.

{
    my @a = map { 1; "$_" } 1,2;
    is("@a", "1 2", "PADTMP");
}
