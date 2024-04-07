# test rounding, accuracy, precision and fallback, round_mode and mixing
# of classes

# Make sure you always quote any bare floating-point values, lest 123.46 will
# be stringified to 123.4599999999 due to limited float prevision.

use strict;
use warnings;

my ($x, $y, $z, $u, $rc);
our ($mbi, $mbf);

###############################################################################
# test defaults and set/get

{
    no strict 'refs';
    is(${"$mbi\::accuracy"},   undef,  qq|\${"$mbi\::accuracy"}|);
    is(${"$mbi\::precision"},  undef,  qq|\${"$mbi\::precision"}|);
    is($mbi->accuracy(),       undef,  qq|$mbi->accuracy()|);
    is($mbi->precision(),      undef,  qq|$mbi->precision()|);
    is(${"$mbi\::div_scale"},  40,     qq|\${"$mbi\::div_scale"}|);
    is(${"$mbi\::round_mode"}, 'even', qq|\${"$mbi\::round_mode"}|);
    is($mbi->round_mode(),     'even', qq|$mbi->round_mode()|);

    is(${"$mbf\::accuracy"},   undef,  qq|\${"$mbf\::accuracy"}|);
    is(${"$mbf\::precision"},  undef,  qq|\${"$mbf\::precision"}|);
    is($mbf->precision(),      undef,  qq|$mbf->precision()|);
    is($mbf->precision(),      undef,  qq|$mbf->precision()|);
    is(${"$mbf\::div_scale"},  40,     qq|\${"$mbf\::div_scale"}|);
    is(${"$mbf\::round_mode"}, 'even', qq|\${"$mbf\::round_mode"}|);
    is($mbf->round_mode(),     'even', qq|$mbf->round_mode()|);
}

# accessors
foreach my $class ($mbi, $mbf) {
    is($class->accuracy(),        undef,  qq|$class->accuracy()|);
    is($class->precision(),       undef,  qq|$class->precision()|);
    is($class->round_mode(),      "even", qq|$class->round_mode()|);
    is($class->div_scale(),       40,     qq|$class->div_scale()|);

    is($class->div_scale(20),     20,     qq|$class->div_scale(20)|);
    $class->div_scale(40);
    is($class->div_scale(),       40,     qq|$class->div_scale()|);

    is($class->round_mode("odd"), "odd",  qq|$class->round_mode("odd")|);
    $class->round_mode("even");
    is($class->round_mode(),      "even", qq|$class->round_mode()|);

    is($class->accuracy(2),       2,      qq|$class->accuracy(2)|);
    $class->accuracy(3);
    is($class->accuracy(),        3,      qq|$class->accuracy()|);
    is($class->accuracy(undef),   undef,  qq|$class->accuracy(undef)|);

    is($class->precision(2),      2,      qq|$class->precision(2)|);
    is($class->precision(-2),     -2,     qq|$class->precision(-2)|);
    $class->precision(3);
    is($class->precision(),       3,      qq|$class->precision()|);
    is($class->precision(undef),  undef,  qq|$class->precision(undef)|);
}

{
    no strict 'refs';

    # accuracy
    foreach (qw/5 42 -1 0/) {
        is(${"$mbf\::accuracy"} = $_, $_, qq|\${"$mbf\::accuracy"} = $_|);
        is(${"$mbi\::accuracy"} = $_, $_, qq|\${"$mbi\::accuracy"} = $_|);
    }
    is(${"$mbf\::accuracy"} = undef, undef, qq|\${"$mbf\::accuracy"} = undef|);
    is(${"$mbi\::accuracy"} = undef, undef, qq|\${"$mbi\::accuracy"} = undef|);

    # precision
    foreach (qw/5 42 -1 0/) {
        is(${"$mbf\::precision"} = $_, $_, qq|\${"$mbf\::precision"} = $_|);
        is(${"$mbi\::precision"} = $_, $_, qq|\${"$mbi\::precision"} = $_|);
    }
    is(${"$mbf\::precision"} = undef, undef,
       qq|\${"$mbf\::precision"} = undef|);
    is(${"$mbi\::precision"} = undef, undef,
       qq|\${"$mbi\::precision"} = undef|);

    # fallback
    foreach (qw/5 42 1/) {
        is(${"$mbf\::div_scale"} = $_, $_, qq|\${"$mbf\::div_scale"} = $_|);
        is(${"$mbi\::div_scale"} = $_, $_, qq|\${"$mbi\::div_scale"} = $_|);
    }
    # illegal values are possible for fallback due to no accessor

    # round_mode
    foreach (qw/odd even zero trunc +inf -inf/) {
        is(${"$mbf\::round_mode"} = $_, $_,
           qq|\${"$mbf\::round_mode"} = "$_"|);
        is(${"$mbi\::round_mode"} = $_, $_,
           qq|\${"$mbi\::round_mode"} = "$_"|);
    }
    ${"$mbf\::round_mode"} = 'zero';
    is(${"$mbf\::round_mode"}, 'zero', qq|\${"$mbf\::round_mode"}|);
    is(${"$mbi\::round_mode"}, '-inf', qq|\${"$mbi\::round_mode"}|);

    # reset for further tests
    ${"$mbi\::accuracy"}  = undef;
    ${"$mbi\::precision"} = undef;
    ${"$mbf\::div_scale"} = 40;
}

# local copies
$x = $mbf->new('123.456');
is($x->accuracy(),       undef, q|$x->accuracy()|);
is($x->accuracy(5),      5,     q|$x->accuracy(5)|);
is($x->accuracy(undef),  undef, q|$x->accuracy(undef)|);
is($x->precision(),      undef, q|$x->precision()|);
is($x->precision(5),     5,     q|$x->precision(5)|);
is($x->precision(undef), undef, q|$x->precision(undef)|);

{
    no strict 'refs';
    # see if MBF changes MBIs values
    is(${"$mbi\::accuracy"} = 42, 42, qq|\${"$mbi\::accuracy"} = 42|);
    is(${"$mbf\::accuracy"} = 64, 64, qq|\${"$mbf\::accuracy"} = 64|);
    is(${"$mbi\::accuracy"},      42, qq|\${"$mbi\::accuracy"} = 42|);
    is(${"$mbf\::accuracy"},      64, qq|\${"$mbf\::accuracy"} = 64|);
}

###############################################################################
# see if creating a number under set A or P will round it

{
    no strict 'refs';
    ${"$mbi\::accuracy"}  = 4;
    ${"$mbi\::precision"} = undef;

    is($mbi->new(123456), 123500, qq|$mbi->new(123456) = 123500|); # with A
    ${"$mbi\::accuracy"}  = undef;
    ${"$mbi\::precision"} = 3;
    is($mbi->new(123456), 123000, qq|$mbi->new(123456) = 123000|); # with P

    ${"$mbf\::accuracy"}  = 4;
    ${"$mbf\::precision"} = undef;
    ${"$mbi\::precision"} = undef;

    is($mbf->new("123.456"), "123.5", qq|$mbf->new("123.456") = 123.5|);
    ${"$mbf\::accuracy"}  = undef;
    ${"$mbf\::precision"} = -1;
    is($mbf->new("123.456"), "123.5", qq|$mbf->new("123.456") = 123.5|);

    ${"$mbf\::precision"} = undef; # reset
}

###############################################################################
# see if MBI leaves MBF's private parts alone

{
    no strict 'refs';
    ${"$mbi\::precision"} = undef;
    ${"$mbf\::precision"} = undef;
    ${"$mbi\::accuracy"}  = 4;
    ${"$mbf\::accuracy"}  = undef;
    is($mbf->new("123.456"), "123.456", qq|$mbf->new("123.456") = 123.456|);
    ${"$mbi\::accuracy"}  = undef; # reset
}

###############################################################################
# see if setting accuracy/precision actually rounds the number

$x = $mbf->new("123.456");
$x->accuracy(4);
is($x, "123.5", qq|\$x = $mbf->new("123.456"); \$x->accuracy(4)|);

$x = $mbf->new("123.456");
$x->precision(-2);
is($x, "123.46", qq|\$x = $mbf->new("123.456"); \$x->precision(-2)|);

$x = $mbi->new(123456);
$x->accuracy(4);
is($x, 123500, qq|\$x = $mbi->new(123456); \$x->accuracy(4)|);

$x = $mbi->new(123456);
$x->precision(2);
is($x, 123500, qq|\$x = $mbi->new(123456); \$x->precision(2)|);

###############################################################################
# test actual rounding via round()

$x = $mbf->new("123.456");
is($x->copy()->round(5), "123.46",
   qq|\$x = $mbf->new("123.456"); \$x->copy()->round(5)|);
is($x->copy()->round(4), "123.5",
   qq|\$x = $mbf->new("123.456"); \$x->copy()->round(4)|);
is($x->copy()->round(5, 2), "NaN",
   qq|\$x = $mbf->new("123.456"); \$x->copy()->round(5, 2)|);
is($x->copy()->round(undef, -2), "123.46",
   qq|\$x = $mbf->new("123.456"); \$x->copy()->round(undef, -2)|);
is($x->copy()->round(undef, 2), 120,
   qq|\$x = $mbf->new("123.456"); \$x->copy()->round(undef, 2)|);

$x = $mbi->new("123");
is($x->round(5, 2), "NaN",
   qq|\$x = $mbi->new("123"); \$x->round(5, 2)|);

$x = $mbf->new("123.45000");
is($x->copy()->round(undef, -1, "odd"), "123.5",
   qq|\$x = $mbf->new("123.45000"); \$x->copy()->round(undef, -1, "odd")|);

# see if rounding is 'sticky'
$x = $mbf->new("123.4567");
$y = $x->copy()->bround();              # no-op since nowhere A or P defined

is($y, 123.4567,
   qq|\$x = $mbf->new("123.4567"); \$y = \$x->copy()->bround()|);
$y = $x->copy()->round(5);
is($y->accuracy(), 5,
   q|$y = $x->copy()->round(5); $y->accuracy()|);
is($y->precision(), undef,              # A has precedence, so P still unset
   q|$y = $x->copy()->round(5); $y->precision()|);
$y = $x->copy()->round(undef, 2);
is($y->precision(), 2,
   q|$y = $x->copy()->round(undef, 2); $y->precision()|);
is($y->accuracy(), undef,               # P has precedence, so A still unset
   q|$y = $x->copy()->round(undef, 2); $y->accuracy()|);

# see if setting A clears P and vice versa
$x = $mbf->new("123.4567");
is($x, "123.4567", q|$x = $mbf->new("123.4567")|);
is($x->accuracy(4), 4, q|$x->accuracy(4)|);
is($x->precision(-2), -2, q|$x->precision(-2)|);                # clear A
is($x->accuracy(), undef, q|$x->accuracy()|);

$x = $mbf->new("123.4567");
is($x, "123.4567", q|$x = $mbf->new("123.4567")|);
is($x->precision(-2), -2, q|$x->precision(-2)|);
is($x->accuracy(4), 4, q|$x->accuracy(4)|);                     # clear P
is($x->precision(), undef, q|$x->precision()|);

# does copy work?
$x = $mbf->new(123.456);
$x->accuracy(4);
$x->precision(2);

$z = $x->copy();
is($z->accuracy(),  undef, q|$z = $x->copy(); $z->accuracy()|);
is($z->precision(), 2,     q|$z = $x->copy(); $z->precision()|);

# does $x->bdiv($y, d) work when $d > div_scale?
$x = $mbf->new("0.008");
$x->accuracy(8);

for my $e (4, 8, 16, 32) {
    is(scalar $x->copy()->bdiv(3, $e), "0.002" . ("6" x ($e - 2)) . "7",
       qq|\$x->copy()->bdiv(3, $e)|);
}

# does accuracy()/precision work on zeros?
foreach my $class ($mbi, $mbf) {

    $x = $class->bzero();
    $x->accuracy(5);
    is($x->{_a}, 5, qq|\$x = $class->bzero(); \$x->accuracy(5); \$x->{_a}|);

    $x = $class->bzero();
    $x->precision(5);
    is($x->{_p}, 5, qq|\$x = $class->bzero(); \$x->precision(5); \$x->{_p}|);

    $x = $class->new(0);
    $x->accuracy(5);
    is($x->{_a}, 5, qq|\$x = $class->new(0); \$x->accuracy(5); \$x->{_a}|);

    $x = $class->new(0);
    $x->precision(5);
    is($x->{_p}, 5, qq|\$x = $class->new(0); \$x->precision(5); \$x->{_p}|);

    $x = $class->bzero();
    $x->round(5);
    is($x->{_a}, 5, qq|\$x = $class->bzero(); \$x->round(5); \$x->{_a}|);

    $x = $class->bzero();
    $x->round(undef, 5);
    is($x->{_p}, 5, qq|\$x = $class->bzero(); \$x->round(undef, 5); \$x->{_p}|);

    $x = $class->new(0);
    $x->round(5);
    is($x->{_a}, 5, qq|\$x = $class->new(0); \$x->round(5); \$x->{_a}|);

    $x = $class->new(0);
    $x->round(undef, 5);
    is($x->{_p}, 5, qq|\$x = $class->new(0); \$x->round(undef, 5); \$x->{_p}|);

    # see if trying to increasing A in bzero() doesn't do something
    $x = $class->bzero();
    $x->{_a} = 3;
    $x->round(5);
    is($x->{_a}, 3,
       qq|\$x = $class->bzero(); \$x->{_a} = 3; \$x->round(5); \$x->{_a}|);
}

###############################################################################
# test whether an opp calls objectify properly or not (or at least does what
# it should do given non-objects, w/ or w/o objectify())

foreach my $class ($mbi, $mbf) {
    #  ${"$class\::precision"} = undef;         # reset
    #  ${"$class\::accuracy"} = undef;          # reset

    is($class->new(123)->badd(123), 246, qq|$class->new(123)->badd(123)|);
    is($class->badd(123, 321), 444, qq|$class->badd(123, 321)|);
    is($class->badd(123, $class->new(321)), 444,
       qq|$class->badd(123, $class->new(321))|);

    is($class->new(123)->bsub(122), 1, qq|$class->new(123)->bsub(122)|);
    is($class->bsub(321, 123), 198, qq|$class->bsub(321, 123)|);
    is($class->bsub(321, $class->new(123)), 198,
       qq|$class->bsub(321, $class->new(123))|);

    is($class->new(123)->bmul(123), 15129, qq|$class->new(123)->bmul(123)|);
    is($class->bmul(123, 123), 15129, qq|$class->bmul(123, 123)|);
    is($class->bmul(123, $class->new(123)), 15129,
       qq|$class->bmul(123, $class->new(123))|);

    # is($class->new(15129)->bdiv(123), 123, qq|$class->new(15129)->bdiv(123)|);
    # is($class->bdiv(15129, 123), 123, qq|$class->bdiv(15129, 123)|);
    # is($class->bdiv(15129, $class->new(123)), 123,
    #    qq|$class->bdiv(15129, $class->new(123))|);

    is($class->new(15131)->bmod(123), 2, qq|$class->new(15131)->bmod(123)|);
    is($class->bmod(15131, 123), 2, qq|$class->bmod(15131, 123)|);
    is($class->bmod(15131, $class->new(123)), 2,
       qq|$class->bmod(15131, $class->new(123))|);

    is($class->new(2)->bpow(16), 65536, qq|$class->new(2)->bpow(16)|);
    is($class->bpow(2, 16), 65536, qq|$class->bpow(2, 16)|);
    is($class->bpow(2, $class->new(16)), 65536,
       qq|$class->bpow(2, $class->new(16))|);

    is($class->new(2**15)->brsft(1), 2**14, qq|$class->new(2**15)->brsft(1)|);
    is($class->brsft(2**15, 1), 2**14, qq|$class->brsft(2**15, 1)|);
    is($class->brsft(2**15, $class->new(1)), 2**14,
       qq|$class->brsft(2**15, $class->new(1))|);

    is($class->new(2**13)->blsft(1), 2**14, qq|$class->new(2**13)->blsft(1)|);
    is($class->blsft(2**13, 1), 2**14, qq|$class->blsft(2**13, 1)|);
    is($class->blsft(2**13, $class->new(1)), 2**14,
       qq|$class->blsft(2**13, $class->new(1))|);
}

###############################################################################
# Test whether operations round properly afterwards.
# These tests are not complete, since they do not exercise every "return"
# statement in the op's. But heh, it's better than nothing...

$x = $mbf->new("123.456");
$y = $mbf->new("654.321");
$x->{_a} = 5;           # $x->accuracy(5) would round $x straight away
$y->{_a} = 4;           # $y->accuracy(4) would round $x straight away

$z = $x + $y;
is($z, "777.8", q|$z = $x + $y|);

$z = $y - $x;
is($z, "530.9", q|$z = $y - $x|);

$z = $y * $x;
is($z, "80780", q|$z = $y * $x|);

$z = $x ** 2;
is($z, "15241", q|$z = $x ** 2|);

$z = $x * $x;
is($z, "15241", q|$z = $x * $x|);

# not:
#$z = -$x;
#is($z, '-123.46');
#is($x, '123.456');

$z = $x->copy();
$z->{_a} = 2;
$z = $z / 2;
is($z, 62, q|$z = $z / 2|);

$x = $mbf->new(123456);
$x->{_a} = 4;
$z = $x->copy;
$z++;
is($z, 123500, q|$z++|);

$x = $mbi->new(123456);
$y = $mbi->new(654321);
$x->{_a} = 5;           # $x->accuracy(5) would round $x straight away
$y->{_a} = 4;           # $y->accuracy(4) would round $x straight away

$z = $x + $y;
is($z, 777800, q|$z = $x + $y|);

$z = $y - $x;
is($z, 530900, q|$z = $y - $x|);

$z = $y * $x;
is($z, 80780000000, q|$z = $y * $x|);

$z = $x ** 2;
is($z, 15241000000, q|$z = $x ** 2|);

# not yet: $z = -$x;
# is($z, -123460, qq|$z|);
# is($x, 123456, qq|$x|);

$z = $x->copy;
$z++;
is($z, 123460, q|$z++|);

$z = $x->copy();
$z->{_a} = 2;
$z = $z / 2;
is($z, 62000, q|$z = $z / 2|);

$x = $mbi->new(123400);
$x->{_a} = 4;
is($x->bnot(), -123400, q|$x->bnot()|);         # not -1234001

# to be consistent with other methods, babs() and bneg() also support rounding

$x = $mbi->new(-123401);
$x->{_a} = 4;
is($x->babs(), 123400, q|$x->babs()|);

$x = $mbi->new(-123401);
$x->{_a} = 4;
is($x->bneg(), 123400, q|$x->bneg()|);

# test bdiv rounding to A and R (bug in v1.48 and maybe earlier versions)

$mbf->round_mode('even');
$x = $mbf->new('740.7')->bdiv('6', 4, undef, 'zero');
is($x, '123.4', q|$x|);

$x = $mbi->new('123456');
$y = $mbi->new('123456');
$y->{_a} = 6;
is($x->bdiv($y), 1, q|$x->bdiv($y)|);
is($x->{_a}, 6, q|$x->{_a}|);                   # carried over

$x = $mbi->new('123456');
$y = $mbi->new('123456');
$x->{_a} = 6;
is($x->bdiv($y), 1, q|$x->bdiv($y)|);
is($x->{_a}, 6, q|$x->{_a}|);                   # carried over

$x = $mbi->new('123456');
$y = $mbi->new('223456');
$y->{_a} = 6;
is($x->bdiv($y), 0, q|$x->bdiv($y)|);
is($x->{_a}, 6, q|$x->{_a}|);                   # carried over

$x = $mbi->new('123456');
$y = $mbi->new('223456');
$x->{_a} = 6;
is($x->bdiv($y), 0, q|$x->bdiv($y)|);
is($x->{_a}, 6, q|$x->{_a}|);                   # carried over

###############################################################################
# test that bop(0) does the same than bop(undef)

$x = $mbf->new('1234567890');
is($x->copy()->bsqrt(0), $x->copy()->bsqrt(undef),
   q|$x->copy()->bsqrt(...)|);
is($x->copy->bsqrt(0), '35136.41828644462161665823116758077037159',
   q|$x->copy->bsqrt(...)|);

is($x->{_a}, undef, q|$x->{_a}|);

# test that bsqrt() modifies $x and does not just return something else
# (especially under Math::BigInt::BareCalc)
$z = $x->bsqrt();
is($z, $x, q|$z = $x->bsqrt(); $z|);
is($x, '35136.41828644462161665823116758077037159', q|$z = $x->bsqrt(); $x|);

$x = $mbf->new('1.234567890123456789');

is($x->copy()->bpow('0.5', 0),
   $x->copy()->bpow('0.5', undef),
   q|$x->copy()->bpow(...)|);

is($x->copy()->bpow('0.5', 0),
   $x->copy()->bsqrt(undef),
   q|$x->copy()->bpow(...) vs. $x->copy()->bsqrt(...)|);

is($x->copy()->bpow('2', 0), '1.524157875323883675019051998750190521',
   q|$x->copy()->bpow('2', 0)|);

###############################################################################
# test (also under Bare) that bfac() rounds at last step

is($mbi->new(12)->bfac(),  '479001600', q|$mbi->new(12)->bfac()|);
is($mbi->new(12)->bfac(2), '480000000', q|$mbi->new(12)->bfac(2)|);

$x = $mbi->new(12);
$x->accuracy(2);
is($x->bfac(), '480000000',
   qq|\$x = $mbi->new(12); \$x->accuracy(2); \$x->bfac()|);

$x = $mbi->new(13);
$x->accuracy(2);
is($x->bfac(), '6200000000',
   qq|\$x = $mbi->new(13); \$x->accuracy(2); \$x->bfac()|);

$x = $mbi->new(13);
$x->accuracy(3);
is($x->bfac(), '6230000000',
   qq|\$x = $mbi->new(13); \$x->accuracy(3); \$x->bfac()|);

$x = $mbi->new(13);
$x->accuracy(4);
is($x->bfac(), '6227000000',
   qq|\$x = $mbi->new(13); \$x->accuracy(4); \$x->bfac()|);

# this does 1, 2, 3...9, 10, 11, 12...20
$x = $mbi->new(20);
$x->accuracy(1);
is($x->bfac(), '2000000000000000000',
   qq|\$x = $mbi->new(20); \$x->accuracy(1); \$x->bfac()|);

###############################################################################
# test bsqrt) rounding to given A/P/R (bug prior to v1.60)

$x = $mbi->new('123456')->bsqrt(2, undef);
is($x, '350', qq|\$x = $mbi->new("123456")->bsqrt(2, undef)|); # not 351

$x = $mbi->new('3')->bsqrt(2, undef);
is($x->accuracy(), 2, q|$x->accuracy()|);

$mbi->round_mode('even');
$x = $mbi->new('126025')->bsqrt(2, undef, '+inf');
is($x, '360', q|$x = 360|);     # not 355 nor 350

$x = $mbi->new('126025')->bsqrt(undef, 2);
is($x, '400', q|$x = 400|);      # not 355

###############################################################################
# test mixed arguments

$x = $mbf->new(10);
$u = $mbf->new(2.5);
$y = $mbi->new(2);

$z = $x + $y;
is($z, 12, q|$z = $x + $y;|);
is(ref($z), $mbf, qq|\$z is a "$mbf" object|);

$z = $x / $y;
is($z, 5, q|$z = $x / $y;|);
is(ref($z), $mbf, qq|\$z is a "$mbf" object|);

$z = $u * $y;
is($z, 5, q|$z = $u * $y;|);
is(ref($z), $mbf, qq|\$z is a "$mbf" object|);

$y = $mbi->new(12345);
$z = $u->copy()->bmul($y, 2, undef, 'odd');
is($z, 31000, q|$z = 31000|);

$z = $u->copy()->bmul($y, 3, undef, 'odd');
is($z, 30900, q|$z = 30900|);

$z = $u->copy()->bmul($y, undef, 0, 'odd');
is($z, 30863, q|$z = 30863|);

$z = $u->copy()->bmul($y, undef, 1, 'odd');
is($z, 30863, q|$z = 30863|);

$z = $u->copy()->bmul($y, undef, 2, 'odd');
is($z, 30860, q|$z = 30860|);

$z = $u->copy()->bmul($y, undef, 3, 'odd');
is($z, 30900, q|$z = 30900|);

$z = $u->copy()->bmul($y, undef, -1, 'odd');
is($z, 30862.5, q|$z = 30862.5|);

my $warn = '';
$SIG{__WARN__} = sub { $warn = shift; };

# These should no longer warn, even though '3.17' is a NaN in Math::BigInt
# (>= returns now false, bug until v1.80).

$warn = '';
eval '$z = 3.17 <= $y';
is($z, '', q|$z = ""|);
unlike($warn, qr/^Use of uninitialized value (\$y )?(in numeric le \(<=\) |)at/,
       q|"$z = $y >= 3.17" gives warning as expected|);

$warn = '';
eval '$z = $y >= 3.17';
is($z, '', q|$z = ""|);
unlike($warn, qr/^Use of uninitialized value (\$y )?(in numeric ge \(>=\) |)at/,
      q|"$z = $y >= 3.17" gives warning as expected|);

# XXX TODO breakage:
#
# $z = $y->copy()->bmul($u, 2, 0, 'odd');
# is($z, 31000);
#
# $z = $y * $u;
# is($z, 5);
# is(ref($z), $mbi, q|\$z is a $mbi object|);
#
# $z = $y + $x;
# is($z, 12);
# is(ref($z), $mbi, q|\$z is a $mbi object|);
#
# $z = $y / $x;
# is($z, 0);
# is(ref($z), $mbi, q|\$z is a $mbi object|);

###############################################################################
# rounding in bdiv with fallback and already set A or P

{
    no strict 'refs';
    ${"$mbf\::accuracy"}  = undef;
    ${"$mbf\::precision"} = undef;
    ${"$mbf\::div_scale"} = 40;
}

$x = $mbf->new(10);
$x->{_a} = 4;
is($x->bdiv(3), '3.333', q|$x->bdiv(3)|);
is($x->{_a}, 4, q|$x->{_a}|);                # set's it since no fallback

$x = $mbf->new(10);
$x->{_a} = 4;
$y = $mbf->new(3);
is($x->bdiv($y), '3.333', q|$x->bdiv($y)|);
is($x->{_a}, 4, q|$x->{_a}|);                   # set's it since no fallback

# rounding to P of x
$x = $mbf->new(10);
$x->{_p} = -2;
is($x->bdiv(3), '3.33', q|$x->bdiv(3)|);

# round in div with requested P
$x = $mbf->new(10);
is($x->bdiv(3, undef, -2), '3.33', q|$x->bdiv(3, undef, -2)|);

# round in div with requested P greater than fallback
{
    no strict 'refs';
    ${"$mbf\::div_scale"} = 5;
    $x = $mbf->new(10);
    is($x->bdiv(3, undef, -8), "3.33333333",
       q|$x->bdiv(3, undef, -8) = "3.33333333"|);
    ${"$mbf\::div_scale"} = 40;
}

$x = $mbf->new(10);
$y = $mbf->new(3);
$y->{_a} = 4;
is($x->bdiv($y), '3.333', q|$x->bdiv($y) = '3.333'|);
is($x->{_a}, 4, q|$x->{_a} = 4|);
is($y->{_a}, 4, q|$y->{_a} = 4|);       # set's it since no fallback
is($x->{_p}, undef, q|$x->{_p} = undef|);
is($y->{_p}, undef, q|$y->{_p} = undef|);

# rounding to P of y
$x = $mbf->new(10);
$y = $mbf->new(3);
$y->{_p} = -2;
is($x->bdiv($y), '3.33', q|$x->bdiv($y) = '3.33'|);
is($x->{_p}, -2, q|$x->{_p} = -2|);
 is($y->{_p}, -2, q|$y->{_p} = -2|);
is($x->{_a}, undef, q|$x->{_a} = undef|);
is($y->{_a}, undef, q|$y->{_a} = undef|);

###############################################################################
# test whether bround(-n) fails in MBF (undocumented in MBI)
eval { $x = $mbf->new(1);
       $x->bround(-2);
     };
like($@, qr/^bround\(\) needs positive accuracy/,
    qq|"\$x->bround(-2)" gives warning as expected|);

note("test whether rounding to higher accuracy is no-op");

$x = $mbf->new(1);
$x->{_a} = 4;
is($x, "1.000", q|$x = "1.000"|);
$x->bround(6);                  # must be no-op
is($x->{_a}, 4, q|$x->{_a} = 4|);
is($x, "1.000", q|$x = "1.000"|);

$x = $mbi->new(1230);
$x->{_a} = 3;
is($x, "1230", q|$x = "1230"|);
$x->bround(6);                  # must be no-op
is($x->{_a}, 3, q|$x->{_a} = 3|);
is($x, "1230", q|$x = "1230"|);

note("bround(n) should set _a");

$x->bround(2);                  # smaller works
is($x, "1200", q|$x = "1200"|);
is($x->{_a}, 2, q|$x->{_a} = 2|);

# bround(-n) is undocumented and only used by MBF

note("bround(-n) should set _a");

$x = $mbi->new(12345);
$x->bround(-1);
is($x, "12300", q|$x = "12300"|);
is($x->{_a}, 4, q|$x->{_a} = 4|);

note("bround(-n) should set _a");

$x = $mbi->new(12345);
$x->bround(-2);
is($x, "12000", q|$x = "12000"|);
is($x->{_a}, 3, q|$x->{_a} = 3|);

note("bround(-n) should set _a");

$x = $mbi->new(12345);
$x->{_a} = 5;
$x->bround(-3);
is($x, "10000", q|$x = "10000"|);
is($x->{_a}, 2, q|$x->{_a} = 2|);

note("bround(-n) should set _a");

$x = $mbi->new(12345);
$x->{_a} = 5;
$x->bround(-4);
is($x, "0", q|$x = "0"|);
is($x->{_a}, 1, q|$x->{_a} = 1|);

note("bround(-n) should be no-op if n too big");

$x = $mbi->new(12345);
$x->bround(-5);
is($x, "0", q|$x = "0"|);               # scale to "big" => 0
is($x->{_a}, 0, q|$x->{_a} = 0|);

note("bround(-n) should be no-op if n too big");

$x = $mbi->new(54321);
$x->bround(-5);
is($x, "100000", q|$x = "100000"|);     # used by MBF to round 0.0054321 at 0.0_6_00000
is($x->{_a}, 0, q|$x->{_a} = 0|);

note("bround(-n) should be no-op if n too big");

$x = $mbi->new(54321);
$x->{_a} = 5;
$x->bround(-6);
is($x, "100000", q|$x = "100000"|);     # no-op
is($x->{_a}, 0, q|$x->{_a} = 0|);

note("bround(n) should set _a");

$x = $mbi->new(12345);
$x->{_a} = 5;
$x->bround(5);                          # must be no-op
is($x, "12345", q|$x = "12345"|);
is($x->{_a}, 5, q|$x->{_a} = 5|);

note("bround(n) should set _a");

$x = $mbi->new(12345);
$x->{_a} = 5;
$x->bround(6);                          # must be no-op
is($x, "12345", q|$x = "12345"|);

$x = $mbf->new("0.0061");
$x->bfround(-2);
is($x, "0.01", q|$x = "0.01"|);
$x = $mbf->new("0.004");
$x->bfround(-2);
is($x, "0.00", q|$x = "0.00"|);
$x = $mbf->new("0.005");
$x->bfround(-2);
is($x, "0.00", q|$x = "0.00"|);

$x = $mbf->new("12345");
$x->bfround(2);
is($x, "12340", q|$x = "12340"|);
$x = $mbf->new("12340");
$x->bfround(2);
is($x, "12340", q|$x = "12340"|);

note("MBI::bfround should clear A for negative P");

$x = $mbi->new("1234");
$x->accuracy(3);
$x->bfround(-2);
is($x->{_a}, undef, q|$x->{_a} = undef|);

note("test that bfround() and bround() work with large numbers");

$x = $mbf->new(1)->bdiv(5678, undef, -63);
is($x, "0.000176118351532229658330398027474462839027826699542092286016203",
   q|$x = "0.000176118351532229658330398027474462839027826699542092286016203"|);

$x = $mbf->new(1)->bdiv(5678, undef, -90);
is($x, "0.00017611835153222965833039802747446283902782"
     . "6699542092286016202888340965128566396618527651",
   q|$x = "0.00017611835153222965833039802747446283902782|
       . q|6699542092286016202888340965128566396618527651"|);

$x = $mbf->new(1)->bdiv(5678, 80);
is($x, "0.00017611835153222965833039802747446283902782"
     . "669954209228601620288834096512856639662",
   q|$x = "0.00017611835153222965833039802747446283902782|
       . q|669954209228601620288834096512856639662"|);

###############################################################################

note("rounding with already set precision/accuracy");

$x = $mbf->new(1);
$x->{_p} = -5;
is($x, "1.00000", q|$x = "1.00000"|);

note("further rounding down");

is($x->bfround(-2), "1.00", q|$x->bfround(-2) = "1.00"|);
is($x->{_p}, -2, q|$x->{_p} = -2|);

$x = $mbf->new(12345);
$x->{_a} = 5;
is($x->bround(2), "12000", q|$x->bround(2) = "12000"|);
is($x->{_a}, 2, q|$x->{_a} = 2|);

$x = $mbf->new("1.2345");
$x->{_a} = 5;
is($x->bround(2), "1.2", q|$x->bround(2) = "1.2"|);
is($x->{_a}, 2, q|$x->{_a} = 2|);

note("mantissa/exponent format and A/P");

$x = $mbf->new("12345.678");
$x->accuracy(4);
is($x, "12350", q|$x = "12350"|);
is($x->{_a}, 4, q|$x->{_a} = 4|);
is($x->{_p}, undef, q|$x->{_p} = undef|);

#is($x->{_m}->{_a}, undef, q|$x->{_m}->{_a} = undef|);
#is($x->{_e}->{_a}, undef, q|$x->{_e}->{_a} = undef|);
#is($x->{_m}->{_p}, undef, q|$x->{_m}->{_p} = undef|);
#is($x->{_e}->{_p}, undef, q|$x->{_e}->{_p} = undef|);

note("check for no A/P in case of fallback result");

$x = $mbf->new(100) / 3;
is($x->{_a}, undef, q|$x->{_a} = undef|);
is($x->{_p}, undef, q|$x->{_p} = undef|);

note("result & remainder");

$x = $mbf->new(100) / 3;
($x, $y) = $x->bdiv(3);
is($x->{_a}, undef, q|$x->{_a} = undef|);
is($x->{_p}, undef, q|$x->{_p} = undef|);
is($y->{_a}, undef, q|$y->{_a} = undef|);
is($y->{_p}, undef, q|$y->{_p} = undef|);

###############################################################################
# math with two numbers with different A and P

$x = $mbf->new(12345);
$x->accuracy(4); # "12340"
$y = $mbf->new(12345);
$y->accuracy(2); # "12000"
is($x+$y, 24000, q|$x+$y = 24000|);     # 12340+12000=> 24340 => 24000

$x = $mbf->new(54321);
$x->accuracy(4); # "12340"
$y = $mbf->new(12345);
$y->accuracy(3); # "12000"
is($x-$y, 42000, q|$x-$y = 42000|);     # 54320+12300=> 42020 => 42000

$x = $mbf->new("1.2345");
$x->precision(-2); # "1.23"
$y = $mbf->new("1.2345");
$y->precision(-4); # "1.2345"
is($x+$y, "2.46", q|$x+$y = "2.46"|);   # 1.2345+1.2300=> 2.4645 => 2.46

###############################################################################
# round should find and use proper class

#$x = Foo->new();
#is($x->round($Foo::accuracy), "a" x $Foo::accuracy);
#is($x->round(undef, $Foo::precision), "p" x $Foo::precision);
#is($x->bfround($Foo::precision), "p" x $Foo::precision);
#is($x->bround($Foo::accuracy), "a" x $Foo::accuracy);

###############################################################################
# find out whether _find_round_parameters is doing what's it's supposed to do

{
    no strict 'refs';
    ${"$mbi\::accuracy"} = undef;
    ${"$mbi\::precision"} = undef;
    ${"$mbi\::div_scale"} = 40;
    ${"$mbi\::round_mode"} = 'odd';
}

$x = $mbi->new(123);
my @params = $x->_find_round_parameters();
is(scalar(@params), 1, q|scalar(@params) = 1|);       # nothing to round

@params = $x->_find_round_parameters(1);
is(scalar(@params), 4, q|scalar(@params) = 4|);       # a=1
is($params[0], $x, q|$params[0] = $x|);               # self
is($params[1], 1, q|$params[1] = 1|);                 # a
is($params[2], undef, q|$params[2] = undef|);         # p
is($params[3], "odd", q|$params[3] = "odd"|);         # round_mode

@params = $x->_find_round_parameters(undef, 2);
is(scalar(@params), 4, q|scalar(@params) = 4|);       # p=2
is($params[0], $x, q|$params[0] = $x|);               # self
is($params[1], undef, q|$params[1] = undef|);         # a
is($params[2], 2, q|$params[2] = 2|);                 # p
is($params[3], "odd", q|$params[3] = "odd"|);         # round_mode

eval { @params = $x->_find_round_parameters(undef, 2, "foo"); };
like($@, qr/^Unknown round mode 'foo'/,
    q|round mode "foo" gives a warning as expected|);

@params = $x->_find_round_parameters(undef, 2, "+inf");
is(scalar(@params), 4, q|scalar(@params) = 4|);       # p=2
is($params[0], $x, q|$params[0] = $x|);               # self
is($params[1], undef, q|$params[1] = undef|);         # a
is($params[2], 2, q|$params[2] = 2|);                 # p
is($params[3], "+inf", q|$params[3] = "+inf"|);       # round_mode

@params = $x->_find_round_parameters(2, -2, "+inf");
is(scalar(@params), 1, q|scalar(@params) = 1|);       # error, A and P defined
is($params[0], $x, q|$params[0] = $x|);               # self

{
    no strict 'refs';
    ${"$mbi\::accuracy"} = 1;
    @params = $x->_find_round_parameters(undef, -2);
    is(scalar(@params), 1, q|scalar(@params) = 1|);   # error, A and P defined
    is($params[0], $x, q|$params[0] = $x|);           # self
    is($x->is_nan(), 1, q|$x->is_nan() = 1|);         # and must be NaN

    ${"$mbi\::accuracy"} = undef;
    ${"$mbi\::precision"} = 1;
    @params = $x->_find_round_parameters(1, undef);
    is(scalar(@params), 1, q|scalar(@params) = 1|);   # error, A and P defined
    is($params[0], $x, q|$params[0] = $x|);           # self
    is($x->is_nan(), 1, q|$x->is_nan() = 1|);         # and must be NaN

    ${"$mbi\::precision"} = undef; # reset
}

###############################################################################
# test whether bone/bzero take additional A & P, or reset it etc

foreach my $class ($mbi, $mbf) {
    $x = $class->new(2)->bzero();
    is($x->{_a}, undef, qq|\$x = $class->new(2)->bzero(); \$x->{_a}|);
    is($x->{_p}, undef, qq|\$x = $class->new(2)->bzero(); \$x->{_p}|);

    $x = $class->new(2)->bone();
    is($x->{_a}, undef, qq|\$x = $class->new(2)->bone(); \$x->{_a}|);
    is($x->{_p}, undef, qq|\$x = $class->new(2)->bone(); \$x->{_p}|);

    $x = $class->new(2)->binf();
    is($x->{_a}, undef, qq|\$x = $class->new(2)->binf(); \$x->{_a}|);
    is($x->{_p}, undef, qq|\$x = $class->new(2)->binf(); \$x->{_p}|);

    $x = $class->new(2)->bnan();
    is($x->{_a}, undef, qq|\$x = $class->new(2)->bnan(); \$x->{_a}|);
    is($x->{_p}, undef, qq|\$x = $class->new(2)->bnan(); \$x->{_p}|);

    note "Verify that bnan() does not delete/undefine accuracy and precision.";

    $x = $class->new(2);
    $x->{_a} = 1;
    $x->bnan();
    is($x->{_a}, 1, qq|\$x = $class->new(2); \$x->{_a} = 1; \$x->bnan(); \$x->{_a}|);

    $x = $class->new(2);
    $x->{_p} = 1;
    $x->bnan();
    is($x->{_p}, 1, qq|\$x = $class->new(2); \$x->{_p} = 1; \$x->bnan(); \$x->{_p}|);

    note "Verify that binf() does not delete/undefine accuracy and precision.";

    $x = $class->new(2);
    $x->{_a} = 1;
    $x->binf();
    is($x->{_a}, 1, qq|\$x = $class->new(2); \$x->{_a} = 1; \$x->binf(); \$x->{_a}|);

    $x = $class->new(2);
    $x->{_p} = 1;
    $x->binf();
    is($x->{_p}, 1, qq|\$x = $class->new(2); \$x->{_p} = 1; \$x->binf(); \$x->{_p}|);

    note "Verify that accuracy can be set as argument to new().";

    $x = $class->new(2, 1);
    is($x->{_a}, 1,     qq|\$x = $class->new(2, 1); \$x->{_a}|);
    is($x->{_p}, undef, qq|\$x = $class->new(2, 1); \$x->{_p}|);

    note "Verify that precision can be set as argument to new().";

    $x = $class->new(2, undef, 1);
    is($x->{_a}, undef, qq|\$x = $class->new(2, undef, 1); \$x->{_a}|);
    is($x->{_p}, 1,     qq|\$x = $class->new(2, undef, 1); \$x->{_p}|);

    note "Verify that accuracy set with new() is preserved after calling bzero().";

    $x = $class->new(2, 1)->bzero();
    is($x->{_a}, 1,     qq|\$x = $class->new(2, 1)->bzero(); \$x->{_a}|);
    is($x->{_p}, undef, qq|\$x = $class->new(2, 1)->bzero(); \$x->{_p}|);

    note "Verify that precision set with new() is preserved after calling bzero().";

    $x = $class->new(2, undef, 1)->bzero();
    is($x->{_a}, undef, qq|\$x = $class->new(2, undef, 1)->bzero(); \$x->{_a}|);
    is($x->{_p}, 1,     qq|\$x = $class->new(2, undef, 1)->bzero(); \$x->{_p}|);

    note "Verify that accuracy set with new() is preserved after calling bone().";

    $x = $class->new(2, 1)->bone();
    is($x->{_a}, 1,     qq|\$x = $class->new(2, 1)->bone(); \$x->{_a}|);
    is($x->{_p}, undef, qq|\$x = $class->new(2, 1)->bone(); \$x->{_p}|);

    note "Verify that precision set with new() is preserved after calling bone().";

    $x = $class->new(2, undef, 1)->bone();
    is($x->{_a}, undef, qq|\$x = $class->new(2, undef, 1)->bone(); \$x->{_a}|);
    is($x->{_p}, 1,     qq|\$x = $class->new(2, undef, 1)->bone(); \$x->{_p}|);

    note "Verify that accuracy can be set with instance method bone('+').";

    $x = $class->new(2);
    $x->bone('+', 2, undef);
    is($x->{_a}, 2,     qq|\$x = $class->new(2); \$x->{_a}|);
    is($x->{_p}, undef, qq|\$x = $class->new(2); \$x->{_p}|);

    note "Verify that precision can be set with instance method bone('+').";

    $x = $class->new(2);
    $x->bone('+', undef, 2);
    is($x->{_a}, undef, qq|\$x = $class->new(2); \$x->bone('+', undef, 2); \$x->{_a}|);
    is($x->{_p}, 2,     qq|\$x = $class->new(2); \$x->bone('+', undef, 2); \$x->{_p}|);

    note "Verify that accuracy can be set with instance method bone('-').";

    $x = $class->new(2);
    $x->bone('-', 2, undef);
    is($x->{_a}, 2,     qq|\$x = $class->new(2); \$x->bone('-', 2, undef); \$x->{_a}|);
    is($x->{_p}, undef, qq|\$x = $class->new(2); \$x->bone('-', 2, undef); \$x->{_p}|);

    note "Verify that precision can be set with instance method bone('-').";

    $x = $class->new(2);
    $x->bone('-', undef, 2);
    is($x->{_a}, undef, qq|\$x = $class->new(2); \$x->bone('-', undef, 2); \$x->{_a}|);
    is($x->{_p}, 2,     qq|\$x = $class->new(2); \$x->bone('-', undef, 2); \$x->{_p}|);

    note "Verify that accuracy can be set with instance method bzero().";

    $x = $class->new(2);
    $x->bzero(2, undef);
    is($x->{_a}, 2,     qq|\$x = $class->new(2);\$x->bzero(2, undef); \$x->{_a}|);
    is($x->{_p}, undef, qq|\$x = $class->new(2);\$x->bzero(2, undef); \$x->{_p}|);

    note "Verify that precision can be set with instance method bzero().";

    $x = $class->new(2);
    $x->bzero(undef, 2);
    is($x->{_a}, undef, qq|\$x = $class->new(2); \$x->bzero(undef, 2); \$x->{_a}|);
    is($x->{_p}, 2,     qq|\$x = $class->new(2); \$x->bzero(undef, 2); \$x->{_p}|);
}

###############################################################################
# test whether bone/bzero honour class variables

for my $class ($mbi, $mbf) {

    note "Verify that class accuracy is copied into new objects.";

    $class->accuracy(3);                # set

    $x = $class->bzero();
    is($x->accuracy(), 3,
       qq|$class->accuracy(3); \$x = $class->bzero(); \$x->accuracy()|);

    $x = $class->bone();
    is($x->accuracy(), 3,
       qq|$class->accuracy(3); \$x = $class->bone(); \$x->accuracy()|);

    $x = $class->new(2);
    is($x->accuracy(), 3,
       qq|$class->accuracy(3); \$x = $class->new(2); \$x->accuracy()|);

    $class->accuracy(undef);            # reset

    note "Verify that class precision is copied into new objects.";

    $class->precision(-4);              # set

    $x = $class->bzero();
    is($x->precision(), -4,
       qq|$class->precision(-4); \$x = $class->bzero(); \$x->precision()|);

    $x = $class->bone();
    is($x->precision(), -4,
       qq|$class->precision(-4); \$x = $class->bone(); \$x->precision()|);

    $x = $class->new(2);
    is($x->precision(), -4,
       qq|$class->precision(-4); \$x = $class->new(2); \$x->precision()|);

    $class->precision(undef);           # reset

    note "Verify that setting accuracy as method argument overrides class variable";

    $class->accuracy(2);                # set

    $x = $class->bzero(5);
    is($x->accuracy(), 5,
       qq|$class->accuracy(2); \$x = $class->bzero(5); \$x->accuracy()|);

    SKIP: {
          skip 1, "this won't work until we have a better OO implementation";

          $x = $class->bzero(undef);
          is($x->accuracy(), undef,
             qq|$class->accuracy(2); \$x = $class->bzero(undef); \$x->accuracy()|);
      }

    $x = $class->bone("+", 5);
    is($x->accuracy(), 5,
       qq|$class->accuracy(2); \$x = $class->bone("+", 5); \$x->accuracy()|);

    SKIP: {
          skip 1, "this won't work until we have a better OO implementation";

          $x = $class->bone("+", undef);
          is($x->accuracy(), undef,
             qq|$class->accuracy(2); \$x = $class->bone("+", undef); \$x->accuracy()|);
      }

    $x = $class->new(2, 5);
    is($x->accuracy(), 5,
       qq|$class->accuracy(2); \$x = $class->new(2, 5); \$x->accuracy()|);

    SKIP: {
          skip 1, "this won't work until we have a better OO implementation";

          $x = $class->new(2, undef);
          is($x->accuracy(), undef,
             qq|$class->accuracy(2); \$x = $class->new(2, undef); \$x->accuracy()|);
      }

    $class->accuracy(undef);            # reset

    note "Verify that setting precision as method argument overrides class variable";

    $class->precision(-2);              # set

    $x = $class->bzero(undef, -6);
    is($x->precision(), -6,
       qq|$class->precision(-2); \$x = $class->bzero(undef, -6); \$x->precision()|);

    SKIP: {
          skip 1, "this won't work until we have a better OO implementation";

          $x = $class->bzero(undef, undef);
          is($x->precision(), undef,
             qq|$class->precision(-2); \$x = $class->bzero(undef, undef); \$x->precision()|);
      }

    $x = $class->bone("+", undef, -6);
    is($x->precision(), -6,
       qq|$class->precision(-2); \$x = $class->bone("+", undef, -6); \$x->precision()|);

    SKIP: {
          skip 1, "this won't work until we have a better OO implementation";

          $x = $class->bone("+", undef, undef);
          is($x->precision(), undef,
             qq|$class->precision(-2); \$x = $class->bone("+", undef, undef); \$x->precision()|);
      }

    $x = $class->new(2, undef, -6);
    is($x->precision(), -6,
       qq|$class->precision(-2); \$x = $class->new(2, undef, -6); \$x->precision()|);

    SKIP: {
          skip 1, "this won't work until we have a better OO implementation";

          $x = $class->new(2, undef, undef);
          is($x->precision(), undef,
             qq|$class->precision(-2); \$x = $class->new(2, undef, undef); \$x->precision()|);
      }

    $class->precision(undef);           # reset
}

###############################################################################
# check whether mixing A and P creates a NaN

# new with set accuracy/precision and with parameters
{
    no strict 'refs';
    foreach my $class ($mbi, $mbf) {
        is($class->new(123, 4, -3), 'NaN',      # with parameters
           "mixing A and P creates a NaN");
        ${"$class\::accuracy"} = 42;
        ${"$class\::precision"} = 2;
        is($class->new(123), "NaN",             # with globals
           q|$class->new(123) = "NaN"|);
        ${"$class\::accuracy"} = undef;
        ${"$class\::precision"} = undef;
    }
}

# binary ops
foreach my $class ($mbi, $mbf) {
    #foreach (qw/add sub mul div pow mod/) {
    foreach my $method (qw/add sub mul pow mod/) {
        my $try = "my \$x = $class->new(1234); \$x->accuracy(5);";
        $try .= " my \$y = $class->new(12); \$y->precision(-3);";
        $try .= " \$x->b$method(\$y);";
        $rc = eval $try;
        is($rc, "NaN", $try);
    }
}

# unary ops
foreach my $method (qw/new bsqrt/) {
    my $try = "my \$x = $mbi->$method(1234, 5, -3);";
    $rc = eval $try;
    is($rc, "NaN", $try);
}

# see if $x->bsub(0) and $x->badd(0) really round
foreach my $class ($mbi, $mbf) {
    $x = $class->new(123);
    $class->accuracy(2);
    $x->bsub(0);
    is($x, 120, q|$x = 120|);

    $class->accuracy(undef);            # reset

    $x = $class->new(123);
    $class->accuracy(2);
    $x->badd(0);
    is($x, 120, q|$x = 120|);

    $class->accuracy(undef);            # reset
}

###############################################################################
# test whether shortcuts returning zero/one preserve A and P

my ($got, $f, $a, $p, $xp, $yp, $xa, $ya, $try, $want, @args);

my $LIB = Math::BigInt->config('lib');

while (<DATA>) {
    s/#.*$//;                   # remove comments
    s/\s+$//;                   # remove trailing whitespace
    next unless length;         # skip empty lines

    if (s/^&//) {
        $f = $_;                # function
        next;
    }

    @args = split(/:/, $_);
    my $want = pop(@args);

    ($x, $xa, $xp) = split (/,/, $args[0]);
    $xa = $xa || '';
    $xp = $xp || '';
    $try  = qq|\$x = $mbi->new("$x");|;
    $try .= qq| \$x->accuracy($xa);|  if $xa ne '';
    $try .= qq| \$x->precision($xp);| if $xp ne '';

    ($y, $ya, $yp) = split (/,/, $args[1]);
    $ya = $ya || '';
    $yp = $yp || '';
    $try .= qq| \$y = $mbi->new("$y");|;
    $try .= qq| \$y->accuracy($ya);|  if $ya ne '';
    $try .= qq| \$y->precision($yp);| if $yp ne '';

    $try .= ' $x->' . $f . '($y);';

    # print "trying $try\n";
    $rc = eval $try;
    print "# Error: $@\n" if $@;

    # convert hex/binary targets to decimal
    if ($want =~ /^(0x0x|0b0b)/) {
        $want =~ s/^0[xb]//;
        $want = $mbi->new($want)->bstr();
    }
    is($rc, $want, $try);
    # check internal state of number objects
    is_valid($rc, $f) if ref $rc;

    # now check whether A and P are set correctly
    # only one of $a or $p will be set (no crossing here)
    $a = $xa || $ya;
    $p = $xp || $yp;

    # print "Check a=$a p=$p\n";
    # print "# Tried: '$try'\n";
    if ($a ne '') {
        unless (is($x->{_a}, $a,    qq|\$x->{_a} == $a|) &&
                is($x->{_p}, undef, qq|\$x->{_p} is undef|))
        {
            print "# Check: A = $a and P = undef\n";
            print "# Tried: $try\n";
        }
    }
    if ($p ne '') {
        unless (is($x->{_p}, $p,    qq|\$x->{_p} == $p|) &&
                is($x->{_a}, undef, qq|\$x->{_a} is undef|))
        {
            print "# Check: A = undef and P = $p\n";
            print "# Tried: $try\n";
        }
    }
}

# all done
1;

###############################################################################
# sub to check validity of a Math::BigInt object internally, to ensure that no
# op leaves a number object in an invalid state (f.i. "-0")

sub is_valid {
    my ($x, $f) = @_;

    my $e = 0;                  # error?

    # ok as reference?
    $e = 'Not a reference' if !ref($x);

    # has ok sign?
    $e = qq|Illegal sign $x->{sign}|
      . q| (expected: "+", "-", "-inf", "+inf" or "NaN")|
        if $e eq '0' && $x->{sign} !~ /^(\+|-|\+inf|-inf|NaN)$/;

    $e = "-0 is invalid!" if $e ne '0' && $x->{sign} eq '-' && $x == 0;
    $e = $LIB->_check($x->{value}) if $e eq '0';

    # test done, see if error did crop up
    if ($e eq '0') {
        pass('is a valid object');
        return;
    }

    fail($e . qq| after op "$f"|);
}

# format is:
# x,A,P:x,A,P:result
# 123,,3 means 123 with precision 3 (A is undef)
# the A or P of the result is calculated automatically
__DATA__
&badd
123,,:123,,:246
123,3,:0,,:123
123,,-3:0,,:123
123,,:0,3,:123
123,,:0,,-3:123
&bmul
123,,:1,,:123
123,3,:0,,:0
123,,-3:0,,:0
123,,:0,3,:0
123,,:0,,-3:0
123,3,:1,,:123
123,,-3:1,,:123
123,,:1,3,:123
123,,:1,,-3:123
1,3,:123,,:123
1,,-3:123,,:123
1,,:123,3,:123
1,,:123,,-3:123
&bdiv
123,,:1,,:123
123,4,:1,,:123
123,,:1,4,:123
123,,:1,,-4:123
123,,-4:1,,:123
1,4,:123,,:0
1,,:123,4,:0
1,,:123,,-4:0
1,,-4:123,,:0
&band
1,,:3,,:1
1234,1,:0,,:0
1234,,:0,1,:0
1234,,-1:0,,:0
1234,,:0,,-1:0
0xFF,,:0x10,,:0x0x10
0xFF,2,:0xFF,,:250
0xFF,,:0xFF,2,:250
0xFF,,1:0xFF,,:250
0xFF,,:0xFF,,1:250
&bxor
1,,:3,,:2
1234,1,:0,,:1000
1234,,:0,1,:1000
1234,,3:0,,:1000
1234,,:0,,3:1000
0xFF,,:0x10,,:239
# 250 ^ 255 => 5
0xFF,2,:0xFF,,:5
0xFF,,:0xFF,2,:5
0xFF,,1:0xFF,,:5
0xFF,,:0xFF,,1:5
# 250 ^ 4095 = 3845 => 3800
0xFF,2,:0xFFF,,:3800
# 255 ^ 4100 = 4347 => 4300
0xFF,,:0xFFF,2,:4300
0xFF,,2:0xFFF,,:3800
# 255 ^ 4100 = 10fb => 4347 => 4300
0xFF,,:0xFFF,,2:4300
&bior
1,,:3,,:3
1234,1,:0,,:1000
1234,,:0,1,:1000
1234,,3:0,,:1000
1234,,:0,,3:1000
0xFF,,:0x10,,:0x0xFF
# FF | FA = FF => 250
250,2,:0xFF,,:250
0xFF,,:250,2,:250
0xFF,,1:0xFF,,:250
0xFF,,:0xFF,,1:250
&bpow
2,,:3,,:8
2,,:0,,:1
2,2,:0,,:1
2,,:0,2,:1
