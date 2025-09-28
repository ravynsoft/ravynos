#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use strict;

use Config;

# Tests of post/pre - increment/decrement operators.

# Verify that addition/subtraction properly upgrade to doubles.
# These tests are only significant on machines with 32 bit longs,
# and two's complement negation, but shouldn't fail anywhere.

my $a = 2147483647;
my $c=$a++;
cmp_ok($a, '==', 2147483648, "postincrement properly upgrades to double");

$a = 2147483647;
$c=++$a;
cmp_ok($a, '==', 2147483648, "preincrement properly upgrades to double");

$a = 2147483647;
$a=$a+1;
cmp_ok($a, '==', 2147483648, "addition properly upgrades to double");

$a = -2147483648;
$c=$a--;
cmp_ok($a, '==', -2147483649, "postdecrement properly upgrades to double");

$a = -2147483648;
$c=--$a;
cmp_ok($a, '==', -2147483649, "predecrement properly upgrades to double");

$a = -2147483648;
$a=$a-1;
cmp_ok($a, '==', -2147483649, "subtraction properly upgrades to double");

$a = 2147483648;
$a = -$a;
$c=$a--;
cmp_ok($a, '==', -2147483649,
    "negation and postdecrement properly upgrade to double");

$a = 2147483648;
$a = -$a;
$c=--$a;
cmp_ok($a, '==', -2147483649,
    "negation and predecrement properly upgrade to double");

$a = 2147483648;
$a = -$a;
$a=$a-1;
cmp_ok($a, '==', -2147483649,
    "negation and subtraction properly upgrade to double");

$a = 2147483648;
$b = -$a;
$c=$b--;
cmp_ok($b, '==', -$a-1, "negation, postdecrement and additional negation");

$a = 2147483648;
$b = -$a;
$c=--$b;
cmp_ok($b, '==', -$a-1, "negation, predecrement and additional negation");

$a = 2147483648;
$b = -$a;
$b=$b-1;
cmp_ok($b, '==', -(++$a),
    "negation, subtraction, preincrement and additional negation");

$a = undef;
is($a++, '0', "postinc undef returns '0'");

$a = undef;
is($a--, undef, "postdec undef returns undef");

# Verify that shared hash keys become unshared.

sub check_same {
  my ($orig, $suspect) = @_;
  my $fail;
  while (my ($key, $value) = each %$suspect) {
    if (exists $orig->{$key}) {
      if ($orig->{$key} ne $value) {
        print "# key '$key' was '$orig->{$key}' now '$value'\n";
        $fail = 1;
      }
    } else {
      print "# key '$key' is '$orig->{$key}', unexpect.\n";
      $fail = 1;
    }
  }
  foreach (keys %$orig) {
    next if (exists $suspect->{$_});
    print "# key '$_' was '$orig->{$_}' now missing\n";
    $fail = 1;
  }
  ok (!$fail, "original hashes unchanged");
}

my (%orig) = my (%inc) = my (%dec) = my (%postinc) = my (%postdec)
  = (1 => 1, ab => "ab");
my %up = (1=>2, ab => 'ac');
my %down = (1=>0, ab => -1);

foreach (keys %inc) {
  my $ans = $up{$_};
  my $up;
  eval {$up = ++$_};
  is($up, $ans, "key '$_' incremented correctly");
  is($@, '', "no error condition");
}

check_same (\%orig, \%inc);

foreach (keys %dec) {
  my $ans = $down{$_};
  my $down;
  eval {$down = --$_};
  is($down, $ans, "key '$_' decremented correctly");
  is($@, '', "no error condition");
}

check_same (\%orig, \%dec);

foreach (keys %postinc) {
  my $ans = $postinc{$_};
  my $up;
  eval {$up = $_++};
  is($up, $ans, "assignment preceded postincrement");
  is($@, '', "no error condition");
}

check_same (\%orig, \%postinc);

foreach (keys %postdec) {
  my $ans = $postdec{$_};
  my $down;
  eval {$down = $_--};
  is($down, $ans, "assignment preceded postdecrement");
  is($@, '', "no error condition");
}

check_same (\%orig, \%postdec);

{
    no warnings 'uninitialized';
    my ($x, $y);
    eval {
	$y ="$x\n";
	++$x;
    };
    cmp_ok($x, '==', 1, "preincrement of previously uninitialized variable");
    is($@, '', "no error condition");

    my ($p, $q);
    eval {
	$q ="$p\n";
	--$p;
    };
    cmp_ok($p, '==', -1, "predecrement of previously uninitialized variable");
    is($@, '', "no error condition");
}

$a = 2147483648;
$c=--$a;
cmp_ok($a, '==', 2147483647, "predecrement properly downgrades from double");


$a = 2147483648;
$c=$a--;
cmp_ok($a, '==', 2147483647, "postdecrement properly downgrades from double");

{
    use integer;
    my $x = 0;
    $x++;
    cmp_ok($x, '==', 1, "(void) i_postinc");
    $x--;
    cmp_ok($x, '==', 0, "(void) i_postdec");
}

SKIP: {
    if ($Config{uselongdouble} &&
        ($Config{d_long_double_style_ieee_doubledouble})) {
        skip "the double-double format is weird", 1;
    }
    unless ($Config{d_double_style_ieee}) {
        skip "the doublekind $Config{doublekind} is not IEEE", 1;
    }

# I'm sure that there's an IBM format with a 48 bit mantissa
# IEEE doubles have a 53 bit mantissa
# 80 bit long doubles have a 64 bit mantissa
# sparcs have a 112 bit mantissa for their long doubles. Just to be awkward :-)

my $h_uv_max = 1 + (~0 >> 1);
my $found;
for my $n (47..113) {
    my $power_of_2 = 2**$n;
    my $plus_1 = $power_of_2 + 1;
    next if $plus_1 != $power_of_2;
    my ($start_p, $start_n);
    if ($h_uv_max > $power_of_2 / 2) {
	my $uv_max = 1 + 2 * (~0 >> 1);
	# UV_MAX is 2**$something - 1, so subtract 1 to get the start value
	$start_p = $uv_max - 1;
	# whereas IV_MIN is -(2**$something), so subtract 2
	$start_n = -$h_uv_max + 2;
	print "# Mantissa overflows at 2**$n ($power_of_2)\n";
	print "# But max UV ($uv_max) is greater so testing that\n";
    } else {
	print "# Testing 2**$n ($power_of_2) which overflows the mantissa\n";
	$start_p = int($power_of_2 - 2);
	$start_n = -$start_p;
	my $check = $power_of_2 - 2;
	die "Something wrong with our rounding assumptions: $check vs $start_p"
	    unless $start_p == $check;
    }

    foreach ([$start_p, '++$i', 'pre-inc', 'inc'],
	     [$start_p, '$i++', 'post-inc', 'inc'],
	     [$start_n, '--$i', 'pre-dec', 'dec'],
	     [$start_n, '$i--', 'post-dec', 'dec']) {
	my ($start, $action, $description, $act) = @$_;
	my $code = eval << "EOC" or die $@;
sub {
    no warnings 'imprecision';
    my \$i = \$start;
    for(0 .. 3) {
        my \$a = $action;
    }
}
EOC

	warning_is($code, undef, "$description under no warnings 'imprecision'");

	$code = eval << "EOC" or die $@;
sub {
    use warnings 'imprecision';
    my \$i = \$start;
    for(0 .. 3) {
        my \$a = $action;
    }
}
EOC

	warnings_like($code, [(qr/Lost precision when ${act}rementing -?\d+/) x 2],
		      "$description under use warnings 'imprecision'");
    }

    # Verify warnings on incrementing/decrementing large values
    # whose integral part will not fit in NVs. [GH #18333]
    foreach ([$start_n - 4, '$i++', 'negative large value', 'inc'],
             [$start_p + 4, '$i--', 'positive large value', 'dec']) {
	my ($start, $action, $description, $act) = @$_;
	my $code = eval << "EOC" or die $@;
sub {
    use warnings 'imprecision';
    my \$i = \$start;
    $action;
}
EOC
        warning_like($code, qr/Lost precision when ${act}rementing /,
                     "${act}rementing $description under use warnings 'imprecision'");
    }

    $found = 1;
    last;
}

ok($found, "found a NV value which overflows the mantissa");

} # SKIP

# these will segfault if they fail

sub PVBM () { 'foo' }
{ my $dummy = index 'foo', PVBM }

isnt(scalar eval { my $pvbm = PVBM; $pvbm++ }, undef, "postincrement defined");
isnt(scalar eval { my $pvbm = PVBM; $pvbm-- }, undef, "postdecrement defined");
isnt(scalar eval { my $pvbm = PVBM; ++$pvbm }, undef, "preincrement defined");
isnt(scalar eval { my $pvbm = PVBM; --$pvbm }, undef, "predecrement defined");

# #9466

# don't use pad TARG when the thing you're copying is a ref, or the referent
# won't get freed.
{
    package P9466;
    my $x;
    sub DESTROY { $x = 1 }
    for (0..1) {
	$x = 0;
	my $a = bless {};
	my $b = $_ ? $a++ : $a--;
	undef $a; undef $b;
	::is($x, 1, "9466 case $_");
    }
}

# *Do* use pad TARG if it is actually a named variable, even when the thing
# you’re copying is a ref.  The fix for #9466 broke this.
{
    package P9466_2;
    my $x;
    sub DESTROY { $x = 1 }
    for (2..3) {
	$x = 0;
	my $a = bless {};
	my $b;
	use integer;
	if ($_ == 2) {
	    $b = $a--; # sassign optimised away
	}
	else {
	    $b = $a++;
	}
	::is(ref $b, __PACKAGE__, 'i_post(in|de)c/TARGMY on ref');
	undef $a; undef $b;
	::is($x, 1, "9466 case $_");
    }
}

$_ = ${qr //};
$_--;
is($_, -1, 'regexp--');
{
    no warnings 'numeric';
    $_ = ${qr //};
    $_++;
    is($_, 1, 'regexp++');
}

if ($::IS_EBCDIC) {
    $_ = v129;
    $_++;
    isnt(ref\$_, 'VSTRING', '++ flattens vstrings');
}
else {
    $_ = v97;
    $_++;
    isnt(ref\$_, 'VSTRING', '++ flattens vstrings');
}

sub TIESCALAR {bless\my $x}
sub STORE { ++$store::called }
tie my $t, "";
{
    $t = $_++;
    $t = $_--;
    use integer;
    $t = $_++;
    $t = $_--;
}
is $store::called, 4, 'STORE called on "my" target';

{
    # Temporarily broken between before 5.6.0 (b162f9ea/21f5b33c) and
    # between 5.21.5 and 5.21.6 (9e319cc4fd)
    my $x = 7;
    $x = $x++;
    is $x, 7, '$lex = $lex++';
    $x = 7;
    # broken in b162f9ea (5.6.0); fixed in 5.21.6
    use integer;
    $x = $x++;
    is $x, 7, '$lex = $lex++ under use integer';
}

{
    # RT #126637 - it should refuse to modify globs
    no warnings 'once';
    *GLOB126637 = [];

    eval 'my $y = ++$_ for *GLOB126637';
    like $@, qr/Modification of a read-only value/, '++*GLOB126637';
    eval 'my $y = --$_ for *GLOB126637';
    like $@, qr/Modification of a read-only value/, '--*GLOB126637';
    eval 'my $y = $_++ for *GLOB126637';
    like $@, qr/Modification of a read-only value/, '*GLOB126637++';
    eval 'my $y = $_-- for *GLOB126637';
    like $@, qr/Modification of a read-only value/, '*GLOB126637--';

    use integer;

    eval 'my $y = ++$_ for *GLOB126637';
    like $@, qr/Modification of a read-only value/, 'use int; ++*GLOB126637';
    eval 'my $y = --$_ for *GLOB126637';
    like $@, qr/Modification of a read-only value/, 'use int; --*GLOB126637';
    eval 'my $y = $_++ for *GLOB126637';
    like $@, qr/Modification of a read-only value/, 'use int; *GLOB126637++';
    eval 'my $y = $_-- for *GLOB126637';
    like $@, qr/Modification of a read-only value/, 'use int; *GLOB126637--';
}

# Exercises sv_inc() incrementing UV to UV, UV to NV
SKIP: {
    $a = ~1; # assumed to be UV_MAX - 1

    if ($Config{uvsize} eq '4') {
        cmp_ok(++$a, '==', 4294967295, "preincrement to UV_MAX");
        cmp_ok(++$a, '==', 4294967296, "preincrement past UV_MAX");
    }
    elsif ($Config{uvsize} eq '8') {
        cmp_ok(++$a, '==', 18446744073709551615, "preincrement to UV_MAX");
        # assumed that NV can hold 2 ** 64 without rounding.
        cmp_ok(++$a, '==', 18446744073709551616, "preincrement past UV_MAX");
    }
    else {
        skip "the uvsize $Config{uvsize} is neither 4 nor 8", 2;
    }
} # SKIP

# Incrementing/decrementing Inf/NaN should not trigger 'imprecision' warnings
# [GH #18333, #18388]
# Note these tests only check for warnings; t/op/infnan.t has tests that
# checks the result of incrementing/decrementing Inf/NaN.
foreach my $infnan ('+Inf', '-Inf', 'NaN') {
    my $start = $infnan + 0;
  SKIP: {
      skip "NV does not have $infnan", 2
          unless ($infnan eq 'NaN' ? $Config{d_double_has_nan} : $Config{d_double_has_inf});
      foreach (['$i++', 'inc'],
               ['$i--', 'dec']) {
          my ($action, $act) = @$_;
          my $code = eval <<"EOC" or die $@;
sub {
    use warnings 'imprecision';
    my \$i = \$start;
    $action;
}
EOC
          warning_is($code, undef, "${act}rementing $infnan under use warnings 'imprecision'");
      }
    } # SKIP
}

done_testing();
