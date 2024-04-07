#!perl -w

use strict;

use POSIX ':math_h_c99';
use POSIX ':nan_payload';
use Test::More;

use Config;

# These tests are mainly to make sure that these arithmetic functions
# exist and are accessible.  They are not meant to be an exhaustive
# test for the interface.

sub between {
    my ($low, $have, $high, $desc) = @_;
    local $Test::Builder::Level = $Test::Builder::Level + 1;

    cmp_ok($have, '>=', $low, $desc);
    cmp_ok($have, '<=', $high, $desc);
}

is(acos(1), 0, "Basic acos(1) test");
between(3.14, acos(-1), 3.15, 'acos(-1)');
between(1.57, acos(0), 1.58, 'acos(0)');
is(asin(0), 0, "Basic asin(0) test");
cmp_ok(asin(1), '>', 1.57, "Basic asin(1) test");
cmp_ok(asin(-1), '<', -1.57, "Basic asin(-1) test");
cmp_ok(asin(1), '==', -asin(-1), 'asin(1) == -asin(-1)');
is(atan(0), 0, "Basic atan(0) test");
between(0.785, atan(1), 0.786, 'atan(1)');
between(-0.786, atan(-1), -0.785, 'atan(-1)');
cmp_ok(atan(1), '==', -atan(-1), 'atan(1) == -atan(-1)');
is(cosh(0), 1, "Basic cosh(0) test");  
between(1.54, cosh(1), 1.55, 'cosh(1)');
between(1.54, cosh(-1), 1.55, 'cosh(-1)');
is(cosh(1), cosh(-1), 'cosh(1) == cosh(-1)');
is(floor(1.23441242), 1, "Basic floor(1.23441242) test");
is(floor(-1.23441242), -2, "Basic floor(-1.23441242) test");
is(fmod(3.5, 2.0), 1.5, "Basic fmod(3.5, 2.0) test");
is(join(" ", frexp(1)), "0.5 1",  "Basic frexp(1) test");
is(ldexp(0,1), 0, "Basic ldexp(0,1) test");
is(log10(1), 0, "Basic log10(1) test"); 
is(log10(10), 1, "Basic log10(10) test");
is(join(" ", modf(1.76)), "0.76 1", "Basic modf(1.76) test");
is(sinh(0), 0, "Basic sinh(0) test"); 
between(1.17, sinh(1), 1.18, 'sinh(1)');
between(-1.18, sinh(-1), -1.17, 'sinh(-1)');
is(tan(0), 0, "Basic tan(0) test");
between(1.55, tan(1), 1.56, 'tan(1)');
between(1.55, tan(1), 1.56, 'tan(-1)');
cmp_ok(tan(1), '==', -tan(-1), 'tan(1) == -tan(-1)');
is(tanh(0), 0, "Basic tanh(0) test"); 
between(0.76, tanh(1), 0.77, 'tanh(1)');
between(-0.77, tanh(-1), -0.76, 'tanh(-1)');
cmp_ok(tanh(1), '==', -tanh(-1), 'tanh(1) == -tanh(-1)');

SKIP: {
    skip "no fpclassify", 4 unless $Config{d_fpclassify};
    is(fpclassify(1), FP_NORMAL, "fpclassify 1");
    is(fpclassify(0), FP_ZERO, "fpclassify 0");
    SKIP: {
        skip("no inf", 1) unless $Config{d_double_has_inf};
        is(fpclassify(INFINITY), FP_INFINITE, "fpclassify INFINITY");
    }
    SKIP: {
        skip("no nan", 1) unless $Config{d_double_has_nan};
        is(fpclassify(NAN), FP_NAN, "fpclassify NAN");
    }
}

sub near {
    my ($got, $want, $msg, $eps) = @_;
    $eps ||= 1e-6;
    cmp_ok(abs($got - $want), '<', $eps, $msg);
}

SKIP: {

    unless ($Config{d_acosh}) {
        skip "no acosh, suspecting no C99 math";
    }
    if ($^O =~ /VMS/) {
        skip "running in $^O, C99 math support uneven";
    }
    if ($Config{cc} =~ /\b(?:cl|icl)/) {
        skip "Microsoft compiler - C99 math support uneven";
    }

    near(M_SQRT2, 1.4142135623731, "M_SQRT2", 1e-9);
    near(M_E, 2.71828182845905, "M_E", 1e-9);
    near(M_PI, 3.14159265358979, "M_PI", 1e-9);
    near(acosh(2), 1.31695789692482, "acosh", 1e-9);
    near(asinh(1), 0.881373587019543, "asinh", 1e-9);
    near(atanh(0.5), 0.549306144334055, "atanh", 1e-9);
    near(cbrt(8), 2, "cbrt", 1e-9);
    near(cbrt(-27), -3, "cbrt", 1e-9);
    near(copysign(3.14, -2), -3.14, "copysign", 1e-9);
    near(expm1(2), 6.38905609893065, "expm1", 1e-9);
    near(expm1(1e-6), 1.00000050000017e-06, "expm1", 1e-9);
    is(fdim(12, 34), 0, "fdim 12 34");
    is(fdim(34, 12), 22, "fdim 34 12");
    is(fmax(12, 34), 34, "fmax 12 34");
    is(fmin(12, 34), 12, "fmin 12 34");
    is(hypot(3, 4), 5, "hypot 3 4");
    near(hypot(-2, 1), sqrt(5), "hypot -1 2", 1e-9);
    is(ilogb(255), 7, "ilogb 255");
    is(ilogb(256), 8, "ilogb 256");
    ok(isfinite(1), "isfinite 1");
    ok(!isinf(42), "isinf 42");
    ok(!isnan(42), "isnan Inf");
  SKIP: {
      skip("no inf", 3) unless $Config{d_double_has_inf};
      ok(!isfinite(Inf), "isfinite Inf");
      ok(isinf(Inf), "isinf Inf");
      ok(!isnan(Inf), "isnan Inf");
    }
  SKIP: {
      skip("no nan", 4) unless $Config{d_double_has_nan};
      ok(!isfinite(NaN), "isfinite NaN");
      ok(!isinf(NaN), "isinf NaN");
      ok(isnan(NaN), "isnan NaN");
      cmp_ok(nan(), '!=', nan(), 'nan');
    }
    near(log1p(2), 1.09861228866811, "log1p", 1e-9);
    near(log1p(1e-6), 9.99999500000333e-07, "log1p", 1e-9);
    near(log2(8), 3, "log2", 1e-9);
    is(signbit(2), 0, "signbit 2"); # zero
    ok(signbit(-2), "signbit -2"); # non-zero
    is(signbit(0), 0, "signbit 0"); # zero
    is(signbit(0.5), 0, "signbit 0.5"); # zero
    ok(signbit(-0.5), "signbit -0.5"); # non-zero
    is(round(2.25), 2, "round 2.25");
    is(round(-2.25), -2, "round -2.25");
    is(round(2.5), 3, "round 2.5");
    is(round(-2.5), -3, "round -2.5");
    is(round(2.75), 3, "round 2.75");
    is(round(-2.75), -3, "round 2.75");
    is(lround(-2.75), -3, "lround -2.75");
    is(lround(-0.25), 0, "lround -0.25");
    is(lround(-0.50), -1, "lround -0.50");
    is(signbit(lround(-0.25)), 0, "signbit lround -0.25 zero");
    ok(signbit(lround(-0.50)), "signbit lround -0.50 non-zero"); # non-zero
    is(trunc(2.25), 2, "trunc 2.25");
    is(trunc(-2.25), -2, "trunc -2.25");
    is(trunc(2.5), 2, "trunc 2.5");
    is(trunc(-2.5), -2, "trunc -2.5");
    is(trunc(2.75), 2, "trunc 2.75");
    is(trunc(-2.75), -2, "trunc -2.75");
    ok(isless(1, 2), "isless 1 2");
    ok(!isless(2, 1), "isless 2 1");
    ok(!isless(1, 1), "isless 1 1");
    ok(isgreater(2, 1), "isgreater 2 1");
    ok(islessequal(1, 1), "islessequal 1 1");

  SKIP: {
      skip("no nan", 2) unless $Config{d_double_has_nan};
      ok(!isless(1, NaN), "isless 1 NaN");
      ok(isunordered(1, NaN), "isunordered 1 NaN");
    }

    near(erf(0.5), 0.520499877813047, "erf 0.5", 1.5e-7);
    near(erf(1), 0.842700792949715, "erf 1", 1.5e-7);
    near(erf(9), 1, "erf 9", 1.5e-7);
    near(erfc(0.5), 0.479500122186953, "erfc 0.5", 1.5e-7);
    near(erfc(1), 0.157299207050285, "erfc 1", 1.5e-7);
    near(erfc(9), 0, "erfc 9", 1.5e-7);

    # tgamma(n) = (n - 1)!
    # lgamma(n) = log(tgamma(n))
    near(tgamma(5), 24, "tgamma 5", 1.5e-7);
    near(tgamma(5.5), 52.3427777845535, "tgamma 5.5", 1.5e-7);
    near(tgamma(9), 40320, "tgamma 9", 1.5e-7);
    near(lgamma(5), 3.17805383034795, "lgamma 4", 1.5e-7);
    near(lgamma(5.5), 3.95781396761872, "lgamma 5.5", 1.5e-7);
    near(lgamma(9), 10.6046029027452, "lgamma 9", 1.5e-7);

  SKIP: {
      skip("no inf/nan", 19) unless $Config{d_double_has_inf} && $Config{d_double_has_nan};

      # These don't work on old mips/hppa platforms
      # because nan with payload zero == Inf (or == -Inf).
      # ok(isnan(setpayload(0)), "setpayload zero");
      # is(getpayload(setpayload(0)), 0, "setpayload + getpayload (zero)");
      #
      # These don't work on most platforms because == Inf (or == -Inf).
      # ok(isnan(setpayloadsig(0)), "setpayload zero");
      # is(getpayload(setpayloadsig(0)), 0, "setpayload + getpayload (zero)");

      # Verify that the payload set be setpayload()
      # (1) still is a nan
      # (2) but the payload can be retrieved
      # (3) but is not signaling
      my $x = 0;
      setpayload($x, 0x12345);
      ok(isnan($x), "setpayload + isnan");
      is(getpayload($x), 0x12345, "setpayload + getpayload");
      ok(!issignaling($x), "setpayload + issignaling");

      # Verify that the signaling payload set be setpayloadsig()
      # (1) still is a nan
      # (2) but the payload can be retrieved
      # (3) and is signaling
      setpayloadsig($x, 0x12345);
      ok(isnan($x), "setpayloadsig + isnan");
      is(getpayload($x), 0x12345, "setpayloadsig + getpayload");
    SKIP: {
        # https://rt.perl.org/Ticket/Display.html?id=125710
        # In the 32-bit x86 ABI cannot preserve the signaling bit
        # (the x87 simply does not preserve that).  But using the
        # 80-bit extended format aka long double, the bit is preserved.
        # https://gcc.gnu.org/bugzilla/show_bug.cgi?id=57484
        my $could_be_x86_32 =
            # This is a really weak test: there are other 32-bit
            # little-endian platforms than just Intel (some embedded
            # processors, for example), but we use this just for not
            # bothering with the test if things look iffy.
            # We could, say, $Config{ccsymbols} =~ /\b__[xi][3-7]86=1\b/,
            # but that feels quite shaky.
            $Config{byteorder} =~ /1234/ &&
            $Config{longdblkind} == 3 &&
            $Config{ptrsize} == 4;
        skip($^O, 1) if $could_be_x86_32 && !$Config{uselongdouble};
        ok(issignaling($x), "setpayloadsig + issignaling");
      }

      # Try a payload more than one byte.
      is(getpayload(nan(0x12345)), 0x12345, "nan + getpayload");

      # Try payloads of 2^k, most importantly at and beyond 2^32.  These
      # tests will fail if NV is just 32-bit float, but that Should Not
      # Happen (tm).
      is(getpayload(nan(2**31)), 2**31, "nan + getpayload 2**31");
      is(getpayload(nan(2**32)), 2**32, "nan + getpayload 2**32");
      is(getpayload(nan(2**33)), 2**33, "nan + getpayload 2**33");

      # Payloads just lower than 2^k.
      is(getpayload(nan(2**31-1)), 2**31-1, "nan + getpayload 2**31-1");
      is(getpayload(nan(2**32-1)), 2**32-1, "nan + getpayload 2**32-1");

      # Payloads not divisible by two (and larger than 2**32).

    SKIP: {
        # solaris gets 10460353202 from getpayload() when it should
        # get 10460353203 (the 3**21). Things go wrong already in
        # the nan() payload setting: [0x2, 0x6f7c52b4] (ivsize=4)
        # instead [0x2, 0x6f7c52b3].  Then at getpayload() things
        # go wrong again, now in other direction: with the (wrong)
        # [0x2, 0x6f7c52b4] encoded in the nan we should decode into
        # 10460353204, but we get 10460353202.  It doesn't seem to
        # help even if we use 'unsigned long long' instead of UV/U32
        # in the POSIX.xs:S_setpayload/S_getpayload.
        #
        # casting bug?  fmod() bug?  Though also broken with
        # -Duselongdouble + fmodl(), so maybe Solaris cc bug
        # in general?
        #
        # Ironically, the large prime seems to work even in Solaris,
        # probably just by blind luck.
        skip($^O, 1) if $^O eq 'solaris';
        is(getpayload(nan(3**21)), 3**21, "nan + getpayload 3**21");
      }
      is(getpayload(nan(4294967311)), 4294967311, "nan + getpayload prime");

      # Truncates towards zero.
      is(getpayload(nan(1234.567)), 1234, "nan (trunc) + getpayload");

      # Not signaling.
      ok(!issignaling(0), "issignaling zero");
      ok(!issignaling(+Inf), "issignaling +Inf");
      ok(!issignaling(-Inf), "issignaling -Inf");
      ok(!issignaling(NaN), "issignaling NaN");
    }
} # SKIP

SKIP: {
    skip('no INFINITY', 4) unless defined &INFINITY;
    # Note that if INFINITY were a bareword, it would be numified to +Inf,
    # which might confuse following tests.
    # But this cannot happen as long as "use strict" is effective.
    ok(isinf(INFINITY), "isinf INFINITY");
    is(INFINITY, 'Inf', "INFINITY is Perl's Inf");
    cmp_ok(INFINITY, '>', ($Config{uselongdouble} ? POSIX::LDBL_MAX : POSIX::DBL_MAX),
           "INFINITY > DBL_MAX");
    ok(!signbit(INFINITY), "signbit(INFINITY)");
}

SKIP: {
    skip('no NAN', 5) unless defined &NAN;
    ok(isnan(NAN()), "isnan NAN");
    # Using like() rather than is() is to deal with non-zero payload
    # (currently this is not the case, but someday Perl might stringify it...)
    like(NAN, qr/^NaN/, "NAN is Perl's NaN");
    cmp_ok(NAN, '!=', NAN, "NAN != NAN");
    ok(!(NAN == NAN), "NAN == NAN");
    ok(!signbit(NAN), "signbit(NAN)");
}

done_testing();
