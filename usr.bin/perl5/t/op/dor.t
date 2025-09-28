#!./perl

# Test // and friends.

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc('../lib');
}

package main;

plan( tests => 34 );

my($x);

$x=1;
is($x // 0, 1,		'	// : left-hand operand defined');

$x = undef;
is($x // 1, 1, 		'	// : left-hand operand undef');

$x='';
is($x // 0, '',		'	// : left-hand operand defined but empty');

like([] // 0, qr/^ARRAY/,	'	// : left-hand operand a reference');

$x=undef;
$x //= 1;
is($x, 1, 		'	//=: left-hand operand undefined');

$x //= 0;
is($x, 1, 		'//=: left-hand operand defined');

$x = '';
$x //= 0;
is($x, '', 		'//=: left-hand operand defined but empty');

@ARGV = (undef, 0, 3);
is(shift       // 7, 7,	'shift // ... works');
is(shift()     // 7, 0,	'shift() // ... works');
is(shift @ARGV // 7, 3,	'shift @array // ... works');

@ARGV = (3, 0, undef);
is(pop         // 7, 7,	'pop // ... works');
is(pop()       // 7, 0,	'pop() // ... works');
is(pop @ARGV   // 7, 3,	'pop @array // ... works');

# Test that various syntaxes are allowed

for (qw(getc pos readline readlink undef umask <> <FOO> <$foo> -f)) {
    eval "sub { $_ // 0 }";
    is($@, '', "$_ // ... compiles");
}

# Test for some ambiguous syntaxes

eval q# sub f ($) { } f $x / 2; #;
is( $@, '', "'/' correctly parsed as arithmetic operator" );
eval q# sub f ($):lvalue { $y } f $x /= 2; #;
is( $@, '', "'/=' correctly parsed as assignment operator" );
eval q# sub f ($) { } f $x /2; #;
like( $@, qr/^Search pattern not terminated/,
    "Caught unterminated search pattern error message: empty subroutine" );
eval q# sub { print $fh / 2 } #;
is( $@, '',
    "'/' correctly parsed as arithmetic operator in sub with built-in function" );
eval q# sub { print $fh /2 } #;
like( $@, qr/^Search pattern not terminated/,
    "Caught unterminated search pattern error message: sub with built-in function" );

# [perl #28123] Perl optimizes // away incorrectly

is(0 // 2, 0, 		'	// : left-hand operand not optimized away');
is('' // 2, '',		'	// : left-hand operand not optimized away');
is(undef // 2, 2, 	'	// : left-hand operand optimized away');

# Test that OP_DORs other branch isn't run when arg is defined
# // returns the value if its defined, and we must test its
# truthness after
my $x = 0;
my $y = 0;

$x // 1 and $y = 1;
is($y, 0, 'y is still 0 after "$x // 1 and $y = 1"');

$y = 0;
# $x is defined, so its value 0 is returned to the if block
# and the block is skipped
if ($x // 1) {
    $y = 1;
}
is($y, 0, 'if ($x // 1) exited out early since $x is defined and 0');

# This is actually (($x // $z) || 'cat'), so 0 from first dor
# evaluates false, we should see 'cat'.
$y = undef;

$y = $x // $z || 'cat';
is($y, 'cat', 'chained or/dor behaves correctly');
