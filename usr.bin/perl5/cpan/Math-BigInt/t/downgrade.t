# -*- mode: perl; -*-

# Note that this does not test Math::BigFloat upgrading.

use strict;
use warnings;

use Test::More tests => 93;

use Math::BigInt   upgrade   => 'Math::BigFloat';
use Math::BigFloat downgrade => 'Math::BigInt';

is(Math::BigFloat->downgrade(), 'Math::BigInt', 'Math::BigFloat->downgrade()');
is(Math::BigInt->upgrade(), 'Math::BigFloat', 'Math::BigInt->upgrade()');

# bug until v1.67:

subtest 'Math::BigFloat->new("0.2E0")' => sub {
    plan tests => 2;
    my $x = Math::BigFloat->new("0.2E0");
    is($x, "0.2", 'value of $x');
    is(ref($x), "Math::BigFloat", '$x is a Math::BigFloat');
};

subtest 'Math::BigFloat->new("0.2E1")' => sub {
    plan tests => 2;
    my $x = Math::BigFloat->new("2");
    is($x, "2", 'value of $x');
    is(ref($x), "Math::BigInt", '$x is downgraded to a Math::BigInt');
};

subtest 'Math::BigFloat->new("0.2E2")' => sub {
    plan tests => 2;
    my $x = Math::BigFloat->new("20");
    is($x, "20", 'value of $x');
    is(ref($x), "Math::BigInt", '$x is downgraded to a Math::BigInt');
};

# $x is a downgraded to a Math::BigInt, but bpow() and bsqrt() upgrades to
# Math::BigFloat.

Math::BigFloat -> div_scale(20);        # make it a bit faster

my ($x, $y, $z);
subtest '$x = Math::BigFloat -> new(2);' => sub {
    plan tests => 2;
    $x = Math::BigFloat -> new(2);     # downgrades
    is(ref($x), 'Math::BigInt', '$x is downgraded to a Math::BigInt');
    cmp_ok($x, "==", 2, 'value of $x');
};

subtest '$y = Math::BigFloat -> bpow("2", "0.5");' => sub {
    plan tests => 2;
    $y = Math::BigFloat -> bpow("2", "0.5");
    is(ref($y), 'Math::BigFloat', '$y is a Math::BigFloat');
    cmp_ok($y, "==", "1.4142135623730950488", 'value of $y');
};

subtest '$z = $x -> bsqrt();' => sub {
    plan tests => 2;
    $z = $x -> bsqrt();
    is(ref($z), 'Math::BigFloat', '$y is a Math::BigFloat');
    cmp_ok($z, "==", "1.4142135623730950488", 'value of $z');
};

# log_2(16) = 4

subtest '$x = Math::BigFloat -> new(16); $y = $x -> blog(2);' => sub {
    plan tests => 4;
    $x = Math::BigFloat -> new(16);
    is(ref($x), 'Math::BigInt', '$x is downgraded to a Math::BigInt');
    cmp_ok($x, "==", 16, 'value of $x');
    $y = $x -> blog(2);
    is(ref($y), 'Math::BigInt', '$y is downgraded to a Math::BigInt');
    cmp_ok($y, "==", 4, 'value of $y');
};

# log_16(2) = 1/4

subtest '$x = Math::BigFloat -> new(2); $y = $x -> blog(16);' => sub {
    plan tests => 4;
    $x = Math::BigFloat -> new(2);
    is(ref($x), 'Math::BigInt', '$x is downgraded to a Math::BigInt');
    cmp_ok($x, "==", 2, 'value of $x');
    $y = $x -> blog(16);
    is(ref($y), 'Math::BigFloat', '$y is a Math::BigFloat');
    cmp_ok($y, "==", 0.25, 'value of $y');
};

################################################################################
# Verify that constructors downgrade when they should.

note("Enable downgrading, and see if constructors downgrade");

note("testing new()");

$x = Math::BigFloat -> new("0.5");
subtest '$x = Math::BigFloat -> new("0.5")' => sub {
    plan tests => 2;
    cmp_ok($x, "==", 0.5, 'value of $x');
    is(ref $x, "Math::BigFloat", "does not downgrade from Math::BigFloat");
};

$x = Math::BigFloat -> new("4");
subtest '$x = Math::BigFloat -> new("4")' => sub {
    plan tests => 2;
    cmp_ok($x, "==", 4, 'value of $x');
    is(ref $x, "Math::BigInt", "downgrades to Math::BigInt");
};

$x = Math::BigFloat -> new("0");
subtest '$x = Math::BigFloat -> new("0")' => sub {
    plan tests => 2;
    cmp_ok($x, "==", 0, 'value of $x');
    is(ref $x, "Math::BigInt", "downgrades to Math::BigInt");
};

$x = Math::BigFloat -> new("1");
subtest '$x = Math::BigFloat -> new("1")' => sub {
    plan tests => 2;
    cmp_ok($x, "==", 1, 'value of $x');
    is(ref $x, "Math::BigInt", "downgrades to Math::BigInt");
};

$x = Math::BigFloat -> new("Inf");
subtest '$x = Math::BigFloat -> new("inf")' => sub {
    plan tests => 2;
    cmp_ok($x, "==", "Inf", 'value of $x');
    is(ref $x, "Math::BigInt", "downgrades to Math::BigInt");
};

$x = Math::BigFloat -> new("NaN");
subtest '$x = Math::BigFloat -> new("NaN")' => sub {
    plan tests => 2;
    is($x, "NaN", );
    is(ref $x, "Math::BigInt", "downgrades to Math::BigInt");
};

note("testing bzero()");

$x = Math::BigFloat -> bzero();
subtest '$x = Math::BigFloat -> bzero()' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 0, 'value of $x');
    is(ref $x, 'Math::BigInt', 'downgrades to Math::BigInt');
};

note("testing bone()");

$x = Math::BigFloat -> bone();
subtest '$x = Math::BigFloat -> bone()' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 1, 'value of $x');
    is(ref $x, 'Math::BigInt', 'downgrades to Math::BigInt');
};

note("testing binf()");

$x = Math::BigFloat -> binf();
subtest '$x = Math::BigFloat -> binf()' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 'Inf', 'value of $x');
    is(ref $x, 'Math::BigInt', 'downgrades to Math::BigInt');
};

note("testing bnan()");

$x = Math::BigFloat -> bnan();
subtest '$x = Math::BigFloat -> bnan()' => sub {
    plan tests => 2;
    is($x, 'NaN', 'value of $x');
    is(ref $x, 'Math::BigInt', 'downgrades to Math::BigInt');
};

note("testing from_dec()");

$x = Math::BigFloat -> from_dec('3.14e2');
subtest '$x = Math::BigFloat -> from_dec("3.14e2")' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 314, 'value of $x');
    is(ref $x, 'Math::BigInt', 'downgrades to Math::BigInt');
};

note("testing from_hex()");

$x = Math::BigFloat -> from_hex('0x1.3ap+8');
subtest '$x = Math::BigFloat -> from_hex("3.14e2")' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 314, 'value of $x');
    is(ref $x, 'Math::BigInt', 'downgrades to Math::BigInt');
};

note("testing from_oct()");

$x = Math::BigFloat -> from_oct('0o1.164p+8');
subtest '$x = Math::BigFloat -> from_oct("0o1.164p+8")' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 314, 'value of $x');
    is(ref $x, 'Math::BigInt', 'downgrades to Math::BigInt');
};

note("testing from_bin()");

$x = Math::BigFloat -> from_bin('0b1.0011101p+8');
subtest '$x = Math::BigFloat -> from_bin("0b1.0011101p+8")' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 314, 'value of $x');
    is(ref $x, 'Math::BigInt', 'downgrades to Math::BigInt');
};

note("testing from_ieee754()");

$x = Math::BigFloat -> from_ieee754("\x43\x9d\x00\x00", "binary32");
subtest '$x = Math::BigFloat -> from_ieee754("\x43\x9d\x00\x00", "binary32")' => sub {
    plan tests => 2;
    cmp_ok($x, "==", 314, 'value of $x');
    is(ref $x, "Math::BigInt", 'downgrades to Math::BigInt');
};

note("Disable downgrading, and see if constructors downgrade");

Math::BigFloat -> downgrade(undef);

my $zero = Math::BigFloat -> bzero();
my $half = Math::BigFloat -> new("0.5");
my $one  = Math::BigFloat -> bone();
my $four = Math::BigFloat -> new("4");
my $inf  = Math::BigFloat -> binf();
my $nan  = Math::BigFloat -> bnan();

is(ref $zero, "Math::BigFloat", "Creating a 0 does not downgrade");
is(ref $half, "Math::BigFloat", "Creating a 0.5 does not downgrade");
is(ref $one,  "Math::BigFloat", "Creating a 1 does not downgrade");
is(ref $four, "Math::BigFloat", "Creating a 4 does not downgrade");
is(ref $inf,  "Math::BigFloat", "Creating an Inf does not downgrade");
is(ref $nan,  "Math::BigFloat", "Creating a NaN does not downgrade");

################################################################################
# Verify that other methods downgrade when they should.

Math::BigFloat -> downgrade("Math::BigInt");

note("testing bneg()");

$x = $zero -> copy() -> bneg();
subtest '$x = $zero -> copy() -> bneg();' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 0, '-(0) = 0');
    is(ref($x), 'Math::BigInt', '-(0) => Math::BigInt');
};

$x = $four -> copy() -> bneg();
subtest '$x = $four -> copy() -> bneg();' => sub {
    plan tests => 2;
    cmp_ok($x, '==', -4, '-(4) = -4');
    is(ref($x), 'Math::BigInt', '-(4) => Math::BigInt');
};

$x = $inf -> copy() -> bneg();
subtest '$x = $inf -> copy() -> bneg();' => sub {
    plan tests => 2;
    cmp_ok($x, '==', '-inf', '-(Inf) = -Inf');
    is(ref($x), 'Math::BigInt', '-(Inf) => Math::BigInt');
};

$x = $nan -> copy() -> bneg();
subtest '$x = $nan -> copy() -> bneg();' => sub {
    plan tests => 2;
    is($x, 'NaN', '-(NaN) = NaN');
    is(ref($x), 'Math::BigInt', '-(NaN) => Math::BigInt');
};

note("testing bnorm()");

$x = $zero -> copy() -> bnorm();
subtest '$x = $zero -> copy() -> bnorm();' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 0, 'value of $x');
    is(ref($x), 'Math::BigInt', 'bnorm(0) => Math::BigInt');
};

$x = $four -> copy() -> bnorm();
subtest '$x = $four -> copy() -> bnorm();' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 4, 'value of $x');
    is(ref($x), 'Math::BigInt', 'bnorm(4) => Math::BigInt');
};

$x = $inf -> copy() -> bnorm();
subtest '$x = $inf -> copy() -> bnorm();' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 'inf', 'value of $x');
    is(ref($x), 'Math::BigInt', 'bnorm(Inf) => Math::BigInt');
};

$x = $nan -> copy() -> bnorm();
subtest '$x = $nan -> copy() -> bnorm();' => sub {
    plan tests => 2;
    is($x, 'NaN', 'bnorm(NaN)');
    is(ref($x), 'Math::BigInt', 'bnorm(NaN) => Math::BigInt');
};

note("testing binc()");

$x = $zero -> copy() -> binc();
subtest '$x = $zero -> copy() -> binc();' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 1, 'binc(0)');
    is(ref($x), 'Math::BigInt', 'binc(0) => Math::BigInt');
};

$x = $four -> copy() -> binc();
subtest '$x = $four -> copy() -> binc();' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 5, 'binc(4)');
    is(ref($x), 'Math::BigInt', 'binc(4) => Math::BigInt');
};

$x = $inf -> copy() -> binc();
subtest '$x = $inf -> copy() -> binc();' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 'inf', 'binc(Inf)');
    is(ref($x), 'Math::BigInt', 'binc(Inf) => Math::BigInt');
};

$x = $nan -> copy() -> binc();
subtest '$x = $nan -> copy() -> binc();' => sub {
    plan tests => 2;
    is($x, 'NaN', 'binc(NaN)');
    is(ref($x), 'Math::BigInt', 'binc(NaN) => Math::BigInt');
};

note("testing bdec()");

$x = $zero -> copy() -> bdec();
subtest '$x = $zero -> copy() -> bdec();' => sub {
    plan tests => 2;
    cmp_ok($x, '==', -1, 'bdec(0)');
    is(ref($x), 'Math::BigInt', 'bdec(0) => Math::BigInt');
};

$x = $four -> copy() -> bdec();
subtest '$x = $four -> copy() -> bdec();' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 3, 'bdec(4)');
    is(ref($x), 'Math::BigInt', 'bdec(4) => Math::BigInt');
};

$x = $inf -> copy() -> bdec();
subtest '$x = $inf -> copy() -> bdec();' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 'inf', 'bdec(Inf)');
    is(ref($x), 'Math::BigInt', 'bdec(Inf) => Math::BigInt');
};

$x = $nan -> copy() -> bdec();
subtest '' => sub {
    plan tests => 2;
    is($x, 'NaN', 'bdec(NaN)');
    is(ref($x), 'Math::BigInt', 'bdec(NaN) => Math::BigInt');
};

note("testing badd()");

$x = $half -> copy() -> badd($nan);
subtest '$x = $half -> copy() -> badd($nan);' => sub {
    plan tests => 2;
    is($x, 'NaN', '0.5 + NaN = NaN');
    is(ref($x), 'Math::BigInt', '0.5 + NaN => Math::BigInt');
};

$x = $half -> copy() -> badd($inf);
subtest '$x = $half -> copy() -> badd($inf);' => sub {
    plan tests => 2;
    cmp_ok($x, '==', '+Inf', '0.5 + Inf = Inf');
    is(ref($x), 'Math::BigInt', '2.5 + Inf => Math::BigInt');
};

$x = $half -> copy() -> badd($half);
subtest '$x = $half -> copy() -> badd($half);' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 1, '0.5 + 0.5 = 1');
    is(ref($x), 'Math::BigInt', '0.5 + 0.5 => Math::BigInt');
};

$x = $half -> copy() -> badd($half -> copy() -> bneg());
subtest '$x = $half -> copy() -> badd($half -> copy() -> bneg());' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 0, '0.5 + -0.5 = 0');
    is(ref($x), 'Math::BigInt', '0.5 + -0.5 => Math::BigInt');
};

$x = $four -> copy() -> badd($zero);
subtest '$x = $four -> copy() -> badd($zero);' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 4, '4 + 0 = 4');
    is(ref($x), 'Math::BigInt', '4 + 0 => Math::BigInt');
};

$x = $zero -> copy() -> badd($four);
subtest '$x = $zero -> copy() -> badd($four);' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 4, '0 + 4 = 4');
    is(ref($x), 'Math::BigInt', '0 + 4 => Math::BigInt');
};

$x = $inf -> copy() -> badd($four);
subtest '$x = $inf -> copy() -> badd($four);' => sub {
    plan tests => 2;
    cmp_ok($x, '==', '+Inf', 'Inf + 4 = Inf');
    is(ref($x), 'Math::BigInt', 'Inf + 4 => Math::BigInt');
};

$x = $nan -> copy() -> badd($four);
subtest '$x = $nan -> copy() -> badd($four);' => sub {
    plan tests => 2;
    is($x, 'NaN', 'NaN + 4 = NaN');
    is(ref($x), 'Math::BigInt', 'NaN + 4 => Math::BigInt');
};

note("testing bsub()");

$x = $half -> copy() -> bsub($nan);
subtest '$x = $half -> copy() -> bsub($nan);' => sub {
    plan tests => 2;
    is($x, 'NaN', '0.5 - NaN = NaN');
    is(ref($x), 'Math::BigInt', '0.5 - NaN => Math::BigInt');
};

$x = $half -> copy() -> bsub($inf);
subtest '$x = $half -> copy() -> bsub($inf);' => sub {
    plan tests => 2;
    cmp_ok($x, '==', '-Inf', '2.5 - Inf = -Inf');
    is(ref($x), 'Math::BigInt', '2.5 - Inf => Math::BigInt');
};

$x = $half -> copy() -> bsub($half);
subtest '$x = $half -> copy() -> bsub($half);' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 0, '0.5 + 0.5 = 0');
    is(ref($x), 'Math::BigInt', '0.5 - 0.5 => Math::BigInt');
};

$x = $half -> copy() -> bsub($half -> copy() -> bneg());
subtest '$x = $half -> copy() -> bsub($half -> copy() -> bneg());' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 1, '0.5 - -0.5 = 1');
    is(ref($x), 'Math::BigInt', '0.5 - -0.5 => Math::BigInt');
};

$x = $four -> copy() -> bsub($zero);
subtest '$x = $four -> copy() -> bsub($zero);' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 4, '4 - 0 = 4');
    is(ref($x), 'Math::BigInt', '4 - 0 => Math::BigInt');
};

$x = $zero -> copy() -> bsub($four);
subtest '$x = $zero -> copy() -> bsub($four);' => sub {
    plan tests => 2;
    cmp_ok($x, '==', -4, '0 - 4 = -4');
    is(ref($x), 'Math::BigInt', '0 - 4 => Math::BigInt');
};

$x = $inf -> copy() -> bsub($four);
subtest '$x = $inf -> copy() -> bsub($four);' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 'Inf', 'Inf - 4 = Inf');
    is(ref($x), 'Math::BigInt', 'Inf - 4 => Math::BigInt');
};

$x = $nan -> copy() -> bsub($four);
subtest '$x = $nan -> copy() -> bsub($four);' => sub {
    plan tests => 2;
    is($x, 'NaN', 'NaN - 4 = NaN');
    is(ref($x), 'Math::BigInt', 'NaN - 4 => Math::BigInt');
};

note("testing bmul()");

$x = $zero -> copy() -> bmul($four);
subtest '$x = $zero -> copy() -> bmul($four);' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 0, 'bmul(0, 4) = 0');
    is(ref($x), 'Math::BigInt', 'bmul(0, 4) => Math::BigInt');
};

$x = $four -> copy() -> bmul($four);
subtest '$x = $four -> copy() -> bmul($four);' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 16, 'bmul(4, 4) = 16');
    is(ref($x), 'Math::BigInt', 'bmul(4, 4) => Math::BigInt');
};

$x = $inf -> copy() -> bmul($four);
subtest '$x = $inf -> copy() -> bmul($four);' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 'inf', 'bmul(Inf, 4) = Inf');
    is(ref($x), 'Math::BigInt', 'bmul(Inf, 4) => Math::BigInt');
};

$x = $nan -> copy() -> bmul($four);
subtest '$x = $nan -> copy() -> bmul($four);' => sub {
    plan tests => 2;
    is($x, 'NaN', 'bmul(NaN, 4) = NaN');
    is(ref($x), 'Math::BigInt', 'bmul(NaN, 4) => Math::BigInt');
};

$x = $four -> copy() -> bmul('0.5');
subtest '' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 2, 'bmul(4, 0.5) = 2');
    is(ref($x), 'Math::BigInt', 'bmul(4, 0.5) => Math::BigInt');
};

note("testing bmuladd()");

$x = $zero -> copy() -> bmuladd($four, $four);
subtest '$x = $zero -> copy() -> bmuladd($four, $four);' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 4, 'bmuladd(0, 4, 4) = 4');
    is(ref($x), 'Math::BigInt', 'bmuladd(0, 4, 4) => Math::BigInt');
};

$x = $four -> copy() -> bmuladd($four, $four);
subtest '$x = $four -> copy() -> bmuladd($four, $four);' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 20, 'bmuladd(4, 4, 4) = 20');
    is(ref($x), 'Math::BigInt', 'bmuladd(4, 4, 4) => Math::BigInt');
};

$x = $four -> copy() -> bmuladd($four, $inf);
subtest '$x = $four -> copy() -> bmuladd($four, $inf);' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 'inf', 'bmuladd(4, 4, Inf) = Inf');
    is(ref($x), 'Math::BigInt', 'bmuladd(4, 4, Inf) => Math::BigInt');
};

$x = $inf -> copy() -> bmuladd($four, $four);
subtest '$x = $inf -> copy() -> bmuladd($four, $four);' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 'inf', 'bmuladd(Inf, 4, 4) = Inf');
    is(ref($x), 'Math::BigInt', 'bmuladd(Inf, 4, 4) => Math::BigInt');
};

$x = $inf -> copy() -> bmuladd($four, $four);
subtest '$x = $inf -> copy() -> bmuladd($four, $four);' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 'inf', 'bmuladd(Inf, 4, 4) = Inf');
    is(ref($x), 'Math::BigInt', 'bmuladd(Inf, 4, 4) => Math::BigInt');
};

$x = $nan -> copy() -> bmuladd($four, $four);
subtest '$x = $nan -> copy() -> bmuladd($four, $four);' => sub {
    plan tests => 2;
    is($x, 'NaN', 'bmuladd(NaN, 4, 4) = NaN');
    is(ref($x), 'Math::BigInt', 'bmuladd(NaN, 4, 4) => Math::BigInt');
};

$x = $four -> copy() -> bmuladd("0.5", $four);
subtest '$x = $four -> copy() -> bmuladd("0.5", $four);' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 6, 'bmuladd(4, 0.5, 4) = 6');
    is(ref($x), 'Math::BigInt', 'bmuladd(4, 0.5, 4) => Math::BigInt');
};

note("testing bdiv()");

$x = $zero -> copy() -> bdiv($one);
subtest '$x = $zero -> copy() -> bdiv($one);' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 0, 'bdiv(0, 1) = 0');
    is(ref($x), 'Math::BigInt', 'bdiv(0, 1) => Math::BigInt');
};

note("testing bmod()");

note("testing bmodpow()");

note("testing bpow()");

note("testing blog()");

note("testing bexp()");

note("testing bnok()");

note("testing bsin()");

note("testing bcos()");

note("testing batan()");

note("testing batan()");

note("testing bsqrt()");

note("testing broot()");

note("testing bfac()");

note("testing bdfac()");

note("testing btfac()");

note("testing bmfac()");

note("testing blsft()");

note("testing brsft()");

note("testing band()");

note("testing bior()");

note("testing bxor()");

note("testing bnot()");

note("testing bround()");

note("testing Add tests for rounding a non-integer to an integer. Fixme!");

$x = $zero -> copy() -> bround();
subtest '$x = $zero -> copy() -> bround();' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 0, 'bround(0)');
    is(ref($x), 'Math::BigInt', 'bround(0) => Math::BigInt');
};

$x = $four -> copy() -> bround();
subtest '$x = $four -> copy() -> bround();' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 4, 'bround(4)');
    is(ref($x), 'Math::BigInt', 'bround(4) => Math::BigInt');
};

$x = $inf -> copy() -> bround();
subtest '$x = $inf -> copy() -> bround();' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 'inf', 'bround(Inf)');
    is(ref($x), 'Math::BigInt', 'bround(Inf) => Math::BigInt');
};

$x = $nan -> copy() -> bround();
subtest '$x = $nan -> copy() -> bround();' => sub {
    plan tests => 2;
    is($x, 'NaN', 'bround(NaN)');
    is(ref($x), 'Math::BigInt', 'bround(NaN) => Math::BigInt');
};

note("testing bfround()");

note("testing Add tests for rounding a non-integer to an integer. Fixme!");

$x = $zero -> copy() -> bfround();
subtest '$x = $zero -> copy() -> bfround();' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 0, 'bfround(0)');
    is(ref($x), 'Math::BigInt', 'bfround(0) => Math::BigInt');
};

$x = $four -> copy() -> bfround();
subtest '$x = $four -> copy() -> bfround();' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 4, 'bfround(4)');
    is(ref($x), 'Math::BigInt', 'bfround(4) => Math::BigInt');
};

$x = $inf -> copy() -> bfround();
subtest '$x = $inf -> copy() -> bfround();' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 'inf', 'bfround(Inf)');
    is(ref($x), 'Math::BigInt', 'bfround(Inf) => Math::BigInt');
};

$x = $nan -> copy() -> bfround();
subtest '$x = $nan -> copy() -> bfround();' => sub {
    plan tests => 2;
    is($x, 'NaN', 'bfround(NaN)');
    is(ref($x), 'Math::BigInt', 'bfround(NaN) => Math::BigInt');
};

note("testing bfloor()");

$x = $half -> copy() -> bfloor();
subtest '$x = $half -> copy() -> bfloor();' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 0, 'bfloor(0)');
    is(ref($x), 'Math::BigInt', 'bfloor(0) => Math::BigInt');
};

$x = $inf -> copy() -> bfloor();
subtest '$x = $inf -> copy() -> bfloor();' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 'Inf', 'bfloor(Inf)');
    is(ref($x), 'Math::BigInt', 'bfloor(Inf) => Math::BigInt');
};

$x = $nan -> copy() -> bfloor();
subtest '$x = $nan -> copy() -> bfloor();' => sub {
    plan tests => 2;
    is($x, 'NaN', 'bfloor(NaN)');
    is(ref($x), 'Math::BigInt', 'bfloor(NaN) => Math::BigInt');
};

note("testing bceil()");

$x = $half -> copy() -> bceil();
subtest '$x = $half -> copy() -> bceil();' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 1, 'bceil(0)');
    is(ref($x), 'Math::BigInt', 'bceil(0) => Math::BigInt');
};

$x = $inf -> copy() -> bceil();
subtest '$x = $inf -> copy() -> bceil();' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 'Inf', 'bceil(Inf)');
    is(ref($x), 'Math::BigInt', 'bceil(Inf) => Math::BigInt');
};

$x = $nan -> copy() -> bceil();
subtest '$x = $nan -> copy() -> bceil();' => sub {
    plan tests => 2;
    is($x, 'NaN', 'bceil(NaN)');
    is(ref($x), 'Math::BigInt', 'bceil(NaN) => Math::BigInt');
};

note("testing bint()");

$x = $half -> copy() -> bint();
subtest '$x = $half -> copy() -> bint();' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 0, 'bint(0)');
    is(ref($x), 'Math::BigInt', 'bint(0) => Math::BigInt');
};

$x = $inf -> copy() -> bint();
subtest '$x = $inf -> copy() -> bint();' => sub {
    plan tests => 2;
    cmp_ok($x, '==', 'Inf', 'bint(Inf)');
    is(ref($x), 'Math::BigInt', 'bint(Inf) => Math::BigInt');
};

$x = $nan -> copy() -> bint();
subtest '$x = $nan -> copy() -> bint();' => sub {
    plan tests => 2;
    is($x, 'NaN', 'bint(NaN)');
    is(ref($x), 'Math::BigInt', 'bint(NaN) => Math::BigInt');
};

note("testing bgcd()");

note("testing blcm()");

note("testing mantissa()");

note("testing exponent()");

note("testing parts()");

note("testing sparts()");

note("testing nparts()");

note("testing eparts()");

note("testing dparts()");

note("testing fparts()");

note("testing numerator()");

note("testing denominator()");
