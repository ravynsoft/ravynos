#!./perl

# This tests the behavior of sort() under the different 'use sort' forms.
# Algorithm by John P. Linderman.

my ($BigWidth, $BigEnough, $RootWidth, $ItemFormat, @TestSizes, $WellSoaked);

BEGIN {
    chdir 't' if -d 't';
    @INC = qw(../lib);
    $BigWidth  = 6;				# Digits in $BigEnough-1
    $BigEnough = 10**$BigWidth;			# Largest array we'll attempt
    $RootWidth = int(($BigWidth+1)/2);		# Digits in sqrt($BigEnough-1)
    $ItemFormat = "%0${RootWidth}d%0${BigWidth}d";	# Array item format
    @TestSizes = (0, 1, 2);			# Small special cases
    # Testing all the way up to $BigEnough takes too long
    # for casual testing.  There are some cutoffs (~256)
    # in pp_sort that should be tested, but 10_000 is ample.
    $WellSoaked = 10_000;			# <= $BigEnough
    for (my $ts = 3; $ts < $WellSoaked; $ts *= 10**(1/3)) {
	push(@TestSizes, int($ts));		# about 3 per decade
    }
}

use strict;
use warnings;

use Test::More;

# Generate array of specified size for testing sort.
#
# We ensure repeated items, where possible, by drawing the $size items
# from a pool of size sqrt($size).  Each randomly chosen item is
# tagged with the item index, so we can detect original input order,
# and reconstruct the original array order.

sub genarray {
    my $size = int(shift);		# fractions not welcome
    my ($items, $i);
    my @a;

    if    ($size < 0) { $size = 0; }	# avoid complexity with sqrt
    elsif ($size > $BigEnough) { $size = $BigEnough; }
    $#a = $size - 1;			# preallocate array
    $items = int(sqrt($size));		# number of distinct items
    for ($i = 0; $i < $size; ++$i) {
	$a[$i] = sprintf($ItemFormat, int($items * rand()), $i);
    }
    return \@a;
}


# Check for correct order (including stability)

sub checkorder {
    my $aref = shift;
    my $status = '';			# so far, so good
    my ($i, $disorder);

    for ($i = 0; $i < $#$aref; ++$i) {
	# Equality shouldn't happen, but catch it in the contents check
	next if ($aref->[$i] le $aref->[$i+1]);
	$disorder = (substr($aref->[$i],   0, $RootWidth) eq
		     substr($aref->[$i+1], 0, $RootWidth)) ?
		     "Instability" : "Disorder";
	# Keep checking if merely unstable... disorder is much worse.
	$status =
	    "$disorder at element $i between $aref->[$i] and $aref->[$i+1]";
	last unless ($disorder eq "Instability");	
    }
    return $status;
}


# Verify that the two array refs reference identical arrays

sub checkequal {
    my ($aref, $bref) = @_;
    my $status = '';
    my $i;

    if (@$aref != @$bref) {
	$status = "Sizes differ: " . @$aref . " vs " . @$bref;
    } else {
	for ($i = 0; $i < @$aref; ++$i) {
	    next if ($aref->[$i] eq $bref->[$i]);
	    $status = "Element $i differs: $aref->[$i] vs $bref->[$i]";
	    last;
	}
    }
    return $status;
}


# Test sort on arrays of various sizes (set up in @TestSizes)

sub main {
    my ($dothesort, $expect_unstable) = @_;
    my ($ts, $unsorted, @sorted, $status);
    my $unstable_num = 0;

    foreach $ts (@TestSizes) {
	$unsorted = genarray($ts);
	# Sort only on item portion of each element.
	# There will typically be many repeated items,
	# and their order had better be preserved.
	@sorted = $dothesort->(sub { substr($a, 0, $RootWidth)
				    cmp
	                 substr($b, 0, $RootWidth) }, $unsorted);
	$status = checkorder(\@sorted);
	# Put the items back into the original order.
	# The contents of the arrays had better be identical.
	if ($expect_unstable && $status =~ /^Instability/) {
	    $status = '';
	    ++$unstable_num;
	}
	is($status, '', "order ok for size $ts");
	@sorted = $dothesort->(sub { substr($a, $RootWidth)
				    cmp
			    substr($b, $RootWidth) }, \@sorted);
	$status = checkequal(\@sorted, $unsorted);
	is($status, '', "contents ok for size $ts");
    }
    if ($expect_unstable) {
	ok($unstable_num > 0, 'Instability ok');
    }
}

# Test with no pragma yet loaded. Stability is expected from default sort.
main(sub { sort {&{$_[0]}} @{$_[1]} }, 0);

# Verify that we have eliminated the segfault that could be triggered
# by invoking a sort as part of a comparison routine.
# No need for an explicit test. If we don't segfault, we're good.

{
    sub dumbsort {
	my ($a, $b) = @_;
	use sort qw( defaults stable );
	my @ignore = sort (5,4,3,2,1);
	return $a <=> $b;
    }
    use sort qw( defaults stable );
    my @nested = sort { dumbsort($a,$b) } (3,2,2,1);
}

{
    use sort qw(stable);
    my $sort_current;
    BEGIN {
        my $a = "" ;
        local $SIG{__WARN__} = sub {$a = $_[0]};
        $sort_current = sort::current();
        like($a, qr/\Asort::current is deprecated\b/, "sort::current warns");
    }
    is($sort_current, 'stable', 'sort::current for stable');
    main(sub { sort {&{$_[0]}} @{$_[1]} }, 0);
}

# Tests added to check "defaults" subpragma, and "no sort"

{
    use sort qw(defaults stable);
    my $sort_current;
    BEGIN {
        my $a = "" ;
        local $SIG{__WARN__} = sub {$a = $_[0]};
        $sort_current = sort::current();
        like($a, qr/\Asort::current is deprecated\b/, "sort::current warns");
    }
    is($sort_current, 'stable', 'sort::current after defaults stable');
    main(sub { sort {&{$_[0]}} @{$_[1]} }, 0);
}

# Tests added to check how sort::current is deprecated

{
    no sort qw(stable);
    my $sort_current;
    BEGIN {
        my $a = "" ;
        local $SIG{__WARN__} = sub {$a = $_[0]};
        $sort_current = sort::current();
        like($a, qr/\Asort::current is deprecated\b/, "sort::current warns");
    }
    is($sort_current, 'stable', 'sort::current *always* stable');
}

{
    use sort qw(defaults);
    my $sort_current;
    BEGIN {
        no warnings qw(deprecated);
        my $a = "" ;
        local $SIG{__WARN__} = sub {$a = $_[0]};
        $sort_current = sort::current();
        is($a, "", "sort::current warning can be disabled");
    }
    is($sort_current, 'stable', 'sort::current *always* stable');
}

{
    use sort qw(stable);
    my $sort_current;
    BEGIN {
        no warnings qw(deprecated);
        my $a = "" ;
        local $SIG{__WARN__} = sub {$a = $_[0]};
        $sort_current = sort::current();
        is($a, "", "sort::current warning can be disabled");
    }
    is($sort_current, 'stable', 'sort::current for stable');
}

done_testing();
