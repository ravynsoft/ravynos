# This is a rather minimalistic library, whose purpose is to test inheritance
# from its parent class.

package Math::BigInt::Lib::Minimal;

use 5.006001;
use strict;
use warnings;

use Carp;
use Math::BigInt::Lib;

our @ISA = ('Math::BigInt::Lib');

my $BASE_LEN = 5;
my $BASE     = 0 + ("1" . ("0" x $BASE_LEN));
my $MAX_VAL  = $BASE - 1;

sub _new {
    my ($class, $str) = @_;
    croak "Invalid input string '$str'" unless $str =~ /^([1-9]\d*|0)\z/;

    my $n = length $str;
    my $p = int($n / $BASE_LEN);
    my $q = $n % $BASE_LEN;

    my $format = $] < 5.008 ? "a$BASE_LEN" x $p
                            : "(a$BASE_LEN)*";
    $format = "a$q" . $format if $q > 0;

    my $self = [ reverse(map { 0 + $_ } unpack($format, $str)) ];
    return bless $self, $class;
}

##############################################################################
# convert to string

sub _str {
    my ($class, $x) = @_;
    my $idx = $#$x;             # index of last element

    # Handle first one differently, since it should not have any leading zeros.

    my $str = int($x->[$idx]);

    if ($idx > 0) {
        my $z = '0' x ($BASE_LEN - 1);
        while (--$idx >= 0) {
            $str .= substr($z . $x->[$idx], -$BASE_LEN);
        }
    }
    $str;
}

##############################################################################
# actual math code

sub _add {
    # (ref to int_num_array, ref to int_num_array)
    #
    # Routine to add two base 1eX numbers stolen from Knuth Vol 2 Algorithm A
    # pg 231. There are separate routines to add and sub as per Knuth pg 233.
    # This routine modifies array x, but not y.

    my ($c, $x, $y) = @_;

    # $x + 0 => $x

    return $x if @$y == 1 && $y->[0] == 0;

    # 0 + $y => $y->copy

    if (@$x == 1 && $x->[0] == 0) {
        @$x = @$y;
        return $x;
    }

    # For each in Y, add Y to X and carry. If after that, something is left in
    # X, foreach in X add carry to X and then return X, carry. Trades one
    # "$j++" for having to shift arrays.

    my $i;
    my $car = 0;
    my $j = 0;
    for $i (@$y) {
        $x->[$j] -= $BASE if $car = (($x->[$j] += $i + $car) >= $BASE) ? 1 : 0;
        $j++;
    }
    while ($car != 0) {
        $x->[$j] -= $BASE if $car = (($x->[$j] += $car) >= $BASE) ? 1 : 0;
        $j++;
    }

    $x;
}

sub _sub {
    # (ref to int_num_array, ref to int_num_array, swap)
    #
    # Subtract base 1eX numbers -- stolen from Knuth Vol 2 pg 232, $x > $y
    # subtract Y from X by modifying x in place
    my ($c, $sx, $sy, $s) = @_;

    my $car = 0;
    my $i;
    my $j = 0;
    if (!$s) {
        for $i (@$sx) {
            last unless defined $sy->[$j] || $car;
            $i += $BASE if $car = (($i -= ($sy->[$j] || 0) + $car) < 0);
            $j++;
        }
        # might leave leading zeros, so fix that
        return __strip_zeros($sx);
    }
    for $i (@$sx) {
        # We can't do an early out if $x < $y, since we need to copy the high
        # chunks from $y. Found by Bob Mathews.
        #last unless defined $sy->[$j] || $car;
        $sy->[$j] += $BASE
          if $car = ($sy->[$j] = $i - ($sy->[$j] || 0) - $car) < 0;
        $j++;
    }
    # might leave leading zeros, so fix that
    __strip_zeros($sy);
}

# The following _mul function is an exact copy of _mul_use_div_64 in
# Math::BigInt::Calc.

sub _mul {
    # (ref to int_num_array, ref to int_num_array)
    # multiply two numbers in internal representation
    # modifies first arg, second need not be different from first
    # works for 64 bit integer with "use integer"
    my ($c, $xv, $yv) = @_;

    use integer;
    if (@$yv == 1) {
        # shortcut for two small numbers, also handles $x == 0
        if (@$xv == 1) {
            # shortcut for two very short numbers (improved by Nathan Zook)
            # works also if xv and yv are the same reference, and handles also $x == 0
            if (($xv->[0] *= $yv->[0]) >= $BASE) {
                $xv->[0] =
                  $xv->[0] - ($xv->[1] = $xv->[0] / $BASE) * $BASE;
            }
            return $xv;
        }
        # $x * 0 => 0
        if ($yv->[0] == 0) {
            @$xv = (0);
            return $xv;
        }
        # multiply a large number a by a single element one, so speed up
        my $y = $yv->[0];
        my $car = 0;
        foreach my $i (@$xv) {
            #$i = $i * $y + $car; $car = $i / $BASE; $i -= $car * $BASE;
            $i = $i * $y + $car;
            $i -= ($car = $i / $BASE) * $BASE;
        }
        push @$xv, $car if $car != 0;
        return $xv;
    }
    # shortcut for result $x == 0 => result = 0
    return $xv if ( ((@$xv == 1) && ($xv->[0] == 0)) );

    # since multiplying $x with $x fails, make copy in this case
    $yv = $c->_copy($xv) if $xv == $yv; # same references?

    my @prod = ();
    my ($prod, $car, $cty, $xi, $yi);
    for $xi (@$xv) {
        $car = 0;
        $cty = 0;
        # looping through this if $xi == 0 is silly - so optimize it away!
        $xi = (shift @prod || 0), next if $xi == 0;
        for $yi (@$yv) {
            $prod = $xi * $yi + ($prod[$cty] || 0) + $car;
            $prod[$cty++] = $prod - ($car = $prod / $BASE) * $BASE;
        }
        $prod[$cty] += $car if $car; # need really to check for 0?
        $xi = shift @prod || 0;      # || 0 makes v5.005_3 happy
    }
    push @$xv, @prod;
    $xv;
}

# The following _div function is an exact copy of _div_use_div_64 in
# Math::BigInt::Calc.

sub _div {
    # ref to array, ref to array, modify first array and return remainder if
    # in list context
    # This version works on 64 bit integers
    my ($c, $x, $yorg) = @_;

    use integer;
    # the general div algorithm here is about O(N*N) and thus quite slow, so
    # we first check for some special cases and use shortcuts to handle them.

    # This works, because we store the numbers in a chunked format where each
    # element contains 5..7 digits (depending on system).

    # if both numbers have only one element:
    if (@$x == 1 && @$yorg == 1) {
        # shortcut, $yorg and $x are two small numbers
        if (wantarray) {
            my $rem = [ $x->[0] % $yorg->[0] ];
            bless $rem, $c;
            $x->[0] = int($x->[0] / $yorg->[0]);
            return ($x, $rem);
        } else {
            $x->[0] = int($x->[0] / $yorg->[0]);
            return $x;
        }
    }
    # if x has more than one, but y has only one element:
    if (@$yorg == 1) {
        my $rem;
        $rem = $c->_mod($c->_copy($x), $yorg) if wantarray;

        # shortcut, $y is < $BASE
        my $j = @$x;
        my $r = 0;
        my $y = $yorg->[0];
        my $b;
        while ($j-- > 0) {
            $b = $r * $BASE + $x->[$j];
            $x->[$j] = int($b/$y);
            $r = $b % $y;
        }
        pop @$x if @$x > 1 && $x->[-1] == 0; # splice up a leading zero
        return ($x, $rem) if wantarray;
        return $x;
    }
    # now x and y have more than one element

    # check whether y has more elements than x, if yet, the result will be 0
    if (@$yorg > @$x) {
        my $rem;
        $rem = $c->_copy($x) if wantarray;    # make copy
        @$x = 0;                        # set to 0
        return ($x, $rem) if wantarray; # including remainder?
        return $x;                      # only x, which is [0] now
    }
    # check whether the numbers have the same number of elements, in that case
    # the result will fit into one element and can be computed efficiently
    if (@$yorg == @$x) {
        my $rem;
        # if $yorg has more digits than $x (it's leading element is longer than
        # the one from $x), the result will also be 0:
        if (length(int($yorg->[-1])) > length(int($x->[-1]))) {
            $rem = $c->_copy($x) if wantarray;     # make copy
            @$x = 0;                          # set to 0
            return ($x, $rem) if wantarray; # including remainder?
            return $x;
        }
        # now calculate $x / $yorg

        if (length(int($yorg->[-1])) == length(int($x->[-1]))) {
            # same length, so make full compare

            my $a = 0;
            my $j = @$x - 1;
            # manual way (abort if unequal, good for early ne)
            while ($j >= 0) {
                last if ($a = $x->[$j] - $yorg->[$j]);
                $j--;
            }
            # $a contains the result of the compare between X and Y
            # a < 0: x < y, a == 0: x == y, a > 0: x > y
            if ($a <= 0) {
                $rem = $c->_zero();                  # a = 0 => x == y => rem 0
                $rem = $c->_copy($x) if $a != 0;       # a < 0 => x < y => rem = x
                @$x = 0;                       # if $a < 0
                $x->[0] = 1 if $a == 0;        # $x == $y
                return ($x, $rem) if wantarray; # including remainder?
                return $x;
            }
            # $x >= $y, so proceed normally
        }
    }

    # all other cases:

    my $y = $c->_copy($yorg);         # always make copy to preserve

    my ($car, $bar, $prd, $dd, $xi, $yi, @q, $v2, $v1, @d, $tmp, $q, $u2, $u1, $u0);

    $car = $bar = $prd = 0;
    if (($dd = int($BASE / ($y->[-1] + 1))) != 1) {
        for $xi (@$x) {
            $xi = $xi * $dd + $car;
            $xi -= ($car = int($xi / $BASE)) * $BASE;
        }
        push(@$x, $car);
        $car = 0;
        for $yi (@$y) {
            $yi = $yi * $dd + $car;
            $yi -= ($car = int($yi / $BASE)) * $BASE;
        }
    } else {
        push(@$x, 0);
    }

    # @q will accumulate the final result, $q contains the current computed
    # part of the final result

    @q = ();
    ($v2, $v1) = @$y[-2, -1];
    $v2 = 0 unless $v2;
    while ($#$x > $#$y) {
        ($u2, $u1, $u0) = @$x[-3..-1];
        $u2 = 0 unless $u2;
        #warn "oups v1 is 0, u0: $u0 $y->[-2] $y->[-1] l ",scalar @$y,"\n"
        # if $v1 == 0;
        $q = (($u0 == $v1) ? $MAX_VAL : int(($u0 * $BASE + $u1) / $v1));
        --$q while ($v2 * $q > ($u0 * $BASE +$ u1- $q*$v1) * $BASE + $u2);
        if ($q) {
            ($car, $bar) = (0, 0);
            for ($yi = 0, $xi = $#$x - $#$y - 1; $yi <= $#$y; ++$yi, ++$xi) {
                $prd = $q * $y->[$yi] + $car;
                $prd -= ($car = int($prd / $BASE)) * $BASE;
                $x->[$xi] += $BASE if ($bar = (($x->[$xi] -= $prd + $bar) < 0));
            }
            if ($x->[-1] < $car + $bar) {
                $car = 0;
                --$q;
                for ($yi = 0, $xi = $#$x - $#$y - 1; $yi <= $#$y; ++$yi, ++$xi) {
                    $x->[$xi] -= $BASE
                      if ($car = (($x->[$xi] += $y->[$yi] + $car) >= $BASE));
                }
            }
        }
        pop(@$x);
        unshift(@q, $q);
    }
    if (wantarray) {
        my $d = bless [], $c;
        if ($dd != 1) {
            $car = 0;
            for $xi (reverse @$x) {
                $prd = $car * $BASE + $xi;
                $car = $prd - ($tmp = int($prd / $dd)) * $dd;
                unshift(@$d, $tmp);
            }
        } else {
            @$d = @$x;
        }
        @$x = @q;
        __strip_zeros($x);
        __strip_zeros($d);
        return ($x, $d);
    }
    @$x = @q;
    __strip_zeros($x);
    $x;
}

# The following _mod function is an exact copy of _mod in Math::BigInt::Calc.

sub _mod {
    # if possible, use mod shortcut
    my ($c, $x, $yo) = @_;

    # slow way since $y too big
    if (@$yo > 1) {
        my ($xo, $rem) = $c->_div($x, $yo);
        @$x = @$rem;
        return $x;
    }

    my $y = $yo->[0];

    # if both are single element arrays
    if (@$x == 1) {
        $x->[0] %= $y;
        return $x;
    }

    # if @$x has more than one element, but @$y is a single element
    my $b = $BASE % $y;
    if ($b == 0) {
        # when BASE % Y == 0 then (B * BASE) % Y == 0
        # (B * BASE) % $y + A % Y => A % Y
        # so need to consider only last element: O(1)
        $x->[0] %= $y;
    } elsif ($b == 1) {
        # else need to go through all elements in @$x: O(N), but loop is a bit
        # simplified
        my $r = 0;
        foreach (@$x) {
            $r = ($r + $_) % $y; # not much faster, but heh...
            #$r += $_ % $y; $r %= $y;
        }
        $r = 0 if $r == $y;
        $x->[0] = $r;
    } else {
        # else need to go through all elements in @$x: O(N)
        my $r = 0;
        my $bm = 1;
        foreach (@$x) {
            $r = ($_ * $bm + $r) % $y;
            $bm = ($bm * $b) % $y;

            #$r += ($_ % $y) * $bm;
            #$bm *= $b;
            #$bm %= $y;
            #$r %= $y;
        }
        $r = 0 if $r == $y;
        $x->[0] = $r;
    }
    @$x = $x->[0];              # keep one element of @$x
    return $x;
}

sub __strip_zeros {
    # Internal normalization function that strips leading zeros from the array.
    # Args: ref to array
    my $x = shift;

    push @$x, 0 if @$x == 0;    # div might return empty results, so fix it
    return $x if @$x == 1;      # early out

    #print "strip: cnt $cnt i $i\n";
    # '0', '3', '4', '0', '0',
    #  0    1    2    3    4
    # cnt = 5, i = 4
    # i = 4
    # i = 3
    # => fcnt = cnt - i (5-2 => 3, cnt => 5-1 = 4, throw away from 4th pos)
    # >= 1: skip first part (this can be zero)

    my $i = $#$x;
    while ($i > 0) {
        last if $x->[$i] != 0;
        $i--;
    }
    $i++;
    splice(@$x, $i) if $i < @$x;
    $x;
}

###############################################################################
# check routine to test internal state for corruptions

sub _check {
    # used by the test suite
    my ($class, $x) = @_;

    return "Undefined" unless defined $x;
    return "$x is not a reference" unless ref($x);
    return "Not an '$class'" unless ref($x) eq $class;

    for (my $i = 0 ; $i <= $#$x ; ++ $i) {
        my $e = $x -> [$i];

        return "Element at index $i is undefined"
          unless defined $e;

        return "Element at index $i is a '" . ref($e) .
          "', which is not a scalar"
          unless ref($e) eq "";

        return "Element at index $i is '$e', which does not look like an" .
          " normal integer"
            #unless $e =~ /^([1-9]\d*|0)\z/;
            unless $e =~ /^\d+\z/;

        return "Element at index $i is '$e', which is negative"
          if $e < 0;

        return "Element at index $i is '$e', which is not smaller than" .
          " the base '$BASE'"
            if $e >= $BASE;

        return "Element at index $i (last element) is zero"
          if $#$x > 0 && $i == $#$x && $e == 0;
    }

    return 0;
}

1;
