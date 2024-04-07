#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require Config;
    if ($Config::Config{'uvsize'} != 8) {
        print "1..0 # Skip -- Perl configured with 32-bit ints\n";
        exit 0;
    }
}

$| = 1;
use Test::More;


my $ii = 36028797018963971;  # 2^55 + 3


### Tests with numerifying large positive int
{ package Oobj;
    use overload '0+' => sub { ${$_[0]} += 1; $ii },
                 'fallback' => 1;
}
my $oo = bless(\do{my $x = 0}, 'Oobj');
my $cnt = 1;

is("$oo", "$ii", '0+ overload with stringification');
is($$oo, $cnt++, 'overload called once');

is($oo>>3, $ii>>3, '0+ overload with bit shift right');
is($$oo, $cnt++, 'overload called once');

is($oo<<2, $ii<<2, '0+ overload with bit shift left');
is($$oo, $cnt++, 'overload called once');

is($oo|0xFF00, $ii|0xFF00, '0+ overload with bitwise or');
is($$oo, $cnt++, 'overload called once');

is($oo&0xFF03, $ii&0xFF03, '0+ overload with bitwise and');
is($$oo, $cnt++, 'overload called once');

ok($oo == $ii, '0+ overload with equality');
is($$oo, $cnt++, 'overload called once');

is(int($oo), $ii, '0+ overload with int()');
is($$oo, $cnt++, 'overload called once');

is(abs($oo), $ii, '0+ overload with abs()');
is($$oo, $cnt++, 'overload called once');

is(-$oo, -$ii, '0+ overload with unary minus');
is($$oo, $cnt++, 'overload called once');

is(0+$oo, $ii, '0+ overload with addition');
is($$oo, $cnt++, 'overload called once');
is($oo+0, $ii, '0+ overload with addition');
is($$oo, $cnt++, 'overload called once');
is($oo+$oo, 2*$ii, '0+ overload with addition');
$cnt++;
is($$oo, $cnt++, 'overload called once');

is(0-$oo, -$ii, '0+ overload with subtraction');
is($$oo, $cnt++, 'overload called once');
is($oo-99, $ii-99, '0+ overload with subtraction');
is($$oo, $cnt++, 'overload called once');

is(2*$oo, 2*$ii, '0+ overload with multiplication');
is($$oo, $cnt++, 'overload called once');
is($oo*3, 3*$ii, '0+ overload with multiplication');
is($$oo, $cnt++, 'overload called once');

is($oo/1, $ii, '0+ overload with division');
is($$oo, $cnt++, 'overload called once');
is($ii/$oo, 1, '0+ overload with division');
is($$oo, $cnt++, 'overload called once');

is($oo%100, $ii%100, '0+ overload with modulo');
is($$oo, $cnt++, 'overload called once');
is($ii%$oo, 0, '0+ overload with modulo');
is($$oo, $cnt++, 'overload called once');

is($oo**1, $ii, '0+ overload with exponentiation');
is($$oo, $cnt++, 'overload called once');


### Tests with numerifying large negative int
{ package Oobj2;
    use overload '0+' => sub { ${$_[0]} += 1; -$ii },
                 'fallback' => 1;
}
$oo = bless(\do{my $x = 0}, 'Oobj2');
$cnt = 1;

is(int($oo), -$ii, '0+ overload with int()');
is($$oo, $cnt++, 'overload called once');

is(abs($oo), $ii, '0+ overload with abs()');
is($$oo, $cnt++, 'overload called once');

is(-$oo, $ii, '0+ overload with unary -');
is($$oo, $cnt++, 'overload called once');

is(0+$oo, -$ii, '0+ overload with addition');
is($$oo, $cnt++, 'overload called once');
is($oo+0, -$ii, '0+ overload with addition');
is($$oo, $cnt++, 'overload called once');
is($oo+$oo, -2*$ii, '0+ overload with addition');
$cnt++;
is($$oo, $cnt++, 'overload called once');

is(0-$oo, $ii, '0+ overload with subtraction');
is($$oo, $cnt++, 'overload called once');

is(2*$oo, -2*$ii, '0+ overload with multiplication');
is($$oo, $cnt++, 'overload called once');
is($oo*3, -3*$ii, '0+ overload with multiplication');
is($$oo, $cnt++, 'overload called once');

is($oo/1, -$ii, '0+ overload with division');
is($$oo, $cnt++, 'overload called once');
is($ii/$oo, -1, '0+ overload with division');
is($$oo, $cnt++, 'overload called once');

is($oo%100, (-$ii)%100, '0+ overload with modulo');
is($$oo, $cnt++, 'overload called once');
is($ii%$oo, 0, '0+ overload with modulo');
is($$oo, $cnt++, 'overload called once');

is($oo**1, -$ii, '0+ overload with exponentiation');
is($$oo, $cnt++, 'overload called once');

### Tests with overloading but no fallback
{ package Oobj3;
    use overload
        'int' => sub { ${$_[0]} += 1; $ii },
        'abs' => sub { ${$_[0]} += 1; $ii },
        'neg' => sub { ${$_[0]} += 1; -$ii },
        '+' => sub {
            ${$_[0]} += 1;
            my $res = (ref($_[0]) eq __PACKAGE__) ? $ii : $_[0];
            $res   += (ref($_[1]) eq __PACKAGE__) ? $ii : $_[1];
        },
        '-' => sub {
            ${$_[0]} += 1;
            my ($l, $r) = ($_[2]) ? (1, 0) : (0, 1);
            my $res = (ref($_[$l]) eq __PACKAGE__) ? $ii : $_[$l];
            $res   -= (ref($_[$r]) eq __PACKAGE__) ? $ii : $_[$r];
        },
        '*' => sub {
            ${$_[0]} += 1;
            my $res = (ref($_[0]) eq __PACKAGE__) ? $ii : $_[0];
            $res   *= (ref($_[1]) eq __PACKAGE__) ? $ii : $_[1];
        },
        '/' => sub {
            ${$_[0]} += 1;
            my ($l, $r) = ($_[2]) ? (1, 0) : (0, 1);
            my $res = (ref($_[$l]) eq __PACKAGE__) ? $ii+1 : $_[$l];
            $res   /= (ref($_[$r]) eq __PACKAGE__) ? $ii+1 : $_[$r];
        },
        '%' => sub {
            ${$_[0]} += 1;
            my ($l, $r) = ($_[2]) ? (1, 0) : (0, 1);
            my $res = (ref($_[$l]) eq __PACKAGE__) ? $ii : $_[$l];
            $res   %= (ref($_[$r]) eq __PACKAGE__) ? $ii : $_[$r];
        },
        '**' => sub {
            ${$_[0]} += 1;
            my ($l, $r) = ($_[2]) ? (1, 0) : (0, 1);
            my $res = (ref($_[$l]) eq __PACKAGE__) ? $ii : $_[$l];
            $res  **= (ref($_[$r]) eq __PACKAGE__) ? $ii : $_[$r];
        },
}
$oo = bless(\do{my $x = 0}, 'Oobj3');
$cnt = 1;

is(int($oo), $ii, 'int() overload');
is($$oo, $cnt++, 'overload called once');

is(abs($oo), $ii, 'abs() overload');
is($$oo, $cnt++, 'overload called once');

is(-$oo, -$ii, 'neg overload');
is($$oo, $cnt++, 'overload called once');

is(0+$oo, $ii, '+ overload');
is($$oo, $cnt++, 'overload called once');
is($oo+0, $ii, '+ overload');
is($$oo, $cnt++, 'overload called once');
is($oo+$oo, 2*$ii, '+ overload');
is($$oo, $cnt++, 'overload called once');

is(0-$oo, -$ii, '- overload');
is($$oo, $cnt++, 'overload called once');
is($oo-99, $ii-99, '- overload');
is($$oo, $cnt++, 'overload called once');

is($oo*2, 2*$ii, '* overload');
is($$oo, $cnt++, 'overload called once');
is(-3*$oo, -3*$ii, '* overload');
is($$oo, $cnt++, 'overload called once');

is($oo/2, ($ii+1)/2, '/ overload');
is($$oo, $cnt++, 'overload called once');
is(($ii+1)/$oo, 1, '/ overload');
is($$oo, $cnt++, 'overload called once');

is($oo%100, $ii%100, '% overload');
is($$oo, $cnt++, 'overload called once');
is($ii%$oo, 0, '% overload');
is($$oo, $cnt++, 'overload called once');

is($oo**1, $ii, '** overload');
is($$oo, $cnt++, 'overload called once');

# RT #77456: when conversion method returns an IV/UV,
# avoid IV -> NV upgrade if possible .

{
    package P77456;
    use overload '0+' => sub  { $_[0][0] }, fallback => 1;

    package main;

    for my $expr (
	'(%531 + 1) - $a531  == 1',			# pp_add
	'$a531 - (%531 - 1) == 1',			# pp_subtract
	'(%531 * 2  + 1) - (%531 * 2)  == 1',		# pp_multiply
	'(%54  / 2  + 1) - (%54 / 2)   == 1',		# pp_divide
	'(%271 ** 2 + 1) - (%271 ** 2) == 1',		# pp_pow
	'(%541 % 2) == 1',				# pp_modulo
	'$a54  + (-%531)*2  == -2',			# pp_negate
	'(abs(%53m)+1) - $a53 == 1',			# pp_abs
	'(%531 << 1) - 2  == $a54',			# pp_left_shift
	'(%541 >> 1) + 1  == $a531',			# pp_right_shift
	'!(%53 == %531)',				# pp_eq
	'(%53 != %531)',				# pp_ne
	'(%53 < %531)',					# pp_lt
	'!(%531 <= %53)',				# pp_le
	'(%531 > %53)',					# pp_gt
	'!(%53 >= %531)',				# pp_ge
	'(%53 <=> %531) == -1',				# pp_ncmp
	'(%531 & %53) == $a53',				# pp_bit_and
	'(%531 | %53) == $a531',			# pp_bit_or
	'~(~ %531 + $a531) == 0',			# pp_complement
    ) {
	for my $int ('', 'use integer; ') {
	    (my $aexpr = "$int$expr") =~ s/\%(\d+m?)/\$a$1/g;
	    (my $bexpr = "$int$expr") =~ s/\%(\d+m?)/\$b$1/g;

	    my $a27   = 1 << 27;
	    my $a271  = $a27 + 1;
	    my $a53   = 1 << 53;
	    my $a53m  = -$a53;
	    my $a531  = $a53 + 1;
	    my $a54   = 1 << 54;
	    my $a541  = $a54 + 1;

	    my $b27   = bless [ $a27   ], 'P77456';
	    my $b271  = bless [ $a271  ], 'P77456';
	    my $b53   = bless [ $a53   ], 'P77456';
	    my $b53m  = bless [ $a53m  ], 'P77456';
	    my $b531  = bless [ $a531  ], 'P77456';
	    my $b54   = bless [ $a54   ], 'P77456';
	    my $b541  = bless [ $a541  ], 'P77456';

	    SKIP: {
		skip("IV/NV not suitable on this platform: $aexpr", 1)
		    unless eval $aexpr;
		ok(eval $bexpr, "IV: $bexpr");
	    }
	}
    }
}

done_testing();
