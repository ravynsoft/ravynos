#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

@tests = split(/\n/, <<EOF);
0 3,			0 1 2,		3 4 5 6 7
0 0 a b c,		,		a b c 0 1 2 3 4 5 6 7
8 0 a b c,		,		0 1 2 3 4 5 6 7 a b c
7 0 6.5,		,		0 1 2 3 4 5 6 6.5 7
1 0 a b c d e f g h i j,,		0 a b c d e f g h i j 1 2 3 4 5 6 7
0 1 a,			0,		a 1 2 3 4 5 6 7
1 6 x y z,		1 2 3 4 5 6,	0 x y z 7
0 7 x y z,		0 1 2 3 4 5 6,	x y z 7
1 7 x y z,		1 2 3 4 5 6 7,	0 x y z
4,			4 5 6 7,	0 1 2 3
-4,			4 5 6 7,	0 1 2 3
EOF

plan tests => 10 + @tests*2;
die "blech" unless @tests;

@x = (1,2,3);
push(@x,@x);
is( join(':',@x), '1:2:3:1:2:3', 'push array onto array');
push(@x,4);
is( join(':',@x), '1:2:3:1:2:3:4', 'push integer onto array');

# test autovivification
push @$undef1, 1, 2, 3;
is( join(':',@$undef1), '1:2:3', 'autovivify array');

# test implicit dereference errors
eval "push 42, 0, 1, 2, 3";
like ( $@, qr/must be array/, 'push onto a literal integer');

$hashref = { };
eval q{ push $hashref, 0, 1, 2, 3 };
like( $@, qr/Experimental push on scalar is now forbidden/, 'push onto a hashref');

eval q{ push bless([]), 0, 1, 2, 3 };
like( $@, qr/Experimental push on scalar is now forbidden/, 'push onto a blessed array ref');

$test = 13;

# test context
{
    my($first, $second) = ([1], [2]);
    sub two_things { return +($first, $second) }
    push @{ two_things() }, 3;
    is( join(':',@$first), '1', "\$first = [ @$first ];");
    is( join(':',@$second), '2:3', "\$second = [ @$second ]");
}

foreach $line (@tests) {
    ($list,$get,$leave) = split(/,\t*/,$line);
    ($pos, $len, @list) = split(' ',$list);
    @get = split(' ',$get);
    @leave = split(' ',$leave);
    @x = (0,1,2,3,4,5,6,7);
    if (defined $len) {
	@got = splice(@x, $pos, $len, @list);
    }
    else {
	@got = splice(@x, $pos);
    }
    is(join(':',@got), join(':',@get),   "got: @got == @get");
    is(join(':',@x),   join(':',@leave), "left: @x == @leave");
}

# See RT#131000
{
    local $@;
    my @readonly_array = 10..11;
    Internals::SvREADONLY(@readonly_array, 1);
    eval { push @readonly_array, () };
    is $@, '', "can push empty list onto readonly array";

    eval { push @readonly_array, 9 };
    like $@, qr/^Modification of a read-only value/,
        "croak when pushing onto readonly array";
}

1;  # this file is require'd by lib/tie-stdpush.t
