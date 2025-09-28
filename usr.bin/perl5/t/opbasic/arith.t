#!./perl -w

# This file has been placed in t/opbasic to indicate that it should not use
# functions imported from t/test.pl or Test::More, as those programs/libraries
# use operators which are what is being tested in this file.

print "1..180\n";

sub try ($$$) {
   print +($_[1] ? "ok" : "not ok") . " $_[0] - $_[2]\n";
}
sub tryeq ($$$$) {
  my $status;
  if ($_[1] == $_[2]) {
    $status = "ok $_[0]";
  } else {
    $status = "not ok $_[0] # $_[1] != $_[2]";
  }
  print "$status - $_[3]\n";
}
sub tryeq_sloppy ($$$$) {
  my $status;
  if ($_[1] == $_[2]) {
    $status = "ok $_[0]";
  } else {
    my $error = abs (($_[1] - $_[2]) / $_[1]);
    if ($error < 1e-9) {
      $status = "ok $_[0] # $_[1] is close to $_[2], \$^O eq $^O";
    } else {
      $status = "not ok $_[0] # $_[1] != $_[2]";
    }
  }
  print "$status - $_[3]\n";
}

my $T = 1;
tryeq $T++,  13 %  4, 1, 'modulo: positive positive';
tryeq $T++, -13 %  4, 3, 'modulo: negative positive';
tryeq $T++,  13 % -4, -3, 'modulo: positive negative';
tryeq $T++, -13 % -4, -1, 'modulo: negative negative';

# Exercise some of the dright/dleft logic in pp_modulo

tryeq $T++, 13.333333 % 5.333333, 3, 'modulo: 13.333333 % 5.333333';
tryeq $T++, 13.333333 % 5,        3, 'modulo: 13.333333 % 5';
tryeq $T++, 13 % 5.333333,        3, 'modulo: 13 % 5.333333';

# Give abs() a good work-out before using it in anger
tryeq $T++, abs(0), 0, 'abs(): 0 0';
tryeq $T++, abs(1), 1, 'abs(): 1 1';
tryeq $T++, abs(-1), 1, 'abs(): -1 1';
tryeq $T++, abs(2147483647), 2147483647, 'abs(): 2**31-1: pos pos';
tryeq $T++, abs(-2147483647), 2147483647, 'abs(): 2**31-1: neg pos';
tryeq $T++, abs(4294967295), 4294967295, 'abs(): 2**32-1: pos pos';
tryeq $T++, abs(-4294967295), 4294967295, 'abs(): 2**32-1: neg pos';
tryeq $T++, abs(9223372036854775807), 9223372036854775807,
    'abs(): 2**63-1: pos pos';
tryeq $T++, abs(-9223372036854775807), 9223372036854775807,
    'abs(): 2**63-1: neg pos';
# Assume no change whatever; no slop needed
tryeq $T++, abs(1e50), 1e50, 'abs(): 1e50: pos pos';
# Assume only sign bit flipped
tryeq $T++, abs(-1e50), 1e50, 'abs(): 1e50: neg pos';

my $limit = 1e6;

# Division (and modulo) of floating point numbers
# seem to be rather sloppy in Cray.
$limit = 1e8 if $^O eq 'unicos';

try $T++, abs( 13e21 %  4e21 -  1e21) < $limit, 'abs() for floating point';
try $T++, abs(-13e21 %  4e21 -  3e21) < $limit, 'abs() for floating point';
try $T++, abs( 13e21 % -4e21 - -3e21) < $limit, 'abs() for floating point';
try $T++, abs(-13e21 % -4e21 - -1e21) < $limit, 'abs() for floating point';

tryeq $T++, 4063328477 % 65535, 27407, 'UV behaves properly: modulo';
tryeq $T++, 4063328477 % 4063328476, 1, 'UV behaves properly: modulo';
tryeq $T++, 4063328477 % 2031664238, 1, 'UV behaves properly: modulo';
tryeq $T++, 2031664238 % 4063328477, 2031664238,
    'UV behaves properly: modulo';

tryeq $T++, 2147483647 + 0, 2147483647,
    'trigger wrapping on 32 bit IVs and UVs';

tryeq $T++, 2147483647 + 1, 2147483648, 'IV + IV promotes to UV';
tryeq $T++, 2147483640 + 10, 2147483650, 'IV + IV promotes to UV';
tryeq $T++, 2147483647 + 2147483647, 4294967294, 'IV + IV promotes to UV';
tryeq $T++, 2147483647 + 2147483649, 4294967296, 'IV + UV promotes to NV';
tryeq $T++, 4294967294 + 2, 4294967296, 'UV + IV promotes to NV';
tryeq $T++, 4294967295 + 4294967295, 8589934590, 'UV + UV promotes to NV';

tryeq $T++, 2147483648 + -1, 2147483647, 'UV + IV promotes to IV';
tryeq $T++, 2147483650 + -10, 2147483640, 'UV + IV promotes to IV';
tryeq $T++, -1 + 2147483648, 2147483647, 'IV + UV promotes to IV';
tryeq $T++, -10 + 4294967294, 4294967284, 'IV + UV promotes to IV';
tryeq $T++, -2147483648 + -2147483648, -4294967296, 'IV + IV promotes to NV';
tryeq $T++, -2147483640 + -10, -2147483650, 'IV + IV promotes to NV';

# Hmm. Do not forget the simple stuff
# addition
tryeq $T++, 1 + 1, 2, 'addition of 2 positive integers';
tryeq $T++, 4 + -2, 2, 'addition of positive and negative integer';
tryeq $T++, -10 + 100, 90, 'addition of negative and positive integer';
tryeq $T++, -7 + -9, -16, 'addition of 2 negative integers';
tryeq $T++, -63 + +2, -61, 'addition of signed negative and positive integers';
tryeq $T++, 4 + -1, 3, 'addition of positive and negative integer';
tryeq $T++, -1 + 1, 0, 'addition which sums to 0';
tryeq $T++, +29 + -29, 0, 'addition which sums to 0';
tryeq $T++, -1 + 4, 3, 'addition of signed negative and positive integers';
tryeq $T++, +4 + -17, -13, 'addition of signed positive and negative integers';

# subtraction
tryeq $T++, 3 - 1, 2, 'subtraction of two positive integers';
tryeq $T++, 3 - 15, -12,
    'subtraction of two positive integers: minuend smaller';
tryeq $T++, 3 - -7, 10, 'subtraction of positive and negative integer';
tryeq $T++, -156 - 5, -161, 'subtraction of negative and positive integer';
tryeq $T++, -156 - -5, -151, 'subtraction of two negative integers';
tryeq $T++, -5 - -12, 7,
    'subtraction of two negative integers: minuend smaller';
tryeq $T++, -3 - -3, 0, 'subtraction of two negative integers with result of 0';
tryeq $T++, 15 - 15, 0, 'subtraction of two positive integers with result of 0';
tryeq $T++, 2147483647 - 0, 2147483647, 'subtraction from large integer';
tryeq $T++, 2147483648 - 0, 2147483648, 'subtraction from large integer';
tryeq $T++, -2147483648 - 0, -2147483648,
    'subtraction from large negative integer';
tryeq $T++, 0 - -2147483647, 2147483647,
    'subtraction of large negative integer from 0';
tryeq $T++, -1 - -2147483648, 2147483647,
    'subtraction of large negative integer from negative integer';
tryeq $T++, 2 - -2147483648, 2147483650,
    'subtraction of large negative integer from positive integer';
tryeq $T++, 4294967294 - 3, 4294967291, 'subtraction from large integer';
tryeq $T++, -2147483648 - -1, -2147483647,
    'subtraction from large negative integer';
tryeq $T++, 2147483647 - -1, 2147483648, 'IV - IV promote to UV';
tryeq $T++, 2147483647 - -2147483648, 4294967295, 'IV - IV promote to UV';
tryeq $T++, 4294967294 - -3, 4294967297, 'UV - IV promote to NV';
tryeq $T++, -2147483648 - +1, -2147483649, 'IV - IV promote to NV';
tryeq $T++, 2147483648 - 2147483650, -2, 'UV - UV promote to IV';
tryeq $T++, 2000000000 - 4000000000, -2000000000, 'IV - UV promote to IV';

# No warnings should appear;
my $a;
$a += 1;
tryeq $T++, $a, 1, '+= with positive';
undef $a;
$a += -1;
tryeq $T++, $a, -1, '+= with negative';
undef $a;
$a += 4294967290;
tryeq $T++, $a, 4294967290, '+= with positive';
undef $a;
$a += -4294967290;
tryeq $T++, $a, -4294967290, '+= with negative';
undef $a;
$a += 4294967297;
tryeq $T++, $a, 4294967297, '+= with positive';
undef $a;
$a += -4294967297;
tryeq $T++, $a, -4294967297, '+= with negative';

my $s;
$s -= 1;
tryeq $T++, $s, -1, '-= with positive';
undef $s;
$s -= -1;
tryeq $T++, $s, +1, '-= with negative';
undef $s;
$s -= -4294967290;
tryeq $T++, $s, +4294967290, '-= with negative';
undef $s;
$s -= 4294967290;
tryeq $T++, $s, -4294967290, '-= with negative';
undef $s;
$s -= 4294967297;
tryeq $T++, $s, -4294967297, '-= with positive';
undef $s;
$s -= -4294967297;
tryeq $T++, $s, +4294967297, '-= with positive';

# multiplication
tryeq $T++, 1 * 3, 3, 'multiplication of two positive integers';
tryeq $T++, -2 * 3, -6, 'multiplication of negative and positive integer';
tryeq $T++, 3 * -3, -9, 'multiplication of positive and negative integer';
tryeq $T++, -4 * -3, 12, 'multiplication of two negative integers';

# check with 0xFFFF and 0xFFFF
tryeq $T++, 65535 * 65535, 4294836225,
    'multiplication: 0xFFFF and 0xFFFF: pos pos';
tryeq $T++, 65535 * -65535, -4294836225,
    'multiplication: 0xFFFF and 0xFFFF: pos neg';
tryeq $T++, -65535 * 65535, -4294836225,
    'multiplication: 0xFFFF and 0xFFFF: pos neg';
tryeq $T++, -65535 * -65535, 4294836225,
    'multiplication: 0xFFFF and 0xFFFF: neg neg';

# check with 0xFFFF and 0x10001
tryeq $T++, 65535 * 65537, 4294967295,
    'multiplication: 0xFFFF and 0x10001: pos pos';
tryeq $T++, 65535 * -65537, -4294967295,
    'multiplication: 0xFFFF and 0x10001: pos neg';
tryeq $T++, -65535 * 65537, -4294967295,
    'multiplication: 0xFFFF and 0x10001: neg pos';
tryeq $T++, -65535 * -65537, 4294967295,
    'multiplication: 0xFFFF and 0x10001: neg neg';

# check with 0x10001 and 0xFFFF
tryeq $T++, 65537 * 65535, 4294967295,
    'multiplication: 0x10001 and 0xFFFF: pos pos';
tryeq $T++, 65537 * -65535, -4294967295,
    'multiplication: 0x10001 and 0xFFFF: pos neg';
tryeq $T++, -65537 * 65535, -4294967295,
    'multiplication: 0x10001 and 0xFFFF: neg pos';
tryeq $T++, -65537 * -65535, 4294967295,
    'multiplication: 0x10001 and 0xFFFF: neg neg';

# These should all be dones as NVs
tryeq $T++, 65537 * 65537, 4295098369, 'multiplication: NV: pos pos';
tryeq $T++, 65537 * -65537, -4295098369, 'multiplication: NV: pos neg';
tryeq $T++, -65537 * 65537, -4295098369, 'multiplication: NV: neg pos';
tryeq $T++, -65537 * -65537, 4295098369, 'multiplication: NV: neg neg';

# will overflow an IV (in 32-bit)
tryeq $T++, 46340 * 46342, 0x80001218,
    'multiplication: overflow an IV in 32-bit: pos pos';
tryeq $T++, 46340 * -46342, -0x80001218,
    'multiplication: overflow an IV in 32-bit: pos neg';
tryeq $T++, -46340 * 46342, -0x80001218,
    'multiplication: overflow an IV in 32-bit: neg pos';
tryeq $T++, -46340 * -46342, 0x80001218,
    'multiplication: overflow an IV in 32-bit: neg neg';

tryeq $T++, 46342 * 46340, 0x80001218,
    'multiplication: overflow an IV in 32-bit: pos pos';
tryeq $T++, 46342 * -46340, -0x80001218,
    'multiplication: overflow an IV in 32-bit: pos neg';
tryeq $T++, -46342 * 46340, -0x80001218,
    'multiplication: overflow an IV in 32-bit: neg pos';
tryeq $T++, -46342 * -46340, 0x80001218,
    'multiplication: overflow an IV in 32-bit: neg neg';

# will overflow a positive IV (in 32-bit)
tryeq $T++, 65536 * 32768, 0x80000000,
    'multiplication: overflow a positive IV in 32-bit: pos pos';
tryeq $T++, 65536 * -32768, -0x80000000,
    'multiplication: overflow a positive IV in 32-bit: pos neg';
tryeq $T++, -65536 * 32768, -0x80000000,
    'multiplication: overflow a positive IV in 32-bit: neg pos';
tryeq $T++, -65536 * -32768, 0x80000000,
    'multiplication: overflow a positive IV in 32-bit: neg neg';

tryeq $T++, 32768 * 65536, 0x80000000,
    'multiplication: overflow a positive IV in 32-bit: pos pos';
tryeq $T++, 32768 * -65536, -0x80000000,
    'multiplication: overflow a positive IV in 32-bit: pos neg';
tryeq $T++, -32768 * 65536, -0x80000000,
    'multiplication: overflow a positive IV in 32-bit: neg pos';
tryeq $T++, -32768 * -65536, 0x80000000,
    'multiplication: overflow a positive IV in 32-bit: neg neg';

# 2147483647 is prime. bah.

tryeq $T++, 46339 * 46341, 0x7ffea80f,
    'multiplication: hex product: pos pos';
tryeq $T++, 46339 * -46341, -0x7ffea80f,
    'multiplication: hex product: pos neg';
tryeq $T++, -46339 * 46341, -0x7ffea80f,
    'multiplication: hex product: neg pos';
tryeq $T++, -46339 * -46341, 0x7ffea80f,
    'multiplication: hex product: neg neg';

# leading space should be ignored

tryeq $T++, 1 + " 1", 2, 'ignore leading space: addition';
tryeq $T++, 3 + " -1", 2, 'ignore leading space: subtraction';
tryeq $T++, 1.2, " 1.2", 'floating point and string equivalent: positive';
tryeq $T++, -1.2, " -1.2", 'floating point and string equivalent: negative';

# division
tryeq $T++, 28/14, 2, 'division of two positive integers';
tryeq $T++, 28/-7, -4, 'division of positive integer by negative';
tryeq $T++, -28/4, -7, 'division of negative integer by positive';
tryeq $T++, -28/-2, 14, 'division of negative integer by negative';

tryeq $T++, 0x80000000/1, 0x80000000,
    'division of positive hex by positive integer';
tryeq $T++, 0x80000000/-1, -0x80000000,
    'division of positive hex by negative integer';
tryeq $T++, -0x80000000/1, -0x80000000,
    'division of negative hex by negative integer';
tryeq $T++, -0x80000000/-1, 0x80000000,
    'division of negative hex by positive integer';

# The example for sloppy divide, rigged to avoid the peephole optimiser.
tryeq_sloppy $T++, "20." / "5.", 4, 'division of floating point without fractional part';

tryeq $T++, 2.5 / 2, 1.25,
    'division of positive floating point by positive integer';
tryeq $T++, 3.5 / -2, -1.75,
    'division of positive floating point by negative integer';
tryeq $T++, -4.5 / 2, -2.25,
    'division of negative floating point by positive integer';
tryeq $T++, -5.5 / -2, 2.75,
    'division of negative floating point by negative integer';

# Bluuurg if your floating point can not accurately cope with powers of 2
# [I suspect this is parsing string->float problems, not actual arith]
tryeq_sloppy $T++, 18446744073709551616/1, 18446744073709551616,
    'division of very large number by 1'; # Bluuurg
tryeq_sloppy $T++, 18446744073709551616/2, 9223372036854775808,
    'division of very large number by 2';
tryeq_sloppy $T++, 18446744073709551616/4294967296, 4294967296,
    'division of two very large numbers';
tryeq_sloppy $T++, 18446744073709551616/9223372036854775808, 2,
    'division of two very large numbers';

{
  # The peephole optimiser is wrong to think that it can substitute intops
  # in place of regular ops, because i_multiply can overflow.
  # Bug reported by "Sisyphus" <kalinabears@hdc.com.au>
  my $n = 1127;

  my $float = ($n % 1000) * 167772160.0;
  tryeq_sloppy $T++, $float, 21307064320, 'integer times floating point';

  # On a 32 bit machine, if the i_multiply op is used, you will probably get
  # -167772160. It is actually undefined behaviour, so anything may happen.
  my $int = ($n % 1000) * 167772160;
  tryeq $T++, $int, 21307064320, 'integer times integer';

  my $float2 = ($n % 1000 + 0.0) * 167772160;
  tryeq $T++, $float2, 21307064320, 'floating point times integer';

  my $int2 = ($n % 1000 + 0) * 167772160;
  tryeq $T++, $int2, 21307064320, 'integer plus zero times integer';

  # zero, but in a way that ought to be able to defeat any future optimizer:
  my $zero = $$ - $$;
  my $int3 = ($n % 1000 + $zero) * 167772160;
  tryeq $T++, $int3, 21307064320, 'defeat any future optimizer';

  my $t = time;
  my $t1000 = time() * 1000;
  try $T++, abs($t1000 -1000 * $t) <= 2000, 'absolute value';
}

{
  # 64 bit variants
  my $n = 1127;

  my $float = ($n % 1000) * 720575940379279360.0;
  tryeq_sloppy $T++, $float, 9.15131444281685e+19,
    '64 bit: integer times floating point';

  my $int = ($n % 1000) * 720575940379279360;
  tryeq_sloppy $T++, $int, 9.15131444281685e+19,
    '64 bit: integer times integer';

  my $float2 = ($n % 1000 + 0.0) * 720575940379279360;
  tryeq_sloppy $T++, $float2, 9.15131444281685e+19,
    '64 bit: floating point times integer';

  my $int2 = ($n % 1000 + 0) * 720575940379279360;
  tryeq_sloppy $T++, $int2, 9.15131444281685e+19,
    '64 bit: integer plus zero times integer';

  # zero, but in a way that ought to be able to defeat any future optimizer:
  my $zero = $$ - $$;
  my $int3 = ($n % 1000 + $zero) * 720575940379279360;
  tryeq_sloppy $T++, $int3, 9.15131444281685e+19,
    '64 bit: defeat any future optimizer';
}

# [perl #109542] $1 and "$1" should be treated the same way
"976562500000000" =~ /(\d+)/;
$a = ($1 * 1024);
$b = ("$1" * 1024);
print "not "x($a ne $b), "ok ", $T++, qq ' - \$1 vs "\$1" * something\n';
$a = (1024 * $1);
$b = (1024 * "$1");
print "not "x($a ne $b), "ok ", $T++, qq ' - something * \$1 vs "\$1"\n';
$a = ($1 + 102400000000000);
$b = ("$1" + 102400000000000);
print "not "x($a ne $b), "ok ", $T++, qq ' - \$1 vs "\$1" + something\n';
$a = (102400000000000 + $1);
$b = (102400000000000 + "$1");
print "not "x($a ne $b), "ok ", $T++, qq ' - something + \$1 vs "\$1"\n';
$a = ($1 - 10240000000000000);
$b = ("$1" - 10240000000000000);
print "not "x($a ne $b), "ok ", $T++, qq ' - \$1 vs "\$1" - something\n';
$a = (10240000000000000 - $1);
$b = (10240000000000000 - "$1");
print "not "x($a ne $b), "ok ", $T++, qq ' - something - \$1 vs "\$1"\n';
"976562500" =~ /(\d+)/;
$a = ($1 ** 2);
$b = ("$1" ** 2);
print "not "x($a ne $b), "ok ", $T++, qq ' - \$1 vs "\$1" ** something\n';
"32" =~ /(\d+)/;
$a = (3 ** $1);
$b = (3 ** "$1");
print "not "x($a ne $b), "ok ", $T++, qq ' - something ** \$1 vs "\$1"\n';
"97656250000000000" =~ /(\d+)/;
$a = ($1 / 10);
$b = ("$1" / 10);
print "not "x($a ne $b), "ok ", $T++, qq ' - \$1 vs "\$1" / something\n';
"10" =~ /(\d+)/;
$a = (97656250000000000 / $1);
$b = (97656250000000000 / "$1");
print "not "x($a ne $b), "ok ", $T++, qq ' - something / \$1 vs "\$1"\n';
"97656250000000000" =~ /(\d+)/;
$a = ($1 <=> 97656250000000001);
$b = ("$1" <=> 97656250000000001);
print "not "x($a ne $b), "ok ", $T++, qq ' - \$1 vs "\$1" <=> something\n';
$a = (97656250000000001 <=> $1);
$b = (97656250000000001 <=> "$1");
print "not "x($a ne $b), "ok ", $T++, qq ' - something <=> \$1 vs "\$1"\n';
"97656250000000001" =~ /(\d+)/;
$a = ($1 % 97656250000000002);
$b = ("$1" % 97656250000000002);
print "not "x($a ne $b), "ok ", $T++, qq ' - \$1 vs "\$1" % something\n';
$a = (97656250000000000 % $1);
$b = (97656250000000000 % "$1");
print "not "x($a ne $b), "ok ", $T++, qq ' - something % \$1 vs "\$1"\n';

# string-to-nv should equal float literals
try $T++, "1.23"   + 0 ==  1.23,  '1.23';
try $T++, " 1.23"  + 0 ==  1.23,  '1.23 with leading space';
try $T++, "1.23 "  + 0 ==  1.23,  '1.23 with trailing space';
try $T++, "+1.23"  + 0 ==  1.23,  '1.23 with unary plus';
try $T++, "-1.23"  + 0 == -1.23,  '1.23 with unary minus';
try $T++, "1.23e4" + 0 ==  12300, '1.23e4';

# trigger various attempts to negate IV_MIN

tryeq $T++,  0x80000000 / -0x80000000, -1, '(IV_MAX+1) / IV_MIN';
tryeq $T++, -0x80000000 /  0x80000000, -1, 'IV_MIN / (IV_MAX+1)';
tryeq $T++,  0x80000000 / -1, -0x80000000, '(IV_MAX+1) / -1';
tryeq $T++,           0 % -0x80000000,  0, '0 % IV_MIN';
tryeq $T++, -0x80000000 % -0x80000000,  0, 'IV_MIN % IV_MIN';
