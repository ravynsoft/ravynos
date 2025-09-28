#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

##Literal test count since evals below can fail
plan tests => 13;

$a = 'ab' . 'c';	# compile time
$b = 'def';

$c = $a . $b;
is( $c, 'abcdef', 'compile time concatenation' );

$c .= 'xyz';
is( $c, 'abcdefxyz', 'concat to self');

$_ = $a;
$_ .= $b;
is( $_, 'abcdef', 'concat using $_');

# test that when right argument of concat is UTF8, and is the same
# variable as the target, and the left argument is not UTF8, it no
# longer frees the wrong string.
{
    sub r2 {
	my $string = '';
	$string .= pack("U0a*", 'mnopqrstuvwx');
	$string = "abcdefghijkl$string";
    }

    isnt(r2(), '', 'UTF8 concat does not free the wrong string');
    isnt(r2(), '', 'second check');
}

# test that nul bytes get copied
{
    my ($a, $ab)   = ("a", "a\0b");
    my ($ua, $uab) = map pack("U0a*", $_), $a, $ab;

    my $ub = pack("U0a*", 'b');

    #aa\0b
    my $t1 = $a; $t1 .= $ab;
    like( $t1, qr/b/, 'null bytes do not stop string copy, aa\0b');

    #a\0a\0b
    my $t2 = $a; $t2 .= $uab;
    ok( eval '$t2 =~ /$ub/', '... a\0a\0b' );

    #\0aa\0b
    my $t3 = $ua; $t3 .= $ab;
    ok( eval '$t3 =~ /$ub/', '... \0aa\0b' );

    my $t4 = $ua; $t4 .= $uab;
    ok( eval '$t4 =~ /$ub/', '... \0a\0a\0b' );

    my $t5 = $a; $t5 = $ab . $t5;
    like( $t5, qr/$ub/, '... a\0ba' );

    my $t6 = $a; $t6 = $uab . $t6;
    ok( eval '$t6 =~ /$ub/', '... \0a\0ba' );

    my $t7 = $ua; $t7 = $ab . $t7;
    like( $t7, qr/$ub/, '... a\0b\0a' );

    my $t8 = $ua; $t8 = $uab . $t8;
    ok( eval '$t8 =~ /$ub/', '... \0a\0b\0a' );
}
