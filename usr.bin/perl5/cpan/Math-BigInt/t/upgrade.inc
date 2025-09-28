# include this file into another for subclass testing

# This file is nearly identical to bigintpm.t, except that certain results are
# _requird_ to be different due to "upgrading" or "promoting" to
# Math::BigFloat. The reverse is not true. Any unmarked results can be either
# Math::BigInt or Math::BigFloat, depending on how good the internal
# optimization is (e.g., it is usually desirable to have 2 ** 2 return a
# Math::BigInt, not a Math::BigFloat).

# Results that are required to be Math::BigFloat are marked with C<^> at the
# end.

# Please note that the testcount goes up by two for each extra result marked
# with ^, since then we test whether it has the proper class and that it left
# the upgrade variable alone.

use strict;
use warnings;

our ($CLASS, $LIB, $EXPECTED_CLASS);

##############################################################################
# for testing inheritance of _swap

package Math::Foo;

use Math::BigInt lib => $main::LIB;
our @ISA = ('Math::BigInt');

use overload
  # customized overload for sub, since original does not use swap there
  '-'     =>      sub { my @a = ref($_[0])->_swap(@_);
                        $a[0]->bsub($a[1]);
                      };

sub _swap {
    # a fake _swap, which reverses the params
    my $self = shift;           # for override in subclass
    if ($_[2]) {
        my $c = ref ($_[0] ) || 'Math::Foo';
        return ( $_[0]->copy(), $_[1] );
    } else {
        return ( Math::Foo->new($_[1]), $_[0] );
    }
}

##############################################################################
package main;

is($CLASS->config('lib'), $LIB, "$CLASS->config('lib')");

my ($x, $y, $z, @args, $a, $m, $e, $try, $got, $want, $exp);
my ($f, $round_mode, $expected_class);

while (<DATA>) {
    s/#.*$//;                   # remove comments
    s/\s+$//;                   # remove trailing whitespace
    next unless length;         # skip empty lines

    if (s/^&//) {
        $f = $_;
        next;
    }

    if (/^\$/) {
        $round_mode = $_;
        $round_mode =~ s/^\$/$CLASS\->/;
        next;
    }

    @args = split(/:/, $_, 99);
    $want = pop(@args);
    $expected_class = $CLASS;

    if ($want =~ /\^$/) {
        $expected_class = $EXPECTED_CLASS;
        $want =~ s/\^$//;
    }

    $try = qq|\$x = $CLASS->new("$args[0]");|;
    if ($f eq "bnorm") {
        $try = qq|\$x = $CLASS->bnorm("$args[0]");|;
    } elsif ($f =~ /^is_(zero|one|odd|even||(non_)?(negative|positive)|nan|int)$/) {
        $try .= " \$x->$f();";
    } elsif ($f =~ /^(to|as)_(hex|oct|bin)$/) {
        $try .= " \$x->$f();";
    } elsif ($f eq "is_inf") {
        $try .= " \$x->is_inf('$args[1]');";
    } elsif ($f eq "binf") {
        $try .= " \$x->binf('$args[1]');";
    } elsif ($f eq "bone") {
        $try .= " \$x->bone('$args[1]');";
    # some unary ops
    } elsif ($f =~ /^b(nan|floor|ceil|int|sstr|neg|abs|inc|dec|not|sqrt)$/) {
        $try .= " \$x->$f();";
    } elsif ($f eq "length") {
        $try .= ' $x->length();';
    } elsif ($f eq "exponent") {
        # ->bstr() to see if an object is returned
        $try .= ' $x = $x->exponent()->bstr();';
    } elsif ($f eq "mantissa") {
        # ->bstr() to see if an object is returned
        $try .= ' $x = $x->mantissa()->bstr();';
    } elsif ($f eq "parts") {
        $try .= ' ($m, $e) = $x->parts();';
        # ->bstr() to see if an object is returned
        $try .= ' $m = $m->bstr(); $m = "NaN" if !defined $m;';
        $try .= ' $e = $e->bstr(); $e = "NaN" if !defined $e;';
        $try .= ' "$m,$e";';
    } else {
        if ($args[1] !~ /\./) {
            $try .= qq| \$y = $CLASS->new("$args[1]");|;
        } else {
            $try .= qq| \$y = $EXPECTED_CLASS->new("$args[1]");|;
        }
        if ($f eq "bcmp") {
            $try .= ' $x->bcmp($y);';
        } elsif ($f eq "bacmp") {
            $try .= ' $x->bacmp($y);';
        } elsif ($f eq "bround") {
            $try .= " $round_mode; \$x->bround(\$y);";
        } elsif ($f eq "broot") {
            $try .= " \$x->broot(\$y);";
        } elsif ($f eq "badd") {
            $try .= ' $x + $y;';
        } elsif ($f eq "bsub") {
            $try .= ' $x - $y;';
        } elsif ($f eq "bmul") {
            $try .= ' $x * $y;';
        } elsif ($f eq "bdiv") {
            $try .= ' $x / $y;';
        } elsif ($f eq "bdiv-list") {
            $try .= ' join(",", $x->bdiv($y));';
            # overload via x=
        } elsif ($f =~ /^.=$/) {
            $try .= " \$x $f \$y;";
            # overload via x
        } elsif ($f =~ /^.$/) {
            $try .= " \$x $f \$y;";
        } elsif ($f eq "bmod") {
            $try .= ' $x % $y;';
        } elsif ($f eq "bgcd") {
            if (defined $args[2]) {
                $try .= qq| \$z = $CLASS->new("$args[2]");|;
            }
            $try .= " $CLASS\::bgcd(\$x, \$y";
            $try .= ", \$z" if defined $args[2];
            $try .= ");";
        } elsif ($f eq "blcm") {
            if (defined $args[2]) {
                $try .= qq| \$z = $CLASS->new("$args[2]");|;
            }
            $try .= " $CLASS\::blcm(\$x, \$y";
            $try .= ", \$z" if defined $args[2];
            $try .= ");";
        } elsif ($f eq "blsft") {
            if (defined $args[2]) {
                $try .= " \$x->blsft(\$y, $args[2]);";
            } else {
                $try .= " \$x << \$y;";
            }
        } elsif ($f eq "brsft") {
            if (defined $args[2]) {
                $try .= " \$x->brsft(\$y, $args[2]);";
            } else {
                $try .= " \$x >> \$y;";
            }
        } elsif ($f eq "band") {
            $try .= " \$x & \$y;";
        } elsif ($f eq "bior") {
            $try .= " \$x | \$y;";
        } elsif ($f eq "bxor") {
            $try .= " \$x ^ \$y;";
        } elsif ($f eq "bpow") {
            $try .= " \$x ** \$y;";
        } elsif ($f eq "digit") {
            $try = qq|\$x = $CLASS->new("$args[0]"); \$x->digit($args[1]);|;
        } else {
            warn "Unknown op '$f'";
        }
    }                           # end else all other ops

    $got = eval $try;
    print "# Error: $@\n" if $@;

    # convert hex/binary targets to decimal
    if ($want =~ /^(0x0x|0b0b)/) {
        $want =~ s/^0[xb]//;
        $want = Math::BigInt->new($want)->bstr();
    }
    if ($want eq "") {
        is($got, undef, $try);
    } else {
        # print "try: $try ans: $got $want\n";
        is($got, $want, $try);
        if ($expected_class ne $CLASS) {
            is(ref($got), $expected_class, 'ref($got)');
            is($Math::BigInt::upgrade, 'Math::BigFloat',
               '$Math::BigInt::upgrade');
        }
    }
    # check internal state of number objects
    is_valid($got, $f) if ref $got;
}                               # endwhile data tests
close DATA;

my $warn = '';
$SIG{__WARN__} = sub { $warn = shift; };

# these should not warn

$warn = '';
eval '$z = 3.17 <= $y';
is($z, 1, '$z = 3.17 <= $y');
is($warn, '', 'the code "$z = 3.17 <= $y" issued no warning');

$warn = '';
eval '$z = $y >= 3.17';
is($z, 1, '$z = $y >= 3.17');
is($warn, '', 'the code "$z = $y >= 3.17" issued no warning');

# all tests done

1;

###############################################################################
# sub to check validity of a Math::BigInt internally, to ensure that no op
# leaves a number object in an invalid state (f.i. "-0")

sub is_valid {
    my ($x, $f, $c) = @_;

    # The checks here are loosened a bit to allow Math::BigInt or
    # Math::BigFloat objects to pass

    my $e = 0;         # error?

    # ok as reference?
    # $e = "Not a reference to $c" if (ref($x) || '') ne $c;

    # has ok sign?
    $e = "Illegal sign $x->{sign} (expected: '+', '-', '-inf', '+inf' or 'NaN'"
      if $e eq '0' && $x->{sign} !~ /^(\+|-|\+inf|-inf|NaN)$/;

    $e = "-0 is invalid!" if $e ne '0' && $x->{sign} eq '-' && $x == 0;
    #$e = $LIB->_check($x->{value}) if $e eq '0';

    # test done, see if error did crop up
    if ($e eq '0') {
        pass('is a valid object');
        return;
    }

    fail($e . " after op '$f'");
}

__DATA__

&.=
1234:-345:1234-345

&+=
1:2:3
-1:-2:-3

&-=
1:2:-1
-1:-2:1

&*=
2:3:6
-1:5:-5

&%=
100:3:1
8:9:8

&/=
100:3:33.33333333333333333333333333333333333333
-8:2:-4

&|=
2:1:3

&&=
5:7:5

&^=
5:7:2

&is_negative
0:0
-1:1
1:0
+inf:0
-inf:1
NaNneg:0

&is_positive
0:0
-1:0
1:1
+inf:1
-inf:0
NaNneg:0

&is_non_negative
0:1
-1:0
1:1
+inf:1
-inf:0
NaN:0

&is_non_positive
0:1
-1:1
1:0
+inf:0
-inf:1
NaN:0

&is_odd
abc:0
0:0
1:1
3:1
-1:1
-3:1
10000001:1
10000002:0
2:0
120:0
121:1

&is_int
NaN:0
inf:0
-inf:0
1:1
12:1
123e12:1

&is_even
abc:0
0:1
1:0
3:0
-1:0
-3:0
10000001:0
10000002:1
2:1
120:1
121:0

&bacmp
+0:-0:0
+0:+1:-1
-1:+1:0
+1:-1:0
-1:+2:-1
+2:-1:1
-123456789:+987654321:-1
+123456789:-987654321:-1
+987654321:+123456789:1
-987654321:+123456789:1
-123:+4567889:-1
# NaNs
acmpNaN:123:
123:acmpNaN:
acmpNaN:acmpNaN:
# infinity
+inf:+inf:0
-inf:-inf:0
+inf:-inf:0
-inf:+inf:0
+inf:123:1
-inf:123:1
+inf:-123:1
-inf:-123:1
# return undef
+inf:NaN:
NaN:inf:
-inf:NaN:
NaN:-inf:

&bnorm
123:123
12.3:12.3^
# binary input
0babc:NaN
0b123:NaN
0b0:0
-0b0:0
-0b1:-1
0b0001:1
0b001:1
0b011:3
0b101:5
0b1001:9
0b10001:17
0b100001:33
0b1000001:65
0b10000001:129
0b100000001:257
0b1000000001:513
0b10000000001:1025
0b100000000001:2049
0b1000000000001:4097
0b10000000000001:8193
0b100000000000001:16385
0b1000000000000001:32769
0b10000000000000001:65537
0b100000000000000001:131073
0b1000000000000000001:262145
0b10000000000000000001:524289
0b100000000000000000001:1048577
0b1000000000000000000001:2097153
0b10000000000000000000001:4194305
0b100000000000000000000001:8388609
0b1000000000000000000000001:16777217
0b10000000000000000000000001:33554433
0b100000000000000000000000001:67108865
0b1000000000000000000000000001:134217729
0b10000000000000000000000000001:268435457
0b100000000000000000000000000001:536870913
0b1000000000000000000000000000001:1073741825
0b10000000000000000000000000000001:2147483649
0b100000000000000000000000000000001:4294967297
0b1000000000000000000000000000000001:8589934593
0b10000000000000000000000000000000001:17179869185
0b1_0_1:5
0b0_0_0_1:1
# hex input
-0x0:0
0xabcdefgh:NaN
0x1234:4660
0xabcdef:11259375
-0xABCDEF:-11259375
-0x1234:-4660
0x12345678:305419896
0x1_2_3_4_56_78:305419896
0xa_b_c_d_e_f:11259375
0x9:9
0x11:17
0x21:33
0x41:65
0x81:129
0x101:257
0x201:513
0x401:1025
0x801:2049
0x1001:4097
0x2001:8193
0x4001:16385
0x8001:32769
0x10001:65537
0x20001:131073
0x40001:262145
0x80001:524289
0x100001:1048577
0x200001:2097153
0x400001:4194305
0x800001:8388609
0x1000001:16777217
0x2000001:33554433
0x4000001:67108865
0x8000001:134217729
0x10000001:268435457
0x20000001:536870913
0x40000001:1073741825
0x80000001:2147483649
0x100000001:4294967297
0x200000001:8589934593
0x400000001:17179869185
0x800000001:34359738369
# inf input
inf:inf
+inf:inf
-inf:-inf
0inf:NaN
# abnormal input
:NaN
abc:NaN
   1 a:NaN
1bcd2:NaN
11111b:NaN
+1z:NaN
-1z:NaN
# only one underscore between two digits
_123:NaN
_123_:NaN
1E_12:NaN
1_E_12:NaN
+_1E12:NaN
+0_1E2:100
+0_0_1E2:100
-0_0_1E2:-100
-0_0_1E+0_0_2:-100
E1:NaN
E23:NaN
1.23E1:12.3^
1.23E-1:0.123^
# bug with two E's in number being valid
1e2e3:NaN
1e2r:NaN
1e2.0:NaN
# leading zeros
012:12
0123:123
01234:1234
012345:12345
0123456:123456
01234567:1234567
012345678:12345678
0123456789:123456789
01234567891:1234567891
012345678912:12345678912
0123456789123:123456789123
01234567891234:1234567891234
# normal input
0:0
+0:0
+00:0
+000:0
000000000000000000:0
-0:0
-0000:0
+1:1
+01:1
+001:1
+00000100000:100000
123456789:123456789
-1:-1
-01:-1
-001:-1
-123456789:-123456789
-00000100000:-100000
1_2_3:123
10000000000E-1_0:1
1E2:100
1E1:10
1E0:1
1.23E2:123
100E-1:10
# floating point input
# .2e2:20
1.E3:1000
1.01E2:101
1010E-1:101
-1010E0:-1010
-1010E1:-10100
1234.00:1234
# non-integer numbers
-1010E-2:-10.1^
-1.01E+1:-10.1^
-1.01E-1:-0.101^

&bnan
1:NaN
2:NaN
abc:NaN

&bone
2:+:1
2:-:-1
boneNaN:-:-1
boneNaN:+:1
2:abc:1
3::1

&binf
1:+:inf
2:-:-inf
3:+inf:inf

&is_nan
123:0
abc:1
NaN:1
-123:0

&is_inf
+inf::1
-inf::1
abc::0
1::0
NaN::0
-1::0
+inf:-:0
+inf:+:1
-inf:-:1
-inf:+:0
+iNfInItY::1
-InFiNiTy::1

&blsft
abc:abc:NaN
+2:+2:8
+1:+32:4294967296
+1:+48:281474976710656
+8:-2:NaN
# exercise base 10
+12345:4:10:123450000
-1234:0:10:-1234
+1234:0:10:1234
+2:2:10:200
+12:2:10:1200
+1234:-3:10:NaN
1234567890123:12:10:1234567890123000000000000

&brsft
abc:abc:NaN
+8:+2:2
+4294967296:+32:1
+281474976710656:+48:1
+2:-2:NaN
# exercise base 10
-1234:0:10:-1234
+1234:0:10:1234
+200:2:10:2
+1234:3:10:1.234
+1234:2:10:12.34
+1234:-3:10:NaN
310000:4:10:31
12300000:5:10:123
1230000000000:10:10:123
09876123456789067890:12:10:9876123.45678906789
1234561234567890123:13:10:123456.1234567890123

&bsstr
1e+34:1e+34
123.456E3:123456e+0
100:1e+2
abc:NaN

&bneg
bnegNaN:NaN
+inf:-inf
-inf:inf
abd:NaN
0:0
1:-1
-1:1
+123456789:-123456789
-123456789:123456789

&babs
babsNaN:NaN
+inf:inf
-inf:inf
0:0
1:1
-1:1
+123456789:123456789
-123456789:123456789

&bcmp
bcmpNaN:bcmpNaN:
bcmpNaN:0:
0:bcmpNaN:
0:0:0
-1:0:-1
0:-1:1
1:0:1
0:1:-1
-1:1:-1
1:-1:1
-1:-1:0
1:1:0
123:123:0
123:12:1
12:123:-1
-123:-123:0
-123:-12:-1
-12:-123:1
123:124:-1
124:123:1
-123:-124:1
-124:-123:-1
100:5:1
-123456789:987654321:-1
+123456789:-987654321:1
-987654321:123456789:-1
-inf:5432112345:-1
+inf:5432112345:1
-inf:-5432112345:-1
+inf:-5432112345:1
+inf:+inf:0
-inf:-inf:0
+inf:-inf:1
-inf:+inf:-1
5:inf:-1
5:inf:-1
-5:-inf:1
-5:-inf:1
# return undef
+inf:NaN:
NaN:inf:
-inf:NaN:
NaN:-inf:

&binc
abc:NaN
+inf:inf
-inf:-inf
+0:1
+1:2
-1:0

&bdec
abc:NaN
+inf:inf
-inf:-inf
+0:-1
+1:0
-1:-2

&badd
abc:abc:NaN
abc:0:NaN
+0:abc:NaN
+inf:-inf:NaN
-inf:+inf:NaN
+inf:+inf:inf
-inf:-inf:-inf
baddNaN:+inf:NaN
baddNaN:+inf:NaN
+inf:baddNaN:NaN
-inf:baddNaN:NaN
0:0:0
1:0:1
0:1:1
1:1:2
-1:0:-1
0:-1:-1
-1:-1:-2
-1:+1:0
+1:-1:0
+9:+1:10
+99:+1:100
+999:+1:1000
+9999:+1:10000
+99999:+1:100000
+999999:+1:1000000
+9999999:+1:10000000
+99999999:+1:100000000
+999999999:+1:1000000000
+9999999999:+1:10000000000
+99999999999:+1:100000000000
+10:-1:9
+100:-1:99
+1000:-1:999
+10000:-1:9999
+100000:-1:99999
+1000000:-1:999999
+10000000:-1:9999999
+100000000:-1:99999999
+1000000000:-1:999999999
+10000000000:-1:9999999999
+123456789:987654321:1111111110
-123456789:987654321:864197532
-123456789:-987654321:-1111111110
+123456789:-987654321:-864197532
2:2.5:4.5^
-123:-1.5:-124.5^
-1.2:1:-0.2^

&bsub
abc:abc:NaN
abc:+0:NaN
+0:abc:NaN
+inf:-inf:inf
-inf:+inf:-inf
+inf:+inf:NaN
-inf:-inf:NaN
+0:+0:0
+1:+0:1
+0:+1:-1
+1:+1:0
-1:+0:-1
+0:-1:1
-1:-1:0
-1:+1:-2
+1:-1:2
+9:+1:8
+99:+1:98
+999:+1:998
+9999:+1:9998
+99999:+1:99998
+999999:+1:999998
+9999999:+1:9999998
+99999999:+1:99999998
+999999999:+1:999999998
+9999999999:+1:9999999998
+99999999999:+1:99999999998
+10:-1:11
+100:-1:101
+1000:-1:1001
+10000:-1:10001
+100000:-1:100001
+1000000:-1:1000001
+10000000:-1:10000001
+100000000:-1:100000001
+1000000000:-1:1000000001
+10000000000:-1:10000000001
+123456789:+987654321:-864197532
-123456789:+987654321:-1111111110
-123456789:-987654321:864197532
+123456789:-987654321:1111111110

&bmul
abc:abc:NaN
abc:+0:NaN
+0:abc:NaN
NaNmul:+inf:NaN
NaNmul:-inf:NaN
-inf:NaNmul:NaN
+inf:NaNmul:NaN
+inf:+inf:inf
+inf:-inf:-inf
-inf:+inf:-inf
-inf:-inf:inf
+0:+0:0
+0:+1:0
+1:+0:0
+0:-1:0
-1:+0:0
123456789123456789:0:0
0:123456789123456789:0
-1:-1:1
-1:+1:-1
+1:-1:-1
+1:+1:1
+2:+3:6
-2:+3:-6
+2:-3:-6
-2:-3:6
111:111:12321
10101:10101:102030201
1001001:1001001:1002003002001
100010001:100010001:10002000300020001
10000100001:10000100001:100002000030000200001
11111111111:9:99999999999
22222222222:9:199999999998
33333333333:9:299999999997
44444444444:9:399999999996
55555555555:9:499999999995
66666666666:9:599999999994
77777777777:9:699999999993
88888888888:9:799999999992
99999999999:9:899999999991
+25:+25:625
+12345:+12345:152399025
+99999:+11111:1111088889
9999:10000:99990000
99999:100000:9999900000
999999:1000000:999999000000
9999999:10000000:99999990000000
99999999:100000000:9999999900000000
999999999:1000000000:999999999000000000
9999999999:10000000000:99999999990000000000
99999999999:100000000000:9999999999900000000000
999999999999:1000000000000:999999999999000000000000
9999999999999:10000000000000:99999999999990000000000000
99999999999999:100000000000000:9999999999999900000000000000
999999999999999:1000000000000000:999999999999999000000000000000
9999999999999999:10000000000000000:99999999999999990000000000000000
99999999999999999:100000000000000000:9999999999999999900000000000000000
999999999999999999:1000000000000000000:999999999999999999000000000000000000
9999999999999999999:10000000000000000000:99999999999999999990000000000000000000
3:3.5:10.5^
3.5:3:10.5^

&bdiv-list
100:20:5,0
4095:4095:1,0
-4095:-4095:1,0
4095:-4095:-1,0
-4095:4095:-1,0
123:2:61,1
9:5:1,4
9:4:2,1
# inf handling and general remainder
5:8:0,5
0:8:0,0
11:2:5,1
11:-2:-6,-1
-11:2:-6,1
# see table in documentation in MBI
0:inf:0,0
0:-inf:0,0
5:inf:0,5
5:-inf:-1,-inf
-5:inf:-1,inf
-5:-inf:0,-5
inf:5:inf,NaN
-inf:5:-inf,NaN
inf:-5:-inf,NaN
-inf:-5:inf,NaN
5:5:1,0
-5:-5:1,0
inf:inf:NaN,NaN
-inf:-inf:NaN,NaN
-inf:inf:NaN,NaN
inf:-inf:NaN,NaN
8:0:inf,8
inf:0:inf,inf
# exceptions to remainder rule
-8:0:-inf,-8
-inf:0:-inf,-inf
0:0:NaN,0

&bdiv
abc:abc:NaN
abc:1:NaN
1:abc:NaN
0:0:NaN
# inf handling (see table in doc)
0:inf:0
0:-inf:0
5:inf:0
5:-inf:-1
-5:inf:-1
-5:-inf:0
inf:5:inf
-inf:5:-inf
inf:-5:-inf
-inf:-5:inf
5:5:1
-5:-5:1
inf:inf:NaN
-inf:-inf:NaN
-inf:inf:NaN
inf:-inf:NaN
8:0:inf
inf:0:inf
-8:0:-inf
-inf:0:-inf
11:2:5.5^
-11:-2:5.5^
-11:2:-5.5^
11:-2:-5.5^
0:1:0
0:-1:0
1:1:1
-1:-1:1
1:-1:-1
-1:1:-1
1:2:0.5^
2:1:2
1000000000:9:111111111.1111111111111111111111111111111^
2000000000:9:222222222.2222222222222222222222222222222^
3000000000:9:333333333.3333333333333333333333333333333^
4000000000:9:444444444.4444444444444444444444444444444^
5000000000:9:555555555.5555555555555555555555555555556^
6000000000:9:666666666.6666666666666666666666666666667^
7000000000:9:777777777.7777777777777777777777777777778^
8000000000:9:888888888.8888888888888888888888888888889^
9000000000:9:1000000000
35500000:113:314159.2920353982300884955752212389380531^
71000000:226:314159.2920353982300884955752212389380531^
106500000:339:314159.2920353982300884955752212389380531^
1000000000:3:333333333.3333333333333333333333333333333^
+10:+5:2
+100:+4:25
+1000:+8:125
+10000:+16:625
999999999999:9:111111111111
999999999999:99:10101010101
999999999999:999:1001001001
999999999999:9999:100010001
999999999999999:99999:10000100001
+1111088889:99999:11111
-5:-3:1.666666666666666666666666666666666666667^
-5:3:-1.666666666666666666666666666666666666667^
4:3:1.333333333333333333333333333333333333333^
4:-3:-1.333333333333333333333333333333333333333^
1:3:0.3333333333333333333333333333333333333333^
1:-3:-0.3333333333333333333333333333333333333333^
-2:-3:0.6666666666666666666666666666666666666667^
-2:3:-0.6666666666666666666666666666666666666667^
8:5:1.6^
-8:5:-1.6^
14:-3:-4.666666666666666666666666666666666666667^
-14:3:-4.666666666666666666666666666666666666667^
-14:-3:4.666666666666666666666666666666666666667^
14:3:4.666666666666666666666666666666666666667^
# bug in Calc with '99999' vs $BASE-1
#10000000000000000000000000000000000000000000000000000000000000000000000000000000000:10000000375084540248994272022843165711074:999999962491547381984643365663244474111576
12:24:0.5^

&bmod
# inf handling, see table in doc
0:inf:0
0:-inf:0
5:inf:5
5:-inf:-inf
-5:inf:inf
-5:-inf:-5
inf:5:NaN
-inf:5:NaN
inf:-5:NaN
-inf:-5:NaN
5:5:0
-5:-5:0
inf:inf:NaN
-inf:-inf:NaN
-inf:inf:NaN
inf:-inf:NaN
8:0:8
inf:0:inf
-inf:0:-inf
-8:0:-8
0:0:0
abc:abc:NaN
abc:1:abc:NaN
1:abc:NaN
0:1:0
1:0:1
0:-1:0
-1:0:-1
1:1:0
-1:-1:0
1:-1:0
-1:1:0
1:2:1
2:1:0
1000000000:9:1
2000000000:9:2
3000000000:9:3
4000000000:9:4
5000000000:9:5
6000000000:9:6
7000000000:9:7
8000000000:9:8
9000000000:9:0
35500000:113:33
71000000:226:66
106500000:339:99
1000000000:3:1
10:5:0
100:4:0
1000:8:0
10000:16:0
999999999999:9:0
999999999999:99:0
999999999999:999:0
999999999999:9999:0
999999999999999:99999:0
-9:+5:1
+9:-5:-1
-9:-5:-4
-5:3:1
-2:3:1
4:3:1
1:3:1
-5:-3:-2
-2:-3:-2
4:-3:-2
1:-3:-2
4095:4095:0
100041000510123:3:0
152403346:12345:4321
9:5:4

&bgcd
abc:abc:NaN
abc:+0:NaN
+0:abc:NaN
+0:+0:0
+0:+1:1
+1:+0:1
+1:+1:1
+2:+3:1
+3:+2:1
-3:+2:1
100:625:25
4096:81:1
1034:804:2
27:90:56:1
27:90:54:9

&blcm
abc:abc:NaN
abc:+0:NaN
+0:abc:NaN
+0:+0:0
+1:+0:0
+0:+1:0
+27:+90:270
+1034:+804:415668

&band
abc:abc:NaN
abc:0:NaN
0:abc:NaN
1:2:0
3:2:2
+8:+2:0
+281474976710656:0:0
+281474976710656:1:0
+281474976710656:+281474976710656:281474976710656
-2:-3:-4
-1:-1:-1
-6:-6:-6
-7:-4:-8
-7:4:0
-4:7:4
# equal arguments are treated special, so also do some test with unequal ones
0xFFFF:0xFFFF:0x0xFFFF
0xFFFFFF:0xFFFFFF:0x0xFFFFFF
0xFFFFFFFF:0xFFFFFFFF:0x0xFFFFFFFF
0xFFFFFFFFFF:0xFFFFFFFFFF:0x0xFFFFFFFFFF
0xFFFFFFFFFFFF:0xFFFFFFFFFFFF:0x0xFFFFFFFFFFFF
0xF0F0:0xF0F0:0x0xF0F0
0x0F0F:0x0F0F:0x0x0F0F
0xF0F0F0:0xF0F0F0:0x0xF0F0F0
0x0F0F0F:0x0F0F0F:0x0x0F0F0F
0xF0F0F0F0:0xF0F0F0F0:0x0xF0F0F0F0
0x0F0F0F0F:0x0F0F0F0F:0x0x0F0F0F0F
0xF0F0F0F0F0:0xF0F0F0F0F0:0x0xF0F0F0F0F0
0x0F0F0F0F0F:0x0F0F0F0F0F:0x0x0F0F0F0F0F
0xF0F0F0F0F0F0:0xF0F0F0F0F0F0:0x0xF0F0F0F0F0F0
0x0F0F0F0F0F0F:0x0F0F0F0F0F0F:0x0x0F0F0F0F0F0F
0x1F0F0F0F0F0F:0x3F0F0F0F0F0F:0x0x1F0F0F0F0F0F

&bior
abc:abc:NaN
abc:0:NaN
0:abc:NaN
1:2:3
+8:+2:10
+281474976710656:0:281474976710656
+281474976710656:1:281474976710657
+281474976710656:281474976710656:281474976710656
-2:-3:-1
-1:-1:-1
-6:-6:-6
-7:4:-3
-4:7:-1
# equal arguments are treated special, so also do some test with unequal ones
0xFFFF:0xFFFF:0x0xFFFF
0xFFFFFF:0xFFFFFF:0x0xFFFFFF
0xFFFFFFFF:0xFFFFFFFF:0x0xFFFFFFFF
0xFFFFFFFFFF:0xFFFFFFFFFF:0x0xFFFFFFFFFF
0xFFFFFFFFFFFF:0xFFFFFFFFFFFF:0x0xFFFFFFFFFFFF
0:0xFFFF:0x0xFFFF
0:0xFFFFFF:0x0xFFFFFF
0:0xFFFFFFFF:0x0xFFFFFFFF
0:0xFFFFFFFFFF:0x0xFFFFFFFFFF
0:0xFFFFFFFFFFFF:0x0xFFFFFFFFFFFF
0xFFFF:0:0x0xFFFF
0xFFFFFF:0:0x0xFFFFFF
0xFFFFFFFF:0:0x0xFFFFFFFF
0xFFFFFFFFFF:0:0x0xFFFFFFFFFF
0xFFFFFFFFFFFF:0:0x0xFFFFFFFFFFFF
0xF0F0:0xF0F0:0x0xF0F0
0x0F0F:0x0F0F:0x0x0F0F
0xF0F0:0x0F0F:0x0xFFFF
0xF0F0F0:0xF0F0F0:0x0xF0F0F0
0x0F0F0F:0x0F0F0F:0x0x0F0F0F
0x0F0F0F:0xF0F0F0:0x0xFFFFFF
0xF0F0F0F0:0xF0F0F0F0:0x0xF0F0F0F0
0x0F0F0F0F:0x0F0F0F0F:0x0x0F0F0F0F
0x0F0F0F0F:0xF0F0F0F0:0x0xFFFFFFFF
0xF0F0F0F0F0:0xF0F0F0F0F0:0x0xF0F0F0F0F0
0x0F0F0F0F0F:0x0F0F0F0F0F:0x0x0F0F0F0F0F
0x0F0F0F0F0F:0xF0F0F0F0F0:0x0xFFFFFFFFFF
0xF0F0F0F0F0F0:0xF0F0F0F0F0F0:0x0xF0F0F0F0F0F0
0x0F0F0F0F0F0F:0x0F0F0F0F0F0F:0x0x0F0F0F0F0F0F
0x0F0F0F0F0F0F:0xF0F0F0F0F0F0:0x0xFFFFFFFFFFFF
0x1F0F0F0F0F0F:0xF0F0F0F0F0F0:0x0xFFFFFFFFFFFF

&bxor
abc:abc:NaN
abc:0:NaN
0:abc:NaN
1:2:3
+8:+2:10
+281474976710656:0:281474976710656
+281474976710656:1:281474976710657
+281474976710656:281474976710656:0
-2:-3:3
-1:-1:0
-6:-6:0
-7:4:-3
-4:7:-5
4:-7:-3
-4:-7:5
# equal arguments are treated special, so also do some test with unequal ones
0xFFFF:0xFFFF:0
0xFFFFFF:0xFFFFFF:0
0xFFFFFFFF:0xFFFFFFFF:0
0xFFFFFFFFFF:0xFFFFFFFFFF:0
0xFFFFFFFFFFFF:0xFFFFFFFFFFFF:0
0:0xFFFF:0x0xFFFF
0:0xFFFFFF:0x0xFFFFFF
0:0xFFFFFFFF:0x0xFFFFFFFF
0:0xFFFFFFFFFF:0x0xFFFFFFFFFF
0:0xFFFFFFFFFFFF:0x0xFFFFFFFFFFFF
0xFFFF:0:0x0xFFFF
0xFFFFFF:0:0x0xFFFFFF
0xFFFFFFFF:0:0x0xFFFFFFFF
0xFFFFFFFFFF:0:0x0xFFFFFFFFFF
0xFFFFFFFFFFFF:0:0x0xFFFFFFFFFFFF
0xF0F0:0xF0F0:0
0x0F0F:0x0F0F:0
0xF0F0:0x0F0F:0x0xFFFF
0xF0F0F0:0xF0F0F0:0
0x0F0F0F:0x0F0F0F:0
0x0F0F0F:0xF0F0F0:0x0xFFFFFF
0xF0F0F0F0:0xF0F0F0F0:0
0x0F0F0F0F:0x0F0F0F0F:0
0x0F0F0F0F:0xF0F0F0F0:0x0xFFFFFFFF
0xF0F0F0F0F0:0xF0F0F0F0F0:0
0x0F0F0F0F0F:0x0F0F0F0F0F:0
0x0F0F0F0F0F:0xF0F0F0F0F0:0x0xFFFFFFFFFF
0xF0F0F0F0F0F0:0xF0F0F0F0F0F0:0
0x0F0F0F0F0F0F:0x0F0F0F0F0F0F:0
0x0F0F0F0F0F0F:0xF0F0F0F0F0F0:0x0xFFFFFFFFFFFF

&bnot
abc:NaN
+0:-1
+8:-9
+281474976710656:-281474976710657
-1:0
-2:1
-12:11

&digit
0:0:0
12:0:2
12:1:1
123:0:3
123:1:2
123:2:1
123:-1:1
123:-2:2
123:-3:3
123456:0:6
123456:1:5
123456:2:4
123456:3:3
123456:4:2
123456:5:1
123456:-1:1
123456:-2:2
123456:-3:3
100000:-3:0
100000:0:0
100000:1:0

&mantissa
abc:NaN
1e4:1
2e0:2
123:123
-1:-1
-2:-2
+inf:inf
-inf:-inf

&exponent
abc:NaN
1e4:4
2e0:0
123:0
-1:0
-2:0
0:0
+inf:inf
-inf:inf

&parts
abc:NaN,NaN
1e4:1,4
2e0:2,0
123:123,0
-1:-1,0
-2:-2,0
0:0,0
+inf:inf,inf
-inf:-inf,inf

&bpow
abc:12:NaN
12:abc:NaN
0:0:1
0:1:0
0:2:0
0:-1:inf
0:-2:inf
1:0:1
1:1:1
1:2:1
1:3:1
1:-1:1
1:-2:1
1:-3:1
2:0:1
2:1:2
2:2:4
2:3:8
3:3:27
2:-1:0.5^
-2:-1:-0.5^
2:-2:0.25^
# Y is even => result positive
-2:-2:0.25^
# Y is odd => result negative
-2:-3:-0.125^
+inf:1234500012:inf
-inf:1234500012:inf
-inf:1234500013:-inf
+inf:-12345000123:0
-inf:-12345000123:0
# 1 ** -x => 1 / (1 ** x)
-1:0:1
-2:0:1
-1:1:-1
-1:2:1
-1:3:-1
-1:4:1
-1:5:-1
-1:-1:-1
-1:-2:1
-1:-3:-1
-1:-4:1
-2:2:4
-2:3:-8
-2:4:16
-2:5:-32
-3:2:9
-3:3:-27
-3:4:81
-3:5:-243
10:2:100
10:3:1000
10:4:10000
10:5:100000
10:6:1000000
10:7:10000000
10:8:100000000
10:9:1000000000
10:20:100000000000000000000
123456:2:15241383936
#2:0.5:1.41^

&length
100:3
10:2
1:1
0:1
12345:5
10000000000000000:17
-123:3
215960156869840440586892398248:30
# broot always upgrades

&broot
144:2:12^
123:2:11.09053650640941716205160010260993291846^
# bsqrt always upgrades

&bsqrt
145:12.04159457879229548012824103037860805243^
144:12^
143:11.95826074310139802112984075619561661399^
16:4
170:13.03840481040529742916594311485836883306^
169:13
168:12.96148139681572046193193487217599331541^
4:2
3:1.732050807568877293527446341505872366943^
2:1.41421356237309504880168872420969807857^
9:3
12:3.464101615137754587054892683011744733886^
256:16
100000000:10000
4000000000000:2000000
152399026:12345.00004050222755607815159966235881398^
152399025:12345
152399024:12344.99995949777231103967404745303741942^
1:1
0:0
-2:NaN
-123:NaN
Nan:NaN
+inf:inf
-inf:NaN

&bround
$round_mode('trunc')
0:12:0
NaNbround:12:NaN
+inf:12:inf
-inf:12:-inf
1234:0:1234
1234:2:1200
123456:4:123400
123456:5:123450
123456:6:123456
+10123456789:5:10123000000
-10123456789:5:-10123000000
+10123456789:9:10123456700
-10123456789:9:-10123456700
+101234500:6:101234000
-101234500:6:-101234000
#+101234500:-4:101234000
#-101234500:-4:-101234000
$round_mode('zero')
+20123456789:5:20123000000
-20123456789:5:-20123000000
+20123456789:9:20123456800
-20123456789:9:-20123456800
+201234500:6:201234000
-201234500:6:-201234000
#+201234500:-4:201234000
#-201234500:-4:-201234000
+12345000:4:12340000
-12345000:4:-12340000
$round_mode('+inf')
+30123456789:5:30123000000
-30123456789:5:-30123000000
+30123456789:9:30123456800
-30123456789:9:-30123456800
+301234500:6:301235000
-301234500:6:-301234000
#+301234500:-4:301235000
#-301234500:-4:-301234000
+12345000:4:12350000
-12345000:4:-12340000
$round_mode('-inf')
+40123456789:5:40123000000
-40123456789:5:-40123000000
+40123456789:9:40123456800
-40123456789:9:-40123456800
+401234500:6:401234000
+401234500:6:401234000
#-401234500:-4:-401235000
#-401234500:-4:-401235000
+12345000:4:12340000
-12345000:4:-12350000
$round_mode('odd')
+50123456789:5:50123000000
-50123456789:5:-50123000000
+50123456789:9:50123456800
-50123456789:9:-50123456800
+501234500:6:501235000
-501234500:6:-501235000
#+501234500:-4:501235000
#-501234500:-4:-501235000
+12345000:4:12350000
-12345000:4:-12350000
$round_mode('even')
+60123456789:5:60123000000
-60123456789:5:-60123000000
+60123456789:9:60123456800
-60123456789:9:-60123456800
+601234500:6:601234000
-601234500:6:-601234000
#+601234500:-4:601234000
#-601234500:-4:-601234000
#-601234500:-9:0
#-501234500:-9:0
#-601234500:-8:0
#-501234500:-8:0
+1234567:7:1234567
+1234567:6:1234570
+12345000:4:12340000
-12345000:4:-12340000

&is_zero
0:1
NaNzero:0
+inf:0
-inf:0
123:0
-1:0
1:0

&is_one
0:0
NaNone:0
+inf:0
-inf:0
1:1
2:0
-1:0
-2:0
# floor and ceil are pretty pointless in integer space, but play safe

&bfloor
0:0
NaNfloor:NaN
+inf:inf
-inf:-inf
-1:-1
-2:-2
2:2
3:3
abc:NaN

&bceil
NaNceil:NaN
+inf:inf
-inf:-inf
0:0
-1:-1
-2:-2
2:2
3:3
abc:NaN

&bint
NaN:NaN
+inf:inf
-inf:-inf
0:0
-1:-1
-2:-2
2:2
3:3

&to_hex
128:80
-128:-80
0:0
-0:0
1:1
0x123456789123456789:123456789123456789
+inf:inf
-inf:-inf
NaNto_hex:NaN

&to_oct
128:200
-128:-200
0:0
-0:0
1:1
0b1010111101010101010110110110110110101:1275252666665
0x123456789123456789:44321263611044321263611
+inf:inf
-inf:-inf
NaNto_oct:NaN

&to_bin
128:10000000
-128:-10000000
0:0
-0:0
1:1
0b1010111101010101010110110110110110101:1010111101010101010110110110110110101
+inf:inf
-inf:-inf
NaNto_bin:NaN

&as_hex
128:0x80
-128:-0x80
0:0x0
-0:0x0
1:0x1
0x123456789123456789:0x123456789123456789
+inf:inf
-inf:-inf
NaNas_hex:NaN

&as_bin
128:0b10000000
-128:-0b10000000
0:0b0
-0:0b0
1:0b1
0b1010111101010101010110110110110110101:0b1010111101010101010110110110110110101
+inf:inf
-inf:-inf
NaNas_bin:NaN

&as_oct
128:0200
-128:-0200
0:00
-0:00
1:01
0b1010111101010101010110110110110110101:01275252666665
0x123456789123456789:044321263611044321263611
+inf:inf
-inf:-inf
NaNas_oct:NaN
