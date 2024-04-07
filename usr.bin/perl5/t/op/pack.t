#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl'; require './charset_tools.pl';
    set_up_inc(qw '../lib ../cpan/Math-BigInt/lib');
}

plan tests => 14722;

use strict;
use warnings qw(FATAL all);
use Config;

my $Perl = which_perl();
my @valid_errors = (qr/^Invalid type '\w'/);

my $ByteOrder = 'unknown';
my $maybe_not_avail = '(?:hto[bl]e|[bl]etoh)';
if ($Config{byteorder} =~ /^1234(?:5678)?$/) {
  $ByteOrder = 'little';
  $maybe_not_avail = '(?:htobe|betoh)';
}
elsif ($Config{byteorder} =~ /^(?:8765)?4321$/) {
  $ByteOrder = 'big';
  $maybe_not_avail = '(?:htole|letoh)';
}
else {
  push @valid_errors, qr/^Can't (?:un)?pack (?:big|little)-endian .*? on this platform/;
}

for my $size ( 16, 32, 64 ) {
  if (defined $Config{"u${size}size"} and ($Config{"u${size}size"}||0) != ($size >> 3)) {
    push @valid_errors, qr/^Perl_my_$maybe_not_avail$size\(\) not available/;
  }
}

my $IsTwosComplement = pack('i', -1) eq "\xFF" x $Config{intsize};
print "# \$IsTwosComplement = $IsTwosComplement\n";

sub is_valid_error
{
  my $err = shift;

  for my $e (@valid_errors) {
    $err =~ $e and return 1;
  }

  return 0;
}

sub encode_list {
  my @result = map {_qq($_)} @_;
  if (@result == 1) {
    return @result;
  }
  return '(' . join (', ', @result) . ')';
}


sub list_eq ($$) {
  my ($l, $r) = @_;
  return 0 unless @$l == @$r;
  for my $i (0..$#$l) {
    if (defined $l->[$i]) {
      return 0 unless defined ($r->[$i]) && $l->[$i] eq $r->[$i];
    } else {
      return 0 if defined $r->[$i]
    }
  }
  return 1;
}

##############################################################################
#
# Here starteth the tests
#

{
    my $format = "c2 x5 C C x s d i l a6";
    # Need the expression in here to force ary[5] to be numeric.  This avoids
    # test2 failing because ary2 goes str->numeric->str and ary doesn't.
    my @ary = (1,-100,127,128,32767,987.654321098 / 100.0,12345,123456,
               "abcdef");
    my $foo = pack($format,@ary);
    my @ary2 = unpack($format,$foo);

    is($#ary, $#ary2);

    my $out1=join(':',@ary);
    my $out2=join(':',@ary2);
    # Using long double NVs may introduce greater accuracy than wanted.
    $out1 =~ s/:9\.87654321097999\d*:/:9.87654321098:/;
    $out2 =~ s/:9\.87654321097999\d*:/:9.87654321098:/;
    is($out1, $out2);

    like($foo, qr/def/);
}
# How about counting bits?

{
    my $x;
    is( ($x = unpack("%32B*", "\001\002\004\010\020\040\100\200\377")), 16 );

    is( ($x = unpack("%32b69", "\001\002\004\010\020\040\100\200\017")), 12 );

    is( ($x = unpack("%32B69", "\001\002\004\010\020\040\100\200\017")), 9 );
}

{
    my $sum = 129; # ASCII
    $sum = 103 if $::IS_EBCDIC;

    my $x;
    is( ($x = unpack("%32B*", "Now is the time for all good blurfl")), $sum );

    my $foo;
    open(BIN, $Perl) || die "Can't open $Perl: $!\n";
    binmode BIN;
    sysread BIN, $foo, 8192;
    close BIN;

    $sum = unpack("%32b*", $foo);
    my $longway = unpack("b*", $foo);
    is( $sum, $longway =~ tr/1/1/ );
}

{
  my $x;
  is( ($x = unpack("I",pack("I", 0xFFFFFFFF))), 0xFFFFFFFF );
}

{
    note("check 'w'");
    my @x = (5,130,256,560,32000,3097152,268435455,1073741844, 2**33,
             '4503599627365785','23728385234614992549757750638446');
    my $x = pack('w*', @x);
    my $y = pack 'H*', '0581028200843081fa0081bd8440ffffff7f8480808014A0808'.
                       '0800087ffffffffffdb19caefe8e1eeeea0c2e1e3e8ede1ee6e';

    is($x, $y);

    my @y = unpack('w*', $y);
    my $a;
    while ($a = pop @x) {
        my $b = pop @y;
        is($a, $b);
    }

    @y = unpack('w2', $x);

    is(scalar(@y), 2);
    is($y[1], 130);

    SKIP: {
        skip "no Scalar::Util (Math::BigInt prerequisite) on miniperl", 10, if is_miniperl();
        $x = pack('w*', 5000000000); $y = '';
        eval q{
            use Math::BigInt;
            $y = pack('w*', Math::BigInt::->new(5000000000));
        };
        is($x, $y, 'pack');

        $x = pack 'w', ~0;
        $y = pack 'w', (~0).'';
        is($x, $y);
        is(unpack ('w',$x), ~0);
        is(unpack ('w',$y), ~0);

        $x = pack 'w', ~0 - 1;
        $y = pack 'w', (~0) - 2;

        if (~0 - 1 == (~0) - 2) {
            is($x, $y, "NV arithmetic");
        } else {
            isnt($x, $y, "IV/NV arithmetic");
        }
        cmp_ok(unpack ('w',$x), '==', ~0 - 1);
        cmp_ok(unpack ('w',$y), '==', ~0 - 2);

        # These should spot that pack 'w' is using NV, not double, on platforms
        # where IVs are smaller than doubles, and harmlessly pass elsewhere.
        # (tests for change 16861)
        my $x0 = 2**54+3;
        my $y0 = 2**54-2;

        $x = pack 'w', $x0;
        $y = pack 'w', $y0;

        if ($x0 == $y0) {
            is($x, $y, "NV arithmetic");
        } else {
            isnt($x, $y, "IV/NV arithmetic");
        }
        cmp_ok(unpack ('w',$x), '==', $x0);
        cmp_ok(unpack ('w',$y), '==', $y0);
    }
}


{
  print "# test exceptions\n";
  my $x;
  eval { $x = unpack 'w', pack 'C*', 0xff, 0xff};
  like($@, qr/^Unterminated compressed integer/);

  eval { $x = unpack 'w', pack 'C*', 0xff, 0xff, 0xff, 0xff};
  like($@, qr/^Unterminated compressed integer/);

  eval { $x = unpack 'w', pack 'C*', 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  like($@, qr/^Unterminated compressed integer/);

  eval { $x = pack 'w', -1 };
  like ($@, qr/^Cannot compress negative numbers/);

  eval { $x = pack 'w', '1'x(1 + length ~0) . 'e0' };
  like ($@, qr/^Can only compress unsigned integers/);

  # Check that the warning behaviour on the modifiers !, < and > is as we
  # expect it for this perl.
  my $can_endian = 'sSiIlLqQjJfFdDpP';
  my $can_shriek = 'sSiIlLnNvV';
  # h and H can't do either, so act as sanity checks in blead
  foreach my $base (split '', 'hHsSiIlLqQjJfFdDpPnNvV') {
    foreach my $mod ('', '<', '>', '!', '<!', '>!', '!<', '!>') {
    SKIP: {
	# Avoid void context warnings.
	my $a = eval {pack "$base$mod"};
	skip "pack can't $base", 1 if $@ =~ /^Invalid type '\w'/;
	# Which error you get when 2 would be possible seems to be emergent
	# behaviour of pack's format parser.

	my $fails_shriek = $mod =~ /!/ && index ($can_shriek, $base) == -1;
	my $fails_endian = $mod =~ /[<>]/ && index ($can_endian, $base) == -1;
	my $shriek_first = $mod =~ /^!/;

	if ($fails_shriek and $fails_endian) {
	  if ($shriek_first) {
	    undef $fails_endian;
	  }
	}

	if ($fails_endian) {
          like ($@, qr/^'[<>]' allowed only after types/,
                "pack can't $base$mod");
	} elsif ($fails_shriek) {
	  like ($@, qr/^'!' allowed only after types/,
		"pack can't $base$mod");
	} else {
	  is ($@, '', "pack can $base$mod");
	}
      }
    }
  }

  for my $mod (qw( ! < > )) {
    eval { $x = pack "a$mod", 42 };
    like ($@, qr/^'$mod' allowed only after types \S+ in pack/);

    eval { $x = unpack "a$mod", 'x'x8 };
    like ($@, qr/^'$mod' allowed only after types \S+ in unpack/);
  }

  for my $mod (qw( <> >< !<> !>< <!> >!< <>! ><! )) {
    eval { $x = pack "sI${mod}s", 42, 47, 11 };
    like ($@, qr/^Can't use both '<' and '>' after type 'I' in pack/);

    eval { $x = unpack "sI${mod}s", 'x'x16 };
    like ($@, qr/^Can't use both '<' and '>' after type 'I' in unpack/);
  }

 SKIP: {
    # Is this a stupid thing to do on VMS, VOS and other unusual platforms?

    skip("-- the IEEE infinity model is unavailable in this configuration.", 1)
       if (($^O eq 'VMS') && !defined($Config{useieee}) || !$Config{d_double_has_inf});

    skip("-- $^O has serious fp indigestion on w-packed infinities", 1)
       if $^O =~ /^svr4/ && -f "/etc/issue" && -f "/etc/.relid";  # NCR MP-RAS

    my $inf = eval '2**1000000';

    skip("Couldn't generate infinity - got error '$@'", 1)
      unless defined $inf and $inf == $inf / 2 and $inf + 1 == $inf;

    local our $TODO;
    $TODO = "VOS needs a fix for posix-1022 to pass this test."
      if ($^O eq 'vos');

    eval { $x = pack 'w', $inf };
    like ($@, qr/^Cannot compress Inf/, "Cannot compress infinity");
  }

 SKIP: {

    skip("-- the full range of an IEEE double may not be available in this configuration.", 3)
       if (($^O eq 'VMS') && !defined($Config{useieee}) || !$Config{d_double_style_ieee});

    # This should be about the biggest thing possible on an IEEE double
    my $big = eval '2**1023';

    skip("Couldn't generate 2**1023 - got error '$@'", 3)
      unless defined $big and $big != $big / 2;

    eval { $x = pack 'w', $big };
    is ($@, '', "Should be able to pack 'w', $big # 2**1023");

    my $y = eval {unpack 'w', $x};
    is ($@, '',
	"Should be able to unpack 'w' the result of pack 'w', $big # 2**1023");

    # I'm getting about 1e-16 on FreeBSD
    my $quotient = int (100 * ($y - $big) / $big);
    ok($quotient < 2 && $quotient > -2,
       "Round trip pack, unpack 'w' of $big is within 1% ($quotient%)");
  }

}

print "# test the 'p' template\n";

# literals
is(unpack("p",pack("p","foo")), "foo");
SKIP: {
  is(unpack("p<",pack("p<","foo")), "foo");
  is(unpack("p>",pack("p>","foo")), "foo");
}
# scalars
is(unpack("p",pack("p",239)), 239);
is(unpack("p<",pack("p<",239)), 239);
is(unpack("p>",pack("p>",239)), 239);

# temps
sub foo { my $a = "a"; return $a . $a++ . $a++ }
{
  use warnings qw(NONFATAL all);;
  my $warning;
  local $SIG{__WARN__} = sub {
      $warning = $_[0];
  };
  my $junk = pack("p", &foo);

  like($warning, qr/temporary val/);
}

# undef should give null pointer
like(pack("p", undef), qr/^\0+$/);
like(pack("p<", undef), qr/^\0+$/);
like(pack("p>", undef), qr/^\0+$/);

# Check for optimizer bug (e.g.  Digital Unix GEM cc with -O4 on DU V4.0B gives
#                                4294967295 instead of -1)
#				 see #ifdef __osf__ in pp.c pp_unpack
is((unpack("i",pack("i",-1))), -1);

print "# test the pack lengths of s S i I l L n N v V + modifiers\n";

my @lengths = (
  qw(s 2 S 2 i -4 I -4 l 4 L 4 n 2 N 4 v 2 V 4 n! 2 N! 4 v! 2 V! 4),
  's!'  => $Config{shortsize}, 'S!'  => $Config{shortsize},
  'i!'  => $Config{intsize},   'I!'  => $Config{intsize},
  'l!'  => $Config{longsize},  'L!'  => $Config{longsize},
);

while (my ($base, $expect) = splice @lengths, 0, 2) {
  my @formats = ($base);
  $base =~ /^[nv]/i or push @formats, "$base>", "$base<";
  for my $format (@formats) {
    my $len = length(pack($format, 0));
    if ($expect > 0) {
      is($expect, $len, "format '$format'");
    } else {
      $expect = -$expect;
      ok ($len >= $expect, "format '$format'") ||
	  print "# format '$format' has length $len, expected >= $expect\n";
    }
  }
}


print "# test unpack-pack lengths\n";

my @templates = qw(c C W i I s S l L n N v V f d q Q);

foreach my $base (@templates) {
    my @tmpl = ($base);
    $base =~ /^[cwnv]/i or push @tmpl, "$base>", "$base<";
    foreach my $t (@tmpl) {
        SKIP: {
            my @t = eval { unpack("$t*", pack("$t*", 12, 34)) };

            skip "cannot pack '$t' on this perl", 4
              if is_valid_error($@);

            is( $@, '', "Template $t works");
            is(scalar @t, 2);

            is($t[0], 12);
            is($t[1], 34);
        }
    }
}

{
    # uuencode/decode

    # Note that first uuencoding known 'text' data and then checking the
    # binary values of the uuencoded version would not be portable between
    # character sets.  Uuencoding is meant for encoding binary data, not
    # text data.

    my $in = pack 'C*', 0 .. 255;

    # just to be anal, we do some random tr/`/ /
    my $uu = <<'EOUU';
M` $"`P0%!@<("0H+# T.#Q`1$A,4%187&!D:&QP='A\@(2(C)"4F)R@I*BLL
M+2XO,#$R,S0U-C<X.3H[/#T^/T!!0D-$149'2$E*2TQ-3D]045)35%565UA9
M6EM<75Y?8&%B8V1E9F=H:6IK;&UN;W!Q<G-T=79W>'EZ>WQ]?G^`@8*#A(6&
MAXB)BHN,C8Z/D)&2DY25EI>8F9J;G)V>GZ"AHJ.DI::GJ*FJJZRMKJ^PL;*S
MM+6VM[BYNKN\O;Z_P,'"P\3%QL?(R<K+S,W.S]#1TM/4U=;7V-G:V]S=WM_@
?X>+CY.7FY^CIZNOL[>[O\/'R\_3U]O?X^?K[_/W^_P `
EOUU

    $_ = $uu;
    tr/ /`/;

    is(pack('u', $in), $_);

    is(unpack('u', $uu), $in);

    $in = "\x1f\x8b\x08\x08\x58\xdc\xc4\x35\x02\x03\x4a\x41\x50\x55\x00\xf3\x2a\x2d\x2e\x51\x48\xcc\xcb\x2f\xc9\x48\x2d\x52\x08\x48\x2d\xca\x51\x28\x2d\x4d\xce\x4f\x49\x2d\xe2\x02\x00\x64\x66\x60\x5c\x1a\x00\x00\x00";
    $uu = <<'EOUU';
M'XL("%C<Q#4"`TI!4%4`\RHM+E%(S,LOR4@M4@A(+<I1*"U-SD])+>("`&1F
&8%P:````
EOUU

    is(unpack('u', $uu), $in);

# This is identical to the above except that backquotes have been
# changed to spaces

    $uu = <<'EOUU';
M'XL("%C<Q#4" TI!4%4 \RHM+E%(S,LOR4@M4@A(+<I1*"U-SD])+>(" &1F
&8%P:
EOUU

    # ' # Grr
    is(unpack('u', $uu), $in);

}

# test the ascii template types (A, a, Z)

foreach (
['p', 'A*',  "foo\0bar\0 ", "foo\0bar\0 "],
['p', 'A11', "foo\0bar\0 ", "foo\0bar\0   "],
['u', 'A*',  "foo\0bar \0", "foo\0bar"],
['u', 'A8',  "foo\0bar \0", "foo\0bar"],
['p', 'a*',  "foo\0bar\0 ", "foo\0bar\0 "],
['p', 'a11', "foo\0bar\0 ", "foo\0bar\0 \0\0"],
['u', 'a*',  "foo\0bar \0", "foo\0bar \0"],
['u', 'a8',  "foo\0bar \0", "foo\0bar "],
['p', 'Z*',  "foo\0bar\0 ", "foo\0bar\0 \0"],
['p', 'Z11', "foo\0bar\0 ", "foo\0bar\0 \0\0"],
['p', 'Z3',  "foo",         "fo\0"],
['u', 'Z*',  "foo\0bar \0", "foo"],
['u', 'Z8',  "foo\0bar \0", "foo"],
)
{
    my ($what, $template, $in, $out) = @$_;
    my $got = $what eq 'u' ? (unpack $template, $in) : (pack $template, $in);
    unless (is($got, $out)) {
        my $un = $what eq 'u' ? 'un' : '';
        print "# ${un}pack ('$template', "._qq($in).') gave '._qq($out).
            ' not '._qq($got)."\n";
    }
}

print "# packing native shorts/ints/longs\n";

is(length(pack("s!", 0)), $Config{shortsize});
is(length(pack("i!", 0)), $Config{intsize});
is(length(pack("l!", 0)), $Config{longsize});
ok(length(pack("s!", 0)) <= length(pack("i!", 0)));
ok(length(pack("i!", 0)) <= length(pack("l!", 0)));
is(length(pack("i!", 0)), length(pack("i", 0)));

sub numbers {
  my $base = shift;
  my @formats = ($base);
  $base =~ /^[silqjfdp]/i and push @formats, "$base>", "$base<";
  for my $format (@formats) {
    numbers_with_total ($format, undef, @_);
  }
}

sub numbers_with_total {
  my $format = shift;
  my $total = shift;
  if (!defined $total) {
    foreach (@_) {
      $total += $_;
    }
  }
  print "# numbers test for $format\n";
  foreach (@_) {
    SKIP: {
        my $out = eval {unpack($format, pack($format, $_))};
        skip "cannot pack '$format' on this perl", 2
          if is_valid_error($@);

        is($@, '', "no error");
        is($out, $_, "unpack pack $format $_");
    }
  }

  my $skip_if_longer_than = ~0; # "Infinity"
  if (~0 - 1 == ~0) {
    # If we're running with -DNO_PERLPRESERVE_IVUV and NVs don't preserve all
    # UVs (in which case ~0 is NV, ~0-1 will be the same NV) then we can't
    # correctly in perl calculate UV totals for long checksums, as pp_unpack
    # is using UV maths, and we've only got NVs.
    $skip_if_longer_than = $Config{nv_preserves_uv_bits};
  }

  foreach ('', 1, 2, 3, 15, 16, 17, 31, 32, 33, 53, 54, 63, 64, 65) {
    SKIP: {
      my $sum = eval {unpack "%$_$format*", pack "$format*", @_};
      skip "cannot pack '$format' on this perl", 3
        if is_valid_error($@);

      is($@, '', "no error");
      ok(defined $sum, "sum bits $_, format $format defined");

      my $len = $_; # Copy, so that we can reassign ''
      $len = 16 unless length $len;

      SKIP: {
        skip "cannot test checksums over $skip_if_longer_than bits", 1
          if $len > $skip_if_longer_than;

        # Our problem with testing this portably is that the checksum code in
        # pp_unpack is able to cast signed to unsigned, and do modulo 2**n
        # arithmetic in unsigned ints, which perl has no operators to do.
        # (use integer; does signed ints, which won't wrap on UTS, which is just
        # fine with ANSI, but not with most people's assumptions.
        # This is why we need to supply the totals for 'Q' as there's no way in
        # perl to calculate them, short of unpack '%0Q' (is that documented?)
        # ** returns NVs; make sure it's IV.
        my $max = 1 + 2 * (int (2 ** ($len-1))-1); # The max possible checksum
        my $max_p1 = $max + 1;
        my ($max_is_integer, $max_p1_is_integer);
        $max_p1_is_integer = 1 unless $max_p1 + 1 == $max_p1;
        $max_is_integer = 1 if $max - 1 < ~0;

        my $calc_sum;
        if (ref $total) {
            $calc_sum = &$total($len);
        } else {
            $calc_sum = $total;
            # Shift into range by some multiple of the total
            my $mult = $max_p1 ? int ($total / $max_p1) : undef;
            # Need this to make sure that -1 + (~0+1) is ~0 (ie still integer)
            $calc_sum = $total - $mult;
            $calc_sum -= $mult * $max;
            if ($calc_sum < 0) {
                $calc_sum += 1;
                $calc_sum += $max;
            }
        }
        if ($calc_sum == $calc_sum - 1 && $calc_sum == $max_p1) {
            # we're into floating point (either by getting out of the range of
            # UV arithmetic, or because we're doing a floating point checksum)
            # and our calculation of the checksum has become rounded up to
            # max_checksum + 1
            $calc_sum = 0;
        }

        if ($calc_sum == $sum) { # HAS to be ==, not eq (so no is()).
            pass ("unpack '%$_$format' gave $sum");
        } else {
            my $delta = 1.000001;
            if ($format =~ tr /dDfF//
                && ($calc_sum <= $sum * $delta && $calc_sum >= $sum / $delta)) {
                pass ("unpack '%$_$format' gave $sum, expected $calc_sum");
            } else {
                my $text = ref $total ? &$total($len) : $total;
                fail;
                print "# For list (" . join (", ", @_) . ") (total $text)"
                    . " packed with $format unpack '%$_$format' gave $sum,"
                    . " expected $calc_sum\n";
            }
        }
      }
    }
  }
}

numbers ('c', -128, -1, 0, 1, 127);
numbers ('C', 0, 1, 127, 128, 255);
numbers ('W', 0, 1, 127, 128, 255, 256, 0x7ff, 0x800, 0xfffd);
numbers ('s', -32768, -1, 0, 1, 32767);
numbers ('S', 0, 1, 32767, 32768, 65535);
numbers ('i', -2147483648, -1, 0, 1, 2147483647);
numbers ('I', 0, 1, 2147483647, 2147483648, 4294967295);
numbers ('l', -2147483648, -1, 0, 1, 2147483647);
numbers ('L', 0, 1, 2147483647, 2147483648, 4294967295);
numbers ('s!', -32768, -1, 0, 1, 32767);
numbers ('S!', 0, 1, 32767, 32768, 65535);
numbers ('i!', -2147483648, -1, 0, 1, 2147483647);
numbers ('I!', 0, 1, 2147483647, 2147483648, 4294967295);
numbers ('l!', -2147483648, -1, 0, 1, 2147483647);
numbers ('L!', 0, 1, 2147483647, 2147483648, 4294967295);
numbers ('n', 0, 1, 32767, 32768, 65535);
numbers ('v', 0, 1, 32767, 32768, 65535);
numbers ('N', 0, 1, 2147483647, 2147483648, 4294967295);
numbers ('V', 0, 1, 2147483647, 2147483648, 4294967295);
numbers ('n!', -32768, -1, 0, 1, 32767);
numbers ('v!', -32768, -1, 0, 1, 32767);
numbers ('N!', -2147483648, -1, 0, 1, 2147483647);
numbers ('V!', -2147483648, -1, 0, 1, 2147483647);
# All these should have exact binary representations:
numbers ('f', -1, 0, 0.5, 42, 2**34);
numbers ('d', -(2**34), -1, 0, 1, 2**34);
## These don't, but 'd' is NV.  XXX wrong, it's double
#numbers ('d', -1, 0, 1, 1-exp(-1), -exp(1));

numbers_with_total ('q', -1,
                    -9223372036854775808, -1, 0, 1,9223372036854775807);
# This total is icky, but the true total is 2**65-1, and need a way to generate
# the expected checksum on any system including those where NVs can preserve
# 65 bits. (long double is 128 bits on sparc, so they certainly can)
# or where rounding is down not up on binary conversion (crays)
numbers_with_total ('Q', sub {
                      my $len = shift;
                      $len = 65 if $len > 65; # unmasked total is 2**65-1 here
                      my $total = 1 + 2 * (int (2**($len - 1)) - 1);
                      return 0 if $total == $total - 1; # Overflowed integers
                      return $total; # NVs still accurate to nearest integer
                    },
                    0, 1,9223372036854775807, 9223372036854775808,
                    18446744073709551615);

print "# pack nvNV byteorders\n";

is(pack("n", 0xdead), "\xde\xad");
is(pack("v", 0xdead), "\xad\xde");
is(pack("N", 0xdeadbeef), "\xde\xad\xbe\xef");
is(pack("V", 0xdeadbeef), "\xef\xbe\xad\xde");

is(pack("n!", 0xdead), "\xde\xad");
is(pack("v!", 0xdead), "\xad\xde");
is(pack("N!", 0xdeadbeef), "\xde\xad\xbe\xef");
is(pack("V!", 0xdeadbeef), "\xef\xbe\xad\xde");

print "# test big-/little-endian conversion\n";

sub byteorder
{
  my $format = shift;
  print "# byteorder test for $format\n";
  for my $value (@_) {
    SKIP: {
      my($nat,$be,$le) = eval { map { pack $format.$_, $value } '', '>', '<' };
      skip "cannot pack '$format' on this perl", 5
        if is_valid_error($@);

      {
        use warnings qw(NONFATAL utf8);
        print "# [$value][$nat][$be][$le][$@]\n";
      }

      SKIP: {
        skip "cannot compare native byteorder with big-/little-endian", 1
            if $ByteOrder eq 'unknown';

        is($nat, $ByteOrder eq 'big' ? $be : $le);
      }
      is($be, reverse($le));
      my @x = eval { unpack "$format$format>$format<", $nat.$be.$le };

      print "# [$value][", join('][', @x), "][$@]\n";

      is($@, '');
      is($x[0], $x[1]);
      is($x[0], $x[2]);
    }
  }
}

byteorder('s', -32768, -1, 0, 1, 32767);
byteorder('S', 0, 1, 32767, 32768, 65535);
byteorder('i', -2147483648, -1, 0, 1, 2147483647);
byteorder('I', 0, 1, 2147483647, 2147483648, 4294967295);
byteorder('l', -2147483648, -1, 0, 1, 2147483647);
byteorder('L', 0, 1, 2147483647, 2147483648, 4294967295);
byteorder('j', -2147483648, -1, 0, 1, 2147483647);
byteorder('J', 0, 1, 2147483647, 2147483648, 4294967295);
byteorder('s!', -32768, -1, 0, 1, 32767);
byteorder('S!', 0, 1, 32767, 32768, 65535);
byteorder('i!', -2147483648, -1, 0, 1, 2147483647);
byteorder('I!', 0, 1, 2147483647, 2147483648, 4294967295);
byteorder('l!', -2147483648, -1, 0, 1, 2147483647);
byteorder('L!', 0, 1, 2147483647, 2147483648, 4294967295);
byteorder('q', -9223372036854775808, -1, 0, 1, 9223372036854775807);
byteorder('Q', 0, 1, 9223372036854775807, 9223372036854775808, 18446744073709551615);
byteorder('f', -1, 0, 0.5, 42, 2**34);
byteorder('F', -1, 0, 0.5, 42, 2**34);
byteorder('d', -(2**34), -1, 0, 1, 2**34);
byteorder('D', -(2**34), -1, 0, 1, 2**34);

print "# test negative numbers\n";

SKIP: {
  skip "platform is not using two's complement for negative integers", 120
    unless $IsTwosComplement;

  for my $format (qw(s i l j s! i! l! q)) {
    SKIP: {
      my($nat,$be,$le) = eval { map { pack $format.$_, -1 } '', '>', '<' };
      skip "cannot pack '$format' on this perl", 15
        if is_valid_error($@);

      my $len = length $nat;
      is($_, "\xFF"x$len) for $nat, $be, $le;

      my(@val,@ref);
      if ($len >= 8) {
        @val = (-2, -81985529216486896, -9223372036854775808);
        @ref = ("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFE",
                "\xFE\xDC\xBA\x98\x76\x54\x32\x10",
                "\x80\x00\x00\x00\x00\x00\x00\x00");
      }
      elsif ($len >= 4) {
        @val = (-2, -19088744, -2147483648);
        @ref = ("\xFF\xFF\xFF\xFE",
                "\xFE\xDC\xBA\x98",
                "\x80\x00\x00\x00");
      }
      else {
        @val = (-2, -292, -32768);
        @ref = ("\xFF\xFE",
                "\xFE\xDC",
                "\x80\x00");
      }
      for my $x (@ref) {
        if ($len > length $x) {
          $x = $x . "\xFF" x ($len - length $x);
        }
      }

      for my $i (0 .. $#val) {
        my($nat,$be,$le) = eval { map { pack $format.$_, $val[$i] } '', '>', '<' };
        is($@, '');

        SKIP: {
          skip "cannot compare native byteorder with big-/little-endian", 1
              if $ByteOrder eq 'unknown';

          is($nat, $ByteOrder eq 'big' ? $be : $le);
        }

        is($be, $ref[$i]);
        is($be, reverse($le));
      }
    }
  }
}

{
  # /

  my ($x, $y, $z, @a);
  eval { ($x) = unpack '/a*','hello' };
  like($@, qr!'/' must follow a numeric type!);
  undef $x;
  eval { $x = unpack '/a*','hello' };
  like($@, qr!'/' must follow a numeric type!);

  # [perl #60204] Unhelpful error message from unpack
  eval { @a = unpack 'v/a*','h' };
  is($@, '');
  is(scalar @a, 0);
  eval { $x = unpack 'v/a*','h' };
  is($@, '');
  is($x, undef);

  undef $x;
  eval { ($z,$x,$y) = unpack 'a3/A C/a* C/Z', "003ok \003yes\004z\000abc" };
  is($@, '');
  is($z, 'ok');
  is($x, 'yes');
  is($y, 'z');
  undef $z;
  eval { $z = unpack 'a3/A C/a* C/Z', "003ok \003yes\004z\000abc" };
  is($@, '');
  is($z, 'ok');


  undef $x;
  eval { ($x) = pack '/a*','hello' };
  like($@,  qr!Invalid type '/'!);
  undef $x;
  eval { $x = pack '/a*','hello' };
  like($@,  qr!Invalid type '/'!);

  $z = pack 'n/a* N/Z* w/A*','string','hi there ','etc';
  my $expect = "\000\006string\0\0\0\012hi there \000\003etc";
  is($z, $expect);

  undef $x;
  $expect = 'hello world';
  eval { ($x) = unpack ("w/a", chr (11) . "hello world!")};
  is($x, $expect);
  is($@, '');

  undef $x;
  # Doing this in scalar context used to fail.
  eval { $x = unpack ("w/a", chr (11) . "hello world!")};
  is($@, '');
  is($x, $expect);

  foreach (
           ['a/a*/a*', '212ab345678901234567','ab3456789012'],
           ['a/a*/a*', '3012ab345678901234567', 'ab3456789012'],
           ['a/a*/b*', '212ab', $::IS_EBCDIC ? '100000010100' : '100001100100'],
  )
  {
    my ($pat, $in, $expect) = @$_;
    undef $x;
    eval { ($x) = unpack $pat, $in };
    is($@, '');
    is($x, $expect) ||
      printf "# list unpack ('$pat', '$in') gave %s, expected '$expect'\n",
             encode_list ($x);

    undef $x;
    eval { $x = unpack $pat, $in };
    is($@, '');
    is($x, $expect) ||
      printf "# scalar unpack ('$pat', '$in') gave %s, expected '$expect'\n",
             encode_list ($x);
  }

  # / with #

  my $pattern = <<'EOU';
 a3/A			# Count in ASCII
 C/a*			# Count in a C char
 C/Z			# Count in a C char but skip after \0
EOU

  $x = $y = $z =undef;
  eval { ($z,$x,$y) = unpack $pattern, "003ok \003yes\004z\000abc" };
  is($@, '');
  is($z, 'ok');
  is($x, 'yes');
  is($y, 'z');
  undef $x;
  eval { $z = unpack $pattern, "003ok \003yes\004z\000abc" };
  is($@, '');
  is($z, 'ok');

  $pattern = <<'EOP';
  n/a*			# Count as network short
  w/A*			# Count a  BER integer
EOP
  $expect = "\000\006string\003etc";
  $z = pack $pattern,'string','etc';
  is($z, $expect);
}


{
    is("1.20.300.4000", sprintf "%vd", pack("U*",1,20,300,4000));
    is("1.20.300.4000", sprintf "%vd", pack("  U*",1,20,300,4000));
}
isnt(v1.20.300.4000, sprintf "%vd", pack("C0U*",1,20,300,4000));

my $rslt = join " ", map { ord } split "", byte_utf8a_to_utf8n("\xc7\xa2");
# The ASCII UTF-8 of U+1E2 is "\xc7\xa2"
is(join(" ", unpack("U0 C*", chr(0x1e2))), $rslt);

# does pack U create Unicode?
is(ord(pack('U', 300)), 300);

# does unpack U deref Unicode?
is((unpack('U', chr(300)))[0], 300);

# is unpack U the reverse of pack U for Unicode string?
is("@{[unpack('U*', pack('U*', 100, 200, 300))]}", "100 200 300");

# is unpack U the reverse of pack U for byte string?
is("@{[unpack('U*', pack('U*', 100, 200))]}", "100 200");

{
    # does pack U0C create Unicode?
    my $cp202 = chr(202);
    utf8::upgrade $cp202;
    my @bytes202;
    {   # This is portable across character sets
        use bytes;
        @bytes202 = map { ord } split "", $cp202;
    }

    # This test requires the first number to be invariant; 64 is invariant on
    # ASCII and EBCDIC.
    is("@{[pack('U0C*', 64, @bytes202)]}", v64.v202);

    # does pack C0U create characters?
    # The U* is expecting Unicode, so convert to that.
    is("@{[pack('C0U*', map { $_ } 64, 202)]}",
       pack("C*", 64, @bytes202));

    # does unpack U0U on byte data fail?
    fresh_perl_like('my $bad = pack("U0C", 202); my @null = unpack("U0U", $bad);',
                    qr/^Malformed UTF-8 character: /,
                    {},
                    "pack doesn't return malformed UTF-8");
}

{
  my $p = pack 'i*', -2147483648, ~0, 0, 1, 2147483647;
  my (@a);
  # bug - % had to be at the start of the pattern, no leading whitespace or
  # comments. %i! didn't work at all.
  foreach my $pat ('%32i*', ' %32i*', "# Muhahahaha\n%32i*", '%32i*  ',
                   '%32i!*', ' %32i!*', "\n#\n#\n\r \t\f%32i!*", '%32i!*#') {
    @a = unpack $pat, $p;
    is($a[0], 0xFFFFFFFF) || print "# $pat\n";
    @a = scalar unpack $pat, $p;
    is($a[0], 0xFFFFFFFF) || print "# $pat\n";
  }


  $p = pack 'I*', 42, 12;
  # Multiline patterns in scalar context failed.
  foreach my $pat ('I', <<EOPOEMSNIPPET, 'I#I', 'I # I', 'I # !!!') {
# On the Ning Nang Nong
# Where the Cows go Bong!
# And the Monkeys all say Boo!
I
EOPOEMSNIPPET
    @a = unpack $pat, $p;
    is(scalar @a, 1);
    is($a[0], 42);
    @a = scalar unpack $pat, $p;
    is(scalar @a, 1);
    is($a[0], 42);
  }

  # shorts (of all flavours) didn't calculate checksums > 32 bits with floating
  # point, so a pathologically long pattern would wrap at 32 bits.
  my $pat = "\xff\xff"x65538; # Start with it long, to save any copying.
  foreach (4,3,2,1,0) {
    my $len = 65534 + $_;
    is(unpack ("%33n$len", $pat), 65535 * $len);
  }
}


# pack x X @
foreach (
         ['x', "N", "\0"],
         ['x4', "N", "\0"x4],
         ['xX', "N", ""],
         ['xXa*', "Nick", "Nick"],
         ['a5Xa5', "cameL", "llama", "camellama"],
         ['@4', 'N', "\0"x4],
         ['a*@8a*', 'Camel', 'Dromedary', "Camel\0\0\0Dromedary"],
         ['a*@4a', 'Perl rules', '!', 'Perl!'],
)
{
  my ($template, @in) = @$_;
  my $out = pop @in;
  my $got = eval {pack $template, @in};
  is($@, '');
  is($out, $got) ||
    printf "# pack ('$template', %s) gave %s expected %s\n",
           encode_list (@in), encode_list ($got), encode_list ($out);
}

# unpack x X @
foreach (
         ['x', "N"],
         ['xX', "N"],
         ['xXa*', "Nick", "Nick"],
         ['a5Xa5', "camellama", "camel", "llama"],
         ['@3', "ice"],
         ['@2a2', "water", "te"],
         ['a*@1a3', "steam", "steam", "tea"],
)
{
  my ($template, $in, @out) = @$_;
  my @got = eval {unpack $template, $in};
  is($@, '');
  ok (list_eq (\@got, \@out)) ||
    printf "# list unpack ('$template', %s) gave %s expected %s\n",
           _qq($in), encode_list (@got), encode_list (@out);

  my $got = eval {unpack $template, $in};
  is($@, '');
  @out ? is( $got, $out[0] ) # 1 or more items; should get first
       : ok( !defined $got ) # 0 items; should get undef
    or printf "# scalar unpack ('$template', %s) gave %s expected %s\n",
              _qq($in), encode_list ($got), encode_list ($out[0]);
}

{
    my $t = 'Z*Z*';
    my ($u, $v) = qw(foo xyzzy);
    my $p = pack($t, $u, $v);
    my @u = unpack($t, $p);
    is(scalar @u, 2);
    is($u[0], $u);
    is($u[1], $v);
}

{
    is((unpack("w/a*", "\x02abc"))[0], "ab");

    # "w/a*" should be seen as one unit

    is(scalar unpack("w/a*", "\x02abc"), "ab");
}

print "# group modifiers\n";

for my $t (qw{ (s<)< (sl>s)> (s(l(sl)<l)s)< }) {
  print "# testing pattern '$t'\n";
  eval { ($_) = unpack($t, 'x'x18); };
  is($@, '');
  eval { $_ = pack($t, (0)x6); };
  is($@, '');
}

for my $t (qw{ (s<)> (sl>s)< (s(l(sl)<l)s)> }) {
  print "# testing pattern '$t'\n";
  eval { ($_) = unpack($t, 'x'x18); };
  like($@, qr/Can't use '[<>]' in a group with different byte-order in unpack/);
  eval { $_ = pack($t, (0)x6); };
  like($@, qr/Can't use '[<>]' in a group with different byte-order in pack/);
}

is(pack('L<L>', (0x12345678)x2),
   pack('(((L1)1)<)(((L)1)1)>1', (0x12345678)x2));

{
  sub compress_template {
    my $t = shift;
    for my $mod (qw( < > )) {
      $t =~ s/((?:(?:[SILQJFDP]!?$mod|[^SILQJFDP\W]!?)(?:\d+|\*|\[(?:[^]]+)\])?\/?){2,})/
              my $x = $1; $x =~ s!$mod!!g ? "($x)$mod" : $x /ieg;
    }
    return $t;
  }

  my %templates = (
    's<'                  => [-42],
    's<c2x![S]S<'         => [-42, -11, 12, 4711],
    '(i<j<[s]l<)3'        => [-11, -22, -33, 1000000, 1100, 2201, 3302,
                              -1000000, 32767, -32768, 1, -123456789 ],
    '(I!<4(J<2L<)3)5'     => [1 .. 65],
    'q<Q<'                => [-50000000005, 60000000006],
    'f<F<d<'              => [3.14159, 111.11, 2222.22],
    'D<cCD<'              => [1e42, -128, 255, 1e-42],
    'n/a*'                => ['/usr/bin/perl'],
    'C/a*S</A*L</Z*I</a*' => [qw(Just another Perl hacker)],
  );

  for my $tle (sort keys %templates) {
    my @d = @{$templates{$tle}};
    my $tbe = $tle;
    $tbe =~ y/</>/;
    for my $t ($tbe, $tle) {
      my $c = compress_template($t);
      print "# '$t' -> '$c'\n";
      SKIP: {
        my $p1 = eval { pack $t, @d };
        skip "cannot pack '$t' on this perl", 5 if is_valid_error($@);
        my $p2 = eval { pack $c, @d };
        is($@, '');
        is($p1, $p2);
        s!(/[aAZ])\*!$1!g for $t, $c;
        my @u1 = eval { unpack $t, $p1 };
        is($@, '');
        my @u2 = eval { unpack $c, $p2 };
        is($@, '');
        is(join('!', @u1), join('!', @u2));
      }
    }
  }
}

{
    # from Wolfgang Laun: fix in change #13163

    my $s = 'ABC' x 10;
    my $t = '*';
    my $x = ord($t);
    my $buf = pack( 'Z*/A* C',  $s, $x );
    my $y;

    my $h = $buf;
    $h =~ s/[^[:print:]]/./g;
    ( $s, $y ) = unpack( "Z*/A* C", $buf );
    is($h, "30.ABCABCABCABCABCABCABCABCABCABC$t");
    is(length $buf, 34);
    is($s, "ABCABCABCABCABCABCABCABCABCABC");
    is($y, $x);
}

{
    # from Wolfgang Laun: fix in change #13288

    eval { my $t=unpack("P*", "abc") };
    like($@, qr/'P' must have an explicit size/);
}

{   # Grouping constructs
    my (@a, @b);
    @a = unpack '(SL)',   pack 'SLSLSL', 67..90;
    is("@a", "67 68");
    @a = unpack '(SL)3',   pack 'SLSLSL', 67..90;
    @b = (67..72);
    is("@a", "@b");
    @a = unpack '(SL)3',   pack 'SLSLSLSL', 67..90;
    is("@a", "@b");
    @a = unpack '(SL)[3]', pack 'SLSLSLSL', 67..90;
    is("@a", "@b");
    @a = unpack '(SL)[2] SL', pack 'SLSLSLSL', 67..90;
    is("@a", "@b");
    @a = unpack 'A/(SL)',  pack 'ASLSLSLSL', 3, 67..90;
    is("@a", "@b");
    @a = unpack 'A/(SL)SL',  pack 'ASLSLSLSL', 2, 67..90;
    is("@a", "@b");
    @a = unpack '(SL)*',   pack 'SLSLSLSL', 67..90;
    @b = (67..74);
    is("@a", "@b");
    @a = unpack '(SL)*SL',   pack 'SLSLSLSL', 67..90;
    is("@a", "@b");
    eval { @a = unpack '(*SL)',   '' };
    like($@, qr/\(\)-group starts with a count/);
    eval { @a = unpack '(3SL)',   '' };
    like($@, qr/\(\)-group starts with a count/);
    eval { @a = unpack '([3]SL)',   '' };
    like($@, qr/\(\)-group starts with a count/);
    eval { @a = pack '(*SL)' };
    like($@, qr/\(\)-group starts with a count/);
    @a = unpack '(SL)3 SL',   pack '(SL)4', 67..74;
    is("@a", "@b");
    @a = unpack '(SL)3 SL',   pack '(SL)[4]', 67..74;
    is("@a", "@b");
    @a = unpack '(SL)3 SL',   pack '(SL)*', 67..74;
    is("@a", "@b");
}

{  # more on grouping (W.Laun)
  # @ absolute within ()-group
  my $badc = pack( '(a)*', unpack( '(@1a @0a @2)*', 'abcd' ) );
  is( $badc, 'badc' );
  my @b = ( 1, 2, 3 );
  my $buf = pack( '(@1c)((@2C)@3c)', @b );
  is( $buf, "\0\1\0\0\2\3" );
  my @a = unpack( '(@1c)((@2c)@3c)', $buf );
  is( "@a", "@b" );

  # various unpack count/code scenarios
  my @Env = ( a => 'AAA', b => 'BBB' );
  my $env = pack( 'S(S/A*S/A*)*', @Env/2, @Env );

  # unpack full length - ok
  my @pup = unpack( 'S/(S/A* S/A*)', $env );
  is( "@pup", "@Env" );

  # warn when count/code goes beyond end of string
  # \0002 \0001 a \0003 AAA \0001 b \0003 BBB
  #     2     4 5     7  10    1213
  eval { @pup = unpack( 'S/(S/A* S/A*)', substr( $env, 0, 13 ) ) };
  like( $@, qr{length/code after end of string} );

  # postfix repeat count
  $env = pack( '(S/A* S/A*)' . @Env/2, @Env );

  # warn when count/code goes beyond end of string
  # \0001 a \0003 AAA \0001  b \0003 BBB
  #     2 3c    5   8    10 11    13  16
  eval { @pup = unpack( '(S/A* S/A*)' . @Env/2, substr( $env, 0, 11 ) ) };
  like( $@, qr{length/code after end of string} );

  # catch stack overflow/segfault
  eval { $_ = pack( ('(' x 105) . 'A' . (')' x 105) ); };
  like( $@, qr{Too deeply nested \(\)-groups} );
}

{ # syntax checks (W.Laun)
  use warnings qw(NONFATAL all);;
  my @warning;
  local $SIG{__WARN__} = sub {
      push( @warning, $_[0] );
  };
  eval { my $s = pack( 'Ax![4c]A', 1..5 ); };
  like( $@, qr{Malformed integer in \[\]} );

  eval { my $buf = pack( '(c/*a*)', 'AAA', 'BB' ); };
  like( $@, qr{'/' does not take a repeat count} );

  eval { my @inf = unpack( 'c/1a', "\x03AAA\x02BB" ); };
  like( $@, qr{'/' does not take a repeat count} );

  eval { my @inf = unpack( 'c/*a', "\x03AAA\x02BB" ); };
  like( $@, qr{'/' does not take a repeat count} );

  # white space where possible
  my @Env = ( a => 'AAA', b => 'BBB' );
  my $env = pack( ' S ( S / A*   S / A* )* ', @Env/2, @Env );
  my @pup = unpack( ' S / ( S / A*   S / A* ) ', $env );
  is( "@pup", "@Env" );

  # white space in 4 wrong places
  for my $temp (  'A ![4]', 'A [4]', 'A *', 'A 4' ){
      eval { my $s = pack( $temp, 'B' ); };
      like( $@, qr{Invalid type } );
  }

  # warning for commas
  @warning = ();
  my $x = pack( 'I,A', 4, 'X' );
  like( $warning[0], qr{Invalid type ','} );
  is($x, pack( 'IA', 4, 'X' ), "Comma was ignored in pack string");

  # comma warning only once
  @warning = ();
  $x = pack( 'C(C,C)C,C', 65..71  );
  cmp_ok( scalar(@warning), '==', 1 );
  is(join(",", unpack 'C(C,,,C),C,,C', $x), join(",", 65..69),
     "Comma was ignored in unpack string");

  # forbidden code in []
  eval { my $x = pack( 'A[@4]', 'XXXX' ); };
  like( $@, qr{Within \[\]-length '\@' not allowed} );

  # @ repeat default 1
  my $s = pack( 'AA@A', 'A', 'B', 'C' );
  my @c = unpack( 'AA@A', $s );
  is( $s, 'AC' );
  is( "@c", "A C C" );

  # no unpack code after /
  eval { my @a = unpack( "C/", "\3" ); };
  like( $@, qr{Code missing after '/'} );

  # modifier warnings
  @warning = ();
  $x = pack "I>>s!!", 47, 11;
  ($x) = unpack "I<<l!>!>", 'x'x20;
  is(scalar @warning, 5);
  like($warning[0], qr/Duplicate modifier '>' after 'I' in pack/);
  like($warning[1], qr/Duplicate modifier '!' after 's' in pack/);
  like($warning[2], qr/Duplicate modifier '<' after 'I' in unpack/);
  like($warning[3], qr/Duplicate modifier '!' after 'l' in unpack/);
  like($warning[4], qr/Duplicate modifier '>' after 'l' in unpack/);
}

{  # Repeat count [SUBEXPR]
   my @codes = qw( x A Z a c C W B b H h s v n S i I l V N L p P f F d
		   s! S! i! I! l! L! j J);
   my $G;
   if (eval { pack 'q', 1 } ) {
     push @codes, qw(q Q);
   } else {
     push @codes, qw(s S);	# Keep the count the same
   }
   if (eval { pack 'D', 1 } ) {
     push @codes, 'D';
   } else {
     push @codes, 'd';	# Keep the count the same
   }

   push @codes, map { /^[silqjfdp]/i ? ("$_<", "$_>") : () } @codes;

   my %val;
   @val{@codes} = map { / [Xx]  (?{ undef })
			| [AZa] (?{ 'something' })
			| C     (?{ 214 })
			| W     (?{ 8188 })
			| c     (?{ 114 })
			| [Bb]  (?{ '101' })
			| [Hh]  (?{ 'b8' })
			| [svnSiIlVNLqQjJ]  (?{ 10111 })
			| [FfDd]  (?{ 1.36514538e37 })
			| [pP]  (?{ "try this buffer" })
			/x; $^R } @codes;
   my @end = (0x12345678, 0x23456781, 0x35465768, 0x15263748);
   my $end = "N4";

   for my $type (@codes) {
     my @list = $val{$type};
     @list = () unless defined $list[0];
     for my $count ('', '3', '[11]') {
       my $c = 1;
       $c = $1 if $count =~ /(\d+)/;
       my @list1 = @list;
       @list1 = (@list1) x $c unless $type =~ /[XxAaZBbHhP]/;
       for my $groupend ('', ')2', ')[8]') {
	   my $groupbegin = ($groupend ? '(' : '');
	   $c = 1;
	   $c = $1 if $groupend =~ /(\d+)/;
	   my @list2 = (@list1) x $c;

           SKIP: {
	     my $junk1 = "$groupbegin $type$count $groupend";
	     # print "# junk1=$junk1\n";
	     my $p = eval { pack $junk1, @list2 };
             skip "cannot pack '$type' on this perl", 12
               if is_valid_error($@);
	     die "pack $junk1 failed: $@" if $@;

	     my $half = int( (length $p)/2 );
	     for my $move ('', "X$half", "X!$half", 'x1', 'x!8', "x$half") {
	       my $junk = "$junk1 $move";
	       # print "# junk='$junk', list=(@list2)\n";
	       $p = pack "$junk $end", @list2, @end;
	       my @l = unpack "x[$junk] $end", $p;
	       is(scalar @l, scalar @end);
	       is("@l", "@end", "skipping x[$junk]");
	     }
           }
       }
     }
   }
}

# / is recognized after spaces in scalar context
# XXXX no spaces are allowed in pack...  In pack only before the slash...
is(scalar unpack('A /A Z20', pack 'A/A* Z20', 'bcde', 'xxxxx'), 'bcde');
is(scalar unpack('A /A /A Z20', '3004bcde'), 'bcde');

{ # X! and x!
  my $t = 'C[3]  x!8 C[2]';
  my @a = (0x73..0x77);
  my $p = pack($t, @a);
  is($p, "\x73\x74\x75\0\0\0\0\0\x76\x77");
  my @b = unpack $t, $p;
  is(scalar @b, scalar @a);
  is("@b", "@a", 'x!8');
  $t = 'x[5] C[6] X!8 C[2]';
  @a = (0x73..0x7a);
  $p = pack($t, @a);
  is($p, "\0\0\0\0\0\x73\x74\x75\x79\x7a");
  @b = unpack $t, $p;
  @a = (0x73..0x75, 0x79, 0x7a, 0x79, 0x7a);
  is(scalar @b, scalar @a);
  is("@b", "@a");
}

{ # struct {char c1; double d; char cc[2];}
  my $t = 'C x![d] d C[2]';
  my @a = (173, 1.283476517e-45, 42, 215);
  my $p = pack $t, @a;
  ok( length $p);
  my @b = unpack "$t X[$t] $t", $p;	# Extract, step back, extract again
  is(scalar @b, 2 * scalar @a);
  $b = "@b";
  $b =~ s/(?:17000+|16999+)\d+(e-0?45) /17$1 /gi; # stringification is gamble
  is($b, "@a @a");

  use warnings qw(NONFATAL all);;
  my $warning;
  local $SIG{__WARN__} = sub {
      $warning = $_[0];
  };
  @b = unpack "x[C] x[$t] X[$t] X[C] $t", "$p\0";

  is($warning, undef);
  is(scalar @b, scalar @a);
  $b = "@b";
  $b =~ s/(?:17000+|16999+)\d+(e-0?45) /17$1 /gi; # stringification is gamble
  is($b, "@a");
}

is(length(pack("j", 0)), $Config{ivsize});
is(length(pack("J", 0)), $Config{uvsize});
is(length(pack("F", 0)), $Config{nvsize});

numbers ('j', -2147483648, -1, 0, 1, 2147483647);
numbers ('J', 0, 1, 2147483647, 2147483648, 4294967295);
numbers ('F', -(2**34), -1, 0, 1, 2**34);
SKIP: {
    my $t = eval { unpack("D*", pack("D", 12.34)) };

    skip "Long doubles not in use", 166 if $@ =~ /Invalid type/;

    is(length(pack("D", 0)), $Config{longdblsize});
    numbers ('D', -(2**34), -1, 0, 1, 2**34);
}

# Maybe this knowledge needs to be "global" for all of pack.t
# Or a "can checksum" which would effectively be all the number types"
my %cant_checksum = map {$_=> 1} qw(A Z u w);
# not a b B h H
foreach my $template (qw(A Z c C s S i I l L n N v V q Q j J f d F D u U w)) {
  SKIP: {
    my $packed = eval {pack "${template}4", 1, 4, 9, 16};
    if ($@) {
      die unless $@ =~ /Invalid type '$template'/;
      skip ("$template not supported on this perl",
            $cant_checksum{$template} ? 4 : 8);
    }
    my @unpack4 = unpack "${template}4", $packed;
    my @unpack = unpack "${template}*", $packed;
    my @unpack1 = unpack "${template}", $packed;
    my @unpack1s = scalar unpack "${template}", $packed;
    my @unpack4s = scalar unpack "${template}4", $packed;
    my @unpacks = scalar unpack "${template}*", $packed;

    my @tests = ( ["${template}4 vs ${template}*", \@unpack4, \@unpack],
                  ["scalar ${template} ${template}", \@unpack1s, \@unpack1],
                  ["scalar ${template}4 vs ${template}", \@unpack4s, \@unpack1],
                  ["scalar ${template}* vs ${template}", \@unpacks, \@unpack1],
                );

    unless ($cant_checksum{$template}) {
      my @unpack4_c = unpack "\%${template}4", $packed;
      my @unpack_c = unpack "\%${template}*", $packed;
      my @unpack1_c = unpack "\%${template}", $packed;
      my @unpack1s_c = scalar unpack "\%${template}", $packed;
      my @unpack4s_c = scalar unpack "\%${template}4", $packed;
      my @unpacks_c = scalar unpack "\%${template}*", $packed;

      push @tests,
        ( ["% ${template}4 vs ${template}*", \@unpack4_c, \@unpack_c],
          ["% scalar ${template} ${template}", \@unpack1s_c, \@unpack1_c],
          ["% scalar ${template}4 vs ${template}*", \@unpack4s_c, \@unpack_c],
          ["% scalar ${template}* vs ${template}*", \@unpacks_c, \@unpack_c],
        );
    }
    foreach my $test (@tests) {
      ok (list_eq ($test->[1], $test->[2]), $test->[0]) ||
        printf "# unpack gave %s expected %s\n",
          encode_list (@{$test->[1]}), encode_list (@{$test->[2]});
    }
  }
}

ok(pack('u2', 'AA'), "[perl #8026]"); # used to hang and eat RAM in perl 5.7.2

$_ = pack('c', 65); # 'A' would not be EBCDIC-friendly
is(unpack('c'), 65, "one-arg unpack (change #18751)"); # defaulting to $_

{
    my $a = "X\x0901234567\n" x 100; # \t would not be EBCDIC TAB
    my @a = unpack("(a1 c/a)*", $a);
    is(scalar @a, 200,       "[perl #15288]");
    is($a[-1], "01234567\n", "[perl #15288]");
    is($a[-2], "X",          "[perl #15288]");
}

{
    use warnings qw(NONFATAL all);;
    my $warning;
    local $SIG{__WARN__} = sub {
        $warning = $_[0];
    };

    # This test is looking for the encoding of the bit pattern "\x66\x6f\x6f",
    # which is ASCII "foo"
    my $out = pack("u99", native_to_uni("foo") x 99);
    like($warning, qr/Field too wide in 'u' format in pack at /,
         "Warn about too wide uuencode");
    is($out, ("_" . "9F]O" x 21 . "\n") x 4 . "M" . "9F]O" x 15 . "\n",
       "Use max width in case of too wide uuencode");
}

# checksums
{
    # verify that unpack advances correctly wrt a checksum
    my (@x) = unpack("b10a", "abcd");
    my (@y) = unpack("%b10a", "abcd");
    is($x[1], $y[1], "checksum advance ok");

    SKIP: {
        skip("-- non-IEEE float", 1) if !$Config{d_double_style_ieee};
        # verify that the checksum is not overflowed with C0
        is(unpack("C0%128U", "abcd"), unpack("U0%128U", "abcd"), "checksum not overflowed");
    }
}

my $U_1FFC_bytes = byte_utf8a_to_utf8n("\341\277\274");
{
    # U0 and C0 must be scoped
    my (@x) = unpack("a(U0)U", "b$U_1FFC_bytes");
    is($x[0], 'b', 'before scope');
    is($x[1], 8188, 'after scope');

    is(pack("a(U0)U", "b", 8188), "b$U_1FFC_bytes");
}

{
    # counted length prefixes shouldn't change C0/U0 mode
    # (note the length is actually 0 in this test, as the C/ is replaced by C0
    # due to the \0 in the string)
    is(join(',', unpack("aC/UU",   "b\0$U_1FFC_bytes")), 'b,8188');
    is(join(',', unpack("aC/CU",   "b\0$U_1FFC_bytes")), 'b,8188');

    # The U expects Unicode, so convert from native
    my $first_byte = ord substr($U_1FFC_bytes, 0, 1);

    is(join(',', unpack("aU0C/UU", "b\0$U_1FFC_bytes")), "b,$first_byte");
    is(join(',', unpack("aU0C/CU", "b\0$U_1FFC_bytes")), "b,$first_byte");
}

{
    # "Z0" (bug #34062)
    my (@x) = unpack("C*", pack("CZ0", 1, "b"));
    is(join(',', @x), '1', 'pack Z0 doesn\'t destroy the character before');
}

{
    # Encoding neutrality
    # String we will pull apart and rebuild in several ways:
    my $down = "\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff\x05\x06";
    my $up   = $down;
    utf8::upgrade($up);

    my %expect =
        # [expected result,
        #  how many chars it should progress,
        #  (optional) expected result of pack]
        (a5 => ["\xf8\xf9\xfa\xfb\xfc", 5],
         A5 => ["\xf8\xf9\xfa\xfb\xfc", 5],
         Z5 => ["\xf8\xf9\xfa\xfb\xfc", 5, "\xf8\xf9\xfa\xfb\x00\xfd"],
         b21 => ["000111111001111101011", 3, "\xf8\xf9\x1a\xfb"],
         B21 => ["111110001111100111111", 3, "\xf8\xf9\xf8\xfb"],
         H5 => ["f8f9f", 3, "\xf8\xf9\xf0\xfb"],
         h5 => ["8f9fa", 3, "\xf8\xf9\x0a\xfb"],
         "s<"  => [-1544, 2],
         "s>"  => [-1799, 2],
         "S<"  => [0xf9f8, 2],
         "S>"  => [0xf8f9, 2],
         "l<"  => [-67438088, 4],
         "l>"  => [-117835013, 4],
         "L>"  => [0xf8f9fafb, 4],
         "L<"  => [0xfbfaf9f8, 4],
         n     => [0xf8f9, 2],
         N     => [0xf8f9fafb, 4],
         v     => [63992, 2],
         V     => [0xfbfaf9f8, 4],
         c     => [-8, 1],
         U0U   => [0xf8, 1],
         w     => ["8715569050387726213", 9],
         q     => ["-283686952306184", 8],
         Q     => ["18446460386757245432", 8],
         );

    for my $string ($down, $up) {
        for my $format (sort {lc($a) cmp lc($b) || $a cmp $b } keys %expect) {
          SKIP: {
              my $expect = $expect{$format};
              # unpack upgraded and downgraded string
              my @result = eval { unpack("$format C0 W", $string) };
              skip "cannot pack/unpack '$format C0 W' on this perl", 5 if
                  $@ && is_valid_error($@);
              is(@result, 2, "Two results from unpack $format C0 W");

              # pack to downgraded
              my $new = pack("$format C0 W", @result);
              is(length($new), $expect->[1]+1,
                 "pack $format C0 W should give $expect->[1]+1 chars");
              is($new, $expect->[2] || substr($string, 0, length $new),
                 "pack $format C0 W returns expected value");

              # pack to upgraded
              $new = pack("a0 $format C0 W", chr(256), @result);
              is(length($new), $expect->[1]+1,
                 "pack a0 $format C0 W should give $expect->[1]+1 chars");
              is($new, $expect->[2] || substr($string, 0, length $new),
                 "pack a0 $format C0 W returns expected value");
          }
        }
    }
}

{
    # Encoding neutrality, numbers
    my $val = -2.68;
    for my $format (qw(s S i I l L j J f d F D q Q
                       s! S! i! I! l! L! n! N! v! V!)) {
      SKIP: {
          my $down = eval { pack($format, $val) };
          skip "cannot pack/unpack $format on this perl", 9 if
              $@ && is_valid_error($@);
          ok(!utf8::is_utf8($down), "Simple $format pack doesn't get upgraded");
          my $up = pack("a0 $format", chr(256), $val);
          ok(utf8::is_utf8($up), "a0 $format with high char leads to upgrade");
          is($down, $up, "$format generated strings are equal though");
          my @down_expanded = unpack("$format W", $down . chr(0xce));
          is(@down_expanded, 2, "Expand to two values");
          is($down_expanded[1], 0xce,
             "unpack $format left us at the expected position");
          my @up_expanded   = unpack("$format W", $up   . chr(0xce));
          is(@up_expanded, 2, "Expand to two values");
          is($up_expanded[1], 0xce,
             "unpack $format left us at the expected position");
          is($down_expanded[0], $up_expanded[0], "$format unpack was neutral");
          is(pack($format, $down_expanded[0]), $down, "Pack $format undoes unpack $format");
      }
    }
}

{
    # C *is* neutral
    my $down = "\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff\x05\x06";
    my $up   = $down;
    utf8::upgrade($up);
    my @down = unpack("C*", $down);
    my @expect_down = (0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff, 0x05, 0x06);
    is("@down", "@expect_down", "byte expand");
    is(pack("C*", @down), $down, "byte join");

    my @up   = unpack("C*", $up);
    my @expect_up = (0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff, 0x05, 0x06);
    is("@up", "@expect_up", "UTF-8 expand");
    is(pack("U0C0C*", @up), $up, "UTF-8 join");
}

{
    # Harder cases for the neutrality test

    # u format
    my $down = "\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff\x05\x06";
    my $up   = $down;
    utf8::upgrade($up);
    is(pack("u", $down), pack("u", $up), "u pack is neutral");
    is(unpack("u", pack("u", $down)), $down, "u unpack to downgraded works");
    is(unpack("U0C0u", pack("u", $down)), $up, "u unpack to upgraded works");

    # p/P format
    # This actually only tests something if the address contains a byte >= 0x80
    my $str = "abc\xa5\x00\xfede";
    $down = pack("p", $str);
    is(pack("P", $str), $down);
    is(pack("U0C0p", $str), $down);
    is(pack("U0C0P", $str), $down);
    is(unpack("p", $down), "abc\xa5", "unpack p downgraded");
    $up   = $down;
    utf8::upgrade($up);
    is(unpack("p", $up), "abc\xa5", "unpack p upgraded");

    is(unpack("P7", $down), "abc\xa5\x00\xfed", "unpack P downgraded");
    is(unpack("P7", $up),   "abc\xa5\x00\xfed", "unpack P upgraded");

    # x, X and @
    $down = "\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff\x05\x06";
    $up   = $down;
    utf8::upgrade($up);

    is(unpack('@4W', $down), 0xfc, "\@positioning on downgraded string");
    is(unpack('@4W', $up),   0xfc, "\@positioning on upgraded string");

    is(unpack('@4x2W', $down), 0xfe, "x moving on downgraded string");
    is(unpack('@4x2W', $up),   0xfe, "x moving on upgraded string");
    is(unpack('@4x!4W', $down), 0xfc, "x! moving on downgraded string");
    is(unpack('@4x!4W', $up),   0xfc, "x! moving on upgraded string");
    is(unpack('@5x!4W', $down), 0x05, "x! moving on downgraded string");
    is(unpack('@5x!4W', $up),   0x05, "x! moving on upgraded string");

    is(unpack('@4X2W', $down), 0xfa, "X moving on downgraded string");
    is(unpack('@4X2W', $up),   0xfa, "X moving on upgraded string");
    is(unpack('@4X!4W', $down), 0xfc, "X! moving on downgraded string");
    is(unpack('@4X!4W', $up),   0xfc, "X! moving on upgraded string");
    is(unpack('@5X!4W', $down), 0xfc, "X! moving on downgraded string");
    is(unpack('@5X!4W', $up),   0xfc, "X! moving on upgraded string");
    is(unpack('@5X!8W', $down), 0xf8, "X! moving on downgraded string");
    is(unpack('@5X!8W', $up),   0xf8, "X! moving on upgraded string");

    is(pack("W2x", 0xfa, 0xe3), "\xfa\xe3\x00", "x on downgraded string");
    is(pack("W2x!4", 0xfa, 0xe3), "\xfa\xe3\x00\x00",
       "x! on downgraded string");
    is(pack("W2x!2", 0xfa, 0xe3), "\xfa\xe3", "x! on downgraded string");
    is(pack("U0C0W2x", 0xfa, 0xe3), "\xfa\xe3\x00", "x on upgraded string");
    is(pack("U0C0W2x!4", 0xfa, 0xe3), "\xfa\xe3\x00\x00",
       "x! on upgraded string");
    is(pack("U0C0W2x!2", 0xfa, 0xe3), "\xfa\xe3", "x! on upgraded string");
    is(pack("W2X", 0xfa, 0xe3), "\xfa", "X on downgraded string");
    is(pack("U0C0W2X", 0xfa, 0xe3), "\xfa", "X on upgraded string");
    is(pack("W2X!2", 0xfa, 0xe3), "\xfa\xe3", "X! on downgraded string");
    is(pack("U0C0W2X!2", 0xfa, 0xe3), "\xfa\xe3", "X! on upgraded string");
    is(pack("W3X!2", 0xfa, 0xe3, 0xa6), "\xfa\xe3", "X! on downgraded string");
    is(pack("U0C0W3X!2", 0xfa, 0xe3, 0xa6), "\xfa\xe3",
       "X! on upgraded string");

    # backward eating through a ( moves the group starting point backwards
    is(pack("a*(Xa)", "abc", "q"), "abq",
       "eating before strbeg moves it back");
    is(pack("a*(Xa)", "ab" . chr(512), "q"), "abq",
       "eating before strbeg moves it back");

    # Check marked_upgrade
    is(pack('W(W(Wa@3W)@6W)@9W', 0xa1, 0xa2, 0xa3, "a", 0xa4, 0xa5, 0xa6),
       "\xa1\xa2\xa3a\x00\xa4\x00\xa5\x00\xa6");
    $up = "a";
    utf8::upgrade($up);
    is(pack('W(W(Wa@3W)@6W)@9W', 0xa1, 0xa2, 0xa3, $up, 0xa4, 0xa5, 0xa6),
       "\xa1\xa2\xa3a\x00\xa4\x00\xa5\x00\xa6", "marked upgrade caused by a");
    is(pack('W(W(WW@3W)@6W)@9W', 0xa1, 0xa2, 0xa3, 256, 0xa4, 0xa5, 0xa6),
       "\xa1\xa2\xa3\x{100}\x00\xa4\x00\xa5\x00\xa6",
       "marked upgrade caused by W");
    is(pack('W(W(WU0aC0@3W)@6W)@9W', 0xa1, 0xa2, 0xa3, "a", 0xa4, 0xa5, 0xa6),
       "\xa1\xa2\xa3a\x00\xa4\x00\xa5\x00\xa6", "marked upgrade caused by U0");

    # a, A and Z
    $down = "\xa4\xa6\xa7";
    $up   = $down;
    utf8::upgrade($up);
    utf8::upgrade(my $high = "\xfeb");

    for my $format ("a0", "A0", "Z0", "U0a0C0", "U0A0C0", "U0Z0C0") {
        is(pack("a* $format a*", "ab", $down, "cd"), "abcd",
           "$format format on plain string");
        is(pack("a* $format a*", "ab", $up,   "cd"), "abcd",
           "$format format on upgraded string");
        is(pack("a* $format a*", $high, $down, "cd"), "\xfebcd",
           "$format format on plain string");
        is(pack("a* $format a*", $high, $up,   "cd"), "\xfebcd",
           "$format format on upgraded string");
        my @down = unpack("a1 $format a*", "\xfeb");
        is("@down", "\xfe  b", "unpack $format");
        my @up = unpack("a1 $format a*", $high);
        is("@up", "\xfe  b", "unpack $format");
    }
    is(pack("a1", $high), "\xfe");
    is(pack("A1", $high), "\xfe");
    is(pack("Z1", $high), "\x00");
    is(pack("a2", $high), "\xfeb");
    is(pack("A2", $high), "\xfeb");
    is(pack("Z2", $high), "\xfe\x00");
    is(pack("a5", $high), "\xfeb\x00\x00\x00");
    is(pack("A5", $high), "\xfeb   ");
    is(pack("Z5", $high), "\xfeb\x00\x00\x00");
    is(pack("a*", $high), "\xfeb");
    is(pack("A*", $high), "\xfeb");
    is(pack("Z*", $high), "\xfeb\x00");

    utf8::upgrade($high = byte_utf8a_to_utf8n("\xc3\xbe") . "b");
    is(pack("U0a2", $high), uni_to_native("\xfe"));
    is(pack("U0A2", $high), uni_to_native("\xfe"));
    is(pack("U0Z1", $high), uni_to_native("\x00"));
    is(pack("U0a3", $high), uni_to_native("\xfe") . "b");
    is(pack("U0A3", $high), uni_to_native("\xfe") . "b");
    is(pack("U0Z3", $high), uni_to_native("\xfe\x00"));
    is(pack("U0a6", $high), uni_to_native("\xfe") . "b" . uni_to_native("\x00\x00\x00"));
    is(pack("U0A6", $high), uni_to_native("\xfe") . "b   ");
    is(pack("U0Z6", $high), uni_to_native("\xfe") . "b" . uni_to_native("\x00\x00\x00"));
    is(pack("U0a*", $high), uni_to_native("\xfe") . "b");
    is(pack("U0A*", $high), uni_to_native("\xfe") . "b");
    is(pack("U0Z*", $high), uni_to_native("\xfe") . "b" . uni_to_native("\x00"));
}
{
    # pack /
    my @array = 1..14;
    my @out = unpack("N/S", pack("N/S", @array) . "abcd");
    is("@out", "@array", "pack N/S works");
    @out = unpack("N/S*", pack("N/S*", @array) . "abcd");
    is("@out", "@array", "pack N/S* works");
    @out = unpack("N/S*", pack("N/S14", @array) . "abcd");
    is("@out", "@array", "pack N/S14 works");
    @out = unpack("N/S*", pack("N/S15", @array) . "abcd");
    is("@out", "@array", "pack N/S15 works");
    @out = unpack("N/S*", pack("N/S13", @array) . "abcd");
    is("@out", "@array[0..12]", "pack N/S13 works");
    @out = unpack("N/S*", pack("N/S0", @array) . "abcd");
    is("@out", "", "pack N/S0 works");
    is(pack("Z*/a0", "abc"), "0\0", "pack Z*/a0 makes a short string");
    is(pack("Z*/Z0", "abc"), "0\0", "pack Z*/Z0 makes a short string");
    is(pack("Z*/a3", "abc"), "3\0abc", "pack Z*/a3 makes a full string");
    is(pack("Z*/Z3", "abc"), "3\0ab\0", "pack Z*/Z3 makes a short string");
    is(pack("Z*/a5", "abc"), "5\0abc\0\0", "pack Z*/a5 makes a long string");
    is(pack("Z*/Z5", "abc"), "5\0abc\0\0", "pack Z*/Z5 makes a long string");
    is(pack("Z*/Z"), "1\0\0", "pack Z*/Z makes an extended string");
    is(pack("Z*/Z", ""), "1\0\0", "pack Z*/Z makes an extended string");
    is(pack("Z*/a", ""), "0\0", "pack Z*/a makes an extended string");
}
{
    # unpack("A*", $unicode) strips general unicode spaces
    is(unpack("A*", "ab \n" . uni_to_native("\xa0") . " \0"), "ab \n" . uni_to_native("\xa0"),
       'normal A* strip leaves \xa0');
    is(unpack("U0C0A*", "ab \n" . uni_to_native("\xa0") . " \0"), "ab \n" . uni_to_native("\xa0"),
       'normal A* strip leaves \xa0 even if it got upgraded for technical reasons');
    is(unpack("A*", pack("a*(U0U)a*", "ab \n", utf8::unicode_to_native(0xa0), " \0")), "ab",
       'upgraded strings A* removes \xa0');
    is(unpack("A*", pack("a*(U0UU)a*", "ab \n", utf8::unicode_to_native(0xa0), 0x1680, " \0")), "ab",
       'upgraded strings A* removes all unicode whitespace');
    is(unpack("A5", pack("a*(U0U)a*", "ab \n", 0x1680, "def", "ab")), "ab",
       'upgraded strings A5 removes all unicode whitespace');
    is(unpack("A*", pack("U", 0x1680)), "",
       'upgraded strings A* with nothing left');
}
{
    # Testing unpack . and .!
    is(unpack(".", "ABCD"), 0, "offset at start of string is 0");
    is(unpack(".", ""), 0, "offset at start of empty string is 0");
    is(unpack("x3.", "ABCDEF"), 3, "simple offset works");
    is(unpack("x3.", "ABC"), 3, "simple offset at end of string works");
    is(unpack("x3.0", "ABC"), 0, "self offset is 0");
    is(unpack("x3(x2.)", "ABCDEF"), 2, "offset is relative to inner group");
    is(unpack("x3(X2.)", "ABCDEF"), -2,
       "negative offset relative to inner group");
    is(unpack("x3(X2.2)", "ABCDEF"), 1, "offset is relative to inner group");
    is(unpack("x3(x2.0)", "ABCDEF"), 0, "self offset in group is still 0");
    is(unpack("x3(x2.2)", "ABCDEF"), 5, "offset counts groups");
    is(unpack("x3(x2.*)", "ABCDEF"), 5, "star offset is relative to start");

    my $high = chr(8188) x 6;
    is(unpack("x3(x2.)", $high), 2, "utf8 offset is relative to inner group");
    is(unpack("x3(X2.)", $high), -2,
       "utf8 negative offset relative to inner group");
    is(unpack("x3(X2.2)", $high), 1, "utf8 offset counts groups");
    is(unpack("x3(x2.0)", $high), 0, "utf8 self offset in group is still 0");
    is(unpack("x3(x2.2)", $high), 5, "utf8 offset counts groups");
    is(unpack("x3(x2.*)", $high), 5, "utf8 star offset is relative to start");

    is(unpack("U0x3(x2.)", $high), 2,
       "U0 mode utf8 offset is relative to inner group");
    is(unpack("U0x3(X2.)", $high), -2,
       "U0 mode utf8 negative offset relative to inner group");
    is(unpack("U0x3(X2.2)", $high), 1,
       "U0 mode utf8 offset counts groups");
    is(unpack("U0x3(x2.0)", $high), 0,
       "U0 mode utf8 self offset in group is still 0");
    is(unpack("U0x3(x2.2)", $high), 5,
       "U0 mode utf8 offset counts groups");
    is(unpack("U0x3(x2.*)", $high), 5,
       "U0 mode utf8 star offset is relative to start");

    is(unpack("x3(x2.!)", $high), 2*3,
       "utf8 offset is relative to inner group");
    is(unpack("x3(X2.!)", $high), -2*3,
       "utf8 negative offset relative to inner group");
    is(unpack("x3(X2.!2)", $high), 1*3,
       "utf8 offset counts groups");
    is(unpack("x3(x2.!0)", $high), 0,
       "utf8 self offset in group is still 0");
    is(unpack("x3(x2.!2)", $high), 5*3,
       "utf8 offset counts groups");
    is(unpack("x3(x2.!*)", $high), 5*3,
       "utf8 star offset is relative to start");

    is(unpack("U0x3(x2.!)", $high), 2,
       "U0 mode utf8 offset is relative to inner group");
    is(unpack("U0x3(X2.!)", $high), -2,
       "U0 mode utf8 negative offset relative to inner group");
    is(unpack("U0x3(X2.!2)", $high), 1,
       "U0 mode utf8 offset counts groups");
    is(unpack("U0x3(x2.!0)", $high), 0,
       "U0 mode utf8 self offset in group is still 0");
    is(unpack("U0x3(x2.!2)", $high), 5,
       "U0 mode utf8 offset counts groups");
    is(unpack("U0x3(x2.!*)", $high), 5,
       "U0 mode utf8 star offset is relative to start");
}
{
    # Testing pack . and .!
    is(pack("(a)5 .", 1..5, 3), "123", ". relative to string start, shorten");
    eval { () = pack("(a)5 .", 1..5, -3) };
    like($@, qr{'\.' outside of string in pack}, "Proper error message");
    is(pack("(a)5 .", 1..5, 8), "12345\x00\x00\x00",
       ". relative to string start, extend");
    is(pack("(a)5 .", 1..5, 5), "12345", ". relative to string start, keep");

    is(pack("(a)5 .0", 1..5, -3), "12",
       ". relative to string current, shorten");
    is(pack("(a)5 .0", 1..5, 2), "12345\x00\x00",
       ". relative to string current, extend");
    is(pack("(a)5 .0", 1..5, 0), "12345",
       ". relative to string current, keep");

    is(pack("(a)5 (.)", 1..5, -3), "12",
       ". relative to group, shorten");
    is(pack("(a)5 (.)", 1..5, 2), "12345\x00\x00",
       ". relative to group, extend");
    is(pack("(a)5 (.)", 1..5, 0), "12345",
       ". relative to group, keep");

    is(pack("(a)3 ((a)2 .)", 1..5, -2), "1",
       ". relative to group, shorten");
    is(pack("(a)3 ((a)2 .)", 1..5, 2), "12345",
       ". relative to group, keep");
    is(pack("(a)3 ((a)2 .)", 1..5, 4), "12345\x00\x00",
       ". relative to group, extend");

    is(pack("(a)3 ((a)2 .2)", 1..5, 2), "12",
       ". relative to counted group, shorten");
    is(pack("(a)3 ((a)2 .2)", 1..5, 7), "12345\x00\x00",
       ". relative to counted group, extend");
    is(pack("(a)3 ((a)2 .2)", 1..5, 5), "12345",
       ". relative to counted group, keep");

    is(pack("(a)3 ((a)2 .*)", 1..5, 2), "12",
       ". relative to start, shorten");
    is(pack("(a)3 ((a)2 .*)", 1..5, 7), "12345\x00\x00",
       ". relative to start, extend");
    is(pack("(a)3 ((a)2 .*)", 1..5, 5), "12345",
       ". relative to start, keep");

    is(pack('(a)5 (. @2 a)', 1..5, -3, "a"), "12\x00\x00a",
       ". based shrink properly updates group starts");

    is(pack("(W)3 ((W)2 .)", 0x301..0x305, -2), "\x{301}",
       "utf8 . relative to group, shorten");
    is(pack("(W)3 ((W)2 .)", 0x301..0x305, 2),
       "\x{301}\x{302}\x{303}\x{304}\x{305}",
       "utf8 . relative to group, keep");
    is(pack("(W)3 ((W)2 .)", 0x301..0x305, 4),
       "\x{301}\x{302}\x{303}\x{304}\x{305}\x00\x00",
       "utf8 . relative to group, extend");

    is(pack("(W)3 ((W)2 .!)", 0x301..0x305, -2), "\x{301}\x{302}",
       "utf8 . relative to group, shorten");
    is(pack("(W)3 ((W)2 .!)", 0x301..0x305, 4),
       "\x{301}\x{302}\x{303}\x{304}\x{305}",
       "utf8 . relative to group, keep");
    is(pack("(W)3 ((W)2 .!)", 0x301..0x305, 6),
       "\x{301}\x{302}\x{303}\x{304}\x{305}\x00\x00",
       "utf8 . relative to group, extend");

    is(pack('(W)5 (. @2 a)', 0x301..0x305, -3, "a"),
       "\x{301}\x{302}\x00\x00a",
       "utf8 . based shrink properly updates group starts");
}
{
    # Testing @!
    is(pack('a* @3',  "abcde"), "abc", 'Test basic @');
    is(pack('a* @!3', "abcde"), "abc", 'Test basic @!');
    is(pack('a* @2', "\x{301}\x{302}\x{303}\x{304}\x{305}"), "\x{301}\x{302}",
       'Test basic utf8 @');
    is(pack('a* @!2', "\x{301}\x{302}\x{303}\x{304}\x{305}"), "\x{301}",
       'Test basic utf8 @!');

    is(unpack('@4 a*',  "abcde"), "e", 'Test basic @');
    is(unpack('@!4 a*', "abcde"), "e", 'Test basic @!');
    is(unpack('@4 a*',  "\x{301}\x{302}\x{303}\x{304}\x{305}"), "\x{305}",
       'Test basic utf8 @');
    is(unpack('@!4 a*', "\x{301}\x{302}\x{303}\x{304}\x{305}"),
       "\x{303}\x{304}\x{305}", 'Test basic utf8 @!');
}
{
    #50256
    # This test is for the bit pattern "\x61\x62", which is ASCII "ab"
    my ($v) = split //, unpack ('(B)*', native_to_uni('ab'));
    is($v, 0); # Doesn't SEGV :-)
}
{
    #73814
    my $x = runperl( prog => 'print split( /,/, unpack(q(%2H*), q(hello world))), qq(\n)' );
    is($x, "0\n", "split /a/, unpack('%2H*'...) didn't crash");

    my $y = runperl( prog => 'print split( /,/, unpack(q(%32u*), q(#,3,Q)), qq(\n)), qq(\n)' );
    is($y, "0\n", "split /a/, unpack('%32u*'...) didn't crash");
}

#90160
is(eval { () = unpack "C0 U*", ""; "ok" }, "ok",
  'medial U* on empty string');

package o {
    use overload
        '""' => sub { ++$o::str; "42" },
        '0+' => sub { ++$o::num; 42 };
}
is pack("c", bless [], "o"), chr(42), 'overloading called';
is $o::str, undef, 'pack "c" does not call string overloading';
is $o::num, 1,     'pack "c" does call num overloading';

#[perl #123874]: argument underflow leads to corrupt length
eval q{ pack "pi/x" };
ok(1, "argument underflow did not crash");

{
    # [perl #126325] pack [hH] with a unicode string
    # the hex encoders would read past the end of the string, using
    # invalid source bytes
    my $twenty_nuls = "\0" x 20;
    # This is the case that failed
    is(pack("WH40", 0x100, ""), "\x{100}$twenty_nuls",
       "check pack H zero fills (utf8 target)");
    my $up_nul = "\0";

    utf8::upgrade($up_nul);
    # check the other combinations too
    is(pack("WH40", 0x100, $up_nul), "\x{100}$twenty_nuls",
       "check pack H zero fills (utf8 target/source)");
    is(pack("H40", ""), $twenty_nuls,
       "check pack H zero fills (utf8 none)");
    is(pack("H40", $up_nul), $twenty_nuls,
       "check pack H zero fills (utf8 source)");
}

SKIP:
{
    # [perl #129149] the code below would write one past the end of the output
    # buffer, only detected by ASAN, not by valgrind
    skip "ASCII-centric test",1 if $::IS_EBCDIC;
    $Config{ivsize} >= 8
      or skip "[perl #129149] need 64-bit for this test", 1;
    fresh_perl_is(<<'EOS', "ok\n", { stderr => 1 }, "pack W overflow");
print pack("ucW", "0000", 0, 140737488355327) eq "\$,#`P,```\n\0\x{7fffffffffff}"
 ? "ok\n" : "not ok\n";
EOS
}

SKIP:
{
  # [perl #131844] pointer addition overflow
    $Config{ptrsize} == 4
      or skip "[perl #131844] need 32-bit build for this test", 4;
    # prevent ASAN just crashing on the allocation failure
    local $ENV{ASAN_OPTIONS} = $ENV{ASAN_OPTIONS};
    $ENV{ASAN_OPTIONS} .= ",allocator_may_return_null=1";
    fresh_perl_like('pack "f999999999"', qr/Out of memory during pack/, { stderr => 1 },
		    "pointer addition overflow");

    # integer (STRLEN) overflow from addition of glen to current length
    fresh_perl_like('pack "c10f1073741823"', qr/Out of memory during pack/, { stderr => 1 },
		    "integer overflow calculating allocation (addition)");

    fresh_perl_like('pack "W10f536870913", 256', qr/Out of memory during pack/, { stderr => 1 },
		    "integer overflow calculating allocation (utf8)");

    fresh_perl_like('pack "c10f1073741824"', qr/Out of memory during pack/, { stderr => 1 },
		    "integer overflow calculating allocation (multiply)");
}

{
    # [perl #132655] heap-buffer-overflow READ of size 11
    # only expect failure under ASAN (and maybe valgrind)
    fresh_perl_is('0.0 + unpack("u", "ab")', "", { stderr => 1 },
                  "ensure unpack u of invalid data nul terminates result");
}

{
	# [GH #16319] SEGV caused by recursion
	my $x = eval { pack "[" x 1_000_000 };
	like("$@", qr{No group ending character \Q']'\E found in template},
			"many opening brackets should not smash the stack");

	$x = eval { pack "[(][)]" };
	like("$@", qr{Mismatched brackets in template},
			"should match brackets correctly even without recursion");
}
