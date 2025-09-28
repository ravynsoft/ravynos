#!./perl

#
# test the logical operators '&&', '||', '!', 'and', 'or', , 'xor', 'not'
#

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan tests => 33;

for my $i (undef, 0 .. 2, "", "0 but true") {
    my $true = 1;
    my $false = 0;
    for my $j (undef, 0 .. 2, "", "0 but true") {
	$true &&= !(
	    ((!$i || !$j) != !($i && $j))
	    or (!($i || $j) != (!$i && !$j))
	    or (!!($i || $j) != !(!$i && !$j))
	    or (!(!$i || !$j) != !!($i && $j))
	);
	$false ||= (
	    ((!$i || !$j) == !!($i && $j))
	    and (!!($i || $j) == (!$i && !$j))
	    and ((!$i || $j) == ($i && !$j))
	    and (($i || !$j) != (!$i && $j))
	);
    }
    my $m = ! defined $i ? 'undef'
       : $i eq ''   ? 'empty string'
       : $i;
    ok( $true, "true: $m");
    ok( ! $false, "false: $m");
}

my $i = 0;
(($i ||= 1) &&= 3) += 4;
is( $i, 7, '||=, &&=');

my ($x, $y) = (1, 8);
$i = !$x || $y;
is( $i, 8, 'negation precedence with ||' );

++$y;
$i = !$x || !$x || !$x || $y;
is( $i, 9, 'negation precedence with ||, multiple operands' );

$x = 0;
++$y;
$i = !$x && $y;
is( $i, 10, 'negation precedence with &&' );

++$y;
$i = !$x && !$x && !$x && $y;
is( $i, 11, 'negation precedence with &&, multiple operands' );

# [perl #127952]. This relates to OP_AND and OP_OR with a negated constant
# on the lhs (either a negated bareword, or a negation of a do{} containing
# a constant) and a negated non-foldable expression on the rhs. These cases
# yielded 42 or "Bare" or "str" before the bug was fixed.
{
    $x = 42;

    $i = !Bare || !$x;
    is( $i, '', 'neg-bareword on lhs of || with non-foldable neg-true on rhs' );

    $i = !Bare && !$x;
    is( $i, '', 'neg-bareword on lhs of && with non-foldable neg-true on rhs' );

    $i = do { !$x if !Bare };
    is( $i, '', 'neg-bareword on rhs of modifier-if with non-foldable neg-true on lhs' );

    $i = do { !$x unless !Bare };
    is( $i, '', 'neg-bareword on rhs of modifier-unless with non-foldable neg-true on lhs' );

    $i = !do { "str" } || !$x;
    is( $i, '', 'neg-do-const on lhs of || with non-foldable neg-true on rhs' );

    $i = !do { "str" } && !$x;
    is( $i, '', 'neg-do-const on lhs of && with non-foldable neg-true on rhs' );
}

# RT #131820
#
# It turns out that in 2017, 23 years after the release of perl5,
# the 'xor' logical operator was still untested in core.

for my $test (
    [ 0, 0, '' ],
    [ 0, 1, 1  ],
    [ 1, 0, 1  ],
    [ 1, 1, '' ],

    [ 0, 2, 1  ],
    [ 2, 0, 1  ],
    [ 2, 2, '' ],

    [ 0, 3, 1  ],
    [ 3, 0, 1  ],
    [ 3, 4, '' ],
) {
    my ($a,$b, $exp) = @$test;
    is(($a xor $b), $exp, "($a xor $b) == '$exp'");
}
