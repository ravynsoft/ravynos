#!perl -w
use strict;

use Test::More;
use Config;
use XS::APItest;
no warnings 'experimental::smartmatch';
use constant TRUTH => '0 but true';

# Tests for grok_number. Not yet comprehensive.
foreach my $leader ('', ' ', '  ') {
    foreach my $trailer ('', ' ', '  ') {
	foreach ((map {"0" x $_} 1 .. 12),
		 (map {("0" x $_) . "1"} 0 .. 12),
		 (map {"1" . ("0" x $_)} 1 .. 9),
		 (map {1 << $_} 0 .. 31),
		 (map {1 << $_} 0 .. 31),
		 (map {0xFFFFFFFF >> $_} reverse (0 .. 31)),
		) {
	    foreach my $sign ('', '-', '+') {
		my $string = $leader . $sign . $_ . $trailer;
		my ($flags, $value) = grok_number($string);
		is($flags & IS_NUMBER_IN_UV, IS_NUMBER_IN_UV,
		   "'$string' is a UV");
		is($flags & IS_NUMBER_NEG, $sign eq '-' ? IS_NUMBER_NEG : 0,
		   "'$string' sign");
		is($value, abs $string, "value is correct");
	    }
	}

	{
	    my (@UV, @NV);
	    if ($Config{ivsize} == 4) {
		@UV = qw(429496729  4294967290 4294967294 4294967295);
		@NV = qw(4294967296 4294967297 4294967300 4294967304);
	    } elsif ($Config{ivsize} == 8) {
		@UV = qw(1844674407370955161  18446744073709551610
			 18446744073709551614 18446744073709551615);
		@NV = qw(18446744073709551616 18446744073709551617
			 18446744073709551620 18446744073709551624);
	    } else { die "Unknown IV size $Config{ivsize}" }
	    foreach (@UV) {
		my $string = $leader . $_ . $trailer;
		my ($flags, $value) = grok_number($string);
		is($flags & IS_NUMBER_IN_UV, IS_NUMBER_IN_UV,
		   "'$string' is a UV");
		is($value, abs $string, "value is correct");
	    }
	    foreach (@NV) {
		my $string = $leader . $_ . $trailer;
		my ($flags, $value) = grok_number($string);
		is($flags & IS_NUMBER_IN_UV, 0, "'$string' is an NV");
		is($value, undef, "value is correct");
	    }
	}

	my $string = $leader . TRUTH . $trailer;
	my ($flags, $value) = grok_number($string);

	if ($string eq TRUTH) {
	    is($flags & IS_NUMBER_IN_UV, IS_NUMBER_IN_UV, "'$string' is a UV");
	    is($value, 0);
	} else {
	    is($flags, 0, "'$string' is not a number");
	    is($value, undef);
	}
    }
}

# format tests
my @groks = (
    # input, in flags, out uv, out flags
    (map {
        # Expect the same answer with or without SCAN_TRAILING
        [ $_->[0], 0,                  $_->[1], $_->[2] ],
        [ $_->[0], PERL_SCAN_TRAILING, $_->[1], $_->[2] ],
    } (
        [ "1",          1,     IS_NUMBER_IN_UV ],
        [ "3.1",        3,     IS_NUMBER_IN_UV | IS_NUMBER_NOT_INT ],
        [ "3e5",        undef, IS_NUMBER_NOT_INT ],
        [ "Inf",        undef, IS_NUMBER_INFINITY | IS_NUMBER_NOT_INT ],
        [ "nan",        undef, IS_NUMBER_NAN | IS_NUMBER_NOT_INT ],
        [ "nanq",       undef, IS_NUMBER_NAN | IS_NUMBER_NOT_INT ],
        [ "nan(123)",   undef, IS_NUMBER_NAN | IS_NUMBER_NOT_INT ],
        # trailing whitespace is ok though
        [ "1 ",         1,     IS_NUMBER_IN_UV ],
        [ "nan(123 ) ", undef, IS_NUMBER_NAN | IS_NUMBER_NOT_INT ],
    )),
    (map {
        # Trailing stuff should cause failure unless SCAN_TRAILING
        [ $_->[0], 0,                  undef,   0 ],
        [ $_->[0], PERL_SCAN_TRAILING, $_->[1], $_->[2] | IS_NUMBER_TRAILING ],
    } (
        [ "1x",         1,     IS_NUMBER_IN_UV ],
        [ "3.1a",       3,     IS_NUMBER_IN_UV | IS_NUMBER_NOT_INT ],
        [ "3e",         3,     IS_NUMBER_IN_UV ],
        [ "3e+",        3,     IS_NUMBER_IN_UV ],
        [ "Infin",      undef, IS_NUMBER_INFINITY | IS_NUMBER_NOT_INT ],
        [ "nanx",       undef, IS_NUMBER_NAN | IS_NUMBER_NOT_INT ],
        [ "nan(123 x)", undef, IS_NUMBER_NAN | IS_NUMBER_NOT_INT ],
        [ "nan(123) x", undef, IS_NUMBER_NAN | IS_NUMBER_NOT_INT ],
        # TODO: this should probably be in the preceding section, parsed
        # as invalid with or without SCAN_TRAILING. See GH #19464.
        [ "In",         undef, 0 ],
    )),
);

my $non_ieee_fp = ($Config{doublekind} == 9 ||
                   $Config{doublekind} == 10 ||
                   $Config{doublekind} == 11);

if ($non_ieee_fp) {
    @groks = grep { $_->[0] !~ /^(?:inf|nan)/i } @groks;
}

for my $grok (@groks) {
  my ($out_flags, $out_uv) = grok_number_flags($grok->[0], $grok->[1]);
  is($out_uv,    $grok->[2], "'$grok->[0]' flags $grok->[1] - check number");
  is($out_flags, $grok->[3], "'$grok->[0]' flags $grok->[1] - check flags");
}

my $ATOU_MAX = ~0;

# atou tests
my @atous =
  (
   # [ input, endsv, out uv, out len ]

   # Basic cases.
   [ "0",    "",   0,   1 ],
   [ "1",    "",   1,   1 ],
   [ "2",    "",   2,   1 ],
   [ "9",    "",   9,   1 ],
   [ "12",   "",   12,  2 ],
   [ "123",  "",   123, 3 ],

   # Trailing whitespace  is accepted or rejected, depending on endptr.
   [ "0 ",   " ",   0,  1 ],
   [ "1 ",   " ",   1,  1 ],
   [ "2 ",   " ",   2,  1 ],
   [ "12 ",  " ",   12, 2 ],

   # Trailing garbage is accepted or rejected, depending on endptr.
   [ "0x",   "x",   0,  1 ],
   [ "1x",   "x",   1,  1 ],
   [ "2x",   "x",   2,  1 ],
   [ "12x",  "x",   12, 2 ],

   # Leading whitespace is failure.
   [ " 0",   undef, 0,  0 ],
   [ " 1",   undef, 0,  0 ],
   [ " 12",  undef, 0,  0 ],

   # Leading garbage is outright failure.
   [ "x0",   undef,  0,  0 ],
   [ "x1",   undef,  0,  0 ],
   [ "x12",  undef, 0,  0 ],

   # We do not parse decimal point.
   [ "12.3", ".3", 12, 2 ],

   # Leading pluses or minuses are no good.
   [ "+12", undef,  0, 0 ],
   [ "-12", undef,  0, 0 ],

   # Extra leading zeros are no good.
   [ "00",   undef,  $ATOU_MAX,  0 ],
   [ "01",   undef,  $ATOU_MAX,  0 ],
   [ "012",  undef, $ATOU_MAX,  0 ],
  );

# Values near overflow point.
if ($Config{uvsize} == 8) {
    push @atous,
      (
       # 32-bit values no problem for 64-bit.
       [ "4294967293", "", 4294967293, 10, ],
       [ "4294967294", "", 4294967294, 10, ],
       [ "4294967295", "", 4294967295, 10, ],
       [ "4294967296", "", 4294967296, 10, ],
       [ "4294967297", "", 4294967297, 10, ],

       # This is well within 64-bit.
       [ "9999999999", "", 9999999999, 10, ],

       # Values valid up to 64-bit, failing beyond.
       [ "18446744073709551613", "", 18446744073709551613, 20, ],
       [ "18446744073709551614", "", 18446744073709551614, 20, ],
       [ "18446744073709551615", "", $ATOU_MAX, 20, ],
       [ "18446744073709551616", undef, $ATOU_MAX, 0, ],
       [ "18446744073709551617", undef, $ATOU_MAX, 0, ],
      );
} elsif ($Config{uvsize} == 4) {
    push @atous,
      (
       # Values valid up to 32-bit, failing beyond.
       [ "4294967293", "", 4294967293, 10, ],
       [ "4294967294", "", 4294967294, 10, ],
       [ "4294967295", "", $ATOU_MAX, 10, ],
       [ "4294967296", undef, $ATOU_MAX, 0, ],
       [ "4294967297", undef, $ATOU_MAX, 0, ],

       # Still beyond 32-bit.
       [ "4999999999", undef, $ATOU_MAX, 0, ],
       [ "5678901234", undef, $ATOU_MAX, 0, ],
       [ "6789012345", undef, $ATOU_MAX, 0, ],
       [ "7890123456", undef, $ATOU_MAX, 0, ],
       [ "8901234567", undef, $ATOU_MAX, 0, ],
       [ "9012345678", undef, $ATOU_MAX, 0, ],
       [ "9999999999", undef, $ATOU_MAX, 0, ],
       [ "10000000000", undef, $ATOU_MAX, 0, ],
       [ "12345678901", undef, $ATOU_MAX, 0, ],

       # 64-bit values are way beyond.
       [ "18446744073709551613", undef, $ATOU_MAX, 0, ],
       [ "18446744073709551614", undef, $ATOU_MAX, 0, ],
       [ "18446744073709551615", undef, $ATOU_MAX, 0, ],
       [ "18446744073709551616", undef, $ATOU_MAX, 0, ],
       [ "18446744073709551617", undef, $ATOU_MAX, 0, ],
      );
}

# These will fail to fail once 128/256-bit systems arrive.
push @atous,
    (
       [ "23456789012345678901", undef, $ATOU_MAX, 0 ],
       [ "34567890123456789012", undef, $ATOU_MAX, 0 ],
       [ "98765432109876543210", undef, $ATOU_MAX, 0 ],
       [ "98765432109876543211", undef, $ATOU_MAX, 0 ],
       [ "99999999999999999999", undef, $ATOU_MAX, 0 ],
    );

for my $grok (@atous) {
    my $input = $grok->[0];
    my $endsv = $grok->[1];
    my $expect_ok = defined $endsv;
    my $strict_ok = $expect_ok && $endsv eq '';

    my ($ok, $out_uv, $out_len);

    # First with endsv.
    ($ok, $out_uv, $out_len) = grok_atoUV($input, $endsv);
    is($expect_ok, $ok, sprintf "'$input' expected %s, got %s",
        ($expect_ok ? 'success' : 'failure'),
        ($ok ? 'success' : 'failure'),
    );
    if ($expect_ok) {
        is($expect_ok, $ok, "'$input' expect success");
        is($out_uv,  $grok->[2],
            "'$input' $endsv - number success (got $out_uv cf $grok->[2])");
        ok($grok->[3] <= length $input, "'$input' $endsv - length sanity 1");
        unless (length $grok->[1]) {
            is($out_len, $grok->[3], "'$input' $endsv - length sanity 2");
        } # else { ... } ?
        if ($out_len) {
            is($endsv, substr($input, $out_len),
                "'$input' $endsv - length sanity 3");
        }
    } else {
        is($expect_ok, $ok, "'$input' expect failure");
        is(0xdeadbeef, $out_uv, "'$input' on failure expect value unchanged");
    }

    # Then without endsv (undef == NULL).
    ($ok, $out_uv, $out_len) = grok_atoUV($input, undef);
    if ($strict_ok) {
        is($strict_ok, $ok, "'$input' expect strict success");
        is($out_uv,  $grok->[2],
            "'$input' $endsv - strict number success (got $out_uv cf $grok->[2])");
    } else {
        is($strict_ok, $ok, "'$input' expect strict failure");
        is(0xdeadbeef, $out_uv, "'$input' on strict failure expect value unchanged");
    }
}

done_testing();
