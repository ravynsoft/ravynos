#!./perl

# Simple tests for the basic math functions.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use Config;

plan tests => 31;

# compile time evaluation

eval { $s = sqrt(-1) }; # Kind of compile time.
like($@, qr/sqrt of -1/, 'compile time sqrt(-1) fails');

$s = sqrt(0);
is($s, 0, 'compile time sqrt(0)');

$s = sqrt(1);
is($s, 1, 'compile time sqrt(1)');

$s = sqrt(2);
is(substr($s,0,5), '1.414', 'compile time sqrt(2) == 1.414');

$s = exp(0);
is($s, 1, 'compile time exp(0) == 1');

$s = exp(1);
is(substr($s,0,7), '2.71828', 'compile time exp(1) == e');

eval { $s = log(0) };  # Kind of compile time.
like($@, qr/log of 0/, 'compile time log(0) fails');

$s = log(1);
is($s, 0, 'compile time log(1) == 0');

$s = log(2);
is(substr($s,0,5), '0.693', 'compile time log(2)');

cmp_ok(exp(log(1)), '==', 1, 'compile time exp(log(1)) == 1');

cmp_ok(round(atan2(1, 2)), '==', '0.463647609', "atan2(1, 2)");

# run time evaluation

$x0 = 0;
$x1 = 1;
$x2 = 2;

eval { $s = sqrt(-$x1) };
like($@, qr/sqrt of -1/, 'run time sqrt(-1) fails');

$s = sqrt($x0);
is($s, 0, 'run time sqrt(0)');

$s = sqrt($x1);
is($s, 1, 'run time sqrt(1)');

$s = sqrt($x2);
is(substr($s,0,5), '1.414', 'run time sqrt(2) == 1.414');

$s = exp($x0);
is($s, 1, 'run time exp(0) = 1');

$s = exp($x1);
is(substr($s,0,7), '2.71828', 'run time exp(1) = e');

eval { $s = log($x0) };
like($@, qr/log of 0/, 'run time log(0) fails');

$s = log($x1);
is($s, 0, 'compile time log(1) == 0');

$s = log($x2);
is(substr($s,0,5), '0.693', 'run time log(2)');

cmp_ok(exp(log($x1)), '==', 1, 'run time exp(log(1)) == 1');

# NOTE: do NOT test the trigonometric functions at [+-]Pi
# and expect to get exact results like 0, 1, -1, because
# you may not be able to feed them exactly [+-]Pi given
# all the variations of different long doubles.

my $pi_2 = 1.5707963267949;

sub round {
   my $result = shift;
   return sprintf("%.9f", $result);
}

# sin() tests
cmp_ok(sin(0), '==', 0.0, 'sin(0) == 0');
cmp_ok(abs(sin($pi_2) - 1), '<', 1e-9, 'sin(pi/2) == 1');
cmp_ok(abs(sin(-1 * $pi_2) - -1), '<', 1e-9, 'sin(-pi/2) == -1');

cmp_ok(round(sin($x1)), '==', '0.841470985', "sin(1)");

# cos() tests
cmp_ok(cos(0), '==', 1.0, 'cos(0) == 1');
cmp_ok(abs(cos($pi_2)), '<', 1e-9, 'cos(pi/2) == 0');
cmp_ok(abs(cos(-1 * $pi_2)), '<', 1e-9, 'cos(-pi/2) == 0');

cmp_ok(round(cos($x1)), '==', '0.540302306', "cos(1)");

cmp_ok(round(atan2($x1, $x2)), '==', '0.463647609', "atan2($x1, $x2)");

# atan2() tests testing with -0.0, 0.0, -1.0, 1.0 were removed due to
# differing results from calls to atan2() on various OS's and
# architectures.  See perlport.pod for more information.

SKIP: {
    unless ($Config{usequadmath}) {
        skip "need usequadmath", 1;
    }
    # For quadmath we have a known precision.  
    is(sqrt(2), '1.4142135623730950488016887242097', "quadmath sqrt");
}
