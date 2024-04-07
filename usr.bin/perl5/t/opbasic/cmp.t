#!./perl

# This file has been placed in t/opbasic to indicate that it should not use
# functions imported from t/test.pl or Test::More, as those programs/libraries
# use operators which are what is being tested in this file.

# 2s complement assumption. Will not break test, just makes the internals of
# the SVs less interesting if were not on 2s complement system.
my $uv_max = ~0;
my $uv_maxm1 = ~0 ^ 1;
my $uv_big = $uv_max;
$uv_big = ($uv_big - 20000) | 1;
my ($iv0, $iv1, $ivm1, $iv_min, $iv_max, $iv_big, $iv_small);
$iv_max = $uv_max; # Do copy, *then* divide
$iv_max /= 2;
$iv_min = $iv_max;
{
  use integer;
  $iv0 = 2 - 2;
  $iv1 = 3 - 2;
  $ivm1 = 2 - 3;
  $iv_max -= 1;
  $iv_min += 0;
  $iv_big = $iv_max - 3;
  $iv_small = $iv_min + 2;
}
my $uv_bigi = $iv_big;
$uv_bigi |= 0x0;

my @array = qw(perl rules);

my @raw, @upgraded, @utf8;
foreach ("\0", "\x{1F4A9}", chr(163), 'N') {
    push @raw, $_;
    my $temp = $_ . chr 256;
    chop $temp;
    push @upgraded, $temp;
    my $utf8 = $_;
    next if utf8::upgrade($utf8) == length $_;
    utf8::encode($utf8);
    push @utf8, $utf8;
}

# Seems one needs to perform the maths on 'Inf' to get the NV correctly primed.
@FOO = ('s', 'N/A', 'a', 'NaN', -1, undef, 0, 1, 3.14, 1e37, 0.632120558, -.5,
	'Inf'+1, '-Inf'-1, 0x0, 0x1, 0x5, 0xFFFFFFFF, $uv_max, $uv_maxm1,
	$uv_big, $uv_bigi, $iv0, $iv1, $ivm1, $iv_min, $iv_max, $iv_big,
	$iv_small, \$array[0], \$array[0], \$array[1], \$^X, @raw, @upgraded,
	@utf8);

$expect = 7 * ($#FOO+2) * ($#FOO+1) + 6 * @raw + 6 * @utf8;
print "1..$expect\n";

my $bad_NaN = 0;

{
    # gcc -ffast-math option may stop NaNs working correctly
    use Config;
    my $ccflags = $Config{ccflags} // '';
    $bad_NaN = 1 if $ccflags =~ /-ffast-math\b/;
}

sub nok ($$$$$$$$) {
  my ($test, $left, $threeway, $right, $result, $i, $j, $boolean) = @_;
  $result = defined $result ? "'$result'" : 'undef';
  if ($bad_NaN && ($left eq 'NaN' || $right eq 'NaN')) {
    print "ok $test # skipping failed NaN test under -ffast-math\n";
  }
  else {
    print "not ok $test # ($left $threeway $right) gives: $result \$i=$i \$j=$j, $boolean disagrees\n";
  }
}

my $ok = 0;
for my $i (0..$#FOO) {
    for my $j ($i..$#FOO) {
	$ok++;
	# Comparison routines may convert these internally, which would change
	# what is used to determine the comparison on later runs. Hence copy
	my ($i1, $i2, $i3, $i4, $i5, $i6, $i7, $i8, $i9, $i10,
	    $i11, $i12, $i13, $i14, $i15, $i16, $i17) =
	  ($FOO[$i], $FOO[$i], $FOO[$i], $FOO[$i], $FOO[$i], $FOO[$i],
	   $FOO[$i], $FOO[$i], $FOO[$i], $FOO[$i], $FOO[$i], $FOO[$i],
	   $FOO[$i], $FOO[$i], $FOO[$i], $FOO[$i], $FOO[$i]);
	my ($j1, $j2, $j3, $j4, $j5, $j6, $j7, $j8, $j9, $j10,
	    $j11, $j12, $j13, $j14, $j15, $j16, $j17) =
	  ($FOO[$j], $FOO[$j], $FOO[$j], $FOO[$j], $FOO[$j], $FOO[$j],
	   $FOO[$j], $FOO[$j], $FOO[$j], $FOO[$j], $FOO[$j], $FOO[$j],
	   $FOO[$j], $FOO[$j], $FOO[$j], $FOO[$j], $FOO[$j]);
	my $cmp = $i1 <=> $j1;
	if (!defined($cmp) ? !($i2 < $j2)
	    : ($cmp == -1 && $i2 < $j2 ||
	       $cmp == 0  && !($i2 < $j2) ||
	       $cmp == 1  && !($i2 < $j2)))
	{
	    print "ok $ok\n";
	}
	else {
	    nok ($ok, $i3, '<=>', $j3, $cmp, $i, $j, '<');
	}
	$ok++;
	if (!defined($cmp) ? !($i4 == $j4)
	    : ($cmp == -1 && !($i4 == $j4) ||
	       $cmp == 0  && $i4 == $j4 ||
	       $cmp == 1  && !($i4 == $j4)))
	{
	    print "ok $ok\n";
	}
	else {
	    nok ($ok, $i3, '<=>', $j3, $cmp, $i, $j, '==');
	}
	$ok++;
	if (!defined($cmp) ? !($i5 > $j5)
	    : ($cmp == -1 && !($i5 > $j5) ||
	       $cmp == 0  && !($i5 > $j5) ||
	       $cmp == 1  && ($i5 > $j5)))
	{
	    print "ok $ok\n";
	}
	else {
	    nok ($ok, $i3, '<=>', $j3, $cmp, $i, $j, '>');
	}
	$ok++;
	if (!defined($cmp) ? !($i6 >= $j6)
	    : ($cmp == -1 && !($i6 >= $j6) ||
	       $cmp == 0  && $i6 >= $j6 ||
	       $cmp == 1  && $i6 >= $j6))
	{
	    print "ok $ok\n";
	}
	else {
	    nok ($ok, $i3, '<=>', $j3, $cmp, $i, $j, '>=');
	}
	$ok++;
	# OK, so the docs are wrong it seems. NaN != NaN
	if (!defined($cmp) ? ($i7 != $j7)
	    : ($cmp == -1 && $i7 != $j7 ||
	       $cmp == 0  && !($i7 != $j7) ||
	       $cmp == 1  && $i7 != $j7))
	{
	    print "ok $ok\n";
	}
	else {
	    nok ($ok, $i3, '<=>', $j3, $cmp, $i, $j, '!=');
	}
	$ok++;
	if (!defined($cmp) ? !($i8 <= $j8)
	    : ($cmp == -1 && $i8 <= $j8 ||
	       $cmp == 0  && $i8 <= $j8 ||
	       $cmp == 1  && !($i8 <= $j8)))
	{
	    print "ok $ok\n";
	}
	else {
	    nok ($ok, $i3, '<=>', $j3, $cmp, $i, $j, '<=');
	}
	$ok++;
        my $pmc =  $j16 <=> $i16; # cmp it in reverse
        # Should give -ve of other answer, or undef for NaNs
        # a + -a should be zero. not zero is truth. which avoids using ==
	if (defined($cmp) ? !($cmp + $pmc) : !defined $pmc)
	{
	    print "ok $ok\n";
	}
	else {
	    nok ($ok, $i3, '<=>', $j3, $cmp, $i, $j, '<=> transposed');
	}


	# String comparisons
	$ok++;
	$cmp = $i9 cmp $j9;
	if ($cmp == -1 && $i10 lt $j10 ||
	    $cmp == 0  && !($i10 lt $j10) ||
	    $cmp == 1  && !($i10 lt $j10))
	{
	    print "ok $ok\n";
	}
	else {
	    nok ($ok, $i3, 'cmp', $j3, $cmp, $i, $j, 'lt');
	}
	$ok++;
	if ($cmp == -1 && !($i11 eq $j11) ||
	    $cmp == 0  && ($i11 eq $j11) ||
	    $cmp == 1  && !($i11 eq $j11))
	{
	    print "ok $ok\n";
	}
	else {
	    nok ($ok, $i3, 'cmp', $j3, $cmp, $i, $j, 'eq');
	}
	$ok++;
	if ($cmp == -1 && !($i12 gt $j12) ||
	    $cmp == 0  && !($i12 gt $j12) ||
	    $cmp == 1  && ($i12 gt $j12))
	{
	    print "ok $ok\n";
	}
	else {
	    nok ($ok, $i3, 'cmp', $j3, $cmp, $i, $j, 'gt');
	}
	$ok++;
	if ($cmp == -1 && $i13 le $j13 ||
	    $cmp == 0  && ($i13 le $j13) ||
	    $cmp == 1  && !($i13 le $j13))
	{
	    print "ok $ok\n";
	}
	else {
	    nok ($ok, $i3, 'cmp', $j3, $cmp, $i, $j, 'le');
	}
	$ok++;
	if ($cmp == -1 && ($i14 ne $j14) ||
	    $cmp == 0  && !($i14 ne $j14) ||
	    $cmp == 1  && ($i14 ne $j14))
	{
	    print "ok $ok\n";
	}
	else {
	    nok ($ok, $i3, 'cmp', $j3, $cmp, $i, $j, 'ne');
	}
	$ok++;
	if ($cmp == -1 && !($i15 ge $j15) ||
	    $cmp == 0  && ($i15 ge $j15) ||
	    $cmp == 1  && ($i15 ge $j15))
	{
	    print "ok $ok\n";
	}
	else {
	    nok ($ok, $i3, 'cmp', $j3, $cmp, $i, $j, 'ge');
	}
	$ok++;
        $pmc =  $j17 cmp $i17; # cmp it in reverse
        # Should give -ve of other answer
        # a + -a should be zero. not zero is truth. which avoids using ==
	if (!($cmp + $pmc))
	{
	    print "ok $ok\n";
	}
	else {
	    nok ($ok, $i3, 'cmp', $j3, $cmp, $i, $j, 'cmp transposed');
	}
    }
}

# We know the answers for these. We can rely on the consistency checks above
# to test the other string comparisons.

while (my ($i, $v) = each @raw) {
    # Copy, to avoid any inadvertent conversion
    my ($raw, $cooked, $not);
    $raw = $v;
    $cooked = $upgraded[$i];
    $not = $raw eq $cooked ? '' : 'not ';
    printf "%sok %d # eq, chr %d\n", $not, ++$ok, ord $raw;

    $raw = $v;
    $cooked = $upgraded[$i];
    $not = $raw ne $cooked ? 'not ' : '';
    printf "%sok %d # ne, chr %d\n", $not, ++$ok, ord $raw;

    $raw = $v;
    $cooked = $upgraded[$i];
    $not = (($raw cmp $cooked) == 0) ? '' : 'not ';
    printf "%sok %d # cmp, chr %d\n", $not, ++$ok, ord $raw;

    # And now, transposed.
    $raw = $v;
    $cooked = $upgraded[$i];
    $not = $cooked eq $raw ? '' : 'not ';
    printf "%sok %d # eq, chr %d\n", $not, ++$ok, ord $raw;

    $raw = $v;
    $cooked = $upgraded[$i];
    $not = $cooked ne $raw ? 'not ' : '';
    printf "%sok %d # ne, chr %d\n", $not, ++$ok, ord $raw;

    $raw = $v;
    $cooked = $upgraded[$i];
    $not = (($cooked cmp $raw) == 0) ? '' : 'not ';
    printf "%sok %d # cmp, chr %d\n", $not, ++$ok, ord $raw;
}

while (my ($i, $v) = each @utf8) {
    # Copy, to avoid any inadvertent conversion
    my ($raw, $cooked, $not);
    $raw = $raw[$i];
    $cooked = $v;
    $not = $raw eq $cooked ? 'not ' : '';
    printf "%sok %d # eq vs octets, chr %d\n", $not, ++$ok, ord $raw;

    $raw = $raw[$i];
    $cooked = $v;
    $not = $raw ne $cooked ? '' : 'not ';
    printf "%sok %d # ne vs octets, chr %d\n", $not, ++$ok, ord $raw;

    $raw = $raw[$i];
    $cooked = $v;
    $not = (($raw cmp $cooked) == 0) ? 'not ' : '';
    printf "%sok %d # cmp vs octects, chr %d\n", $not, ++$ok, ord $raw;

    # And now, transposed.
    $raw = $raw[$i];
    $cooked = $v;
    $not = $cooked eq $raw ? 'not ' : '';
    printf "%sok %d # eq vs octets, chr %d\n", $not, ++$ok, ord $raw;

    $raw = $raw[$i];
    $cooked = $v;
    $not = $cooked ne $raw? '' : 'not ';
    printf "%sok %d # ne vs octets, chr %d\n", $not, ++$ok, ord $raw;

    $raw = $raw[$i];
    $cooked = $v;
    $not = (($cooked cmp $raw) == 0) ? 'not ' : '';
    printf "%sok %d # cmp vs octects, chr %d\n", $not, ++$ok, ord $raw;
}
