package Math::BigInt::Lib;

use 5.006001;
use strict;
use warnings;

our $VERSION = '1.999837';
$VERSION =~ tr/_//d;

use Carp;

use overload

  # overload key: with_assign

  '+'    => sub {
                my $class = ref $_[0];
                my $x = $class -> _copy($_[0]);
                my $y = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                return $class -> _add($x, $y);
            },

  '-'    => sub {
                my $class = ref $_[0];
                my ($x, $y);
                if ($_[2]) {            # if swapped
                    $y = $_[0];
                    $x = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                } else {
                    $x = $class -> _copy($_[0]);
                    $y = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                }
                return $class -> _sub($x, $y);
            },

  '*'    => sub {
                my $class = ref $_[0];
                my $x = $class -> _copy($_[0]);
                my $y = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                return $class -> _mul($x, $y);
            },

  '/'    => sub {
                my $class = ref $_[0];
                my ($x, $y);
                if ($_[2]) {            # if swapped
                    $y = $_[0];
                    $x = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                } else {
                    $x = $class -> _copy($_[0]);
                    $y = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                }
                return $class -> _div($x, $y);
            },

  '%'    => sub {
                my $class = ref $_[0];
                my ($x, $y);
                if ($_[2]) {            # if swapped
                    $y = $_[0];
                    $x = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                } else {
                    $x = $class -> _copy($_[0]);
                    $y = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                }
                return $class -> _mod($x, $y);
            },

  '**'   => sub {
                my $class = ref $_[0];
                my ($x, $y);
                if ($_[2]) {            # if swapped
                    $y = $_[0];
                    $x = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                } else {
                    $x = $class -> _copy($_[0]);
                    $y = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                }
                return $class -> _pow($x, $y);
            },

  '<<'   => sub {
                my $class = ref $_[0];
                my ($x, $y);
                if ($_[2]) {            # if swapped
                    $y = $class -> _num($_[0]);
                    $x = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                } else {
                    $x = $_[0];
                    $y = ref($_[1]) ? $class -> _num($_[1]) : $_[1];
                }
                return $class -> _lsft($x, $y);
            },

  '>>'   => sub {
                my $class = ref $_[0];
                my ($x, $y);
                if ($_[2]) {            # if swapped
                    $y = $_[0];
                    $x = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                } else {
                    $x = $class -> _copy($_[0]);
                    $y = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                }
                return $class -> _rsft($x, $y);
            },

  # overload key: num_comparison

  '<'    => sub {
                my $class = ref $_[0];
                my ($x, $y);
                if ($_[2]) {            # if swapped
                    $y = $_[0];
                    $x = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                } else {
                    $x = $class -> _copy($_[0]);
                    $y = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                }
                return $class -> _acmp($x, $y) < 0;
            },

  '<='   => sub {
                my $class = ref $_[0];
                my ($x, $y);
                if ($_[2]) {            # if swapped
                    $y = $_[0];
                    $x = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                } else {
                    $x = $class -> _copy($_[0]);
                    $y = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                }
                return $class -> _acmp($x, $y) <= 0;
            },

  '>'    => sub {
                my $class = ref $_[0];
                my ($x, $y);
                if ($_[2]) {            # if swapped
                    $y = $_[0];
                    $x = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                } else {
                    $x = $class -> _copy($_[0]);
                    $y = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                }
                return $class -> _acmp($x, $y) > 0;
            },

  '>='   => sub {
                my $class = ref $_[0];
                my ($x, $y);
                if ($_[2]) {            # if swapped
                    $y = $_[0];
                    $x = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                } else {
                    $x = $class -> _copy($_[0]);
                    $y = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                }
                return $class -> _acmp($x, $y) >= 0;
          },

  '=='   => sub {
                my $class = ref $_[0];
                my $x = $class -> _copy($_[0]);
                my $y = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                return $class -> _acmp($x, $y) == 0;
            },

  '!='   => sub {
                my $class = ref $_[0];
                my $x = $class -> _copy($_[0]);
                my $y = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                return $class -> _acmp($x, $y) != 0;
            },

  # overload key: 3way_comparison

  '<=>'  => sub {
                my $class = ref $_[0];
                my ($x, $y);
                if ($_[2]) {            # if swapped
                    $y = $_[0];
                    $x = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                } else {
                    $x = $class -> _copy($_[0]);
                    $y = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                }
                return $class -> _acmp($x, $y);
            },

  # overload key: binary

  '&'    => sub {
                my $class = ref $_[0];
                my ($x, $y);
                if ($_[2]) {            # if swapped
                    $y = $_[0];
                    $x = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                } else {
                    $x = $class -> _copy($_[0]);
                    $y = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                }
                return $class -> _and($x, $y);
            },

  '|'    => sub {
                my $class = ref $_[0];
                my ($x, $y);
                if ($_[2]) {            # if swapped
                    $y = $_[0];
                    $x = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                } else {
                    $x = $class -> _copy($_[0]);
                    $y = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                }
                return $class -> _or($x, $y);
            },

  '^'    => sub {
                my $class = ref $_[0];
                my ($x, $y);
                if ($_[2]) {            # if swapped
                    $y = $_[0];
                    $x = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                } else {
                    $x = $class -> _copy($_[0]);
                    $y = ref($_[1]) ? $_[1] : $class -> _new($_[1]);
                }
                return $class -> _xor($x, $y);
            },

  # overload key: func

  'abs'  => sub { $_[0] },

  'sqrt' => sub {
                my $class = ref $_[0];
                return $class -> _sqrt($class -> _copy($_[0]));
            },

  'int'  => sub { $_[0] },

  # overload key: conversion

  'bool' => sub { ref($_[0]) -> _is_zero($_[0]) ? '' : 1; },

  '""'   => sub { ref($_[0]) -> _str($_[0]); },

  '0+'   => sub { ref($_[0]) -> _num($_[0]); },

  '='    => sub { ref($_[0]) -> _copy($_[0]); },

  ;

sub _new {
    croak "@{[(caller 0)[3]]} method not implemented";
}

sub _zero {
    my $class = shift;
    return $class -> _new("0");
}

sub _one {
    my $class = shift;
    return $class -> _new("1");
}

sub _two {
    my $class = shift;
    return $class -> _new("2");

}
sub _ten {
    my $class = shift;
    return $class -> _new("10");
}

sub _1ex {
    my ($class, $exp) = @_;
    $exp = $class -> _num($exp) if ref($exp);
    return $class -> _new("1" . ("0" x $exp));
}

sub _copy {
    my ($class, $x) = @_;
    return $class -> _new($class -> _str($x));
}

# catch and throw away
sub import { }

##############################################################################
# convert back to string and number

sub _str {
    # Convert number from internal base 1eN format to string format. Internal
    # format is always normalized, i.e., no leading zeros.
    croak "@{[(caller 0)[3]]} method not implemented";
}

sub _num {
    my ($class, $x) = @_;
    0 + $class -> _str($x);
}

##############################################################################
# actual math code

sub _add {
    croak "@{[(caller 0)[3]]} method not implemented";
}

sub _sub {
    croak "@{[(caller 0)[3]]} method not implemented";
}

sub _mul {
    my ($class, $x, $y) = @_;
    my $sum = $class -> _zero();
    my $i   = $class -> _zero();
    while ($class -> _acmp($i, $y) < 0) {
        $sum = $class -> _add($sum, $x);
        $i   = $class -> _inc($i);
    }
    return $sum;
}

sub _div {
    my ($class, $x, $y) = @_;

    croak "@{[(caller 0)[3]]} requires non-zero divisor"
      if $class -> _is_zero($y);

    my $r = $class -> _copy($x);
    my $q = $class -> _zero();
    while ($class -> _acmp($r, $y) >= 0) {
        $q = $class -> _inc($q);
        $r = $class -> _sub($r, $y);
    }

    return $q, $r if wantarray;
    return $q;
}

sub _inc {
    my ($class, $x) = @_;
    $class -> _add($x, $class -> _one());
}

sub _dec {
    my ($class, $x) = @_;
    $class -> _sub($x, $class -> _one());
}

# Signed addition. If the flag is false, $xa might be modified, but not $ya. If
# the false is true, $ya might be modified, but not $xa.

sub _sadd {
    my $class = shift;
    my ($xa, $xs, $ya, $ys, $flag) = @_;
    my ($za, $zs);

    # If the signs are equal we can add them (-5 + -3 => -(5 + 3) => -8)

    if ($xs eq $ys) {
        if ($flag) {
            $za = $class -> _add($ya, $xa);
        } else {
            $za = $class -> _add($xa, $ya);
        }
        $zs = $class -> _is_zero($za) ? '+' : $xs;
        return $za, $zs;
    }

    my $acmp = $class -> _acmp($xa, $ya);       # abs(x) = abs(y)

    if ($acmp == 0) {                           # x = -y or -x = y
        $za = $class -> _zero();
        $zs = '+';
        return $za, $zs;
    }

    if ($acmp > 0) {                            # abs(x) > abs(y)
        $za = $class -> _sub($xa, $ya, $flag);
        $zs = $xs;
    } else {                                    # abs(x) < abs(y)
        $za = $class -> _sub($ya, $xa, !$flag);
        $zs = $ys;
    }
    return $za, $zs;
}

# Signed subtraction. If the flag is false, $xa might be modified, but not $ya.
# If the false is true, $ya might be modified, but not $xa.

sub _ssub {
    my $class = shift;
    my ($xa, $xs, $ya, $ys, $flag) = @_;

    # Swap sign of second operand and let _sadd() do the job.
    $ys = $ys eq '+' ? '-' : '+';
    $class -> _sadd($xa, $xs, $ya, $ys, $flag);
}

##############################################################################
# testing

sub _acmp {
    # Compare two (absolute) values. Return -1, 0, or 1.
    my ($class, $x, $y) = @_;
    my $xstr = $class -> _str($x);
    my $ystr = $class -> _str($y);

    length($xstr) <=> length($ystr) || $xstr cmp $ystr;
}

sub _len {
    my ($class, $x) = @_;
    CORE::length($class -> _str($x));
}

sub _alen {
    my ($class, $x) = @_;
    $class -> _len($x);
}

sub _digit {
    my ($class, $x, $n) = @_;
    substr($class ->_str($x), -($n+1), 1);
}

sub _digitsum {
    my ($class, $x) = @_;

    my $len = $class -> _len($x);
    my $sum = $class -> _zero();
    for (my $i = 0 ; $i < $len ; ++$i) {
        my $digit = $class -> _digit($x, $i);
        $digit = $class -> _new($digit);
        $sum = $class -> _add($sum, $digit);
    }

    return $sum;
}

sub _zeros {
    my ($class, $x) = @_;
    my $str = $class -> _str($x);
    $str =~ /[^0](0*)\z/ ? CORE::length($1) : 0;
}

##############################################################################
# _is_* routines

sub _is_zero {
    # return true if arg is zero
    my ($class, $x) = @_;
    $class -> _str($x) == 0;
}

sub _is_even {
    # return true if arg is even
    my ($class, $x) = @_;
    substr($class -> _str($x), -1, 1) % 2 == 0;
}

sub _is_odd {
    # return true if arg is odd
    my ($class, $x) = @_;
    substr($class -> _str($x), -1, 1) % 2 != 0;
}

sub _is_one {
    # return true if arg is one
    my ($class, $x) = @_;
    $class -> _str($x) == 1;
}

sub _is_two {
    # return true if arg is two
    my ($class, $x) = @_;
    $class -> _str($x) == 2;
}

sub _is_ten {
    # return true if arg is ten
    my ($class, $x) = @_;
    $class -> _str($x) == 10;
}

###############################################################################
# check routine to test internal state for corruptions

sub _check {
    # used by the test suite
    my ($class, $x) = @_;
    return "Input is undefined" unless defined $x;
    return "$x is not a reference" unless ref($x);
    return 0;
}

###############################################################################

sub _mod {
    # modulus
    my ($class, $x, $y) = @_;

    croak "@{[(caller 0)[3]]} requires non-zero second operand"
      if $class -> _is_zero($y);

    if ($class -> can('_div')) {
        $x = $class -> _copy($x);
        my ($q, $r) = $class -> _div($x, $y);
        return $r;
    } else {
        my $r = $class -> _copy($x);
        while ($class -> _acmp($r, $y) >= 0) {
            $r = $class -> _sub($r, $y);
        }
        return $r;
    }
}

##############################################################################
# shifts

sub _rsft {
    my ($class, $x, $n, $b) = @_;
    $b = $class -> _new($b) unless ref $b;
    return scalar $class -> _div($x, $class -> _pow($class -> _copy($b), $n));
}

sub _lsft {
    my ($class, $x, $n, $b) = @_;
    $b = $class -> _new($b) unless ref $b;
    return $class -> _mul($x, $class -> _pow($class -> _copy($b), $n));
}

sub _pow {
    # power of $x to $y
    my ($class, $x, $y) = @_;

    if ($class -> _is_zero($y)) {
        return $class -> _one();        # y == 0 => x => 1
    }

    if (($class -> _is_one($x)) ||      #    x == 1
        ($class -> _is_one($y)))        # or y == 1
    {
        return $x;
    }

    if ($class -> _is_zero($x)) {
        return $class -> _zero();       # 0 ** y => 0 (if not y <= 0)
    }

    my $pow2 = $class -> _one();

    my $y_bin = $class -> _as_bin($y);
    $y_bin =~ s/^0b//;
    my $len = length($y_bin);

    while (--$len > 0) {
        $pow2 = $class -> _mul($pow2, $x) if substr($y_bin, $len, 1) eq '1';
        $x = $class -> _mul($x, $x);
    }

    $x = $class -> _mul($x, $pow2);
    return $x;
}

sub _nok {
    # Return binomial coefficient (n over k).
    my ($class, $n, $k) = @_;

    # If k > n/2, or, equivalently, 2*k > n, compute nok(n, k) as
    # nok(n, n-k), to minimize the number if iterations in the loop.

    {
        my $twok = $class -> _mul($class -> _two(), $class -> _copy($k));
        if ($class -> _acmp($twok, $n) > 0) {
            $k = $class -> _sub($class -> _copy($n), $k);
        }
    }

    # Example:
    #
    # / 7 \       7!       1*2*3*4 * 5*6*7   5 * 6 * 7
    # |   | = --------- =  --------------- = --------- = ((5 * 6) / 2 * 7) / 3
    # \ 3 /   (7-3)! 3!    1*2*3*4 * 1*2*3   1 * 2 * 3
    #
    # Equivalently, _nok(11, 5) is computed as
    #
    # (((((((7 * 8) / 2) * 9) / 3) * 10) / 4) * 11) / 5

    if ($class -> _is_zero($k)) {
        return $class -> _one();
    }

    # Make a copy of the original n, in case the subclass modifies n in-place.

    my $n_orig = $class -> _copy($n);

    # n = 5, f = 6, d = 2 (cf. example above)

    $n = $class -> _sub($n, $k);
    $n = $class -> _inc($n);

    my $f = $class -> _copy($n);
    $f = $class -> _inc($f);

    my $d = $class -> _two();

    # while f <= n (the original n, that is) ...

    while ($class -> _acmp($f, $n_orig) <= 0) {
        $n = $class -> _mul($n, $f);
        $n = $class -> _div($n, $d);
        $f = $class -> _inc($f);
        $d = $class -> _inc($d);
    }

    return $n;
}

#sub _fac {
#    # factorial
#    my ($class, $x) = @_;
#
#    my $two = $class -> _two();
#
#    if ($class -> _acmp($x, $two) < 0) {
#        return $class -> _one();
#    }
#
#    my $i = $class -> _copy($x);
#    while ($class -> _acmp($i, $two) > 0) {
#        $i = $class -> _dec($i);
#        $x = $class -> _mul($x, $i);
#    }
#
#    return $x;
#}

sub _fac {
    # factorial
    my ($class, $x) = @_;

    # This is an implementation of the split recursive algorithm. See
    # http://www.luschny.de/math/factorial/csharp/FactorialSplit.cs.html

    my $p   = $class -> _one();
    my $r   = $class -> _one();
    my $two = $class -> _two();

    my ($log2n) = $class -> _log_int($class -> _copy($x), $two);
    my $h     = $class -> _zero();
    my $shift = $class -> _zero();
    my $k     = $class -> _one();

    while ($class -> _acmp($h, $x)) {
        $shift = $class -> _add($shift, $h);
        $h = $class -> _rsft($class -> _copy($x), $log2n, $two);
        $log2n = $class -> _dec($log2n) if !$class -> _is_zero($log2n);
        my $high = $class -> _copy($h);
        $high = $class -> _dec($high) if $class -> _is_even($h);
        while ($class -> _acmp($k, $high)) {
            $k = $class -> _add($k, $two);
            $p = $class -> _mul($p, $k);
        }
        $r = $class -> _mul($r, $p);
    }
    return $class -> _lsft($r, $shift, $two);
}

sub _dfac {
    # double factorial
    my ($class, $x) = @_;

    my $two = $class -> _two();

    if ($class -> _acmp($x, $two) < 0) {
        return $class -> _one();
    }

    my $i = $class -> _copy($x);
    while ($class -> _acmp($i, $two) > 0) {
        $i = $class -> _sub($i, $two);
        $x = $class -> _mul($x, $i);
    }

    return $x;
}

sub _log_int {
    # calculate integer log of $x to base $base
    # calculate integer log of $x to base $base
    # ref to array, ref to array - return ref to array
    my ($class, $x, $base) = @_;

    # X == 0 => NaN
    return if $class -> _is_zero($x);

    $base = $class -> _new(2)     unless defined($base);
    $base = $class -> _new($base) unless ref($base);

    # BASE 0 or 1 => NaN
    return if $class -> _is_zero($base) || $class -> _is_one($base);

    # X == 1 => 0 (is exact)
    if ($class -> _is_one($x)) {
        return $class -> _zero(), 1;
    }

    my $cmp = $class -> _acmp($x, $base);

    # X == BASE => 1 (is exact)
    if ($cmp == 0) {
        return $class -> _one(), 1;
    }

    # 1 < X < BASE => 0 (is truncated)
    if ($cmp < 0) {
        return $class -> _zero(), 0;
    }

    my $y;

    # log(x) / log(b) = log(xm * 10^xe) / log(bm * 10^be)
    #                 = (log(xm) + xe*(log(10))) / (log(bm) + be*log(10))

    {
        my $x_str = $class -> _str($x);
        my $b_str = $class -> _str($base);
        my $xm    = "." . $x_str;
        my $bm    = "." . $b_str;
        my $xe    = length($x_str);
        my $be    = length($b_str);
        my $log10 = log(10);
        my $guess = int((log($xm) + $xe * $log10) / (log($bm) + $be * $log10));
        $y = $class -> _new($guess);
    }

    my $trial = $class -> _pow($class -> _copy($base), $y);
    my $acmp  = $class -> _acmp($trial, $x);

    # Did we get the exact result?

    return $y, 1 if $acmp == 0;

    # Too small?

    while ($acmp < 0) {
        $trial = $class -> _mul($trial, $base);
        $y     = $class -> _inc($y);
        $acmp  = $class -> _acmp($trial, $x);
    }

    # Too big?

    while ($acmp > 0) {
        $trial = $class -> _div($trial, $base);
        $y     = $class -> _dec($y);
        $acmp  = $class -> _acmp($trial, $x);
    }

    return $y, 1 if $acmp == 0;         # result is exact
    return $y, 0;                       # result is too small
}

sub _sqrt {
    # square-root of $y in place
    my ($class, $y) = @_;

    return $y if $class -> _is_zero($y);

    my $y_str = $class -> _str($y);
    my $y_len = length($y_str);

    # Compute the guess $x.

    my $xm;
    my $xe;
    if ($y_len % 2 == 0) {
        $xm = sqrt("." . $y_str);
        $xe = $y_len / 2;
        $xm = sprintf "%.0f", int($xm * 1e15);
        $xe -= 15;
    } else {
        $xm = sqrt(".0" . $y_str);
        $xe = ($y_len + 1) / 2;
        $xm = sprintf "%.0f", int($xm * 1e16);
        $xe -= 16;
    }

    my $x;
    if ($xe < 0) {
        $x = substr $xm, 0, length($xm) + $xe;
    } else {
        $x = $xm . ("0" x $xe);
    }

    $x = $class -> _new($x);

    # Newton's method for computing square root of y
    #
    # x(i+1) = x(i) - f(x(i)) / f'(x(i))
    #        = x(i) - (x(i)^2 - y) / (2 * x(i))     # use if x(i)^2 > y
    #        = x(i) + (y - x(i)^2) / (2 * x(i))     # use if x(i)^2 < y

    # Determine if x, our guess, is too small, correct, or too large.

    my $xsq = $class -> _mul($class -> _copy($x), $x);          # x(i)^2
    my $acmp = $class -> _acmp($xsq, $y);                       # x(i)^2 <=> y

    # Only assign a value to this variable if we will be using it.

    my $two;
    $two = $class -> _two() if $acmp != 0;

    # If x is too small, do one iteration of Newton's method. Since the
    # function f(x) = x^2 - y is concave and monotonically increasing, the next
    # guess for x will either be correct or too large.

    if ($acmp < 0) {

        # x(i+1) = x(i) + (y - x(i)^2) / (2 * x(i))

        my $numer = $class -> _sub($class -> _copy($y), $xsq);  # y - x(i)^2
        my $denom = $class -> _mul($class -> _copy($two), $x);  # 2 * x(i)
        my $delta = $class -> _div($numer, $denom);

        unless ($class -> _is_zero($delta)) {
            $x    = $class -> _add($x, $delta);
            $xsq  = $class -> _mul($class -> _copy($x), $x);    # x(i)^2
            $acmp = $class -> _acmp($xsq, $y);                  # x(i)^2 <=> y
        }
    }

    # If our guess for x is too large, apply Newton's method repeatedly until
    # we either have got the correct value, or the delta is zero.

    while ($acmp > 0) {

        # x(i+1) = x(i) - (x(i)^2 - y) / (2 * x(i))

        my $numer = $class -> _sub($xsq, $y);                   # x(i)^2 - y
        my $denom = $class -> _mul($class -> _copy($two), $x);  # 2 * x(i)
        my $delta = $class -> _div($numer, $denom);
        last if $class -> _is_zero($delta);

        $x    = $class -> _sub($x, $delta);
        $xsq  = $class -> _mul($class -> _copy($x), $x);        # x(i)^2
        $acmp = $class -> _acmp($xsq, $y);                      # x(i)^2 <=> y
    }

    # When the delta is zero, our value for x might still be too large. We
    # require that the outout is either exact or too small (i.e., rounded down
    # to the nearest integer), so do a final check.

    while ($acmp > 0) {
        $x    = $class -> _dec($x);
        $xsq  = $class -> _mul($class -> _copy($x), $x);        # x(i)^2
        $acmp = $class -> _acmp($xsq, $y);                      # x(i)^2 <=> y
    }

    return $x;
}

sub _root {
    my ($class, $y, $n) = @_;

    return $y if $class -> _is_zero($y) || $class -> _is_one($y) ||
                 $class -> _is_one($n);

    # If y <= n, the result is always (truncated to) 1.

    return $class -> _one() if $class -> _acmp($y, $n) <= 0;

    # Compute the initial guess x of y^(1/n). When n is large, Newton's method
    # converges slowly if the "guess" (initial value) is poor, so we need a
    # good guess. It the guess is too small, the next guess will be too large,
    # and from then on all guesses are too large.

    my $DEBUG = 0;

    # Split y into mantissa and exponent in base 10, so that
    #
    #   y = xm * 10^xe, where 0 < xm < 1 and xe is an integer

    my $y_str  = $class -> _str($y);
    my $ym = "." . $y_str;
    my $ye = length($y_str);

    # From this compute the approximate base 10 logarithm of y
    #
    #   log_10(y) = log_10(ym) + log_10(ye^10)
    #             = log(ym)/log(10) + ye

    my $log10y = log($ym) / log(10) + $ye;

    # And from this compute the approximate base 10 logarithm of x, where
    # x = y^(1/n)
    #
    #   log_10(x) = log_10(y)/n

    my $log10x = $log10y / $class -> _num($n);

    # From this compute xm and xe, the mantissa and exponent (in base 10) of x,
    # where 1 < xm <= 10 and xe is an integer.

    my $xe = int $log10x;
    my $xm = 10 ** ($log10x - $xe);

    # Scale the mantissa and exponent to increase the integer part of ym, which
    # gives us better accuracy.

    if ($DEBUG) {
        print "\n";
        print "y_str  = $y_str\n";
        print "ym     = $ym\n";
        print "ye     = $ye\n";
        print "log10y = $log10y\n";
        print "log10x = $log10x\n";
        print "xm     = $xm\n";
        print "xe     = $xe\n";
    }

    my $d = $xe < 15 ? $xe : 15;
    $xm *= 10 ** $d;
    $xe -= $d;

    if ($DEBUG) {
        print "\n";
        print "xm     = $xm\n";
        print "xe     = $xe\n";
    }

    # If the mantissa is not an integer, round up to nearest integer, and then
    # convert the number to a string. It is important to always round up due to
    # how Newton's method behaves in this case. If the initial guess is too
    # small, the next guess will be too large, after which every succeeding
    # guess converges the correct value from above. Now, if the initial guess
    # is too small and n is large, the next guess will be much too large and
    # require a large number of iterations to get close to the solution.
    # Because of this, we are likely to find the solution faster if we make
    # sure the initial guess is not too small.

    my $xm_int = int($xm);
    my $x_str = sprintf '%.0f', $xm > $xm_int ? $xm_int + 1 : $xm_int;
    $x_str .= "0" x $xe;

    my $x = $class -> _new($x_str);

    if ($DEBUG) {
        print "xm     = $xm\n";
        print "xe     = $xe\n";
        print "\n";
        print "x_str  = $x_str (initial guess)\n";
        print "\n";
    }

    # Use Newton's method for computing n'th root of y.
    #
    # x(i+1) = x(i) - f(x(i)) / f'(x(i))
    #        = x(i) - (x(i)^n - y) / (n * x(i)^(n-1))   # use if x(i)^n > y
    #        = x(i) + (y - x(i)^n) / (n * x(i)^(n-1))   # use if x(i)^n < y

    # Determine if x, our guess, is too small, correct, or too large. Rather
    # than computing x(i)^n and x(i)^(n-1) directly, compute x(i)^(n-1) and
    # then the same value multiplied by x.

    my $nm1     = $class -> _dec($class -> _copy($n));           # n-1
    my $xpownm1 = $class -> _pow($class -> _copy($x), $nm1);     # x(i)^(n-1)
    my $xpown   = $class -> _mul($class -> _copy($xpownm1), $x); # x(i)^n
    my $acmp    = $class -> _acmp($xpown, $y);                   # x(i)^n <=> y

    if ($DEBUG) {
        print "\n";
        print "x      = ", $class -> _str($x), "\n";
        print "x^n    = ", $class -> _str($xpown), "\n";
        print "y      = ", $class -> _str($y), "\n";
        print "acmp   = $acmp\n";
    }

    # If x is too small, do one iteration of Newton's method. Since the
    # function f(x) = x^n - y is concave and monotonically increasing, the next
    # guess for x will either be correct or too large.

    if ($acmp < 0) {

        # x(i+1) = x(i) + (y - x(i)^n) / (n * x(i)^(n-1))

        my $numer = $class -> _sub($class -> _copy($y), $xpown);    # y - x(i)^n
        my $denom = $class -> _mul($class -> _copy($n), $xpownm1);  # n * x(i)^(n-1)
        my $delta = $class -> _div($numer, $denom);

        if ($DEBUG) {
            print "\n";
            print "numer  = ", $class -> _str($numer), "\n";
            print "denom  = ", $class -> _str($denom), "\n";
            print "delta  = ", $class -> _str($delta), "\n";
        }

        unless ($class -> _is_zero($delta)) {
            $x       = $class -> _add($x, $delta);
            $xpownm1 = $class -> _pow($class -> _copy($x), $nm1);     # x(i)^(n-1)
            $xpown   = $class -> _mul($class -> _copy($xpownm1), $x); # x(i)^n
            $acmp    = $class -> _acmp($xpown, $y);                   # x(i)^n <=> y

            if ($DEBUG) {
                print "\n";
                print "x      = ", $class -> _str($x), "\n";
                print "x^n    = ", $class -> _str($xpown), "\n";
                print "y      = ", $class -> _str($y), "\n";
                print "acmp   = $acmp\n";
            }
        }
    }

    # If our guess for x is too large, apply Newton's method repeatedly until
    # we either have got the correct value, or the delta is zero.

    while ($acmp > 0) {

        # x(i+1) = x(i) - (x(i)^n - y) / (n * x(i)^(n-1))

        my $numer = $class -> _sub($class -> _copy($xpown), $y);    # x(i)^n - y
        my $denom = $class -> _mul($class -> _copy($n), $xpownm1);  # n * x(i)^(n-1)

        if ($DEBUG) {
            print "numer  = ", $class -> _str($numer), "\n";
            print "denom  = ", $class -> _str($denom), "\n";
        }

        my $delta = $class -> _div($numer, $denom);

        if ($DEBUG) {
            print "delta  = ", $class -> _str($delta), "\n";
        }

        last if $class -> _is_zero($delta);

        $x       = $class -> _sub($x, $delta);
        $xpownm1 = $class -> _pow($class -> _copy($x), $nm1);     # x(i)^(n-1)
        $xpown   = $class -> _mul($class -> _copy($xpownm1), $x); # x(i)^n
        $acmp    = $class -> _acmp($xpown, $y);                   # x(i)^n <=> y

        if ($DEBUG) {
            print "\n";
            print "x      = ", $class -> _str($x), "\n";
            print "x^n    = ", $class -> _str($xpown), "\n";
            print "y      = ", $class -> _str($y), "\n";
            print "acmp   = $acmp\n";
        }
    }

    # When the delta is zero, our value for x might still be too large. We
    # require that the outout is either exact or too small (i.e., rounded down
    # to the nearest integer), so do a final check.

    while ($acmp > 0) {
        $x     = $class -> _dec($x);
        $xpown = $class -> _pow($class -> _copy($x), $n);     # x(i)^n
        $acmp  = $class -> _acmp($xpown, $y);                 # x(i)^n <=> y
    }

    return $x;
}

##############################################################################
# binary stuff

sub _and {
    my ($class, $x, $y) = @_;

    return $x if $class -> _acmp($x, $y) == 0;

    my $m    = $class -> _one();
    my $mask = $class -> _new("32768");

    my ($xr, $yr);                # remainders after division

    my $xc = $class -> _copy($x);
    my $yc = $class -> _copy($y);
    my $z  = $class -> _zero();

    until ($class -> _is_zero($xc) || $class -> _is_zero($yc)) {
        ($xc, $xr) = $class -> _div($xc, $mask);
        ($yc, $yr) = $class -> _div($yc, $mask);
        my $bits = $class -> _new($class -> _num($xr) & $class -> _num($yr));
        $z = $class -> _add($z, $class -> _mul($bits, $m));
        $m = $class -> _mul($m, $mask);
    }

    return $z;
}

sub _xor {
    my ($class, $x, $y) = @_;

    return $class -> _zero() if $class -> _acmp($x, $y) == 0;

    my $m    = $class -> _one();
    my $mask = $class -> _new("32768");

    my ($xr, $yr);                # remainders after division

    my $xc = $class -> _copy($x);
    my $yc = $class -> _copy($y);
    my $z  = $class -> _zero();

    until ($class -> _is_zero($xc) || $class -> _is_zero($yc)) {
        ($xc, $xr) = $class -> _div($xc, $mask);
        ($yc, $yr) = $class -> _div($yc, $mask);
        my $bits = $class -> _new($class -> _num($xr) ^ $class -> _num($yr));
        $z = $class -> _add($z, $class -> _mul($bits, $m));
        $m = $class -> _mul($m, $mask);
    }

    # The loop above stops when the smallest of the two numbers is exhausted.
    # The remainder of the longer one will survive bit-by-bit, so we simple
    # multiply-add it in.

    $z = $class -> _add($z, $class -> _mul($xc, $m))
      unless $class -> _is_zero($xc);
    $z = $class -> _add($z, $class -> _mul($yc, $m))
      unless $class -> _is_zero($yc);

    return $z;
}

sub _or {
    my ($class, $x, $y) = @_;

    return $x if $class -> _acmp($x, $y) == 0; # shortcut (see _and)

    my $m    = $class -> _one();
    my $mask = $class -> _new("32768");

    my ($xr, $yr);                # remainders after division

    my $xc = $class -> _copy($x);
    my $yc = $class -> _copy($y);
    my $z  = $class -> _zero();

    until ($class -> _is_zero($xc) || $class -> _is_zero($yc)) {
        ($xc, $xr) = $class -> _div($xc, $mask);
        ($yc, $yr) = $class -> _div($yc, $mask);
        my $bits = $class -> _new($class -> _num($xr) | $class -> _num($yr));
        $z = $class -> _add($z, $class -> _mul($bits, $m));
        $m = $class -> _mul($m, $mask);
    }

    # The loop above stops when the smallest of the two numbers is exhausted.
    # The remainder of the longer one will survive bit-by-bit, so we simple
    # multiply-add it in.

    $z = $class -> _add($z, $class -> _mul($xc, $m))
      unless $class -> _is_zero($xc);
    $z = $class -> _add($z, $class -> _mul($yc, $m))
      unless $class -> _is_zero($yc);

    return $z;
}

sub _sand {
    my ($class, $x, $sx, $y, $sy) = @_;

    return ($class -> _zero(), '+')
      if $class -> _is_zero($x) || $class -> _is_zero($y);

    my $sign = $sx eq '-' && $sy eq '-' ? '-' : '+';

    my ($bx, $by);

    if ($sx eq '-') {                   # if x is negative
        # two's complement: inc (dec unsigned value) and flip all "bits" in $bx
        $bx = $class -> _copy($x);
        $bx = $class -> _dec($bx);
        $bx = $class -> _as_hex($bx);
        $bx =~ s/^-?0x//;
        $bx =~ tr<0123456789abcdef>
                <\x0f\x0e\x0d\x0c\x0b\x0a\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00>;
    } else {                            # if x is positive
        $bx = $class -> _as_hex($x);    # get binary representation
        $bx =~ s/^-?0x//;
        $bx =~ tr<fedcba9876543210>
                 <\x0f\x0e\x0d\x0c\x0b\x0a\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00>;
    }

    if ($sy eq '-') {                   # if y is negative
        # two's complement: inc (dec unsigned value) and flip all "bits" in $by
        $by = $class -> _copy($y);
        $by = $class -> _dec($by);
        $by = $class -> _as_hex($by);
        $by =~ s/^-?0x//;
        $by =~ tr<0123456789abcdef>
                <\x0f\x0e\x0d\x0c\x0b\x0a\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00>;
    } else {
        $by = $class -> _as_hex($y);    # get binary representation
        $by =~ s/^-?0x//;
        $by =~ tr<fedcba9876543210>
                <\x0f\x0e\x0d\x0c\x0b\x0a\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00>;
    }

    # now we have bit-strings from X and Y, reverse them for padding
    $bx = reverse $bx;
    $by = reverse $by;

    # padd the shorter string
    my $xx = "\x00"; $xx = "\x0f" if $sx eq '-';
    my $yy = "\x00"; $yy = "\x0f" if $sy eq '-';
    my $diff = CORE::length($bx) - CORE::length($by);
    if ($diff > 0) {
        # if $yy eq "\x00", we can cut $bx, otherwise we need to padd $by
        $by .= $yy x $diff;
    } elsif ($diff < 0) {
        # if $xx eq "\x00", we can cut $by, otherwise we need to padd $bx
        $bx .= $xx x abs($diff);
    }

    # and the strings together
    my $r = $bx & $by;

    # and reverse the result again
    $bx = reverse $r;

    # One of $bx or $by was negative, so need to flip bits in the result. In both
    # cases (one or two of them negative, or both positive) we need to get the
    # characters back.
    if ($sign eq '-') {
        $bx =~ tr<\x0f\x0e\x0d\x0c\x0b\x0a\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00>
                 <0123456789abcdef>;
    } else {
        $bx =~ tr<\x0f\x0e\x0d\x0c\x0b\x0a\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00>
                 <fedcba9876543210>;
    }

    # leading zeros will be stripped by _from_hex()
    $bx = '0x' . $bx;
    $bx = $class -> _from_hex($bx);

    $bx = $class -> _inc($bx) if $sign eq '-';

    # avoid negative zero
    $sign = '+' if $class -> _is_zero($bx);

    return $bx, $sign;
}

sub _sxor {
    my ($class, $x, $sx, $y, $sy) = @_;

    return ($class -> _zero(), '+')
      if $class -> _is_zero($x) && $class -> _is_zero($y);

    my $sign = $sx ne $sy ? '-' : '+';

    my ($bx, $by);

    if ($sx eq '-') {                   # if x is negative
        # two's complement: inc (dec unsigned value) and flip all "bits" in $bx
        $bx = $class -> _copy($x);
        $bx = $class -> _dec($bx);
        $bx = $class -> _as_hex($bx);
        $bx =~ s/^-?0x//;
        $bx =~ tr<0123456789abcdef>
                <\x0f\x0e\x0d\x0c\x0b\x0a\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00>;
    } else {                            # if x is positive
        $bx = $class -> _as_hex($x);    # get binary representation
        $bx =~ s/^-?0x//;
        $bx =~ tr<fedcba9876543210>
                 <\x0f\x0e\x0d\x0c\x0b\x0a\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00>;
    }

    if ($sy eq '-') {                   # if y is negative
        # two's complement: inc (dec unsigned value) and flip all "bits" in $by
        $by = $class -> _copy($y);
        $by = $class -> _dec($by);
        $by = $class -> _as_hex($by);
        $by =~ s/^-?0x//;
        $by =~ tr<0123456789abcdef>
                <\x0f\x0e\x0d\x0c\x0b\x0a\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00>;
    } else {
        $by = $class -> _as_hex($y);    # get binary representation
        $by =~ s/^-?0x//;
        $by =~ tr<fedcba9876543210>
                <\x0f\x0e\x0d\x0c\x0b\x0a\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00>;
    }

    # now we have bit-strings from X and Y, reverse them for padding
    $bx = reverse $bx;
    $by = reverse $by;

    # padd the shorter string
    my $xx = "\x00"; $xx = "\x0f" if $sx eq '-';
    my $yy = "\x00"; $yy = "\x0f" if $sy eq '-';
    my $diff = CORE::length($bx) - CORE::length($by);
    if ($diff > 0) {
        # if $yy eq "\x00", we can cut $bx, otherwise we need to padd $by
        $by .= $yy x $diff;
    } elsif ($diff < 0) {
        # if $xx eq "\x00", we can cut $by, otherwise we need to padd $bx
        $bx .= $xx x abs($diff);
    }

    # xor the strings together
    my $r = $bx ^ $by;

    # and reverse the result again
    $bx = reverse $r;

    # One of $bx or $by was negative, so need to flip bits in the result. In both
    # cases (one or two of them negative, or both positive) we need to get the
    # characters back.
    if ($sign eq '-') {
        $bx =~ tr<\x0f\x0e\x0d\x0c\x0b\x0a\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00>
                 <0123456789abcdef>;
    } else {
        $bx =~ tr<\x0f\x0e\x0d\x0c\x0b\x0a\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00>
                 <fedcba9876543210>;
    }

    # leading zeros will be stripped by _from_hex()
    $bx = '0x' . $bx;
    $bx = $class -> _from_hex($bx);

    $bx = $class -> _inc($bx) if $sign eq '-';

    # avoid negative zero
    $sign = '+' if $class -> _is_zero($bx);

    return $bx, $sign;
}

sub _sor {
    my ($class, $x, $sx, $y, $sy) = @_;

    return ($class -> _zero(), '+')
      if $class -> _is_zero($x) && $class -> _is_zero($y);

    my $sign = $sx eq '-' || $sy eq '-' ? '-' : '+';

    my ($bx, $by);

    if ($sx eq '-') {                   # if x is negative
        # two's complement: inc (dec unsigned value) and flip all "bits" in $bx
        $bx = $class -> _copy($x);
        $bx = $class -> _dec($bx);
        $bx = $class -> _as_hex($bx);
        $bx =~ s/^-?0x//;
        $bx =~ tr<0123456789abcdef>
                <\x0f\x0e\x0d\x0c\x0b\x0a\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00>;
    } else {                            # if x is positive
        $bx = $class -> _as_hex($x);     # get binary representation
        $bx =~ s/^-?0x//;
        $bx =~ tr<fedcba9876543210>
                 <\x0f\x0e\x0d\x0c\x0b\x0a\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00>;
    }

    if ($sy eq '-') {                   # if y is negative
        # two's complement: inc (dec unsigned value) and flip all "bits" in $by
        $by = $class -> _copy($y);
        $by = $class -> _dec($by);
        $by = $class -> _as_hex($by);
        $by =~ s/^-?0x//;
        $by =~ tr<0123456789abcdef>
                <\x0f\x0e\x0d\x0c\x0b\x0a\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00>;
    } else {
        $by = $class -> _as_hex($y);     # get binary representation
        $by =~ s/^-?0x//;
        $by =~ tr<fedcba9876543210>
                <\x0f\x0e\x0d\x0c\x0b\x0a\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00>;
    }

    # now we have bit-strings from X and Y, reverse them for padding
    $bx = reverse $bx;
    $by = reverse $by;

    # padd the shorter string
    my $xx = "\x00"; $xx = "\x0f" if $sx eq '-';
    my $yy = "\x00"; $yy = "\x0f" if $sy eq '-';
    my $diff = CORE::length($bx) - CORE::length($by);
    if ($diff > 0) {
        # if $yy eq "\x00", we can cut $bx, otherwise we need to padd $by
        $by .= $yy x $diff;
    } elsif ($diff < 0) {
        # if $xx eq "\x00", we can cut $by, otherwise we need to padd $bx
        $bx .= $xx x abs($diff);
    }

    # or the strings together
    my $r = $bx | $by;

    # and reverse the result again
    $bx = reverse $r;

    # One of $bx or $by was negative, so need to flip bits in the result. In both
    # cases (one or two of them negative, or both positive) we need to get the
    # characters back.
    if ($sign eq '-') {
        $bx =~ tr<\x0f\x0e\x0d\x0c\x0b\x0a\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00>
                 <0123456789abcdef>;
    } else {
        $bx =~ tr<\x0f\x0e\x0d\x0c\x0b\x0a\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00>
                 <fedcba9876543210>;
    }

    # leading zeros will be stripped by _from_hex()
    $bx = '0x' . $bx;
    $bx = $class -> _from_hex($bx);

    $bx = $class -> _inc($bx) if $sign eq '-';

    # avoid negative zero
    $sign = '+' if $class -> _is_zero($bx);

    return $bx, $sign;
}

sub _to_bin {
    # convert the number to a string of binary digits without prefix
    my ($class, $x) = @_;
    my $str    = '';
    my $tmp    = $class -> _copy($x);
    my $chunk = $class -> _new("16777216");     # 2^24 = 24 binary digits
    my $rem;
    until ($class -> _acmp($tmp, $chunk) < 0) {
        ($tmp, $rem) = $class -> _div($tmp, $chunk);
        $str = sprintf("%024b", $class -> _num($rem)) . $str;
    }
    unless ($class -> _is_zero($tmp)) {
        $str = sprintf("%b", $class -> _num($tmp)) . $str;
    }
    return length($str) ? $str : '0';
}

sub _to_oct {
    # convert the number to a string of octal digits without prefix
    my ($class, $x) = @_;
    my $str    = '';
    my $tmp    = $class -> _copy($x);
    my $chunk = $class -> _new("16777216");     # 2^24 = 8 octal digits
    my $rem;
    until ($class -> _acmp($tmp, $chunk) < 0) {
        ($tmp, $rem) = $class -> _div($tmp, $chunk);
        $str = sprintf("%08o", $class -> _num($rem)) . $str;
    }
    unless ($class -> _is_zero($tmp)) {
        $str = sprintf("%o", $class -> _num($tmp)) . $str;
    }
    return length($str) ? $str : '0';
}

sub _to_hex {
    # convert the number to a string of hexadecimal digits without prefix
    my ($class, $x) = @_;
    my $str    = '';
    my $tmp    = $class -> _copy($x);
    my $chunk = $class -> _new("16777216");     # 2^24 = 6 hexadecimal digits
    my $rem;
    until ($class -> _acmp($tmp, $chunk) < 0) {
        ($tmp, $rem) = $class -> _div($tmp, $chunk);
        $str = sprintf("%06x", $class -> _num($rem)) . $str;
    }
    unless ($class -> _is_zero($tmp)) {
        $str = sprintf("%x", $class -> _num($tmp)) . $str;
    }
    return length($str) ? $str : '0';
}

sub _as_bin {
    # convert the number to a string of binary digits with prefix
    my ($class, $x) = @_;
    return '0b' . $class -> _to_bin($x);
}

sub _as_oct {
    # convert the number to a string of octal digits with prefix
    my ($class, $x) = @_;
    return '0' . $class -> _to_oct($x);         # yes, 0 becomes "00"
}

sub _as_hex {
    # convert the number to a string of hexadecimal digits with prefix
    my ($class, $x) = @_;
    return '0x' . $class -> _to_hex($x);
}

sub _to_bytes {
    # convert the number to a string of bytes
    my ($class, $x) = @_;
    my $str    = '';
    my $tmp    = $class -> _copy($x);
    my $chunk = $class -> _new("65536");
    my $rem;
    until ($class -> _is_zero($tmp)) {
        ($tmp, $rem) = $class -> _div($tmp, $chunk);
        $str = pack('n', $class -> _num($rem)) . $str;
    }
    $str =~ s/^\0+//;
    return length($str) ? $str : "\x00";
}

*_as_bytes = \&_to_bytes;

sub _to_base {
    # convert the number to a string of digits in various bases
    my $class = shift;
    my $x     = shift;
    my $base  = shift;
    $base = $class -> _new($base) unless ref($base);

    my $collseq;
    if (@_) {
        $collseq = shift;
        croak "The collation sequence must be a non-empty string"
          unless defined($collseq) && length($collseq);
    } else {
        if ($class -> _acmp($base, $class -> _new("94")) <= 0) {
            $collseq = '0123456789'                     #  48 ..  57
                     . 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'     #  65 ..  90
                     . 'abcdefghijklmnopqrstuvwxyz'     #  97 .. 122
                     . '!"#$%&\'()*+,-./'               #  33 ..  47
                     . ':;<=>?@'                        #  58 ..  64
                     . '[\\]^_`'                        #  91 ..  96
                     . '{|}~';                          # 123 .. 126
        } else {
            croak "When base > 94, a collation sequence must be given";
        }
    }

    my @collseq = split '', $collseq;

    my $str   = '';
    my $tmp   = $class -> _copy($x);
    my $rem;
    until ($class -> _is_zero($tmp)) {
        ($tmp, $rem) = $class -> _div($tmp, $base);
        my $num = $class -> _num($rem);
        croak "no character to represent '$num' in collation sequence",
          " (collation sequence is too short)" if $num > $#collseq;
        my $chr = $collseq[$num];
        $str = $chr . $str;
    }
    return $collseq[0] unless length $str;
    return $str;
}

sub _to_base_num {
    # Convert the number to an array of integers in any base.
    my ($class, $x, $base) = @_;

    # Make sure the base is an object and >= 2.
    $base = $class -> _new($base) unless ref($base);
    my $two = $class -> _two();
    croak "base must be >= 2" unless $class -> _acmp($base, $two) >= 0;

    my $out   = [];
    my $xcopy = $class -> _copy($x);
    my $rem;

    # Do all except the last (most significant) element.
    until ($class -> _acmp($xcopy, $base) < 0) {
        ($xcopy, $rem) = $class -> _div($xcopy, $base);
        unshift @$out, $rem;
    }

    # Do the last (most significant element).
    unless ($class -> _is_zero($xcopy)) {
        unshift @$out, $xcopy;
    }

    # $out is empty if $x is zero.
    unshift @$out, $class -> _zero() unless @$out;

    return $out;
}

sub _from_hex {
    # Convert a string of hexadecimal digits to a number.

    my ($class, $hex) = @_;
    $hex =~ s/^0[xX]//;

    # Find the largest number of hexadecimal digits that we can safely use with
    # 32 bit integers. There are 4 bits pr hexadecimal digit, and we use only
    # 31 bits to play safe. This gives us int(31 / 4) = 7.

    my $len = length $hex;
    my $rem = 1 + ($len - 1) % 7;

    # Do the first chunk.

    my $ret = $class -> _new(int hex substr $hex, 0, $rem);
    return $ret if $rem == $len;

    # Do the remaining chunks, if any.

    my $shift = $class -> _new(1 << (4 * 7));
    for (my $offset = $rem ; $offset < $len ; $offset += 7) {
        my $part = int hex substr $hex, $offset, 7;
        $ret = $class -> _mul($ret, $shift);
        $ret = $class -> _add($ret, $class -> _new($part));
    }

    return $ret;
}

sub _from_oct {
    # Convert a string of octal digits to a number.

    my ($class, $oct) = @_;

    # Find the largest number of octal digits that we can safely use with 32
    # bit integers. There are 3 bits pr octal digit, and we use only 31 bits to
    # play safe. This gives us int(31 / 3) = 10.

    my $len = length $oct;
    my $rem = 1 + ($len - 1) % 10;

    # Do the first chunk.

    my $ret = $class -> _new(int oct substr $oct, 0, $rem);
    return $ret if $rem == $len;

    # Do the remaining chunks, if any.

    my $shift = $class -> _new(1 << (3 * 10));
    for (my $offset = $rem ; $offset < $len ; $offset += 10) {
        my $part = int oct substr $oct, $offset, 10;
        $ret = $class -> _mul($ret, $shift);
        $ret = $class -> _add($ret, $class -> _new($part));
    }

    return $ret;
}

sub _from_bin {
    # Convert a string of binary digits to a number.

    my ($class, $bin) = @_;
    $bin =~ s/^0[bB]//;

    # The largest number of binary digits that we can safely use with 32 bit
    # integers is 31. We use only 31 bits to play safe.

    my $len = length $bin;
    my $rem = 1 + ($len - 1) % 31;

    # Do the first chunk.

    my $ret = $class -> _new(int oct '0b' . substr $bin, 0, $rem);
    return $ret if $rem == $len;

    # Do the remaining chunks, if any.

    my $shift = $class -> _new(1 << 31);
    for (my $offset = $rem ; $offset < $len ; $offset += 31) {
        my $part = int oct '0b' . substr $bin, $offset, 31;
        $ret = $class -> _mul($ret, $shift);
        $ret = $class -> _add($ret, $class -> _new($part));
    }

    return $ret;
}

sub _from_bytes {
    # convert string of bytes to a number
    my ($class, $str) = @_;
    my $x    = $class -> _zero();
    my $base = $class -> _new("256");
    my $n    = length($str);
    for (my $i = 0 ; $i < $n ; ++$i) {
        $x = $class -> _mul($x, $base);
        my $byteval = $class -> _new(unpack 'C', substr($str, $i, 1));
        $x = $class -> _add($x, $byteval);
    }
    return $x;
}

sub _from_base {
    # convert a string to a decimal number
    my $class = shift;
    my $str   = shift;
    my $base  = shift;
    $base = $class -> _new($base) unless ref($base);

    my $n = length($str);
    my $x = $class -> _zero();

    my $collseq;
    if (@_) {
        $collseq = shift();
    } else {
        if ($class -> _acmp($base, $class -> _new("36")) <= 0) {
            $str = uc $str;
            $collseq = '0123456789' . 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
        } elsif ($class -> _acmp($base, $class -> _new("94")) <= 0) {
            $collseq = '0123456789'                     #  48 ..  57
                     . 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'     #  65 ..  90
                     . 'abcdefghijklmnopqrstuvwxyz'     #  97 .. 122
                     . '!"#$%&\'()*+,-./'               #  33 ..  47
                     . ':;<=>?@'                        #  58 ..  64
                     . '[\\]^_`'                        #  91 ..  96
                     . '{|}~';                          # 123 .. 126
        } else {
            croak "When base > 94, a collation sequence must be given";
        }
        $collseq = substr $collseq, 0, $class -> _num($base);
    }

    # Create a mapping from each character in the collation sequence to the
    # corresponding integer. Check for duplicates in the collation sequence.

    my @collseq = split '', $collseq;
    my %collseq;
    for my $num (0 .. $#collseq) {
        my $chr = $collseq[$num];
        die "duplicate character '$chr' in collation sequence"
          if exists $collseq{$chr};
        $collseq{$chr} = $num;
    }

    for (my $i = 0 ; $i < $n ; ++$i) {
        my $chr = substr($str, $i, 1);
        die "input character '$chr' does not exist in collation sequence"
          unless exists $collseq{$chr};
        $x = $class -> _mul($x, $base);
        my $num = $class -> _new($collseq{$chr});
        $x = $class -> _add($x, $num);
    }

    return $x;
}

sub _from_base_num {
    # Convert an array in the given base to a number.
    my ($class, $in, $base) = @_;

    # Make sure the base is an object and >= 2.
    $base = $class -> _new($base) unless ref($base);
    my $two = $class -> _two();
    croak "base must be >= 2" unless $class -> _acmp($base, $two) >= 0;

    # @$in = map { ref($_) ? $_ : $class -> _new($_) } @$in;

    my $ele = $in -> [0];

    $ele = $class -> _new($ele) unless ref($ele);
    my $x = $class -> _copy($ele);

    for my $i (1 .. $#$in) {
        $x = $class -> _mul($x, $base);
        $ele = $in -> [$i];
        $ele = $class -> _new($ele) unless ref($ele);
        $x = $class -> _add($x, $ele);
    }

    return $x;
}

##############################################################################
# special modulus functions

sub _modinv {
    # modular multiplicative inverse
    my ($class, $x, $y) = @_;

    # modulo zero
    if ($class -> _is_zero($y)) {
        return;
    }

    # modulo one
    if ($class -> _is_one($y)) {
        return ($class -> _zero(), '+');
    }

    my $u = $class -> _zero();
    my $v = $class -> _one();
    my $a = $class -> _copy($y);
    my $b = $class -> _copy($x);

    # Euclid's Algorithm for bgcd().

    my $q;
    my $sign = 1;
    {
        ($a, $q, $b) = ($b, $class -> _div($a, $b));
        last if $class -> _is_zero($b);

        my $vq = $class -> _mul($class -> _copy($v), $q);
        my $t = $class -> _add($vq, $u);
        $u = $v;
        $v = $t;
        $sign = -$sign;
        redo;
    }

    # if the gcd is not 1, there exists no modular multiplicative inverse
    return unless $class -> _is_one($a);

    ($v, $sign == 1 ? '+' : '-');
}

sub _modpow {
    # modulus of power ($x ** $y) % $z
    my ($class, $num, $exp, $mod) = @_;

    # a^b (mod 1) = 0 for all a and b
    if ($class -> _is_one($mod)) {
        return $class -> _zero();
    }

    # 0^a (mod m) = 0 if m != 0, a != 0
    # 0^0 (mod m) = 1 if m != 0
    if ($class -> _is_zero($num)) {
        return $class -> _is_zero($exp) ? $class -> _one()
                                        : $class -> _zero();
    }

    #  $num = $class -> _mod($num, $mod);   # this does not make it faster

    my $acc = $class -> _copy($num);
    my $t   = $class -> _one();

    my $expbin = $class -> _as_bin($exp);
    $expbin =~ s/^0b//;
    my $len = length($expbin);

    while (--$len >= 0) {
        if (substr($expbin, $len, 1) eq '1') {
            $t = $class -> _mul($t, $acc);
            $t = $class -> _mod($t, $mod);
        }
        $acc = $class -> _mul($acc, $acc);
        $acc = $class -> _mod($acc, $mod);
    }
    return $t;
}

sub _gcd {
    # Greatest common divisor.

    my ($class, $x, $y) = @_;

    # gcd(0, 0) = 0
    # gcd(0, a) = a, if a != 0

    if ($class -> _acmp($x, $y) == 0) {
        return $class -> _copy($x);
    }

    if ($class -> _is_zero($x)) {
        if ($class -> _is_zero($y)) {
            return $class -> _zero();
        } else {
            return $class -> _copy($y);
        }
    } else {
        if ($class -> _is_zero($y)) {
            return $class -> _copy($x);
        } else {

            # Until $y is zero ...

            $x = $class -> _copy($x);
            until ($class -> _is_zero($y)) {

                # Compute remainder.

                $x = $class -> _mod($x, $y);

                # Swap $x and $y.

                my $tmp = $x;
                $x = $class -> _copy($y);
                $y = $tmp;
            }

            return $x;
        }
    }
}

sub _lcm {
    # Least common multiple.

    my ($class, $x, $y) = @_;

    # lcm(0, x) = 0 for all x

    return $class -> _zero()
      if ($class -> _is_zero($x) ||
          $class -> _is_zero($y));

    my $gcd = $class -> _gcd($class -> _copy($x), $y);
    $x = $class -> _div($x, $gcd);
    $x = $class -> _mul($x, $y);
    return $x;
}

sub _lucas {
    my ($class, $n) = @_;

    $n = $class -> _num($n) if ref $n;

    # In list context, use lucas(n) = lucas(n-1) + lucas(n-2)

    if (wantarray) {
        my @y;

        push @y, $class -> _two();
        return @y if $n == 0;

        push @y, $class -> _one();
        return @y if $n == 1;

        for (my $i = 2 ; $i <= $n ; ++ $i) {
            $y[$i] = $class -> _add($class -> _copy($y[$i - 1]), $y[$i - 2]);
        }

        return @y;
    }

    # In scalar context use that lucas(n) = fib(n-1) + fib(n+1).
    #
    # Remember that _fib() behaves differently in scalar context and list
    # context, so we must add scalar() to get the desired behaviour.

    return $class -> _two() if $n == 0;

    return $class -> _add(scalar($class -> _fib($n - 1)),
                          scalar($class -> _fib($n + 1)));
}

sub _fib {
    my ($class, $n) = @_;

    $n = $class -> _num($n) if ref $n;

    # In list context, use fib(n) = fib(n-1) + fib(n-2)

    if (wantarray) {
        my @y;

        push @y, $class -> _zero();
        return @y if $n == 0;

        push @y, $class -> _one();
        return @y if $n == 1;

        for (my $i = 2 ; $i <= $n ; ++ $i) {
            $y[$i] = $class -> _add($class -> _copy($y[$i - 1]), $y[$i - 2]);
        }

        return @y;
    }

    # In scalar context use a fast algorithm that is much faster than the
    # recursive algorith used in list context.

    my $cache = {};
    my $two = $class -> _two();
    my $fib;

    $fib = sub {
        my $n = shift;
        return $class -> _zero() if $n <= 0;
        return $class -> _one()  if $n <= 2;
        return $cache -> {$n}    if exists $cache -> {$n};

        my $k = int($n / 2);
        my $a = $fib -> ($k + 1);
        my $b = $fib -> ($k);
        my $y;

        if ($n % 2 == 1) {
            # a*a + b*b
            $y = $class -> _add($class -> _mul($class -> _copy($a), $a),
                                $class -> _mul($class -> _copy($b), $b));
        } else {
            # (2*a - b)*b
            $y = $class -> _mul($class -> _sub($class -> _mul(
                   $class -> _copy($two), $a), $b), $b);
        }

        $cache -> {$n} = $y;
        return $y;
    };

    return $fib -> ($n);
}

##############################################################################
##############################################################################

1;

__END__

=pod

=head1 NAME

Math::BigInt::Lib - virtual parent class for Math::BigInt libraries

=head1 SYNOPSIS

    # In the backend library for Math::BigInt et al.

    package Math::BigInt::MyBackend;

    use Math::BigInt::Lib;
    our @ISA = qw< Math::BigInt::Lib >;

    sub _new { ... }
    sub _str { ... }
    sub _add { ... }
    str _sub { ... }
    ...

    # In your main program.

    use Math::BigInt lib => 'MyBackend';

=head1 DESCRIPTION

This module provides support for big integer calculations. It is not intended
to be used directly, but rather as a parent class for backend libraries used by
Math::BigInt, Math::BigFloat, Math::BigRat, and related modules.

Other backend libraries include Math::BigInt::Calc, Math::BigInt::FastCalc,
Math::BigInt::GMP, and Math::BigInt::Pari.

In order to allow for multiple big integer libraries, Math::BigInt was
rewritten to use a plug-in library for core math routines. Any module which
conforms to the API can be used by Math::BigInt by using this in your program:

        use Math::BigInt lib => 'libname';

'libname' is either the long name, like 'Math::BigInt::Pari', or only the short
version, like 'Pari'.

=head2 General Notes

A library only needs to deal with unsigned big integers. Testing of input
parameter validity is done by the caller, so there is no need to worry about
underflow (e.g., in C<_sub()> and C<_dec()>) or about division by zero (e.g.,
in C<_div()> and C<_mod()>)) or similar cases.

Some libraries use methods that don't modify their argument, and some libraries
don't even use objects, but rather unblessed references. Because of this,
liberary methods are always called as class methods, not instance methods:

    $x = Class -> method($x, $y);     # like this
    $x = $x -> method($y);            # not like this ...
    $x -> method($y);                 # ... or like this

And with boolean methods

    $bool = Class -> method($x, $y);  # like this
    $bool = $x -> method($y);         # not like this

Return values are always objects, strings, Perl scalars, or true/false for
comparison routines.

=head3 API version

=over 4

=item CLASS-E<gt>api_version()

This method is no longer used and can be omitted. Methods that are not
implemented by a subclass will be inherited from this class.

=back

=head3 Constructors

The following methods are mandatory: _new(), _str(), _add(), and _sub().
However, computations will be very slow without _mul() and _div().

=over 4

=item CLASS-E<gt>_new(STR)

Convert a string representing an unsigned decimal number to an object
representing the same number. The input is normalized, i.e., it matches
C<^(0|[1-9]\d*)$>.

=item CLASS-E<gt>_zero()

Return an object representing the number zero.

=item CLASS-E<gt>_one()

Return an object representing the number one.

=item CLASS-E<gt>_two()

Return an object representing the number two.

=item CLASS-E<gt>_ten()

Return an object representing the number ten.

=item CLASS-E<gt>_from_bin(STR)

Return an object given a string representing a binary number. The input has a
'0b' prefix and matches the regular expression C<^0[bB](0|1[01]*)$>.

=item CLASS-E<gt>_from_oct(STR)

Return an object given a string representing an octal number. The input has a
'0' prefix and matches the regular expression C<^0[1-7]*$>.

=item CLASS-E<gt>_from_hex(STR)

Return an object given a string representing a hexadecimal number. The input
has a '0x' prefix and matches the regular expression
C<^0x(0|[1-9a-fA-F][\da-fA-F]*)$>.

=item CLASS-E<gt>_from_bytes(STR)

Returns an object given a byte string representing the number. The byte string
is in big endian byte order, so the two-byte input string "\x01\x00" should
give an output value representing the number 256.

=item CLASS-E<gt>_from_base(STR, BASE, COLLSEQ)

Returns an object given a string STR, a base BASE, and a collation sequence
COLLSEQ. Each character in STR represents a numerical value identical to the
character's position in COLLSEQ. All characters in STR must be present in
COLLSEQ.

If BASE is less than or equal to 94, and a collation sequence is not specified,
the following default collation sequence is used. It contains of all the 94
printable ASCII characters except space/blank:

    0123456789                  # ASCII  48 to  57
    ABCDEFGHIJKLMNOPQRSTUVWXYZ  # ASCII  65 to  90
    abcdefghijklmnopqrstuvwxyz  # ASCII  97 to 122
    !"#$%&'()*+,-./             # ASCII  33 to  47
    :;<=>?@                     # ASCII  58 to  64
    [\]^_`                      # ASCII  91 to  96
    {|}~                        # ASCII 123 to 126

If the default collation sequence is used, and the BASE is less than or equal
to 36, the letter case in STR is ignored.

For instance, with base 3 and collation sequence "-/|", the character "-"
represents 0, "/" represents 1, and "|" represents 2. So if STR is "/|-", the
output is 1 * 3**2 + 2 * 3**1 + 0 * 3**0 = 15.

The following examples show standard binary, octal, decimal, and hexadecimal
conversion. All examples return 250.

    $x = $class -> _from_base("11111010", 2)
    $x = $class -> _from_base("372", 8)
    $x = $class -> _from_base("250", 10)
    $x = $class -> _from_base("FA", 16)

Some more examples, all returning 250:

    $x = $class -> _from_base("100021", 3)
    $x = $class -> _from_base("3322", 4)
    $x = $class -> _from_base("2000", 5)
    $x = $class -> _from_base("caaa", 5, "abcde")
    $x = $class -> _from_base("42", 62)
    $x = $class -> _from_base("2!", 94)

=item CLASS-E<gt>_from_base_num(ARRAY, BASE)

Returns an object given an array of values and a base. This method is
equivalent to C<_from_base()>, but works on numbers in an array rather than
characters in a string. Unlike C<_from_base()>, all input values may be
arbitrarily large.

    $x = $class -> _from_base_num([1, 1, 0, 1], 2)    # $x is 13
    $x = $class -> _from_base_num([3, 125, 39], 128)  # $x is 65191

=back

=head3 Mathematical functions

=over 4

=item CLASS-E<gt>_add(OBJ1, OBJ2)

Addition. Returns the result of adding OBJ2 to OBJ1.

=item CLASS-E<gt>_mul(OBJ1, OBJ2)

Multiplication. Returns the result of multiplying OBJ2 and OBJ1.

=item CLASS-E<gt>_div(OBJ1, OBJ2)

Division. In scalar context, returns the quotient after dividing OBJ1 by OBJ2
and truncating the result to an integer. In list context, return the quotient
and the remainder.

=item CLASS-E<gt>_sub(OBJ1, OBJ2, FLAG)

=item CLASS-E<gt>_sub(OBJ1, OBJ2)

Subtraction. Returns the result of subtracting OBJ2 by OBJ1. If C<flag> is false
or omitted, OBJ1 might be modified. If C<flag> is true, OBJ2 might be modified.

=item CLASS-E<gt>_sadd(OBJ1, SIGN1, OBJ2, SIGN2)

Signed addition. Returns the result of adding OBJ2 with sign SIGN2 to OBJ1 with
sign SIGN1.

    ($obj3, $sign3) = $class -> _sadd($obj1, $sign1, $obj2, $sign2);

=item CLASS-E<gt>_ssub(OBJ1, SIGN1, OBJ2, SIGN2)

Signed subtraction. Returns the result of subtracting OBJ2 with sign SIGN2 to
OBJ1 with sign SIGN1.

    ($obj3, $sign3) = $class -> _sadd($obj1, $sign1, $obj2, $sign2);

=item CLASS-E<gt>_dec(OBJ)

Returns the result after decrementing OBJ by one.

=item CLASS-E<gt>_inc(OBJ)

Returns the result after incrementing OBJ by one.

=item CLASS-E<gt>_mod(OBJ1, OBJ2)

Returns OBJ1 modulo OBJ2, i.e., the remainder after dividing OBJ1 by OBJ2.

=item CLASS-E<gt>_sqrt(OBJ)

Returns the square root of OBJ, truncated to an integer.

=item CLASS-E<gt>_root(OBJ, N)

Returns the Nth root of OBJ, truncated to an integer.

=item CLASS-E<gt>_fac(OBJ)

Returns the factorial of OBJ, i.e., the product of all positive integers up to
and including OBJ.

=item CLASS-E<gt>_dfac(OBJ)

Returns the double factorial of OBJ. If OBJ is an even integer, returns the
product of all positive, even integers up to and including OBJ, i.e.,
2*4*6*...*OBJ. If OBJ is an odd integer, returns the product of all positive,
odd integers, i.e., 1*3*5*...*OBJ.

=item CLASS-E<gt>_pow(OBJ1, OBJ2)

Returns OBJ1 raised to the power of OBJ2. By convention, 0**0 = 1.

=item CLASS-E<gt>_modinv(OBJ1, OBJ2)

Returns the modular multiplicative inverse, i.e., return OBJ3 so that

    (OBJ3 * OBJ1) % OBJ2 = 1 % OBJ2

The result is returned as two arguments. If the modular multiplicative inverse
does not exist, both arguments are undefined. Otherwise, the arguments are a
number (object) and its sign ("+" or "-").

The output value, with its sign, must either be a positive value in the range
1,2,...,OBJ2-1 or the same value subtracted OBJ2. For instance, if the input
arguments are objects representing the numbers 7 and 5, the method must either
return an object representing the number 3 and a "+" sign, since (3*7) % 5 = 1
% 5, or an object representing the number 2 and a "-" sign, since (-2*7) % 5 = 1
% 5.

=item CLASS-E<gt>_modpow(OBJ1, OBJ2, OBJ3)

Returns the modular exponentiation, i.e., (OBJ1 ** OBJ2) % OBJ3.

=item CLASS-E<gt>_rsft(OBJ, N, B)

Returns the result after shifting OBJ N digits to thee right in base B. This is
equivalent to performing integer division by B**N and discarding the remainder,
except that it might be much faster.

For instance, if the object $obj represents the hexadecimal number 0xabcde,
then C<_rsft($obj, 2, 16)> returns an object representing the number 0xabc. The
"remainer", 0xde, is discarded and not returned.

=item CLASS-E<gt>_lsft(OBJ, N, B)

Returns the result after shifting OBJ N digits to the left in base B. This is
equivalent to multiplying by B**N, except that it might be much faster.

=item CLASS-E<gt>_log_int(OBJ, B)

Returns the logarithm of OBJ to base BASE truncted to an integer. This method
has two output arguments, the OBJECT and a STATUS. The STATUS is Perl scalar;
it is 1 if OBJ is the exact result, 0 if the result was truncted to give OBJ,
and undef if it is unknown whether OBJ is the exact result.

=item CLASS-E<gt>_gcd(OBJ1, OBJ2)

Returns the greatest common divisor of OBJ1 and OBJ2.

=item CLASS-E<gt>_lcm(OBJ1, OBJ2)

Return the least common multiple of OBJ1 and OBJ2.

=item CLASS-E<gt>_fib(OBJ)

In scalar context, returns the nth Fibonacci number: _fib(0) returns 0, _fib(1)
returns 1, _fib(2) returns 1, _fib(3) returns 2 etc. In list context, returns
the Fibonacci numbers from F(0) to F(n): 0, 1, 1, 2, 3, 5, 8, 13, 21, 34, ...

=item CLASS-E<gt>_lucas(OBJ)

In scalar context, returns the nth Lucas number: _lucas(0) returns 2, _lucas(1)
returns 1, _lucas(2) returns 3, etc. In list context, returns the Lucas numbers
from L(0) to L(n): 2, 1, 3, 4, 7, 11, 18, 29,47, 76, ...

=back

=head3 Bitwise operators

=over 4

=item CLASS-E<gt>_and(OBJ1, OBJ2)

Returns bitwise and.

=item CLASS-E<gt>_or(OBJ1, OBJ2)

Returns bitwise or.

=item CLASS-E<gt>_xor(OBJ1, OBJ2)

Returns bitwise exclusive or.

=item CLASS-E<gt>_sand(OBJ1, OBJ2, SIGN1, SIGN2)

Returns bitwise signed and.

=item CLASS-E<gt>_sor(OBJ1, OBJ2, SIGN1, SIGN2)

Returns bitwise signed or.

=item CLASS-E<gt>_sxor(OBJ1, OBJ2, SIGN1, SIGN2)

Returns bitwise signed exclusive or.

=back

=head3 Boolean operators

=over 4

=item CLASS-E<gt>_is_zero(OBJ)

Returns a true value if OBJ is zero, and false value otherwise.

=item CLASS-E<gt>_is_one(OBJ)

Returns a true value if OBJ is one, and false value otherwise.

=item CLASS-E<gt>_is_two(OBJ)

Returns a true value if OBJ is two, and false value otherwise.

=item CLASS-E<gt>_is_ten(OBJ)

Returns a true value if OBJ is ten, and false value otherwise.

=item CLASS-E<gt>_is_even(OBJ)

Return a true value if OBJ is an even integer, and a false value otherwise.

=item CLASS-E<gt>_is_odd(OBJ)

Return a true value if OBJ is an even integer, and a false value otherwise.

=item CLASS-E<gt>_acmp(OBJ1, OBJ2)

Compare OBJ1 and OBJ2 and return -1, 0, or 1, if OBJ1 is numerically less than,
equal to, or larger than OBJ2, respectively.

=back

=head3 String conversion

=over 4

=item CLASS-E<gt>_str(OBJ)

Returns a string representing OBJ in decimal notation. The returned string
should have no leading zeros, i.e., it should match C<^(0|[1-9]\d*)$>.

=item CLASS-E<gt>_to_bin(OBJ)

Returns the binary string representation of OBJ.

=item CLASS-E<gt>_to_oct(OBJ)

Returns the octal string representation of the number.

=item CLASS-E<gt>_to_hex(OBJ)

Returns the hexadecimal string representation of the number.

=item CLASS-E<gt>_to_bytes(OBJ)

Returns a byte string representation of OBJ. The byte string is in big endian
byte order, so if OBJ represents the number 256, the output should be the
two-byte string "\x01\x00".

=item CLASS-E<gt>_to_base(OBJ, BASE, COLLSEQ)

Returns a string representation of OBJ in base BASE with collation sequence
COLLSEQ.

    $val = $class -> _new("210");
    $str = $class -> _to_base($val, 10, "xyz")  # $str is "zyx"

    $val = $class -> _new("32");
    $str = $class -> _to_base($val, 2, "-|")  # $str is "|-----"

See _from_base() for more information.

=item CLASS-E<gt>_to_base_num(OBJ, BASE)

Converts the given number to the given base. This method is equivalent to
C<_to_base()>, but returns numbers in an array rather than characters in a
string. In the output, the first element is the most significant. Unlike
C<_to_base()>, all input values may be arbitrarily large.

    $x = $class -> _to_base_num(13, 2)        # $x is [1, 1, 0, 1]
    $x = $class -> _to_base_num(65191, 128)   # $x is [3, 125, 39]

=item CLASS-E<gt>_as_bin(OBJ)

Like C<_to_bin()> but with a '0b' prefix.

=item CLASS-E<gt>_as_oct(OBJ)

Like C<_to_oct()> but with a '0' prefix.

=item CLASS-E<gt>_as_hex(OBJ)

Like C<_to_hex()> but with a '0x' prefix.

=item CLASS-E<gt>_as_bytes(OBJ)

This is an alias to C<_to_bytes()>.

=back

=head3 Numeric conversion

=over 4

=item CLASS-E<gt>_num(OBJ)

Returns a Perl scalar number representing the number OBJ as close as
possible. Since Perl scalars have limited precision, the returned value might
not be exactly the same as OBJ.

=back

=head3 Miscellaneous

=over 4

=item CLASS-E<gt>_copy(OBJ)

Returns a true copy OBJ.

=item CLASS-E<gt>_len(OBJ)

Returns the number of the decimal digits in OBJ. The output is a Perl scalar.

=item CLASS-E<gt>_zeros(OBJ)

Returns the number of trailing decimal zeros. The output is a Perl scalar. The
number zero has no trailing decimal zeros.

=item CLASS-E<gt>_digit(OBJ, N)

Returns the Nth digit in OBJ as a Perl scalar. N is a Perl scalar, where zero
refers to the rightmost (least significant) digit, and negative values count
from the left (most significant digit). If $obj represents the number 123, then

    CLASS->_digit($obj,  0)     # returns 3
    CLASS->_digit($obj,  1)     # returns 2
    CLASS->_digit($obj,  2)     # returns 1
    CLASS->_digit($obj, -1)     # returns 1

=item CLASS-E<gt>_digitsum(OBJ)

Returns the sum of the base 10 digits.

=item CLASS-E<gt>_check(OBJ)

Returns true if the object is invalid and false otherwise. Preferably, the true
value is a string describing the problem with the object. This is a check
routine to test the internal state of the object for corruption.

=item CLASS-E<gt>_set(OBJ)

xxx

=back

=head2 API version 2

The following methods are required for an API version of 2 or greater.

=head3 Constructors

=over 4

=item CLASS-E<gt>_1ex(N)

Return an object representing the number 10**N where N E<gt>= 0 is a Perl
scalar.

=back

=head3 Mathematical functions

=over 4

=item CLASS-E<gt>_nok(OBJ1, OBJ2)

Return the binomial coefficient OBJ1 over OBJ1.

=back

=head3 Miscellaneous

=over 4

=item CLASS-E<gt>_alen(OBJ)

Return the approximate number of decimal digits of the object. The output is a
Perl scalar.

=back

=head1 WRAP YOUR OWN

If you want to port your own favourite C library for big numbers to the
Math::BigInt interface, you can take any of the already existing modules as a
rough guideline. You should really wrap up the latest Math::BigInt and
Math::BigFloat testsuites with your module, and replace in them any of the
following:

        use Math::BigInt;

by this:

        use Math::BigInt lib => 'yourlib';

This way you ensure that your library really works 100% within Math::BigInt.

=head1 BUGS

Please report any bugs or feature requests to
C<bug-math-bigint at rt.cpan.org>, or through the web interface at
L<https://rt.cpan.org/Ticket/Create.html?Queue=Math-BigInt>
(requires login).
We will be notified, and then you'll automatically be notified of progress on
your bug as I make changes.

=head1 SUPPORT

You can find documentation for this module with the perldoc command.

    perldoc Math::BigInt::Calc

You can also look for information at:

=over 4

=item * RT: CPAN's request tracker

L<https://rt.cpan.org/Public/Dist/Display.html?Name=Math-BigInt>

=item * AnnoCPAN: Annotated CPAN documentation

L<http://annocpan.org/dist/Math-BigInt>

=item * CPAN Ratings

L<https://cpanratings.perl.org/dist/Math-BigInt>

=item * MetaCPAN

L<https://metacpan.org/release/Math-BigInt>

=item * CPAN Testers Matrix

L<http://matrix.cpantesters.org/?dist=Math-BigInt>

=item * The Bignum mailing list

=over 4

=item * Post to mailing list

C<bignum at lists.scsys.co.uk>

=item * View mailing list

L<http://lists.scsys.co.uk/pipermail/bignum/>

=item * Subscribe/Unsubscribe

L<http://lists.scsys.co.uk/cgi-bin/mailman/listinfo/bignum>

=back

=back

=head1 LICENSE

This program is free software; you may redistribute it and/or modify it under
the same terms as Perl itself.

=head1 AUTHOR

Peter John Acklam, E<lt>pjacklam@gmail.comE<gt>

Code and documentation based on the Math::BigInt::Calc module by Tels
E<lt>nospam-abuse@bloodgate.comE<gt>

=head1 SEE ALSO

L<Math::BigInt>, L<Math::BigInt::Calc>, L<Math::BigInt::GMP>,
L<Math::BigInt::FastCalc> and L<Math::BigInt::Pari>.

=cut
