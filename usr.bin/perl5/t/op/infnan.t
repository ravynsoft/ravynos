#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use strict;

use Config;

BEGIN {
    if ($^O eq 'aix' && $Config{uselongdouble}) {
        # FWIW: NaN actually seems to be working decently,
        # but Inf is completely broken (e.g. Inf + 0 -> NaN).
        skip_all "$^O with long doubles does not have sane inf/nan";
    }
    unless ($Config{d_double_has_inf} && $Config{d_double_has_nan}) {
        skip_all "the doublekind $Config{doublekind} does not have inf/nan";
    }
}

my $PInf = "Inf"  + 0;
my $NInf = "-Inf" + 0;
my $NaN;
{
    local $^W = 0; # warning-ness tested later.
    $NaN  = "NaN" + 0;
}

my @PInf = ("Inf", "inf", "INF", "+Inf",
            "Infinity",
            "1.#INF", "1#INF", "1.#INF00");
my @NInf = map { "-$_" } grep { ! /^\+/ } @PInf;

my @NaN = ("NAN", "nan", "qnan", "SNAN", "NanQ", "NANS",
           "1.#QNAN", "+1#SNAN", "-1.#NAN", "1#IND", "1.#IND00",
           "NAN(123)");

my @printf_fmt = qw(e f g a d u o i b x);
my @packi_fmt = qw(c C s S l L i I n N v V j J w W U);
my @packf_fmt = qw(f d F);
my @packs_fmt = qw(a4 A4 Z5 b20 B20 h10 H10 u);

if ($Config{ivsize} == 8) {
    push @packi_fmt, qw(q Q);
}

if ($Config{uselongdouble} && $Config{nvsize} > $Config{doublesize}) {
    push @packf_fmt, 'D';
}

# === Inf tests ===

cmp_ok($PInf, '>', 0, "positive infinity");
cmp_ok($NInf, '<', 0, "negative infinity");

cmp_ok($PInf, '>', $NInf, "positive > negative");
cmp_ok($NInf, '==', -$PInf, "negative == -positive");
cmp_ok(-$NInf, '==', $PInf, "--negative == positive");

is($PInf,  "Inf", "$PInf value stringifies as Inf");
is($NInf, "-Inf", "$NInf value stringifies as -Inf");

cmp_ok($PInf + 0, '==', $PInf, "+Inf + zero is +Inf");
cmp_ok($NInf + 0, '==', $NInf, "-Inf + zero is -Inf");

cmp_ok($PInf + 1, '==', $PInf, "+Inf + one is +Inf");
cmp_ok($NInf + 1, '==', $NInf, "-Inf + one is -Inf");

cmp_ok($PInf + $PInf, '==', $PInf, "+Inf + Inf is +Inf");
cmp_ok($NInf + $NInf, '==', $NInf, "-Inf - Inf is -Inf");

cmp_ok($PInf * 2, '==', $PInf, "twice Inf is Inf");
cmp_ok($PInf / 2, '==', $PInf, "half of Inf is Inf");

cmp_ok($PInf * $PInf, '==', $PInf, "+Inf * +Inf is +Inf");
cmp_ok($PInf * $NInf, '==', $NInf, "+Inf * -Inf is -Inf");
cmp_ok($NInf * $PInf, '==', $NInf, "-Inf * +Inf is -Inf");
cmp_ok($NInf * $NInf, '==', $PInf, "-Inf * -Inf is +Inf");

is(sprintf("%g", $PInf), "Inf", "$PInf sprintf %g is Inf");
is(sprintf("%a", $PInf), "Inf", "$PInf sprintf %a is Inf");

for my $f (@printf_fmt) {
    is(sprintf("%$f", $PInf), "Inf", "$PInf sprintf %$f is Inf");
}

is(sprintf("%+g", $PInf), "+Inf", "$PInf sprintf %+g");
is(sprintf("%+g", $NInf), "-Inf", "$PInf sprintf %+g");

is(sprintf("%4g",  $PInf), " Inf", "$PInf sprintf %4g");
is(sprintf("%-4g", $PInf), "Inf ", "$PInf sprintf %-4g");

is(sprintf("%+-5g", $PInf), "+Inf ", "$PInf sprintf %+-5g");
is(sprintf("%-+5g", $PInf), "+Inf ", "$PInf sprintf %-+5g");

is(sprintf("%-+5g", $NInf), "-Inf ", "$NInf sprintf %-+5g");
is(sprintf("%+-5g", $NInf), "-Inf ", "$NInf sprintf %+-5g");

ok(!defined eval { $a = sprintf("%c", $PInf)}, "sprintf %c +Inf undef");
like($@, qr/Cannot printf/, "$PInf sprintf fails");
ok(!defined eval { $a = sprintf("%c", "Inf")},
  "stringy sprintf %c +Inf undef");
like($@, qr/Cannot printf/, "stringy $PInf sprintf %c fails");

ok(!defined eval { $a = chr($PInf) }, "chr(+Inf) undef");
like($@, qr/Cannot chr/, "+Inf chr() fails");
ok(!defined eval { $a = chr("Inf") }, "chr(stringy +Inf) undef");
like($@, qr/Cannot chr/, "stringy +Inf chr() fails");

ok(!defined eval { $a = sprintf("%c", $NInf)}, "sprintf %c -Inf undef");
like($@, qr/Cannot printf/, "$NInf sprintf fails");
ok(!defined eval { $a = sprintf("%c", "-Inf")},
  "sprintf %c stringy -Inf undef");
like($@, qr/Cannot printf/, "stringy $NInf sprintf %c fails");

ok(!defined eval { $a = chr($NInf) }, "chr(-Inf) undef");
like($@, qr/Cannot chr/, "-Inf chr() fails");
ok(!defined eval { $a = chr("-Inf") }, "chr(stringy -Inf) undef");
like($@, qr/Cannot chr/, "stringy -Inf chr() fails");

for my $f (@packi_fmt) {
    undef $a;
    ok(!defined eval { $a = pack($f, $PInf) }, "pack $f +Inf undef");
    like($@, $f eq 'w' ? qr/Cannot compress Inf/: qr/Cannot pack Inf/,
         "+Inf pack $f fails");
    undef $a;
    ok(!defined eval { $a = pack($f, "Inf") },
      "pack $f stringy +Inf undef");
    like($@, $f eq 'w' ? qr/Cannot compress Inf/: qr/Cannot pack Inf/,
         "stringy +Inf pack $f fails");
    undef $a;
    ok(!defined eval { $a = pack($f, $NInf) }, "pack $f -Inf undef");
    like($@, $f eq 'w' ? qr/Cannot compress -Inf/: qr/Cannot pack -Inf/,
         "-Inf pack $f fails");
    undef $a;
    ok(!defined eval { $a = pack($f, "-Inf") },
      "pack $f stringy -Inf undef");
    like($@, $f eq 'w' ? qr/Cannot compress -Inf/: qr/Cannot pack -Inf/,
         "stringy -Inf pack $f fails");
}

for my $f (@packf_fmt) {
    undef $a;
    undef $b;
    ok(defined eval { $a = pack($f, $PInf) }, "pack $f +Inf defined");
    eval { $b = unpack($f, $a) };
    cmp_ok($b, '==', $PInf, "pack $f +Inf equals $PInf");

    undef $a;
    undef $b;
    ok(defined eval { $a = pack($f, $NInf) }, "pack $f -Inf defined");
    eval { $b = unpack($f, $a) };
    cmp_ok($b, '==', $NInf, "pack $f -Inf equals $NInf");
}

for my $f (@packs_fmt) {
    undef $a;
    ok(defined eval { $a = pack($f, $PInf) }, "pack $f +Inf defined");
    is($a, pack($f, "Inf"), "pack $f +Inf same as 'Inf'");

    undef $a;
    ok(defined eval { $a = pack($f, $NInf) }, "pack $f -Inf defined");
    is($a, pack($f, "-Inf"), "pack $f -Inf same as 'Inf'");
}

is eval { unpack "p", pack 'p', $PInf }, "Inf", "pack p +Inf";
is eval { unpack "P3", pack 'P', $PInf }, "Inf", "pack P +Inf";
is eval { unpack "p", pack 'p', $NInf }, "-Inf", "pack p -Inf";
is eval { unpack "P4", pack 'P', $NInf }, "-Inf", "pack P -Inf";

for my $i (@PInf) {
    cmp_ok($i + 0 , '==', $PInf, "$i is +Inf");
    cmp_ok($i, '>', 0, "$i is positive");
    is("@{[$i+0]}", "Inf", "$i value stringifies as Inf");
}

for my $i (@NInf) {
    cmp_ok($i + 0, '==', $NInf, "$i is -Inf");
    cmp_ok($i, '<', 0, "$i is negative");
    is("@{[$i+0]}", "-Inf", "$i value stringifies as -Inf");
}

is($PInf + $PInf, $PInf, "+Inf plus +Inf is +Inf");
is($NInf + $NInf, $NInf, "-Inf plus -Inf is -Inf");

is(1/$PInf, 0, "one per +Inf is zero");
is(1/$NInf, 0, "one per -Inf is zero");

my ($PInfPP, $PInfMM) = ($PInf, $PInf);
my ($NInfPP, $NInfMM) = ($NInf, $NInf);;
$PInfPP++;
$PInfMM--;
$NInfPP++;
$NInfMM--;
is($PInfPP, $PInf, "+Inf++ is +Inf");
is($PInfMM, $PInf, "+Inf-- is +Inf");
is($NInfPP, $NInf, "-Inf++ is -Inf");
is($NInfMM, $NInf, "-Inf-- is -Inf");

ok($PInf, "+Inf is true");
ok($NInf, "-Inf is true");

is(abs($PInf), $PInf, "abs(+Inf) is +Inf");
is(abs($NInf), $PInf, "abs(-Inf) is +Inf");

# One could argue of NaN as the result.
is(int($PInf), $PInf, "int(+Inf) is +Inf");
is(int($NInf), $NInf, "int(-Inf) is -Inf");

is(sqrt($PInf), $PInf, "sqrt(+Inf) is +Inf");
# sqrt $NInf doesn't work because negative is caught

is(exp($PInf), $PInf, "exp(+Inf) is +Inf");
is(exp($NInf), 0, "exp(-Inf) is zero");

SKIP: {
    if ($PInf == 0) {
        skip "if +Inf == 0 cannot log(+Inf)", 1;
    }
    is(log($PInf), $PInf, "log(+Inf) is +Inf");
}
# log $NInf doesn't work because negative is caught

is(rand($PInf), $PInf, "rand(+Inf) is +Inf");
is(rand($NInf), $NInf, "rand(-Inf) is -Inf");

# XXX Bit operations?
# +Inf & 1 == +Inf?
# +Inf | 1 == +Inf?
# +Inf ^ 1 == +Inf?
# ~+Inf    == 0? or NaN?
# -Inf ... ???
# NaN & 1 == NaN?
# NaN | 1 == NaN?
# NaN ^ 1 == NaN?
# ~NaN    == NaN???
# Or just declare insanity and die?

TODO: {
    local $::TODO;
    my $here = "$^O $Config{osvers}";
    $::TODO = "$here: pow (9**9**9) doesn't give Inf"
        if $here =~ /^(?:hpux 10)/;
    is(9**9**9, $PInf, "9**9**9 is Inf");
}

SKIP: {
    my @FInf = qw(Infinite Info Inf123 Infiniti Infinityz);
    if ($Config{usequadmath}) {
        skip "quadmath strtoflt128() accepts false infinities", scalar @FInf;
    }
    for my $i (@FInf) {
        # Silence "isn't numeric in addition", that's kind of the point.
        local $^W = 0;
        cmp_ok("$i" + 0, '==', $PInf, "false infinity $i");
    }
}

{
    # Silence "Non-finite repeat count", that is tested elsewhere.
    local $^W = 0;
    is("a" x $PInf, "", "x +Inf");
    is("a" x $NInf, "", "x -Inf");
}

{
    eval 'for my $x (0..$PInf) { last }';
    like($@, qr/Range iterator outside integer range/, "0..+Inf fails");

    eval 'for my $x ($NInf..0) { last }';
    like($@, qr/Range iterator outside integer range/, "-Inf..0 fails");
}

# === NaN ===

cmp_ok($NaN, '!=', $NaN, "NaN is NaN numerically (by not being NaN)");
ok($NaN eq $NaN, "NaN is NaN stringifically");

is("$NaN", "NaN", "$NaN value stringifies as NaN");

{
    local $^W = 0; # warning-ness tested later.
    is("+NaN" + 0, "NaN", "+NaN is NaN");
    is("-NaN" + 0, "NaN", "-NaN is NaN");
}

is($NaN + 0, $NaN, "NaN + zero is NaN");

is($NaN + 1, $NaN, "NaN + one is NaN");

is($NaN * 2, $NaN, "twice NaN is NaN");
is($NaN / 2, $NaN, "half of NaN is NaN");

is($NaN * $NaN, $NaN, "NaN * NaN is NaN");
SKIP: {
    if ($NaN == 0) {
        skip "NaN looks like zero, avoiding dividing by it", 1;
    }
    is($NaN / $NaN, $NaN, "NaN / NaN is NaN");
}

for my $f (@printf_fmt) {
    is(sprintf("%$f", $NaN), "NaN", "$NaN sprintf %$f is NaN");
}

is(sprintf("%+g", $NaN), "NaN", "$NaN sprintf %+g");

is(sprintf("%4g",  $NaN), " NaN", "$NaN sprintf %4g");
is(sprintf("%-4g", $NaN), "NaN ", "$NaN sprintf %-4g");

is(sprintf("%+-5g", $NaN), "NaN  ", "$NaN sprintf %+-5g");
is(sprintf("%-+5g", $NaN), "NaN  ", "$NaN sprintf %-+5g");

ok(!defined eval { $a = sprintf("%c", $NaN)}, "sprintf %c NaN undef");
like($@, qr/Cannot printf/, "$NaN sprintf fails");
ok(!defined eval { $a = sprintf("%c", "NaN")},
  "sprintf %c stringy NaN undef");
like($@, qr/Cannot printf/, "stringy $NaN sprintf %c fails");

ok(!defined eval { $a = chr($NaN) }, "chr NaN undef");
like($@, qr/Cannot chr/, "NaN chr() fails");
ok(!defined eval { $a = chr("NaN") }, "chr stringy NaN undef");
like($@, qr/Cannot chr/, "stringy NaN chr() fails");

for my $f (@packi_fmt) {
    ok(!defined eval { $a = pack($f, $NaN) }, "pack $f NaN undef");
    like($@, $f eq 'w' ? qr/Cannot compress NaN/: qr/Cannot pack NaN/,
         "NaN pack $f fails");
    ok(!defined eval { $a = pack($f, "NaN") },
       "pack $f stringy NaN undef");
    like($@, $f eq 'w' ? qr/Cannot compress NaN/: qr/Cannot pack NaN/,
         "stringy NaN pack $f fails");
}

for my $f (@packf_fmt) {
    ok(defined eval { $a = pack($f, $NaN) }, "pack $f NaN defined");
    eval { $b = unpack($f, $a) };
    cmp_ok($b, '!=', $b, "pack $f NaN not-equals $NaN");
}

for my $f (@packs_fmt) {
    ok(defined eval { $a = pack($f, $NaN) }, "pack $f NaN defined");
    is($a, pack($f, "NaN"), "pack $f NaN same as 'NaN'");
}

is eval { unpack "p", pack 'p', $NaN }, "NaN", "pack p +NaN";
is eval { unpack "P3", pack 'P', $NaN }, "NaN", "pack P +NaN";

for my $i (@NaN) {
    cmp_ok($i + 0, '!=', $i + 0, "$i is NaN numerically (by not being NaN)");
    is("@{[$i+0]}", "NaN", "$i value stringifies as NaN");
}

ok(!($NaN <  0), "NaN is not lt zero");
ok(!($NaN == 0), "NaN is not == zero");
ok(!($NaN >  0), "NaN is not gt zero");

ok(!($NaN < $NaN), "NaN is not lt NaN");
ok(!($NaN > $NaN), "NaN is not gt NaN");

# is() okay with $NaN because it uses eq.
is($NaN * 0, $NaN, "NaN times zero is NaN");
is($NaN * 2, $NaN, "NaN times two is NaN");

my ($NaNPP, $NaNMM) = ($NaN, $NaN);
$NaNPP++;
$NaNMM--;
is($NaNPP, $NaN, "+NaN++ is NaN");
is($NaNMM, $NaN, "+NaN-- is NaN");

# You might find this surprising (isn't NaN kind of like of undef?)
# but this is how it is.
ok($NaN, "NaN is true");

is(abs($NaN), $NaN, "abs(NaN) is NaN");
is(int($NaN), $NaN, "int(NaN) is NaN");
is(sqrt($NaN), $NaN, "sqrt(NaN) is NaN");
is(exp($NaN), $NaN, "exp(NaN) is NaN");

SKIP: {
    if ($NaN == 0) {
        skip "if +NaN == 0 cannot log(+NaN)", 1;
    }
    is(log($NaN), $NaN, "log(NaN) is NaN");
}

is(sin($NaN), $NaN, "sin(NaN) is NaN");
is(rand($NaN), $NaN, "rand(NaN) is NaN");

TODO: {
    local $::TODO;
    my $here = "$^O $Config{osvers}";
    $::TODO = "$here: pow (9**9**9) doesn't give Inf"
        if $here =~ /^(?:hpux 10)/;
    is(sin(9**9**9), $NaN, "sin(9**9**9) is NaN");
}

SKIP: {
    my @FNaN = qw(NaX XNAN Ind Inx);
    # Silence "isn't numeric in addition", that's kind of the point.
    local $^W = 0;
    for my $i (@FNaN) {
        cmp_ok("$i" + 0, '==', 0, "false nan $i");
    }
}

{
    # Silence "Non-finite repeat count", that is tested elsewhere.
    local $^W = 0;
    is("a" x $NaN, "", "x NaN");
}

# === Tests combining Inf and NaN ===

# is() okay with $NaN because it uses eq.
is($PInf * 0,     $NaN, "Inf times zero is NaN");
is($PInf * $NaN,  $NaN, "Inf times NaN is NaN");
is($PInf + $NaN,  $NaN, "Inf plus NaN is NaN");
is($PInf - $PInf, $NaN, "Inf minus inf is NaN");
is($PInf / $PInf, $NaN, "Inf div inf is NaN");
is($PInf % $PInf, $NaN, "Inf mod inf is NaN");

ok(!($NaN <  $PInf), "NaN is not lt +Inf");
ok(!($NaN == $PInf), "NaN is not eq +Inf");
ok(!($NaN >  $PInf), "NaN is not gt +Inf");

ok(!($NaN <  $NInf), "NaN is not lt -Inf");
ok(!($NaN == $NInf), "NaN is not eq -Inf");
ok(!($NaN >  $NInf), "NaN is not gt -Inf");

is(sin($PInf), $NaN, "sin(+Inf) is NaN");

{
    eval 'for my $x (0..$NaN) { last }';
    like($@, qr/Range iterator outside integer range/, "0..NaN fails");

    eval 'for my $x ($NaN..0) { last }';
    like($@, qr/Range iterator outside integer range/, "NaN..0 fails");
}

# === Overflows and Underflows ===

# 1e9999 (and 1e-9999) are large (and small) enough for even
# IEEE quadruple precision (magnitude 10**4932, and 10**-4932).

cmp_ok(1e9999,     '==', $PInf, "overflow to +Inf (compile time)");
cmp_ok('1e9999',   '==', $PInf, "overflow to +Inf (runtime)");
cmp_ok(-1e9999,    '==', $NInf, "overflow to -Inf (compile time)");
cmp_ok('-1e9999',  '==', $NInf, "overflow to -Inf (runtime)");
cmp_ok(1e-9999,    '==', 0,     "underflow to 0 (compile time) from pos");
cmp_ok('1e-9999',  '==', 0,     "underflow to 0 (runtime) from pos");
cmp_ok(-1e-9999,   '==', 0,     "underflow to 0 (compile time) from neg");
cmp_ok('-1e-9999', '==', 0,     "underflow to 0 (runtime) from neg");

# === Warnings triggered when and only when appropriate ===
{
    my $w;
    local $SIG{__WARN__} = sub { $w = shift };
    local $^W = 1;

    my $T =
        [
         [ "inf",          0, $PInf ],
         [ "infinity",     0, $PInf ],
         [ "infxy",        1, $PInf ],
         [ "inf34",        1, $PInf ],
         [ "1.#INF",       0, $PInf ],
         [ "1.#INFx",      1, $PInf ],
         [ "1.#INF00",     0, $PInf ],
         [ "1.#INFxy",     1, $PInf ],
         [ " inf",         0, $PInf ],
         [ "inf ",         0, $PInf ],
         [ " inf ",        0, $PInf ],

         [ "nan",          0, $NaN ],
         [ "nanxy",        1, $NaN ],
         [ "nan34",        1, $NaN ],
         [ "nanq",         0, $NaN ],
         [ "nans",         0, $NaN ],
         [ "nanx",         1, $NaN ],
         [ "nanqy",        1, $NaN ],
         [ "nan(123)",     0, $NaN ],
         [ "nan(0x123)",   0, $NaN ],
         [ "nan(123xy)",   1, $NaN ],
         [ "nan(0x123xy)", 1, $NaN ],
         [ "nanq(123)",    0, $NaN ],
         [ "nan(123",      1, $NaN ],
         [ "nan(",         1, $NaN ],
         [ "1.#NANQ",      0, $NaN ],
         [ "1.#QNAN",      0, $NaN ],
         [ "1.#NANQx",     1, $NaN ],
         [ "1.#QNANx",     1, $NaN ],
         [ "1.#IND",       0, $NaN ],
         [ "1.#IND00",     0, $NaN ],
         [ "1.#INDxy",     1, $NaN ],
         [ " nan",         0, $NaN ],
         [ "nan ",         0, $NaN ],
         [ " nan ",        0, $NaN ],
        ];

    for my $t (@$T) {
        print "# '$t->[0]' compile time\n";
        my $a;
        $w = '';
        eval '$a = "'.$t->[0].'" + 1';
        is("$a", "$t->[2]", "$t->[0] plus one is $t->[2]");
        if ($t->[1]) {
            like($w, qr/^Argument \Q"$t->[0]"\E isn't numeric/,
                 "$t->[2] numify warn");
        } else {
            is($w, "", "no warning expected");
        }
        print "# '$t->[0]' runtime\n";
        my $n = $t->[0];
        my $b;
        $w = '';
        eval '$b = $n + 1';
        is("$b", "$t->[2]", "$n plus one is $t->[2]");
        if ($t->[1]) {
            like($w, qr/^Argument \Q"$n"\E isn't numeric/,
                 "$n numify warn");
        } else {
            is($w, "", "no warning expected");
        }
    }
}

# Size qualifiers shouldn't affect printing Inf/Nan
#
# Prior to the commit which introduced these tests and the fix,
# the code path taken when int-ish formats saw an Inf/Nan was to
# jump to the floating-point handler, but then that would
# warn about (valid) qualifiers.

{
    my @w;
    local $SIG{__WARN__} = sub { push @w, $_[0] };

    for my $format (qw(B b c D d i O o U u X x)) {
        # skip unportable: j L q
        for my $size (qw(hh h l ll t z)) {
            for my $num ($NInf, $PInf, $NaN) {
                @w = ();
                my $res = eval { sprintf "%${size}${format}", $num; };
                my $desc = "sprintf(\"%${size}${format}\", $num)";
                if ($format eq 'c') {
                    like($@, qr/Cannot printf $num with 'c'/, "$desc: like");
                }
                else {
                    is($res, $num, "$desc: equality");
                }

                is (@w, 0, "$desc: warnings")
                    or do {
                        diag("got warning: [$_]") for map { chomp; $_} @w;
                    };
            }
        }
    }
}

# "+Inf" should be converted to UV consistently
{
    my $uv_max = ~0;
    my $x = 42;     # Some arbitrary integer.
    $x = ' Inf ';   # Spaces will detect unwanted string/NV round-trip
    is($x << 0, $uv_max, "' Inf ' converted to UV");
    # Test twice just in case if SvUV is tricked by cached NV
    is($x << 0, $uv_max, "second conversion of ' Inf ' to UV");
    # Cached NV should be Inf even after conversion to UV returned UV_MAX
    cmp_ok($x, '==', $PInf, "NV value of ' Inf ' after UV conversion");
    # String value should not be changed
    is($x, ' Inf ', "string shall be ' Inf ' after UV/NV conversion");
}

# "-Inf" should be converted to IV consistently
{
    use integer;
    my $x = '-Inf';
    my $y = $NInf;      # $NInf and $y shall be NV -Inf
    my $z = $x | 0;     # "|" under "use integer;" requires IV operands
    is($z, $y | 0, "'-Inf' should be converted to IV consistently");
    # $z shall be IV_MIN here, but as the actual value of IV_MIN is not
    # (easily) available in Perl so check its negative-ness as the next best.
    cmp_ok($z, '<', 0, "'-Inf' should be converted to a negative IV");
}

done_testing();
