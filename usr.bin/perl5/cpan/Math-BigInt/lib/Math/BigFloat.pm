package Math::BigFloat;

#
# Mike grinned. 'Two down, infinity to go' - Mike Nostrus in 'Before and After'
#

# The following hash values are used internally:
# sign  : "+", "-", "+inf", "-inf", or "NaN" if not a number
#   _m  : mantissa ($LIB thingy)
#   _es : sign of _e
#   _e  : exponent ($LIB thingy)
#   _a  : accuracy
#   _p  : precision

use 5.006001;
use strict;
use warnings;

use Carp          qw< carp croak >;
use Scalar::Util  qw< blessed >;
use Math::BigInt  qw< >;

our $VERSION = '1.999837';
$VERSION =~ tr/_//d;

require Exporter;
our @ISA        = qw/Math::BigInt/;
our @EXPORT_OK  = qw/bpi/;

# $_trap_inf/$_trap_nan are internal and should never be accessed from outside
our ($AUTOLOAD, $accuracy, $precision, $div_scale, $round_mode, $rnd_mode,
     $upgrade, $downgrade, $_trap_nan, $_trap_inf);

use overload

  # overload key: with_assign

  '+'     =>      sub { $_[0] -> copy() -> badd($_[1]); },

  '-'     =>      sub { my $c = $_[0] -> copy();
                        $_[2] ? $c -> bneg() -> badd($_[1])
                              : $c -> bsub($_[1]); },

  '*'     =>      sub { $_[0] -> copy() -> bmul($_[1]); },

  '/'     =>      sub { $_[2] ? ref($_[0]) -> new($_[1]) -> bdiv($_[0])
                              : $_[0] -> copy() -> bdiv($_[1]); },

  '%'     =>      sub { $_[2] ? ref($_[0]) -> new($_[1]) -> bmod($_[0])
                              : $_[0] -> copy() -> bmod($_[1]); },

  '**'    =>      sub { $_[2] ? ref($_[0]) -> new($_[1]) -> bpow($_[0])
                              : $_[0] -> copy() -> bpow($_[1]); },

  '<<'    =>      sub { $_[2] ? ref($_[0]) -> new($_[1]) -> blsft($_[0])
                              : $_[0] -> copy() -> blsft($_[1]); },

  '>>'    =>      sub { $_[2] ? ref($_[0]) -> new($_[1]) -> brsft($_[0])
                              : $_[0] -> copy() -> brsft($_[1]); },

  # overload key: assign

  '+='    =>      sub { $_[0] -> badd($_[1]); },

  '-='    =>      sub { $_[0] -> bsub($_[1]); },

  '*='    =>      sub { $_[0] -> bmul($_[1]); },

  '/='    =>      sub { scalar $_[0] -> bdiv($_[1]); },

  '%='    =>      sub { $_[0] -> bmod($_[1]); },

  '**='   =>      sub { $_[0] -> bpow($_[1]); },

  '<<='   =>      sub { $_[0] -> blsft($_[1]); },

  '>>='   =>      sub { $_[0] -> brsft($_[1]); },

#  'x='    =>      sub { },

#  '.='    =>      sub { },

  # overload key: num_comparison

  '<'     =>      sub { $_[2] ? ref($_[0]) -> new($_[1]) -> blt($_[0])
                              : $_[0] -> blt($_[1]); },

  '<='    =>      sub { $_[2] ? ref($_[0]) -> new($_[1]) -> ble($_[0])
                              : $_[0] -> ble($_[1]); },

  '>'     =>      sub { $_[2] ? ref($_[0]) -> new($_[1]) -> bgt($_[0])
                              : $_[0] -> bgt($_[1]); },

  '>='    =>      sub { $_[2] ? ref($_[0]) -> new($_[1]) -> bge($_[0])
                              : $_[0] -> bge($_[1]); },

  '=='    =>      sub { $_[0] -> beq($_[1]); },

  '!='    =>      sub { $_[0] -> bne($_[1]); },

  # overload key: 3way_comparison

  '<=>'   =>      sub { my $cmp = $_[0] -> bcmp($_[1]);
                        defined($cmp) && $_[2] ? -$cmp : $cmp; },

  'cmp'   =>      sub { $_[2] ? "$_[1]" cmp $_[0] -> bstr()
                              : $_[0] -> bstr() cmp "$_[1]"; },

  # overload key: str_comparison

#  'lt'    =>      sub { $_[2] ? ref($_[0]) -> new($_[1]) -> bstrlt($_[0])
#                              : $_[0] -> bstrlt($_[1]); },
#
#  'le'    =>      sub { $_[2] ? ref($_[0]) -> new($_[1]) -> bstrle($_[0])
#                              : $_[0] -> bstrle($_[1]); },
#
#  'gt'    =>      sub { $_[2] ? ref($_[0]) -> new($_[1]) -> bstrgt($_[0])
#                              : $_[0] -> bstrgt($_[1]); },
#
#  'ge'    =>      sub { $_[2] ? ref($_[0]) -> new($_[1]) -> bstrge($_[0])
#                              : $_[0] -> bstrge($_[1]); },
#
#  'eq'    =>      sub { $_[0] -> bstreq($_[1]); },
#
#  'ne'    =>      sub { $_[0] -> bstrne($_[1]); },

  # overload key: binary

  '&'     =>      sub { $_[2] ? ref($_[0]) -> new($_[1]) -> band($_[0])
                              : $_[0] -> copy() -> band($_[1]); },

  '&='    =>      sub { $_[0] -> band($_[1]); },

  '|'     =>      sub { $_[2] ? ref($_[0]) -> new($_[1]) -> bior($_[0])
                              : $_[0] -> copy() -> bior($_[1]); },

  '|='    =>      sub { $_[0] -> bior($_[1]); },

  '^'     =>      sub { $_[2] ? ref($_[0]) -> new($_[1]) -> bxor($_[0])
                              : $_[0] -> copy() -> bxor($_[1]); },

  '^='    =>      sub { $_[0] -> bxor($_[1]); },

#  '&.'    =>      sub { },

#  '&.='   =>      sub { },

#  '|.'    =>      sub { },

#  '|.='   =>      sub { },

#  '^.'    =>      sub { },

#  '^.='   =>      sub { },

  # overload key: unary

  'neg'   =>      sub { $_[0] -> copy() -> bneg(); },

#  '!'     =>      sub { },

  '~'     =>      sub { $_[0] -> copy() -> bnot(); },

#  '~.'    =>      sub { },

  # overload key: mutators

  '++'    =>      sub { $_[0] -> binc() },

  '--'    =>      sub { $_[0] -> bdec() },

  # overload key: func

  'atan2' =>      sub { $_[2] ? ref($_[0]) -> new($_[1]) -> batan2($_[0])
                              : $_[0] -> copy() -> batan2($_[1]); },

  'cos'   =>      sub { $_[0] -> copy() -> bcos(); },

  'sin'   =>      sub { $_[0] -> copy() -> bsin(); },

  'exp'   =>      sub { $_[0] -> copy() -> bexp($_[1]); },

  'abs'   =>      sub { $_[0] -> copy() -> babs(); },

  'log'   =>      sub { $_[0] -> copy() -> blog(); },

  'sqrt'  =>      sub { $_[0] -> copy() -> bsqrt(); },

  'int'   =>      sub { $_[0] -> copy() -> bint(); },

  # overload key: conversion

  'bool'  =>      sub { $_[0] -> is_zero() ? '' : 1; },

  '""'    =>      sub { $_[0] -> bstr(); },

  '0+'    =>      sub { $_[0] -> numify(); },

  '='     =>      sub { $_[0] -> copy(); },

  ;

##############################################################################
# global constants, flags and assorted stuff

# the following are public, but their usage is not recommended. Use the
# accessor methods instead.

# class constants, use Class->constant_name() to access
# one of 'even', 'odd', '+inf', '-inf', 'zero', 'trunc' or 'common'
$round_mode = 'even';
$accuracy   = undef;
$precision  = undef;
$div_scale  = 40;

$upgrade = undef;
$downgrade = undef;
# the package we are using for our private parts, defaults to:
# Math::BigInt->config('lib')
my $LIB = 'Math::BigInt::Calc';

# are NaNs ok? (otherwise it dies when encountering an NaN) set w/ config()
$_trap_nan = 0;
# the same for infinity
$_trap_inf = 0;

# constant for easier life
my $nan = 'NaN';

my $IMPORT = 0; # was import() called yet? used to make require work

# some digits of accuracy for blog(undef, 10); which we use in blog() for speed
my $LOG_10 =
 '2.3025850929940456840179914546843642076011014886287729760333279009675726097';
my $LOG_10_A = length($LOG_10)-1;
# ditto for log(2)
my $LOG_2 =
 '0.6931471805599453094172321214581765680755001343602552541206800094933936220';
my $LOG_2_A = length($LOG_2)-1;
my $HALF = '0.5';                       # made into an object if nec.

##############################################################################
# the old code had $rnd_mode, so we need to support it, too

sub TIESCALAR {
    my ($class) = @_;
    bless \$round_mode, $class;
}

sub FETCH {
    return $round_mode;
}

sub STORE {
    $rnd_mode = $_[0]->round_mode($_[1]);
}

BEGIN {
    # when someone sets $rnd_mode, we catch this and check the value to see
    # whether it is valid or not.
    $rnd_mode   = 'even';
    tie $rnd_mode, 'Math::BigFloat';

    *as_number = \&as_int;
}

sub DESTROY {
    # going through AUTOLOAD for every DESTROY is costly, avoid it by empty sub
}

sub AUTOLOAD {
    # make fxxx and bxxx both work by selectively mapping fxxx() to MBF::bxxx()
    my $name = $AUTOLOAD;
    $name =~ s/(.*):://;        # split package
    my $c = $1 || __PACKAGE__;
    no strict 'refs';
    $c->import() if $IMPORT == 0;
    if (!_method_alias($name)) {
        if (!defined $name) {
            # delayed load of Carp and avoid recursion
            croak("$c: Can't call a method without name");
        }
        if (!_method_hand_up($name)) {
            # delayed load of Carp and avoid recursion
            croak("Can't call $c\-\>$name, not a valid method");
        }
        # try one level up, but subst. bxxx() for fxxx() since MBI only got
        # bxxx()
        $name =~ s/^f/b/;
        return &{"Math::BigInt"."::$name"}(@_);
    }
    my $bname = $name;
    $bname =~ s/^f/b/;
    $c .= "::$name";
    *{$c} = \&{$bname};
    &{$c};                      # uses @_
}

##############################################################################

{
    # valid method aliases for AUTOLOAD
    my %methods = map { $_ => 1 }
      qw / fadd fsub fmul fdiv fround ffround fsqrt fmod fstr fsstr fpow fnorm
           fint facmp fcmp fzero fnan finf finc fdec ffac fneg
           fceil ffloor frsft flsft fone flog froot fexp
         /;
    # valid methods that can be handed up (for AUTOLOAD)
    my %hand_ups = map { $_ => 1 }
      qw / is_nan is_inf is_negative is_positive is_pos is_neg
           accuracy precision div_scale round_mode fabs fnot
           objectify upgrade downgrade
           bone binf bnan bzero
           bsub
         /;

    sub _method_alias { exists $methods{$_[0]||''}; }
    sub _method_hand_up { exists $hand_ups{$_[0]||''}; }
}

sub isa {
    my ($self, $class) = @_;
    return if $class =~ /^Math::BigInt/; # we aren't one of these
    UNIVERSAL::isa($self, $class);
}

sub config {
    # return (later set?) configuration data as hash ref
    my $class = shift || 'Math::BigFloat';

    # Getter/accessor.

    if (@_ == 1 && ref($_[0]) ne 'HASH') {
        my $param = shift;
        return $class if $param eq 'class';
        return $LIB   if $param eq 'with';
        return $class->SUPER::config($param);
    }

    # Setter.

    my $cfg = $class->SUPER::config(@_);

    # now we need only to override the ones that are different from our parent
    $cfg->{class} = $class;
    $cfg->{with} = $LIB;
    $cfg;
}

###############################################################################
# Constructor methods
###############################################################################

sub new {
    # Create a new Math::BigFloat object from a string or another bigfloat
    # object.
    # _e: exponent
    # _m: mantissa
    # sign  => ("+", "-", "+inf", "-inf", or "NaN")

    my $self    = shift;
    my $selfref = ref $self;
    my $class   = $selfref || $self;

    # Make "require" work.

    $class -> import() if $IMPORT == 0;

    # Although this use has been discouraged for more than 10 years, people
    # apparently still use it, so we still support it.

    return $class -> bzero() unless @_;

    my ($wanted, @r) = @_;

    if (!defined($wanted)) {
        #if (warnings::enabled("uninitialized")) {
        #    warnings::warn("uninitialized",
        #                   "Use of uninitialized value in new()");
        #}
        return $class -> bzero(@r);
    }

    if (!ref($wanted) && $wanted eq "") {
        #if (warnings::enabled("numeric")) {
        #    warnings::warn("numeric",
        #                   q|Argument "" isn't numeric in new()|);
        #}
        #return $class -> bzero(@r);
        return $class -> bnan(@r);
    }

    # Initialize a new object.

    $self = bless {}, $class unless $selfref;

    # Math::BigFloat or subclass

    if (defined(blessed($wanted)) && $wanted -> isa($class)) {

        # Don't copy the accuracy and precision, because a new object should get
        # them from the global configuration.

        $self -> {sign} = $wanted -> {sign};
        $self -> {_m}   = $LIB -> _copy($wanted -> {_m});
        $self -> {_es}  = $wanted -> {_es};
        $self -> {_e}   = $LIB -> _copy($wanted -> {_e});
        $self = $self->round(@r)
          unless @r >= 2 && !defined($r[0]) && !defined($r[1]);
        return $self;
    }

    # Shortcut for Math::BigInt and its subclasses. This should be improved.

    if (defined(blessed($wanted))) {
        if ($wanted -> isa('Math::BigInt')) {
            $self->{sign} = $wanted -> {sign};
            $self->{_m}   = $LIB -> _copy($wanted -> {value});
            $self->{_es}  = '+';
            $self->{_e}   = $LIB -> _zero();
            return $self -> bnorm();
        }

        if ($wanted -> can("as_number")) {
            $self->{sign} = $wanted -> sign();
            $self->{_m}   = $wanted -> as_number() -> {value};
            $self->{_es}  = '+';
            $self->{_e}   = $LIB -> _zero();
            return $self -> bnorm();
        }
    }

    # Shortcut for simple forms like '123' that have no trailing zeros. Trailing
    # zeros would require a non-zero exponent.

    if ($wanted =~
        / ^
          \s*                           # optional leading whitespace
          ( [+-]? )                     # optional sign
          0*                            # optional leading zeros
          ( [1-9] (?: [0-9]* [1-9] )? ) # significand
          \s*                           # optional trailing whitespace
          $
        /x)
    {
        return $downgrade -> new($1 . $2) if defined $downgrade;
        $self->{sign} = $1 || '+';
        $self->{_m}   = $LIB -> _new($2);
        $self->{_es}  = '+';
        $self->{_e}   = $LIB -> _zero();
        $self = $self->round(@r)
          unless @r >= 2 && !defined $r[0] && !defined $r[1];
        return $self;
    }

    # Handle Infs.

    if ($wanted =~ / ^
                     \s*
                     ( [+-]? )
                     inf (?: inity )?
                     \s*
                     \z
                   /ix)
    {
        my $sgn = $1 || '+';
        return $class -> binf($sgn, @r);
    }

    # Handle explicit NaNs (not the ones returned due to invalid input).

    if ($wanted =~ / ^
                     \s*
                     ( [+-]? )
                     nan
                     \s*
                     \z
                   /ix)
    {
        return $class -> bnan(@r);
    }

    my @parts;

    if (
        # Handle hexadecimal numbers. We auto-detect hexadecimal numbers if they
        # have a "0x", "0X", "x", or "X" prefix, cf. CORE::oct().

        $wanted =~ /^\s*[+-]?0?[Xx]/ and
        @parts = $class -> _hex_str_to_flt_lib_parts($wanted)

          or

        # Handle octal numbers. We auto-detect octal numbers if they have a
        # "0o", "0O", "o", "O" prefix, cf. CORE::oct().

        $wanted =~ /^\s*[+-]?0?[Oo]/ and
        @parts = $class -> _oct_str_to_flt_lib_parts($wanted)

          or

        # Handle binary numbers. We auto-detect binary numbers if they have a
        # "0b", "0B", "b", or "B" prefix, cf. CORE::oct().

        $wanted =~ /^\s*[+-]?0?[Bb]/ and
        @parts = $class -> _bin_str_to_flt_lib_parts($wanted)

          or

        # At this point, what is left are decimal numbers that aren't handled
        # above and octal floating point numbers that don't have any of the
        # "0o", "0O", "o", or "O" prefixes. First see if it is a decimal number.

        @parts = $class -> _dec_str_to_flt_lib_parts($wanted)
          or

        # See if it is an octal floating point number. The extra check is
        # included because _oct_str_to_flt_lib_parts() accepts octal numbers
        # that don't have a prefix (this is needed to make it work with, e.g.,
        # from_oct() that don't require a prefix). However, Perl requires a
        # prefix for octal floating point literals. For example, "1p+0" is not
        # valid, but "01p+0" and "0__1p+0" are.

        $wanted =~ /^\s*[+-]?0_*\d/ and
        @parts = $class -> _oct_str_to_flt_lib_parts($wanted))
    {
        ($self->{sign}, $self->{_m}, $self->{_es}, $self->{_e}) = @parts;

        $self = $self->round(@r)
          unless @r >= 2 && !defined($r[0]) && !defined($r[1]);

        return $downgrade -> new($self -> bdstr(), @r)
          if defined($downgrade) && $self -> is_int();
        return $self;
    }

    # If we get here, the value is neither a valid decimal, binary, octal, or
    # hexadecimal number. It is not an explicit Inf or a NaN either.

    return $class -> bnan(@r);
}

sub from_dec {
    my $self    = shift;
    my $selfref = ref $self;
    my $class   = $selfref || $self;

    # Don't modify constant (read-only) objects.

    return $self if $selfref && $self->modify('from_dec');

    my $str = shift;
    my @r = @_;

    # If called as a class method, initialize a new object.

    $self = bless {}, $class unless $selfref;

    if (my @parts = $class -> _dec_str_to_flt_lib_parts($str)) {
        ($self->{sign}, $self->{_m}, $self->{_es}, $self->{_e}) = @parts;

        $self = $self->round(@r)
          unless @r >= 2 && !defined($r[0]) && !defined($r[1]);

        return $downgrade -> new($self -> bdstr(), @r)
          if defined($downgrade) && $self -> is_int();
        return $self;
    }

    return $self -> bnan(@r);
}

sub from_hex {
    my $self    = shift;
    my $selfref = ref $self;
    my $class   = $selfref || $self;

    # Don't modify constant (read-only) objects.

    return $self if $selfref && $self->modify('from_hex');

    my $str = shift;
    my @r = @_;

    # If called as a class method, initialize a new object.

    $self = bless {}, $class unless $selfref;

    if (my @parts = $class -> _hex_str_to_flt_lib_parts($str)) {
        ($self->{sign}, $self->{_m}, $self->{_es}, $self->{_e}) = @parts;

        $self = $self->round(@r)
          unless @r >= 2 && !defined($r[0]) && !defined($r[1]);

        return $downgrade -> new($self -> bdstr(), @r)
          if defined($downgrade) && $self -> is_int();
        return $self;
    }

    return $self -> bnan(@r);
}

sub from_oct {
    my $self    = shift;
    my $selfref = ref $self;
    my $class   = $selfref || $self;

    # Don't modify constant (read-only) objects.

    return $self if $selfref && $self->modify('from_oct');

    my $str = shift;
    my @r = @_;

    # If called as a class method, initialize a new object.

    $self = bless {}, $class unless $selfref;

    if (my @parts = $class -> _oct_str_to_flt_lib_parts($str)) {
        ($self->{sign}, $self->{_m}, $self->{_es}, $self->{_e}) = @parts;

        $self = $self->round(@r)
          unless @r >= 2 && !defined($r[0]) && !defined($r[1]);

        return $downgrade -> new($self -> bdstr(), @r)
          if defined($downgrade) && $self -> is_int();
        return $self;
    }

    return $self -> bnan(@r);
}

sub from_bin {
    my $self    = shift;
    my $selfref = ref $self;
    my $class   = $selfref || $self;

    # Don't modify constant (read-only) objects.

    return $self if $selfref && $self->modify('from_bin');

    my $str = shift;
    my @r = @_;

    # If called as a class method, initialize a new object.

    $self = bless {}, $class unless $selfref;

    if (my @parts = $class -> _bin_str_to_flt_lib_parts($str)) {
        ($self->{sign}, $self->{_m}, $self->{_es}, $self->{_e}) = @parts;

        $self = $self->round(@r)
          unless @r >= 2 && !defined($r[0]) && !defined($r[1]);

        return $downgrade -> new($self -> bdstr(), @r)
          if defined($downgrade) && $self -> is_int();
        return $self;
    }

    return $self -> bnan(@r);
}

sub from_ieee754 {
    my $self    = shift;
    my $selfref = ref $self;
    my $class   = $selfref || $self;

    # Don't modify constant (read-only) objects.

    return $self if $selfref && $self->modify('from_ieee754');

    my $in     = shift;     # input string (or raw bytes)
    my $format = shift;     # format ("binary32", "decimal64" etc.)
    my $enc;                # significand encoding (applies only to decimal)
    my $k;                  # storage width in bits
    my $b;                  # base
    my @r = @_;             # rounding parameters, if any

    if ($format =~ /^binary(\d+)\z/) {
        $k = $1;
        $b = 2;
    } elsif ($format =~ /^decimal(\d+)(dpd|bcd)?\z/) {
        $k = $1;
        $b = 10;
        $enc = $2 || 'dpd';     # default is dencely-packed decimals (DPD)
    } elsif ($format eq 'half') {
        $k = 16;
        $b = 2;
    } elsif ($format eq 'single') {
        $k = 32;
        $b = 2;
    } elsif ($format eq 'double') {
        $k = 64;
        $b = 2;
    } elsif ($format eq 'quadruple') {
        $k = 128;
        $b = 2;
    } elsif ($format eq 'octuple') {
        $k = 256;
        $b = 2;
    } elsif ($format eq 'sexdecuple') {
        $k = 512;
        $b = 2;
    }

    if ($b == 2) {

        # Get the parameters for this format.

        my $p;                      # precision (in bits)
        my $t;                      # number of bits in significand
        my $w;                      # number of bits in exponent

        if ($k == 16) {             # binary16 (half-precision)
            $p = 11;
            $t = 10;
            $w =  5;
        } elsif ($k == 32) {        # binary32 (single-precision)
            $p = 24;
            $t = 23;
            $w =  8;
        } elsif ($k == 64) {        # binary64 (double-precision)
            $p = 53;
            $t = 52;
            $w = 11;
        } else {                    # binaryN (quadruple-precision and above)
            if ($k < 128 || $k != 32 * sprintf('%.0f', $k / 32)) {
                croak "Number of bits must be 16, 32, 64, or >= 128 and",
                  " a multiple of 32";
            }
            $p = $k - sprintf('%.0f', 4 * log($k) / log(2)) + 13;
            $t = $p - 1;
            $w = $k - $t - 1;
        }

        # The maximum exponent, minimum exponent, and exponent bias.

        my $emax = Math::BigFloat -> new(2) -> bpow($w - 1) -> bdec();
        my $emin = 1 - $emax;
        my $bias = $emax;

        # Undefined input.

        unless (defined $in) {
            carp("Input is undefined");
            return $self -> bzero(@r);
        }

        # Make sure input string is a string of zeros and ones.

        my $len = CORE::length $in;
        if (8 * $len == $k) {                   # bytes
            $in = unpack "B*", $in;
        } elsif (4 * $len == $k) {              # hexadecimal
            if ($in =~ /([^\da-f])/i) {
                croak "Illegal hexadecimal digit '$1'";
            }
            $in = unpack "B*", pack "H*", $in;
        } elsif ($len == $k) {                  # bits
            if ($in =~ /([^01])/) {
                croak "Illegal binary digit '$1'";
            }
        } else {
            croak "Unknown input -- $in";
        }

        # Split bit string into sign, exponent, and mantissa/significand.

        my $sign = substr($in, 0, 1) eq '1' ? '-' : '+';
        my $expo = $class -> from_bin(substr($in, 1, $w));
        my $mant = $class -> from_bin(substr($in, $w + 1));

        my $x;

        $expo = $expo -> bsub($bias);           # subtract bias

        if ($expo < $emin) {                    # zero and subnormals
            if ($mant == 0) {                   # zero
                $x = $class -> bzero();
            } else {                            # subnormals
                # compute (1/$b)**(N) rather than ($b)**(-N)
                $x = $class -> new("0.5");      # 1/$b
                $x = $x -> bpow($bias + $t - 1) -> bmul($mant);
                $x = $x -> bneg() if $sign eq '-';
            }
        }

        elsif ($expo > $emax) {                 # inf and nan
            if ($mant == 0) {                   # inf
                $x = $class -> binf($sign);
            } else {                            # nan
                $x = $class -> bnan(@r);
            }
        }

        else {                                  # normals
            $mant = $class -> new(2) -> bpow($t) -> badd($mant);
            if ($expo < $t) {
                # compute (1/$b)**(N) rather than ($b)**(-N)
                $x = $class -> new("0.5");      # 1/$b
                $x = $x -> bpow($t - $expo) -> bmul($mant);
            } else {
                $x = $class -> new(2);
                $x = $x -> bpow($expo - $t) -> bmul($mant);
            }
            $x = $x -> bneg() if $sign eq '-';
        }

        if ($selfref) {
            $self -> {sign} = $x -> {sign};
            $self -> {_m}   = $x -> {_m};
            $self -> {_es}  = $x -> {_es};
            $self -> {_e}   = $x -> {_e};
        } else {
            $self = $x;
        }

        return $downgrade -> new($self -> bdstr(), @r)
          if defined($downgrade) && $self -> is_int();
        return $self -> round(@r);
    }

    croak("The format '$format' is not yet supported.");
}

sub bzero {
    # create/assign '+0'

    # Class::method(...) -> Class->method(...)
    unless (@_ && (defined(blessed($_[0])) && $_[0] -> isa(__PACKAGE__) ||
                   $_[0] =~ /^[a-z]\w*(?:::[a-z]\w*)*$/i))
    {
        #carp "Using ", (caller(0))[3], "() as a function is deprecated;",
        #  " use is as a method instead";
        unshift @_, __PACKAGE__;
    }

    my $self    = shift;
    my $selfref = ref $self;
    my $class   = $selfref || $self;

    $self->import() if $IMPORT == 0;            # make require work

    # Don't modify constant (read-only) objects.

    return $self if $selfref && $self->modify('bzero');

    # Get the rounding parameters, if any.

    my @r = @_;

    return $downgrade -> bzero(@r) if defined $downgrade;

    # If called as a class method, initialize a new object.

    $self = bless {}, $class unless $selfref;

    $self -> {sign} = '+';
    $self -> {_m}   = $LIB -> _zero();
    $self -> {_es}  = '+';
    $self -> {_e}   = $LIB -> _zero();

    # If rounding parameters are given as arguments, use them. If no rounding
    # parameters are given, and if called as a class method initialize the new
    # instance with the class variables.

    #return $self -> round(@r);  # this should work, but doesnt; fixme!

    if (@r) {
        croak "can't specify both accuracy and precision"
          if @r >= 2 && defined($r[0]) && defined($r[1]);
        $self->{_a} = $r[0];
        $self->{_p} = $r[1];
    } else {
        unless($selfref) {
            $self->{_a} = $class -> accuracy();
            $self->{_p} = $class -> precision();
        }
    }

    return $self;
}

sub bone {
    # Create or assign '+1' (or -1 if given sign '-').

    # Class::method(...) -> Class->method(...)
    unless (@_ && (defined(blessed($_[0])) && $_[0] -> isa(__PACKAGE__) ||
                   $_[0] =~ /^[a-z]\w*(?:::[a-z]\w*)*$/i))
    {
        #carp "Using ", (caller(0))[3], "() as a function is deprecated;",
        #  " use is as a method instead";
        unshift @_, __PACKAGE__;
    }

    my $self    = shift;
    my $selfref = ref $self;
    my $class   = $selfref || $self;

    $self->import() if $IMPORT == 0;            # make require work

    # Don't modify constant (read-only) objects.

    return $self if $selfref && $self->modify('bone');

    return $downgrade -> bone(@_) if defined $downgrade;

    # Get the sign.

    my $sign = '+';     # default is to return +1
    if (defined($_[0]) && $_[0] =~ /^\s*([+-])\s*$/) {
        $sign = $1;
        shift;
    }

    # Get the rounding parameters, if any.

    my @r = @_;

    # If called as a class method, initialize a new object.

    $self = bless {}, $class unless $selfref;

    $self -> {sign} = $sign;
    $self -> {_m}   = $LIB -> _one();
    $self -> {_es}  = '+';
    $self -> {_e}   = $LIB -> _zero();

    # If rounding parameters are given as arguments, use them. If no rounding
    # parameters are given, and if called as a class method initialize the new
    # instance with the class variables.

    #return $self -> round(@r);  # this should work, but doesnt; fixme!

    if (@r) {
        croak "can't specify both accuracy and precision"
          if @r >= 2 && defined($r[0]) && defined($r[1]);
        $self->{_a} = $_[0];
        $self->{_p} = $_[1];
    } else {
        unless($selfref) {
            $self->{_a} = $class -> accuracy();
            $self->{_p} = $class -> precision();
        }
    }

    return $self;
}

sub binf {
    # create/assign a '+inf' or '-inf'

    # Class::method(...) -> Class->method(...)
    unless (@_ && (defined(blessed($_[0])) && $_[0] -> isa(__PACKAGE__) ||
                   $_[0] =~ /^[a-z]\w*(?:::[a-z]\w*)*$/i))
    {
        #carp "Using ", (caller(0))[3], "() as a function is deprecated;",
        #  " use is as a method instead";
        unshift @_, __PACKAGE__;
    }

    my $self    = shift;
    my $selfref = ref $self;
    my $class   = $selfref || $self;

    {
        no strict 'refs';
        if (${"${class}::_trap_inf"}) {
            croak("Tried to create +-inf in $class->binf()");
        }
    }

    $self->import() if $IMPORT == 0;            # make require work

    # Don't modify constant (read-only) objects.

    return $self if $selfref && $self->modify('binf');

    return $downgrade -> binf(@_) if $downgrade;

    # Get the sign.

    my $sign = '+';     # default is to return positive infinity
    if (defined($_[0]) && $_[0] =~ /^\s*([+-])(inf|$)/i) {
        $sign = $1;
        shift;
    }

    # Get the rounding parameters, if any.

    my @r = @_;

    # If called as a class method, initialize a new object.

    $self = bless {}, $class unless $selfref;

    $self -> {sign} = $sign . 'inf';
    $self -> {_m}   = $LIB -> _zero();
    $self -> {_es}  = '+';
    $self -> {_e}   = $LIB -> _zero();

    # If rounding parameters are given as arguments, use them. If no rounding
    # parameters are given, and if called as a class method initialize the new
    # instance with the class variables.

    #return $self -> round(@r);  # this should work, but doesnt; fixme!

    if (@r) {
        croak "can't specify both accuracy and precision"
          if @r >= 2 && defined($r[0]) && defined($r[1]);
        $self->{_a} = $r[0];
        $self->{_p} = $r[1];
    } else {
        unless($selfref) {
            $self->{_a} = $class -> accuracy();
            $self->{_p} = $class -> precision();
        }
    }

    return $self;
}

sub bnan {
    # create/assign a 'NaN'

    # Class::method(...) -> Class->method(...)
    unless (@_ && (defined(blessed($_[0])) && $_[0] -> isa(__PACKAGE__) ||
                   $_[0] =~ /^[a-z]\w*(?:::[a-z]\w*)*$/i))
    {
        #carp "Using ", (caller(0))[3], "() as a function is deprecated;",
        #  " use is as a method instead";
        unshift @_, __PACKAGE__;
    }

    my $self    = shift;
    my $selfref = ref $self;
    my $class   = $selfref || $self;

    {
        no strict 'refs';
        if (${"${class}::_trap_nan"}) {
            croak("Tried to create NaN in $class->bnan()");
        }
    }

    $self->import() if $IMPORT == 0;            # make require work

    # Don't modify constant (read-only) objects.

    return $self if $selfref && $self->modify('bnan');

    return $downgrade -> bnan(@_) if defined $downgrade;

    # Get the rounding parameters, if any.

    my @r = @_;

    # If called as a class method, initialize a new object.

    $self = bless {}, $class unless $selfref;

    $self -> {sign} = $nan;
    $self -> {_m}   = $LIB -> _zero();
    $self -> {_es}  = '+';
    $self -> {_e}   = $LIB -> _zero();

    # If rounding parameters are given as arguments, use them. If no rounding
    # parameters are given, and if called as a class method initialize the new
    # instance with the class variables.

    #return $self -> round(@r);  # this should work, but doesnt; fixme!

    if (@r) {
        croak "can't specify both accuracy and precision"
          if @r >= 2 && defined($r[0]) && defined($r[1]);
        $self->{_a} = $r[0];
        $self->{_p} = $r[1];
    } else {
        unless($selfref) {
            $self->{_a} = $class -> accuracy();
            $self->{_p} = $class -> precision();
        }
    }

    return $self;
}

sub bpi {

    # Class::method(...) -> Class->method(...)
    unless (@_ && (defined(blessed($_[0])) && $_[0] -> isa(__PACKAGE__) ||
                   $_[0] =~ /^[a-z]\w*(?:::[a-z]\w*)*$/i))
    {
        #carp "Using ", (caller(0))[3], "() as a function is deprecated;",
        #  " use is as a method instead";
        unshift @_, __PACKAGE__;
    }

    # Called as                 Argument list
    # ---------                 -------------
    # Math::BigFloat->bpi()     ("Math::BigFloat")
    # Math::BigFloat->bpi(10)   ("Math::BigFloat", 10)
    # $x->bpi()                 ($x)
    # $x->bpi(10)               ($x, 10)
    # Math::BigFloat::bpi()     ()
    # Math::BigFloat::bpi(10)   (10)
    #
    # In ambiguous cases, we favour the OO-style, so the following case
    #
    #   $n = Math::BigFloat->new("10");
    #   $x = Math::BigFloat->bpi($n);
    #
    # which gives an argument list with the single element $n, is resolved as
    #
    #   $n->bpi();

    my $self    = shift;
    my $selfref = ref $self;
    my $class   = $selfref || $self;
    my @r       = @_;                   # rounding paramters

    if ($selfref) {                     # bpi() called as an instance method
        return $self if $self -> modify('bpi');
    } else {                            # bpi() called as a class method
        $self  = bless {}, $class;      # initialize new instance
    }

    ($self, @r) = $self -> _find_round_parameters(@r);

    # The accuracy, i.e., the number of digits. Pi has one digit before the
    # dot, so a precision of 4 digits is equivalent to an accuracy of 5 digits.

    my $n = defined $r[0] ? $r[0]
          : defined $r[1] ? 1 - $r[1]
          : $self -> div_scale();

    my $rmode = defined $r[2] ? $r[2] : $self -> round_mode();

    my $pi;

    if ($n <= 1000) {

        # 75 x 14 = 1050 digits

        my $all_digits = <<EOF;
314159265358979323846264338327950288419716939937510582097494459230781640628
620899862803482534211706798214808651328230664709384460955058223172535940812
848111745028410270193852110555964462294895493038196442881097566593344612847
564823378678316527120190914564856692346034861045432664821339360726024914127
372458700660631558817488152092096282925409171536436789259036001133053054882
046652138414695194151160943305727036575959195309218611738193261179310511854
807446237996274956735188575272489122793818301194912983367336244065664308602
139494639522473719070217986094370277053921717629317675238467481846766940513
200056812714526356082778577134275778960917363717872146844090122495343014654
958537105079227968925892354201995611212902196086403441815981362977477130996
051870721134999999837297804995105973173281609631859502445945534690830264252
230825334468503526193118817101000313783875288658753320838142061717766914730
359825349042875546873115956286388235378759375195778185778053217122680661300
192787661119590921642019893809525720106548586327886593615338182796823030195
EOF

        # Should we round up?

        my $round_up;

        # From the string above, we need to extract the number of digits we
        # want plus extra characters for the newlines.

        my $nchrs = $n + int($n / 75);

        # Extract the digits we want.

        my $digits = substr($all_digits, 0, $nchrs);

        # Find out whether we should round up or down. Rounding is easy, since
        # pi is trancendental. With directed rounding, it doesn't matter what
        # the following digits are. With rounding to nearest, we only have to
        # look at one extra digit.

        if ($rmode eq 'trunc') {
            $round_up = 0;
        } else {
            my $next_digit = substr($all_digits, $nchrs, 1);
            $round_up = $next_digit lt '5' ? 0 : 1;
        }

        # Remove the newlines.

        $digits =~ tr/0-9//cd;

        # Now do the rounding. We could easily make the regex substitution
        # handle all cases, but we avoid using the regex engine when it is
        # simple to avoid it.

        if ($round_up) {
            my $last_digit = substr($digits, -1, 1);
            if ($last_digit lt '9') {
                substr($digits, -1, 1) = ++$last_digit;
            } else {
                $digits =~ s{([0-8])(9+)$}
                            { ($1 + 1) . ("0" x CORE::length($2)) }e;
            }
        }

        # Convert to an object.

        $pi = bless {
                     sign => '+',
                     _m   => $LIB -> _new($digits),
                     _es  => '-',
                     _e   => $LIB -> _new($n - 1),
                    }, $class;

    } else {

        # For large accuracy, the arctan formulas become very inefficient with
        # Math::BigFloat, so use Brent-Salamin (aka AGM or Gauss-Legendre).

        # Use a few more digits in the intermediate computations.
        $n += 8;

        $HALF = $class -> new($HALF) unless ref($HALF);
        my ($an, $bn, $tn, $pn)
          = ($class -> bone, $HALF -> copy() -> bsqrt($n),
             $HALF -> copy() -> bmul($HALF), $class -> bone);
        while ($pn < $n) {
            my $prev_an = $an -> copy();
            $an = $an -> badd($bn) -> bmul($HALF, $n);
            $bn = $bn -> bmul($prev_an) -> bsqrt($n);
            $prev_an = $prev_an -> bsub($an);
            $tn = $tn -> bsub($pn * $prev_an * $prev_an);
            $pn = $pn -> badd($pn);
        }
        $an = $an -> badd($bn);
        $an = $an -> bmul($an, $n) -> bdiv(4 * $tn, $n);

        $an = $an -> round(@r);
        $pi = $an;
    }

    if (defined $r[0]) {
        $pi -> accuracy($r[0]);
    } elsif (defined $r[1]) {
        $pi -> precision($r[1]);
    }

    for my $key (qw/ sign _m _es _e _a _p /) {
        $self -> {$key} = $pi -> {$key};
    }

    return $downgrade -> new($self -> bdstr(), @r)
      if defined($downgrade) && $self->is_int();
    return $self;
}

sub copy {
    my ($x, $class);
    if (ref($_[0])) {           # $y = $x -> copy()
        $x = shift;
        $class = ref($x);
    } else {                    # $y = Math::BigInt -> copy($y)
        $class = shift;
        $x = shift;
    }

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @_;

    my $copy = bless {}, $class;

    $copy->{sign} = $x->{sign};
    $copy->{_es}  = $x->{_es};
    $copy->{_m}   = $LIB->_copy($x->{_m});
    $copy->{_e}   = $LIB->_copy($x->{_e});
    $copy->{_a}   = $x->{_a} if exists $x->{_a};
    $copy->{_p}   = $x->{_p} if exists $x->{_p};

    return $copy;
}

sub as_int {
    # return copy as a bigint representation of this Math::BigFloat number
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);
    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    return $x -> copy() if $x -> isa("Math::BigInt");

    # disable upgrading and downgrading

    require Math::BigInt;
    my $upg = Math::BigInt -> upgrade();
    my $dng = Math::BigInt -> downgrade();
    Math::BigInt -> upgrade(undef);
    Math::BigInt -> downgrade(undef);

    my $y;
    if ($x -> is_inf()) {
        $y = Math::BigInt -> binf($x->sign());
    } elsif ($x -> is_nan()) {
        $y = Math::BigInt -> bnan();
    } else {
        $y = $LIB->_copy($x->{_m});
        if ($x->{_es} eq '-') {                     # < 0
            $y = $LIB->_rsft($y, $x->{_e}, 10);
        } elsif (! $LIB->_is_zero($x->{_e})) {      # > 0
            $y = $LIB->_lsft($y, $x->{_e}, 10);
        }
        $y = Math::BigInt->new($x->{sign} . $LIB->_str($y));
    }

    # reset upgrading and downgrading

    Math::BigInt -> upgrade($upg);
    Math::BigInt -> downgrade($dng);

    return $y;
}

sub as_float {
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);
    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    return $x -> copy() if $x -> isa("Math::BigFloat");

    # disable upgrading and downgrading

    require Math::BigFloat;
    my $upg = Math::BigFloat -> upgrade();
    my $dng = Math::BigFloat -> downgrade();
    Math::BigFloat -> upgrade(undef);
    Math::BigFloat -> downgrade(undef);

    my $y = Math::BigFloat -> new($x);

    # reset upgrading and downgrading

    Math::BigFloat -> upgrade($upg);
    Math::BigFloat -> downgrade($dng);

    return $y;
}

###############################################################################
# Boolean methods
###############################################################################

sub is_zero {
    # return true if arg (BFLOAT or num_str) is zero
    my (undef, $x) = ref($_[0]) ? (undef, @_) : objectify(1, @_);

    ($x->{sign} eq '+' && $LIB->_is_zero($x->{_m})) ? 1 : 0;
}

sub is_one {
    # return true if arg (BFLOAT or num_str) is +1 or -1 if signis given
    my (undef, $x, $sign) = ref($_[0]) ? (undef, @_) : objectify(1, @_);

    $sign = '+' if !defined $sign || $sign ne '-';

    ($x->{sign} eq $sign &&
     $LIB->_is_zero($x->{_e}) &&
     $LIB->_is_one($x->{_m})) ? 1 : 0;
}

sub is_odd {
    # return true if arg (BFLOAT or num_str) is odd or false if even
    my (undef, $x) = ref($_[0]) ? (undef, @_) : objectify(1, @_);

    (($x->{sign} =~ /^[+-]$/) && # NaN & +-inf aren't
     ($LIB->_is_zero($x->{_e})) &&
     ($LIB->_is_odd($x->{_m}))) ? 1 : 0;
}

sub is_even {
    # return true if arg (BINT or num_str) is even or false if odd
    my (undef, $x) = ref($_[0]) ? (undef, @_) : objectify(1, @_);

    (($x->{sign} =~ /^[+-]$/) &&        # NaN & +-inf aren't
     ($x->{_es} eq '+') &&              # 123.45 isn't
     ($LIB->_is_even($x->{_m}))) ? 1 : 0; # but 1200 is
}

sub is_int {
    # return true if arg (BFLOAT or num_str) is an integer
    my (undef, $x) = ref($_[0]) ? (undef, @_) : objectify(1, @_);

    (($x->{sign} =~ /^[+-]$/) && # NaN and +-inf aren't
     ($x->{_es} eq '+')) ? 1 : 0; # 1e-1 => no integer
}

###############################################################################
# Comparison methods
###############################################################################

sub bcmp {
    # Compares 2 values.  Returns one of undef, <0, =0, >0. (suitable for sort)

    # set up parameters
    my ($class, $x, $y, @r) = ref($_[0]) && ref($_[0]) eq ref($_[1])
                            ? (ref($_[0]), @_)
                            : objectify(2, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    # Handle all 'nan' cases.

    return    if ($x->{sign} eq $nan) || ($y->{sign} eq $nan);

    # Handle all '+inf' and '-inf' cases.

    return  0 if ($x->{sign} eq '+inf' && $y->{sign} eq '+inf' ||
                  $x->{sign} eq '-inf' && $y->{sign} eq '-inf');
    return +1 if $x->{sign} eq '+inf'; # x = +inf and y < +inf
    return -1 if $x->{sign} eq '-inf'; # x = -inf and y > -inf
    return -1 if $y->{sign} eq '+inf'; # x < +inf and y = +inf
    return +1 if $y->{sign} eq '-inf'; # x > -inf and y = -inf

    # Handle all cases with opposite signs.

    return +1 if $x->{sign} eq '+' && $y->{sign} eq '-'; # also does 0 <=> -y
    return -1 if $x->{sign} eq '-' && $y->{sign} eq '+'; # also does -x <=> 0

    # Handle all remaining zero cases.

    my $xz = $x->is_zero();
    my $yz = $y->is_zero();
    return  0 if $xz && $yz;             # 0 <=> 0
    return -1 if $xz && $y->{sign} eq '+'; # 0 <=> +y
    return +1 if $yz && $x->{sign} eq '+'; # +x <=> 0

    # Both arguments are now finite, non-zero numbers with the same sign.

    my $cmp;

    # The next step is to compare the exponents, but since each mantissa is an
    # integer of arbitrary value, the exponents must be normalized by the length
    # of the mantissas before we can compare them.

    my $mxl = $LIB->_len($x->{_m});
    my $myl = $LIB->_len($y->{_m});

    # If the mantissas have the same length, there is no point in normalizing
    # the exponents by the length of the mantissas, so treat that as a special
    # case.

    if ($mxl == $myl) {

        # First handle the two cases where the exponents have different signs.

        if ($x->{_es} eq '+' && $y->{_es} eq '-') {
            $cmp = +1;
        } elsif ($x->{_es} eq '-' && $y->{_es} eq '+') {
            $cmp = -1;
        }

        # Then handle the case where the exponents have the same sign.

        else {
            $cmp = $LIB->_acmp($x->{_e}, $y->{_e});
            $cmp = -$cmp if $x->{_es} eq '-';
        }

        # Adjust for the sign, which is the same for x and y, and bail out if
        # we're done.

        $cmp = -$cmp if $x->{sign} eq '-'; # 124 > 123, but -124 < -123
        return $cmp if $cmp;

    }

    # We must normalize each exponent by the length of the corresponding
    # mantissa. Life is a lot easier if we first make both exponents
    # non-negative. We do this by adding the same positive value to both
    # exponent. This is safe, because when comparing the exponents, only the
    # relative difference is important.

    my $ex;
    my $ey;

    if ($x->{_es} eq '+') {

        # If the exponent of x is >= 0 and the exponent of y is >= 0, there is
        # no need to do anything special.

        if ($y->{_es} eq '+') {
            $ex = $LIB->_copy($x->{_e});
            $ey = $LIB->_copy($y->{_e});
        }

        # If the exponent of x is >= 0 and the exponent of y is < 0, add the
        # absolute value of the exponent of y to both.

        else {
            $ex = $LIB->_copy($x->{_e});
            $ex = $LIB->_add($ex, $y->{_e}); # ex + |ey|
            $ey = $LIB->_zero();             # -ex + |ey| = 0
        }

    } else {

        # If the exponent of x is < 0 and the exponent of y is >= 0, add the
        # absolute value of the exponent of x to both.

        if ($y->{_es} eq '+') {
            $ex = $LIB->_zero(); # -ex + |ex| = 0
            $ey = $LIB->_copy($y->{_e});
            $ey = $LIB->_add($ey, $x->{_e}); # ey + |ex|
        }

        # If the exponent of x is < 0 and the exponent of y is < 0, add the
        # absolute values of both exponents to both exponents.

        else {
            $ex = $LIB->_copy($y->{_e}); # -ex + |ey| + |ex| = |ey|
            $ey = $LIB->_copy($x->{_e}); # -ey + |ex| + |ey| = |ex|
        }

    }

    # Now we can normalize the exponents by adding lengths of the mantissas.

    $ex = $LIB->_add($ex, $LIB->_new($mxl));
    $ey = $LIB->_add($ey, $LIB->_new($myl));

    # We're done if the exponents are different.

    $cmp = $LIB->_acmp($ex, $ey);
    $cmp = -$cmp if $x->{sign} eq '-'; # 124 > 123, but -124 < -123
    return $cmp if $cmp;

    # Compare the mantissas, but first normalize them by padding the shorter
    # mantissa with zeros (shift left) until it has the same length as the
    # longer mantissa.

    my $mx = $x->{_m};
    my $my = $y->{_m};

    if ($mxl > $myl) {
        $my = $LIB->_lsft($LIB->_copy($my), $LIB->_new($mxl - $myl), 10);
    } elsif ($mxl < $myl) {
        $mx = $LIB->_lsft($LIB->_copy($mx), $LIB->_new($myl - $mxl), 10);
    }

    $cmp = $LIB->_acmp($mx, $my);
    $cmp = -$cmp if $x->{sign} eq '-'; # 124 > 123, but -124 < -123
    return $cmp;

}

sub bacmp {
    # Compares 2 values, ignoring their signs.
    # Returns one of undef, <0, =0, >0. (suitable for sort)

    # set up parameters
    my ($class, $x, $y, @r) = ref($_[0]) && ref($_[0]) eq ref($_[1])
                            ? (ref($_[0]), @_)
                            : objectify(2, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    # handle +-inf and NaN's
    if ($x->{sign} !~ /^[+-]$/ || $y->{sign} !~ /^[+-]$/) {
        return    if (($x->{sign} eq $nan) || ($y->{sign} eq $nan));
        return  0 if ($x->is_inf() && $y->is_inf());
        return  1 if ($x->is_inf() && !$y->is_inf());
        return -1;
    }

    # shortcut
    my $xz = $x->is_zero();
    my $yz = $y->is_zero();
    return 0 if $xz && $yz;     # 0 <=> 0
    return -1 if $xz && !$yz;   # 0 <=> +y
    return 1 if $yz && !$xz;    # +x <=> 0

    # adjust so that exponents are equal
    my $lxm = $LIB->_len($x->{_m});
    my $lym = $LIB->_len($y->{_m});
    my ($xes, $yes) = (1, 1);
    $xes = -1 if $x->{_es} ne '+';
    $yes = -1 if $y->{_es} ne '+';
    # the numify somewhat limits our length, but makes it much faster
    my $lx = $lxm + $xes * $LIB->_num($x->{_e});
    my $ly = $lym + $yes * $LIB->_num($y->{_e});
    my $l = $lx - $ly;
    return $l <=> 0 if $l != 0;

    # lengths (corrected by exponent) are equal
    # so make mantissa equal-length by padding with zero (shift left)
    my $diff = $lxm - $lym;
    my $xm = $x->{_m};          # not yet copy it
    my $ym = $y->{_m};
    if ($diff > 0) {
        $ym = $LIB->_copy($y->{_m});
        $ym = $LIB->_lsft($ym, $LIB->_new($diff), 10);
    } elsif ($diff < 0) {
        $xm = $LIB->_copy($x->{_m});
        $xm = $LIB->_lsft($xm, $LIB->_new(-$diff), 10);
    }
    $LIB->_acmp($xm, $ym);
}

###############################################################################
# Arithmetic methods
###############################################################################

sub bneg {
    # (BINT or num_str) return BINT
    # negate number or make a negated number from string
    my (undef, $x, @r) = ref($_[0]) ? (undef, @_) : objectify(1, @_);

    return $x if $x->modify('bneg');

    return $x -> bnan(@r) if $x -> is_nan();

    # For +0 do not negate (to have always normalized +0).
    $x->{sign} =~ tr/+-/-+/
      unless $x->{sign} eq '+' && $LIB->_is_zero($x->{_m});

    return $downgrade -> new($x -> bdstr(), @r) if defined($downgrade)
      && ($x -> is_int() || $x -> is_inf() || $x -> is_nan());
    return $x -> round(@r);
}

sub bnorm {
    # bnorm() can't support rounding, because bround() and bfround() call
    # bnorm(), which would recurse indefinitely.

    # adjust m and e so that m is smallest possible
    my (undef, $x, @r) = ref($_[0]) ? (undef, @_) : objectify(1, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    # inf, nan etc
    if ($x->{sign} !~ /^[+-]$/) {
        return $downgrade -> new($x) if defined $downgrade;
        return $x;
    }

    my $zeros = $LIB->_zeros($x->{_m}); # correct for trailing zeros
    if ($zeros != 0) {
        my $z = $LIB->_new($zeros);
        $x->{_m} = $LIB->_rsft($x->{_m}, $z, 10);
        if ($x->{_es} eq '-') {
            if ($LIB->_acmp($x->{_e}, $z) >= 0) {
                $x->{_e} = $LIB->_sub($x->{_e}, $z);
                $x->{_es} = '+' if $LIB->_is_zero($x->{_e});
            } else {
                $x->{_e} = $LIB->_sub($LIB->_copy($z), $x->{_e});
                $x->{_es} = '+';
            }
        } else {
            $x->{_e} = $LIB->_add($x->{_e}, $z);
        }
    } else {
        # $x can only be 0Ey if there are no trailing zeros ('0' has 0 trailing
        # zeros). So, for something like 0Ey, set y to 0, and -0 => +0
        if ($LIB->_is_zero($x->{_m})) {
            $x->{sign} = '+';
            $x->{_es}  = '+';
            $x->{_e}   = $LIB->_zero();
        }
    }

    return $downgrade -> new($x)
      if defined($downgrade) && $x->is_int();
    return $x;
}

sub binc {
    # increment arg by one
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    return $x if $x->modify('binc');

    # Inf and NaN

    return $x -> bnan(@r)             if $x -> is_nan();
    return $x -> binf($x->{sign}, @r) if $x -> is_inf();

    # Non-integer

    if ($x->{_es} eq '-') {
        return $x->badd($class->bone(), @r);
    }

    # If the exponent is non-zero, convert the internal representation, so that,
    # e.g., 12e+3 becomes 12000e+0 and we can easily increment the mantissa.

    if (!$LIB->_is_zero($x->{_e})) {
        $x->{_m} = $LIB->_lsft($x->{_m}, $x->{_e}, 10); # 1e2 => 100
        $x->{_e} = $LIB->_zero();                       # normalize
        $x->{_es} = '+';
        # we know that the last digit of $x will be '1' or '9', depending on the
        # sign
    }

    # now $x->{_e} == 0
    if ($x->{sign} eq '+') {
        $x->{_m} = $LIB->_inc($x->{_m});
        return $x->bnorm()->bround(@r);
    } elsif ($x->{sign} eq '-') {
        $x->{_m} = $LIB->_dec($x->{_m});
        $x->{sign} = '+' if $LIB->_is_zero($x->{_m}); # -1 +1 => -0 => +0
        return $x->bnorm()->bround(@r);
    }

    return $downgrade -> new($x -> bdstr(), @r)
      if defined($downgrade) && $x -> is_int();
    return $x;
}

sub bdec {
    # decrement arg by one
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    return $x if $x->modify('bdec');

    # Inf and NaN

    return $x -> bnan(@r)             if $x -> is_nan();
    return $x -> binf($x->{sign}, @r) if $x -> is_inf();

    # Non-integer

    if ($x->{_es} eq '-') {
        return $x->badd($class->bone('-'), @r);
    }

    # If the exponent is non-zero, convert the internal representation, so that,
    # e.g., 12e+3 becomes 12000e+0 and we can easily increment the mantissa.

    if (!$LIB->_is_zero($x->{_e})) {
        $x->{_m} = $LIB->_lsft($x->{_m}, $x->{_e}, 10); # 1e2 => 100
        $x->{_e} = $LIB->_zero();                       # normalize
        $x->{_es} = '+';
    }

    # now $x->{_e} == 0
    my $zero = $x->is_zero();
    if (($x->{sign} eq '-') || $zero) {           # x <= 0
        $x->{_m} = $LIB->_inc($x->{_m});
        $x->{sign} = '-' if $zero;                # 0 => 1 => -1
        $x->{sign} = '+' if $LIB->_is_zero($x->{_m}); # -1 +1 => -0 => +0
        return $x->bnorm()->round(@r);
    }
    elsif ($x->{sign} eq '+') {                   # x > 0
        $x->{_m} = $LIB->_dec($x->{_m});
        return $x->bnorm()->round(@r);
    }

    return $downgrade -> new($x -> bdstr(), @r)
      if defined($downgrade) && $x -> is_int();
    return $x -> round(@r);
}

sub badd {
    # set up parameters
    my ($class, $x, $y, @r) = ref($_[0]) && ref($_[0]) eq ref($_[1])
                            ? (ref($_[0]), @_)
                            : objectify(2, @_);

    return $x if $x->modify('badd');

    # inf and NaN handling
    if ($x->{sign} !~ /^[+-]$/ || $y->{sign} !~ /^[+-]$/) {

        # $x is NaN and/or $y is NaN
        if ($x->{sign} eq $nan || $y->{sign} eq $nan) {
            $x = $x->bnan();
        }

        # $x is Inf and $y is Inf
        elsif ($x->{sign} =~ /^[+-]inf$/ && $y->{sign} =~ /^[+-]inf$/) {
            # +Inf + +Inf or -Inf + -Inf => same, rest is NaN
            $x = $x->bnan() if $x->{sign} ne $y->{sign};
        }

        # +-inf + something => +-inf; something +-inf => +-inf
        elsif ($y->{sign} =~ /^[+-]inf$/) {
            $x->{sign} = $y->{sign};
        }

        return $downgrade -> new($x -> bdstr(), @r) if defined $downgrade;
        return $x -> round(@r);
    }

    return $upgrade->badd($x, $y, @r) if defined $upgrade;

    $r[3] = $y;                 # no push!

    # for speed: no add for $x + 0
    if ($y->is_zero()) {
        $x = $x->round(@r);
    }

    # for speed: no add for 0 + $y
    elsif ($x->is_zero()) {
        # make copy, clobbering up x (modify in place!)
        $x->{_e} = $LIB->_copy($y->{_e});
        $x->{_es} = $y->{_es};
        $x->{_m} = $LIB->_copy($y->{_m});
        $x->{sign} = $y->{sign} || $nan;
        $x = $x->round(@r);
    }

    # both $x and $y are non-zero
    else {

        # take lower of the two e's and adapt m1 to it to match m2
        my $e = $y->{_e};
        $e = $LIB->_zero() if !defined $e; # if no BFLOAT?
        $e = $LIB->_copy($e);              # make copy (didn't do it yet)

        my $es;

        ($e, $es) = $LIB -> _ssub($e, $y->{_es} || '+', $x->{_e}, $x->{_es});

        my $add = $LIB->_copy($y->{_m});

        if ($es eq '-') {                       # < 0
            $x->{_m} = $LIB->_lsft($x->{_m}, $e, 10);
            ($x->{_e}, $x->{_es}) = $LIB -> _sadd($x->{_e}, $x->{_es}, $e, $es);
        } elsif (!$LIB->_is_zero($e)) {         # > 0
            $add = $LIB->_lsft($add, $e, 10);
        }

        # else: both e are the same, so just leave them

        if ($x->{sign} eq $y->{sign}) {
            $x->{_m} = $LIB->_add($x->{_m}, $add);
        } else {
            ($x->{_m}, $x->{sign}) =
              $LIB -> _sadd($x->{_m}, $x->{sign}, $add, $y->{sign});
        }

        # delete trailing zeros, then round
        $x = $x->bnorm()->round(@r);
    }

    return $downgrade -> new($x -> bdstr(), @r)
      if defined($downgrade) && $x -> is_int();
    return $x;          # rounding already done above
}

sub bsub {
    # set up parameters
    my ($class, $x, $y, @r) = ref($_[0]) && ref($_[0]) eq ref($_[1])
                            ? (ref($_[0]), @_)
                            : objectify(2, @_);

    return $x if $x -> modify('bsub');

    if ($y -> is_zero()) {
        $x = $x -> round(@r);
    } else {

        # To correctly handle the special case $x -> bsub($x), we note the sign
        # of $x, then flip the sign of $y, and if the sign of $x changed too,
        # then we know that $x and $y are the same object.

        my $xsign = $x -> {sign};
        $y -> {sign} =~ tr/+-/-+/;      # does nothing for NaN
        if ($xsign ne $x -> {sign}) {
            # special case of $x -> bsub($x) results in 0
            if ($xsign =~ /^[+-]$/) {
                $x = $x -> bzero(@r);
            } else {
                $x = $x -> bnan();      # NaN, -inf, +inf
            }
            return $downgrade -> new($x -> bdstr(), @r) if defined $downgrade;
            return $x -> round(@r);
        }
        $x = $x -> badd($y, @r);        # badd does not leave internal zeros
        $y -> {sign} =~ tr/+-/-+/;      # reset $y (does nothing for NaN)
    }

    return $downgrade -> new($x -> bdstr(), @r)
      if defined($downgrade) && ($x->is_int() || $x->is_inf() || $x->is_nan());
    $x;                         # already rounded by badd() or no rounding
}

sub bmul {
    # multiply two numbers

    # set up parameters
    my ($class, $x, $y, @r) = ref($_[0]) && ref($_[0]) eq ref($_[1])
                            ? (ref($_[0]), @_)
                            : objectify(2, @_);

    return $x if $x->modify('bmul');

    return $x->bnan(@r) if ($x->{sign} eq $nan) || ($y->{sign} eq $nan);

    # inf handling
    if (($x->{sign} =~ /^[+-]inf$/) || ($y->{sign} =~ /^[+-]inf$/)) {
        return $x->bnan(@r) if $x->is_zero() || $y->is_zero();
        # result will always be +-inf:
        # +inf * +/+inf => +inf, -inf * -/-inf => +inf
        # +inf * -/-inf => -inf, -inf * +/+inf => -inf
        return $x->binf(@r) if ($x->{sign} =~ /^\+/ && $y->{sign} =~ /^\+/);
        return $x->binf(@r) if ($x->{sign} =~ /^-/ && $y->{sign} =~ /^-/);
        return $x->binf('-', @r);
    }

    return $upgrade->bmul($x, $y, @r) if defined $upgrade;

    # aEb * cEd = (a*c)E(b+d)
    $x->{_m} = $LIB->_mul($x->{_m}, $y->{_m});
    ($x->{_e}, $x->{_es})
      = $LIB -> _sadd($x->{_e}, $x->{_es}, $y->{_e}, $y->{_es});

    $r[3] = $y;                 # no push!

    # adjust sign:
    $x->{sign} = $x->{sign} ne $y->{sign} ? '-' : '+';
    $x = $x->bnorm->round(@r);

    return $downgrade -> new($x -> bdstr(), @r)
      if defined($downgrade) && ($x->is_int() || $x->is_inf() || $x->is_nan());
    return $x;
}

sub bmuladd {
    # multiply two numbers and add the third to the result

    # set up parameters
    my ($class, $x, $y, $z, @r)
      = ref($_[0]) && ref($_[0]) eq ref($_[1]) && ref($_[1]) eq ref($_[2])
      ? (ref($_[0]), @_)
      : objectify(3, @_);

    return $x if $x->modify('bmuladd');

    return $x->bnan(@r) if (($x->{sign} eq $nan) ||
                            ($y->{sign} eq $nan) ||
                            ($z->{sign} eq $nan));

    # inf handling
    if (($x->{sign} =~ /^[+-]inf$/) || ($y->{sign} =~ /^[+-]inf$/)) {
        return $x->bnan(@r) if $x->is_zero() || $y->is_zero();
        # result will always be +-inf:
        # +inf * +/+inf => +inf, -inf * -/-inf => +inf
        # +inf * -/-inf => -inf, -inf * +/+inf => -inf
        return $x->binf(@r) if ($x->{sign} =~ /^\+/ && $y->{sign} =~ /^\+/);
        return $x->binf(@r) if ($x->{sign} =~ /^-/ && $y->{sign} =~ /^-/);
        return $x->binf('-', @r);
    }

    # aEb * cEd = (a*c)E(b+d)
    $x->{_m} = $LIB->_mul($x->{_m}, $y->{_m});
    ($x->{_e}, $x->{_es})
      = $LIB -> _sadd($x->{_e}, $x->{_es}, $y->{_e}, $y->{_es});

    $r[3] = $y;                 # no push!

    # adjust sign:
    $x->{sign} = $x->{sign} ne $y->{sign} ? '-' : '+';

    # z=inf handling (z=NaN handled above)
    if ($z->{sign} =~ /^[+-]inf$/) {
        $x->{sign} = $z->{sign};
        return $downgrade -> new($x -> bdstr(), @r) if defined $downgrade;
        return $x -> round(@r);
    }

    # take lower of the two e's and adapt m1 to it to match m2
    my $e = $z->{_e};
    $e = $LIB->_zero() if !defined $e; # if no BFLOAT?
    $e = $LIB->_copy($e);              # make copy (didn't do it yet)

    my $es;

    ($e, $es) = $LIB -> _ssub($e, $z->{_es} || '+', $x->{_e}, $x->{_es});

    my $add = $LIB->_copy($z->{_m});

    if ($es eq '-')             # < 0
    {
        $x->{_m} = $LIB->_lsft($x->{_m}, $e, 10);
        ($x->{_e}, $x->{_es}) = $LIB -> _sadd($x->{_e}, $x->{_es}, $e, $es);
    } elsif (!$LIB->_is_zero($e)) # > 0
    {
        $add = $LIB->_lsft($add, $e, 10);
    }
    # else: both e are the same, so just leave them

    if ($x->{sign} eq $z->{sign}) {
        # add
        $x->{_m} = $LIB->_add($x->{_m}, $add);
    } else {
        ($x->{_m}, $x->{sign}) =
          $LIB -> _sadd($x->{_m}, $x->{sign}, $add, $z->{sign});
    }

    # delete trailing zeros, then round
    $x = $x->bnorm()->round(@r);

    return $downgrade -> new($x -> bdstr(), @r)
      if defined($downgrade) && ($x->is_int() || $x->is_inf() || $x->is_nan());
    return $x;
}

sub bdiv {
    # (dividend: BFLOAT or num_str, divisor: BFLOAT or num_str) return
    # (BFLOAT, BFLOAT) (quo, rem) or BFLOAT (only quo)

    # set up parameters
    my ($class, $x, $y, @r) = (ref($_[0]), @_);
    # objectify is costly, so avoid it
    if ((!ref($_[0])) || (ref($_[0]) ne ref($_[1]))) {
        ($class, $x, $y, @r) = objectify(2, @_);
    }

    return $x if $x->modify('bdiv');

    my $wantarray = wantarray;  # call only once

    # At least one argument is NaN. This is handled the same way as in
    # Math::BigInt -> bdiv().

    if ($x -> is_nan() || $y -> is_nan()) {
        return $wantarray ? ($x -> bnan(@r), $class -> bnan(@r))
                          : $x -> bnan(@r);
    }

    # Divide by zero and modulo zero. This is handled the same way as in
    # Math::BigInt -> bdiv(). See the comment in the code for Math::BigInt ->
    # bdiv() for further details.

    if ($y -> is_zero()) {
        my ($quo, $rem);
        if ($wantarray) {
            $rem = $x -> copy() -> round(@r);
            $rem = $downgrade -> new($rem, @r)
              if defined($downgrade) && $rem -> is_int();
        }
        if ($x -> is_zero()) {
            $quo = $x -> bnan(@r);
        } else {
            $quo = $x -> binf($x -> {sign}, @r);
        }
        return $wantarray ? ($quo, $rem) : $quo;
    }

    # Numerator (dividend) is +/-inf. This is handled the same way as in
    # Math::BigInt -> bdiv(). See the comment in the code for Math::BigInt ->
    # bdiv() for further details.

    if ($x -> is_inf()) {
        my ($quo, $rem);
        $rem = $class -> bnan(@r) if $wantarray;
        if ($y -> is_inf()) {
            $quo = $x -> bnan(@r);
        } else {
            my $sign = $x -> bcmp(0) == $y -> bcmp(0) ? '+' : '-';
            $quo = $x -> binf($sign, @r);
        }
        return $wantarray ? ($quo, $rem) : $quo;
    }

    # Denominator (divisor) is +/-inf. This is handled the same way as in
    # Math::BigInt -> bdiv(), with one exception: In scalar context,
    # Math::BigFloat does true division (although rounded), not floored division
    # (F-division), so a finite number divided by +/-inf is always zero. See the
    # comment in the code for Math::BigInt -> bdiv() for further details.

    if ($y -> is_inf()) {
        my ($quo, $rem);
        if ($wantarray) {
            if ($x -> is_zero() || $x -> bcmp(0) == $y -> bcmp(0)) {
                $rem = $x -> copy() -> round(@r);
                $rem = $downgrade -> new($rem, @r)
                  if defined($downgrade) && $rem -> is_int();
                $quo = $x -> bzero(@r);
            } else {
                $rem = $class -> binf($y -> {sign}, @r);
                $quo = $x -> bone('-', @r);
            }
            return ($quo, $rem);
        } else {
            if ($y -> is_inf()) {
                if ($x -> is_nan() || $x -> is_inf()) {
                    return $x -> bnan(@r);
                } else {
                    return $x -> bzero(@r);
                }
            }
        }
    }

    # At this point, both the numerator and denominator are finite numbers, and
    # the denominator (divisor) is non-zero.

    # x == 0?
    if ($x->is_zero()) {
        my ($quo, $rem);
        $quo = $x->round(@r);
        $quo = $downgrade -> new($quo, @r)
          if defined($downgrade) && $quo -> is_int();
        if ($wantarray) {
            $rem = $class -> bzero(@r);
            return $quo, $rem;
        }
        return $quo;
    }

    # Division might return a value that we can not represent exactly, so
    # upgrade, if upgrading is enabled.

    return $upgrade -> bdiv($x, $y, @r)
      if defined($upgrade) && !wantarray && !$LIB -> _is_one($y -> {_m});

    # we need to limit the accuracy to protect against overflow
    my $fallback = 0;
    my (@params, $scale);
    ($x, @params) = $x->_find_round_parameters($r[0], $r[1], $r[2], $y);

    return $x -> round(@r) if $x->is_nan();  # error in _find_round_parameters?

    # no rounding at all, so must use fallback
    if (scalar @params == 0) {
        # simulate old behaviour
        $params[0] = $class->div_scale(); # and round to it as accuracy
        $scale = $params[0]+4;            # at least four more for proper round
        $params[2] = $r[2];               # round mode by caller or undef
        $fallback = 1;                    # to clear a/p afterwards
    } else {
        # the 4 below is empirical, and there might be cases where it is not
        # enough...
        $scale = abs($params[0] || $params[1]) + 4; # take whatever is defined
    }

    my $rem;
    $rem = $class -> bzero() if wantarray;

    $y = $class->new($y) unless $y->isa('Math::BigFloat');

    my $lx = $LIB -> _len($x->{_m});
    my $ly = $LIB -> _len($y->{_m});
    $scale = $lx if $lx > $scale;
    $scale = $ly if $ly > $scale;
    my $diff = $ly - $lx;
    $scale += $diff if $diff > 0; # if lx << ly, but not if ly << lx!

    # check that $y is not 1 nor -1 and cache the result:
    my $y_not_one = !($LIB->_is_zero($y->{_e}) && $LIB->_is_one($y->{_m}));

    # flipping the sign of $y will also flip the sign of $x for the special
    # case of $x->bsub($x); so we can catch it below:
    my $xsign = $x->{sign};
    $y->{sign} =~ tr/+-/-+/;

    if ($xsign ne $x->{sign}) {
        # special case of $x /= $x results in 1
        $x = $x->bone();        # "fixes" also sign of $y, since $x is $y
    } else {
        # correct $y's sign again
        $y->{sign} =~ tr/+-/-+/;
        # continue with normal div code:

        # make copy of $x in case of list context for later remainder
        # calculation
        if (wantarray && $y_not_one) {
            $rem = $x->copy();
        }

        $x->{sign} = $x->{sign} ne $y->sign() ? '-' : '+';

        # check for / +-1 (+/- 1E0)
        if ($y_not_one) {
            # promote Math::BigInt and its subclasses (except when already a
            # Math::BigFloat)
            $y = $class->new($y) unless $y->isa('Math::BigFloat');

            # calculate the result to $scale digits and then round it
            # a * 10 ** b / c * 10 ** d => a/c * 10 ** (b-d)
            $x->{_m} = $LIB->_lsft($x->{_m}, $LIB->_new($scale), 10);
            $x->{_m} = $LIB->_div($x->{_m}, $y->{_m}); # a/c

            # correct exponent of $x
            ($x->{_e}, $x->{_es})
              = $LIB -> _ssub($x->{_e}, $x->{_es}, $y->{_e}, $y->{_es});
            # correct for 10**scale
            ($x->{_e}, $x->{_es})
              = $LIB -> _ssub($x->{_e}, $x->{_es}, $LIB->_new($scale), '+');
            $x = $x->bnorm();   # remove trailing 0's
        }
    }                           # end else $x != $y

    # shortcut to not run through _find_round_parameters again
    if (defined $params[0]) {
        delete $x->{_a};               # clear before round
        $x = $x->bround($params[0], $params[2]); # then round accordingly
    } else {
        delete $x->{_p};                # clear before round
        $x = $x->bfround($params[1], $params[2]); # then round accordingly
    }
    if ($fallback) {
        # clear a/p after round, since user did not request it
        delete $x->{_a};
        delete $x->{_p};
    }

    if (wantarray) {
        if ($y_not_one) {
            $x = $x -> bfloor();
            $rem = $rem->bmod($y, @params); # copy already done
        }
        if ($fallback) {
            # clear a/p after round, since user did not request it
            delete $rem->{_a};
            delete $rem->{_p};
        }
        $x = $downgrade -> new($x -> bdstr(), @r)
          if defined($downgrade) && $x -> is_int();
        $rem = $downgrade -> new($rem -> bdstr(), @r)
          if defined($downgrade) && $rem -> is_int();
        return ($x, $rem);
    }

    $x = $downgrade -> new($x, @r)
      if defined($downgrade) && $x -> is_int();
    $x;         # rounding already done above
}

sub bmod {
    # (dividend: BFLOAT or num_str, divisor: BFLOAT or num_str) return remainder

    # set up parameters
    my ($class, $x, $y, @r) = ref($_[0]) && ref($_[0]) eq ref($_[1])
                            ? (ref($_[0]), @_)
                            : objectify(2, @_);

    return $x if $x->modify('bmod');

    # At least one argument is NaN. This is handled the same way as in
    # Math::BigInt -> bmod().

    return $x -> bnan(@r) if $x -> is_nan() || $y -> is_nan();

    # Modulo zero. This is handled the same way as in Math::BigInt -> bmod().

    if ($y -> is_zero()) {
        return $x -> round(@r);
    }

    # Numerator (dividend) is +/-inf. This is handled the same way as in
    # Math::BigInt -> bmod().

    if ($x -> is_inf()) {
        return $x -> bnan(@r);
    }

    # Denominator (divisor) is +/-inf. This is handled the same way as in
    # Math::BigInt -> bmod().

    if ($y -> is_inf()) {
        if ($x -> is_zero() || $x -> bcmp(0) == $y -> bcmp(0)) {
            return $x -> round(@r);
        } else {
            return $x -> binf($y -> sign(), @r);
        }
    }

    return $x->bzero(@r) if $x->is_zero()
      || ($x->is_int() &&
          # check that $y == +1 or $y == -1:
          ($LIB->_is_zero($y->{_e}) && $LIB->_is_one($y->{_m})));

    my $cmp = $x->bacmp($y);    # equal or $x < $y?
    if ($cmp == 0) {            # $x == $y => result 0
        return $x -> bzero(@r);
    }

    # only $y of the operands negative?
    my $neg = $x->{sign} ne $y->{sign} ? 1 : 0;

    $x->{sign} = $y->{sign};     # calc sign first
    if ($cmp < 0 && $neg == 0) { # $x < $y => result $x
        return $x -> round(@r);
    }

    my $ym = $LIB->_copy($y->{_m});

    # 2e1 => 20
    $ym = $LIB->_lsft($ym, $y->{_e}, 10)
      if $y->{_es} eq '+' && !$LIB->_is_zero($y->{_e});

    # if $y has digits after dot
    my $shifty = 0;             # correct _e of $x by this
    if ($y->{_es} eq '-')       # has digits after dot
    {
        # 123 % 2.5 => 1230 % 25 => 5 => 0.5
        $shifty = $LIB->_num($y->{_e});  # no more digits after dot
        # 123 => 1230, $y->{_m} is already 25
        $x->{_m} = $LIB->_lsft($x->{_m}, $y->{_e}, 10);
    }
    # $ym is now mantissa of $y based on exponent 0

    my $shiftx = 0;             # correct _e of $x by this
    if ($x->{_es} eq '-')       # has digits after dot
    {
        # 123.4 % 20 => 1234 % 200
        $shiftx = $LIB->_num($x->{_e}); # no more digits after dot
        $ym = $LIB->_lsft($ym, $x->{_e}, 10); # 123 => 1230
    }
    # 123e1 % 20 => 1230 % 20
    if ($x->{_es} eq '+' && !$LIB->_is_zero($x->{_e})) {
        $x->{_m} = $LIB->_lsft($x->{_m}, $x->{_e}, 10); # es => '+' here
    }

    $x->{_e} = $LIB->_new($shiftx);
    $x->{_es} = '+';
    $x->{_es} = '-' if $shiftx != 0 || $shifty != 0;
    $x->{_e} = $LIB->_add($x->{_e}, $LIB->_new($shifty)) if $shifty != 0;

    # now mantissas are equalized, exponent of $x is adjusted, so calc result

    $x->{_m} = $LIB->_mod($x->{_m}, $ym);

    $x->{sign} = '+' if $LIB->_is_zero($x->{_m}); # fix sign for -0
    $x = $x->bnorm();

    # if one of them negative => correct in place
    if ($neg != 0 && ! $x -> is_zero()) {
        my $r = $y - $x;
        $x->{_m} = $r->{_m};
        $x->{_e} = $r->{_e};
        $x->{_es} = $r->{_es};
        $x->{sign} = '+' if $LIB->_is_zero($x->{_m}); # fix sign for -0
        $x = $x->bnorm();
    }

    $x = $x->round($r[0], $r[1], $r[2], $y);
    return $downgrade -> new($x -> bdstr(), @r)
      if defined($downgrade) && ($x->is_int() || $x->is_inf() || $x->is_nan());
    return $x;
}

sub bmodpow {
    # takes a very large number to a very large exponent in a given very
    # large modulus, quickly, thanks to binary exponentiation. Supports
    # negative exponents.
    my ($class, $num, $exp, $mod, @r)
      = ref($_[0]) && ref($_[0]) eq ref($_[1]) && ref($_[1]) eq ref($_[2])
      ? (ref($_[0]), @_)
      : objectify(3, @_);

    return $num if $num->modify('bmodpow');

    return $num -> bnan(@r)
      if $mod->is_nan() || $exp->is_nan() || $mod->is_nan();

    # check modulus for valid values
    return $num->bnan(@r) if $mod->{sign} ne '+' || $mod->is_zero();

    # check exponent for valid values
    if ($exp->{sign} =~ /\w/) {
        # i.e., if it's NaN, +inf, or -inf...
        return $num->bnan(@r);
    }

    $num = $num->bmodinv($mod, @r) if $exp->{sign} eq '-';

    # check num for valid values (also NaN if there was no inverse but $exp < 0)
    return $num->bnan(@r) if $num->{sign} !~ /^[+-]$/;

    # $mod is positive, sign on $exp is ignored, result also positive

    # XXX TODO: speed it up when all three numbers are integers
    $num = $num->bpow($exp)->bmod($mod);

    return $downgrade -> new($num -> bdstr(), @r) if defined($downgrade)
      && ($num->is_int() || $num->is_inf() || $num->is_nan());
    return $num -> round(@r);
}

sub bpow {
    # (BFLOAT or num_str, BFLOAT or num_str) return BFLOAT
    # compute power of two numbers, second arg is used as integer
    # modifies first argument

    # set up parameters
    my ($class, $x, $y, $a, $p, $r) = (ref($_[0]), @_);
    # objectify is costly, so avoid it
    if ((!ref($_[0])) || (ref($_[0]) ne ref($_[1]))) {
        ($class, $x, $y, $a, $p, $r) = objectify(2, @_);
    }

    return $x if $x -> modify('bpow');

    # $x and/or $y is a NaN
    return $x -> bnan() if $x -> is_nan() || $y -> is_nan();

    # $x and/or $y is a +/-Inf
    if ($x -> is_inf("-")) {
        return $x -> bzero()   if $y -> is_negative();
        return $x -> bnan()    if $y -> is_zero();
        return $x            if $y -> is_odd();
        return $x -> bneg();
    } elsif ($x -> is_inf("+")) {
        return $x -> bzero()   if $y -> is_negative();
        return $x -> bnan()    if $y -> is_zero();
        return $x;
    } elsif ($y -> is_inf("-")) {
        return $x -> bnan()    if $x -> is_one("-");
        return $x -> binf("+") if $x > -1 && $x < 1;
        return $x -> bone()    if $x -> is_one("+");
        return $x -> bzero();
    } elsif ($y -> is_inf("+")) {
        return $x -> bnan()    if $x -> is_one("-");
        return $x -> bzero()   if $x > -1 && $x < 1;
        return $x -> bone()    if $x -> is_one("+");
        return $x -> binf("+");
    }

    if ($x -> is_zero()) {
        return $x -> bone() if $y -> is_zero();
        return $x -> binf() if $y -> is_negative();
        return $x;
    }

    # We don't support complex numbers, so upgrade or return NaN.

    if ($x -> is_negative() && !$y -> is_int()) {
        return $upgrade -> bpow($x, $y, $a, $p, $r) if defined $upgrade;
        return $x -> bnan();
    }

    if ($x -> is_one("+") || $y -> is_one()) {
        return $x;
    }

    if ($x -> is_one("-")) {
        return $x if $y -> is_odd();
        return $x -> bneg();
    }

    return $x -> _pow($y, $a, $p, $r) if !$y -> is_int();

    my $y1 = $y -> as_int()->{value}; # make MBI part

    my $new_sign = '+';
    $new_sign = $LIB -> _is_odd($y1) ? '-' : '+' if $x->{sign} ne '+';

    # calculate $x->{_m} ** $y and $x->{_e} * $y separately (faster)
    $x->{_m} = $LIB -> _pow($x->{_m}, $y1);
    $x->{_e} = $LIB -> _mul($x->{_e}, $y1);

    $x->{sign} = $new_sign;
    $x = $x -> bnorm();

    # x ** (-y) = 1 / (x ** y)

    if ($y->{sign} eq '-') {
        # modify $x in place!
        my $z = $x -> copy();
        $x = $x -> bone();
        # round in one go (might ignore y's A!)
        return scalar $x -> bdiv($z, $a, $p, $r);
    }

    $x = $x -> round($a, $p, $r, $y);

    return $downgrade -> new($x)
      if defined($downgrade) && ($x->is_int() || $x->is_inf() || $x->is_nan());
    return $x;
}

sub blog {
    # Return the logarithm of the operand. If a second operand is defined, that
    # value is used as the base, otherwise the base is assumed to be Euler's
    # constant.

    my ($class, $x, $base, @r);

    # Only objectify the base if it is defined, since an undefined base, as in
    # $x->blog() or $x->blog(undef) signals that the base is Euler's number.

    if (!ref($_[0]) && $_[0] =~ /^[A-Za-z]|::/) {
        # E.g., Math::BigFloat->blog(256, 2)
        ($class, $x, $base, @r) =
          defined $_[2] ? objectify(2, @_) : objectify(1, @_);
    } else {
        # E.g., Math::BigFloat::blog(256, 2) or $x->blog(2)
        ($class, $x, $base, @r) =
          defined $_[1] ? objectify(2, @_) : objectify(1, @_);
    }

    return $x if $x->modify('blog');

    return $x -> bnan(@r) if $x -> is_nan();

    return $upgrade -> blog($x, $base, @r)
      if defined($upgrade) && $x -> is_neg();

    # we need to limit the accuracy to protect against overflow
    my $fallback = 0;
    my ($scale, @params);
    ($x, @params) = $x->_find_round_parameters(@r);

    # no rounding at all, so must use fallback
    if (scalar @params == 0) {
        # simulate old behaviour
        $params[0] = $class->div_scale(); # and round to it as accuracy
        $params[1] = undef;               # P = undef
        $scale = $params[0]+4;            # at least four more for proper round
        $params[2] = $r[2];               # round mode by caller or undef
        $fallback = 1;                    # to clear a/p afterwards
    } else {
        # the 4 below is empirical, and there might be cases where it is not
        # enough...
        $scale = abs($params[0] || $params[1]) + 4; # take whatever is defined
    }

    my $done = 0;
    if (defined $base) {
        $base = $class -> new($base)
          unless defined(blessed($base)) && $base -> isa($class);
        if ($base -> is_nan() || $base -> is_one()) {
            $x = $x -> bnan();
            $done = 1;
        } elsif ($base -> is_inf() || $base -> is_zero()) {
            if ($x -> is_inf() || $x -> is_zero()) {
                $x = $x -> bnan();
            } else {
                $x = $x -> bzero(@params);
            }
            $done = 1;
        } elsif ($base -> is_negative()) { # -inf < base < 0
            if ($x -> is_one()) {          #     x = 1
                $x = $x -> bzero(@params);
            } elsif ($x == $base) {
                $x = $x -> bone('+', @params); #     x = base
            } else {
                $x = $x -> bnan();   #     otherwise
            }
            $done = 1;
        } elsif ($x == $base) {
            $x = $x -> bone('+', @params); # 0 < base && 0 < x < inf
            $done = 1;
        }
    }

    # We now know that the base is either undefined or positive and finite.

    unless ($done) {
        if ($x -> is_inf()) {   #   x = +/-inf
            my $sign = defined $base && $base < 1 ? '-' : '+';
            $x = $x -> binf($sign);
            $done = 1;
        } elsif ($x -> is_neg()) { #   -inf < x < 0
            $x = $x -> bnan();
            $done = 1;
        } elsif ($x -> is_one()) { #   x = 1
            $x = $x -> bzero(@params);
            $done = 1;
        } elsif ($x -> is_zero()) { #   x = 0
            my $sign = defined $base && $base < 1 ? '+' : '-';
            $x = $x -> binf($sign);
            $done = 1;
        }
    }

    if ($done) {
        if ($fallback) {
            # clear a/p after round, since user did not request it
            delete $x->{_a};
            delete $x->{_p};
        }
        return $downgrade -> new($x -> bdstr(), @r)
          if defined($downgrade) && $x->is_int();
        return $x;
    }

    # when user set globals, they would interfere with our calculation, so
    # disable them and later re-enable them
    no strict 'refs';
    my $abr = "$class\::accuracy";
    my $ab = $$abr;
    $$abr = undef;
    my $pbr = "$class\::precision";
    my $pb = $$pbr;
    $$pbr = undef;
    # we also need to disable any set A or P on $x (_find_round_parameters took
    # them already into account), since these would interfere, too
    delete $x->{_a};
    delete $x->{_p};

    $done = 0;

    # If both the invocand and the base are integers, try to calculate integer
    # result first. This is very fast, and in case the real result was found, we
    # can stop right here.

    if (defined($base) && $base -> is_int() && $x -> is_int()) {
        my $x_lib = $LIB -> _new($x -> bdstr());
        my $b_lib = $LIB -> _new($base -> bdstr());
        ($x_lib, my $exact) = $LIB -> _log_int($x_lib, $b_lib);
        if ($exact) {
            $x->{_m} = $x_lib;
            $x->{_e} = $LIB -> _zero();
            $x = $x -> bnorm();
            $done = 1;
        }
    }

    unless ($done) {

        # First calculate the log to base e (using reduction by 10 and possibly
        # also by 2), and if a different base was requested, convert the result.

        $x = $x->_log_10($scale);
        if (defined $base) {
            # log_b(x) = ln(x) / ln(b), so compute ln(b)
            my $base_log_e = $base->copy()->_log_10($scale);
            $x = $x->bdiv($base_log_e, $scale);
        }
    }

    # shortcut to not run through _find_round_parameters again
    if (defined $params[0]) {
        $x = $x->bround($params[0], $params[2]); # then round accordingly
    } else {
        $x = $x->bfround($params[1], $params[2]); # then round accordingly
    }
    if ($fallback) {
        # clear a/p after round, since user did not request it
        delete $x->{_a};
        delete $x->{_p};
    }
    # restore globals
    $$abr = $ab;
    $$pbr = $pb;

    return $downgrade -> new($x -> bdstr(), @r)
      if defined($downgrade) && $x->is_int();
    return $x;
}

sub bexp {
    # Calculate e ** X (Euler's number to the power of X)
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    return $x if $x->modify('bexp');

    return $x->bnan(@r)  if $x -> is_nan();
    return $x->binf(@r)  if $x->{sign} eq '+inf';
    return $x->bzero(@r) if $x->{sign} eq '-inf';

    # we need to limit the accuracy to protect against overflow
    my $fallback = 0;
    my ($scale, @params);
    ($x, @params) = $x->_find_round_parameters(@r);

    # error in _find_round_parameters?
    return $x->bnan(@r) if $x->{sign} eq 'NaN';

    # no rounding at all, so must use fallback
    if (scalar @params == 0) {
        # simulate old behaviour
        $params[0] = $class->div_scale(); # and round to it as accuracy
        $params[1] = undef;               # P = undef
        $scale = $params[0]+4;            # at least four more for proper round
        $params[2] = $r[2];               # round mode by caller or undef
        $fallback = 1;                    # to clear a/p afterwards
    } else {
        # the 4 below is empirical, and there might be cases where it's not
        # enough ...
        $scale = abs($params[0] || $params[1]) + 4; # take whatever is defined
    }

    return $x->bone(@params) if $x->is_zero();

    if (!$x->isa('Math::BigFloat')) {
        $x = Math::BigFloat->new($x);
        $class = ref($x);
    }

    # when user set globals, they would interfere with our calculation, so
    # disable them and later re-enable them
    no strict 'refs';
    my $abr = "$class\::accuracy";
    my $ab = $$abr;
    $$abr = undef;
    my $pbr = "$class\::precision";
    my $pb = $$pbr;
    $$pbr = undef;
    # we also need to disable any set A or P on $x (_find_round_parameters took
    # them already into account), since these would interfere, too
    delete $x->{_a};
    delete $x->{_p};

    # Disabling upgrading and downgrading is no longer necessary to avoid an
    # infinite recursion, but it avoids unnecessary upgrading and downgrading in
    # the intermediate computations.

    local $Math::BigInt::upgrade = undef;
    local $Math::BigFloat::downgrade = undef;

    my $x_org = $x->copy();

    # We use the following Taylor series:

    #           x    x^2   x^3   x^4
    #  e = 1 + --- + --- + --- + --- ...
    #           1!    2!    3!    4!

    # The difference for each term is X and N, which would result in:
    # 2 copy, 2 mul, 2 add, 1 inc, 1 div operations per term

    # But it is faster to compute exp(1) and then raising it to the
    # given power, esp. if $x is really big and an integer because:

    #  * The numerator is always 1, making the computation faster
    #  * the series converges faster in the case of x == 1
    #  * We can also easily check when we have reached our limit: when the
    #    term to be added is smaller than "1E$scale", we can stop - f.i.
    #    scale == 5, and we have 1/40320, then we stop since 1/40320 < 1E-5.
    #  * we can compute the *exact* result by simulating bigrat math:

    #  1   1    gcd(3, 4) = 1    1*24 + 1*6    5
    #  - + -                  = ---------- =  --
    #  6   24                      6*24       24

    # We do not compute the gcd() here, but simple do:
    #  1   1    1*24 + 1*6   30
    #  - + -  = --------- =  --
    #  6   24       6*24     144

    # In general:
    #  a   c    a*d + c*b         and note that c is always 1 and d = (b*f)
    #  - + -  = ---------
    #  b   d       b*d

    # This leads to:         which can be reduced by b to:
    #  a   1     a*b*f + b    a*f + 1
    #  - + -   = --------- =  -------
    #  b   b*f     b*b*f        b*f

    # The first terms in the series are:

    # 1     1    1    1    1    1     1     1     13700
    # -- + -- + -- + -- + -- + --- + --- + ---- = -----
    # 1     1    2    6   24   120   720   5040   5040

    # Note that we cannot simply reduce 13700/5040 to 685/252, but must keep
    # the numerator and the denominator!

    if ($scale <= 75) {
        # set $x directly from a cached string form
        $x->{_m} = $LIB->_new("2718281828459045235360287471352662497757" .
                              "2470936999595749669676277240766303535476");
        $x->{sign} = '+';
        $x->{_es} = '-';
        $x->{_e} = $LIB->_new(79);
    } else {
        # compute A and B so that e = A / B.

        # After some terms we end up with this, so we use it as a starting
        # point:
        my $A = $LIB->_new("9093339520860578540197197" .
                           "0164779391644753259799242");
        my $F = $LIB->_new(42);
        my $step = 42;

        # Compute how many steps we need to take to get $A and $B sufficiently
        # big
        my $steps = _len_to_steps($scale - 4);
        #    print STDERR "# Doing $steps steps for ", $scale-4, " digits\n";
        while ($step++ <= $steps) {
            # calculate $a * $f + 1
            $A = $LIB->_mul($A, $F);
            $A = $LIB->_inc($A);
            # increment f
            $F = $LIB->_inc($F);
        }

        # Compute $B as factorial of $steps (this is faster than doing it
        # manually)
        my $B = $LIB->_fac($LIB->_new($steps));

        #  print "A ", $LIB->_str($A), "\nB ", $LIB->_str($B), "\n";

        # compute A/B with $scale digits in the result (truncate, not round)
        $A = $LIB->_lsft($A, $LIB->_new($scale), 10);
        $A = $LIB->_div($A, $B);

        $x->{_m} = $A;
        $x->{sign} = '+';
        $x->{_es} = '-';
        $x->{_e} = $LIB->_new($scale);
    }

    # $x contains now an estimate of e, with some surplus digits, so we can
    # round
    if (!$x_org->is_one()) {
        # Reduce size of fractional part, followup with integer power of two.
        my $lshift = 0;
        while ($lshift < 30 && $x_org->bacmp(2 << $lshift) > 0) {
            $lshift++;
        }
        # Raise $x to the wanted power and round it.
        if ($lshift == 0) {
            $x = $x->bpow($x_org, @params);
        } else {
            my($mul, $rescale) = (1 << $lshift, $scale+1+$lshift);
            $x = $x -> bpow(scalar $x_org->bdiv($mul, $rescale), $rescale)
                    -> bpow($mul, @params);
        }
    } else {
        # else just round the already computed result
        delete $x->{_a};
        delete $x->{_p};
        # shortcut to not run through _find_round_parameters again
        if (defined $params[0]) {
            $x = $x->bround($params[0], $params[2]); # then round accordingly
        } else {
            $x = $x->bfround($params[1], $params[2]); # then round accordingly
        }
    }
    if ($fallback) {
        # clear a/p after round, since user did not request it
        delete $x->{_a};
        delete $x->{_p};
    }
    # restore globals
    $$abr = $ab;
    $$pbr = $pb;

    return $downgrade -> new($x -> bdstr(), @r)
      if defined($downgrade) && $x -> is_int();
    $x;
}

sub bnok {
    # Calculate n over k (binomial coefficient or "choose" function) as integer.
    # set up parameters
    my ($class, $x, $y, @r) = ref($_[0]) && ref($_[0]) eq ref($_[1])
                            ? (ref($_[0]), @_)
                            : objectify(2, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    return $x if $x->modify('bnok');

    return $x->bnan() if $x->is_nan() || $y->is_nan();
    return $x->bnan() if (($x->is_finite() && !$x->is_int()) ||
                          ($y->is_finite() && !$y->is_int()));

    my $xint = Math::BigInt -> new($x -> bsstr());
    my $yint = Math::BigInt -> new($y -> bsstr());
    $xint = $xint -> bnok($yint);

    return $xint if defined $downgrade;

    my $xflt = Math::BigFloat -> new($xint);

    $x->{_m}   = $xflt->{_m};
    $x->{_e}   = $xflt->{_e};
    $x->{_es}  = $xflt->{_es};
    $x->{sign} = $xflt->{sign};

    return $x;
}

sub bsin {
    # Calculate a sinus of x.
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    # taylor:      x^3   x^5   x^7   x^9
    #    sin = x - --- + --- - --- + --- ...
    #               3!    5!    7!    9!

    return $x if $x->modify('bsin');

    return $x -> bzero(@r) if $x->is_zero();
    return $x -> bnan(@r)  if $x->is_nan() || $x->is_inf();

    # we need to limit the accuracy to protect against overflow
    my $fallback = 0;
    my ($scale, @params);
    ($x, @params) = $x->_find_round_parameters(@r);

    # error in _find_round_parameters?
    return $x->bnan(@r) if $x->is_nan();

    # no rounding at all, so must use fallback
    if (scalar @params == 0) {
        # simulate old behaviour
        $params[0] = $class->div_scale(); # and round to it as accuracy
        $params[1] = undef;               # disable P
        $scale = $params[0]+4;            # at least four more for proper round
        $params[2] = $r[2];               # round mode by caller or undef
        $fallback = 1;                    # to clear a/p afterwards
    } else {
        # the 4 below is empirical, and there might be cases where it is not
        # enough...
        $scale = abs($params[0] || $params[1]) + 4; # take whatever is defined
    }

    # when user set globals, they would interfere with our calculation, so
    # disable them and later re-enable them
    no strict 'refs';
    my $abr = "$class\::accuracy";
    my $ab = $$abr;
    $$abr = undef;
    my $pbr = "$class\::precision";
    my $pb = $$pbr;
    $$pbr = undef;
    # we also need to disable any set A or P on $x (_find_round_parameters took
    # them already into account), since these would interfere, too
    delete $x->{_a};
    delete $x->{_p};

    # Disabling upgrading and downgrading is no longer necessary to avoid an
    # infinite recursion, but it avoids unnecessary upgrading and downgrading in
    # the intermediate computations.

    local $Math::BigInt::upgrade = undef;
    local $Math::BigFloat::downgrade = undef;

    my $over = $x * $x;         # X ^ 2
    my $x2 = $over->copy();     # X ^ 2; difference between terms
    $over = $over->bmul($x);    # X ^ 3 as starting value
    my $sign = 1;               # start with -=
    my $below = $class->new(6);
    my $factorial = $class->new(4);
    delete $x->{_a};
    delete $x->{_p};

    my $limit = $class->new("1E-". ($scale-1));
    while (1) {
        # we calculate the next term, and add it to the last
        # when the next term is below our limit, it won't affect the outcome
        # anymore, so we stop:
        my $next = $over->copy()->bdiv($below, $scale);
        last if $next->bacmp($limit) <= 0;

        if ($sign == 0) {
            $x = $x->badd($next);
        } else {
            $x = $x->bsub($next);
        }
        $sign = 1-$sign;        # alternate
        # calculate things for the next term
        $over = $over->bmul($x2);                       # $x*$x
        $below = $below->bmul($factorial);              # n*(n+1)
        $factorial = $factorial->binc();
        $below = $below -> bmul($factorial);              # n*(n+1)
        $factorial = $factorial->binc();
    }

    # shortcut to not run through _find_round_parameters again
    if (defined $params[0]) {
        $x = $x->bround($params[0], $params[2]); # then round accordingly
    } else {
        $x = $x->bfround($params[1], $params[2]); # then round accordingly
    }
    if ($fallback) {
        # clear a/p after round, since user did not request it
        delete $x->{_a};
        delete $x->{_p};
    }
    # restore globals
    $$abr = $ab;
    $$pbr = $pb;

    return $downgrade -> new($x -> bdstr(), @r)
      if defined($downgrade) && $x -> is_int();
    $x;
}

sub bcos {
    # Calculate a cosinus of x.
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    # Taylor:      x^2   x^4   x^6   x^8
    #    cos = 1 - --- + --- - --- + --- ...
    #               2!    4!    6!    8!

    # we need to limit the accuracy to protect against overflow
    my $fallback = 0;
    my ($scale, @params);
    ($x, @params) = $x->_find_round_parameters(@r);

    #         constant object       or error in _find_round_parameters?
    return $x if $x->modify('bcos') || $x->is_nan();
    return $x->bnan()   if $x->is_inf();
    return $x->bone(@r) if $x->is_zero();

    # no rounding at all, so must use fallback
    if (scalar @params == 0) {
        # simulate old behaviour
        $params[0] = $class->div_scale(); # and round to it as accuracy
        $params[1] = undef;               # disable P
        $scale = $params[0]+4;            # at least four more for proper round
        $params[2] = $r[2];               # round mode by caller or undef
        $fallback = 1;                    # to clear a/p afterwards
    } else {
        # the 4 below is empirical, and there might be cases where it is not
        # enough...
        $scale = abs($params[0] || $params[1]) + 4; # take whatever is defined
    }

    # when user set globals, they would interfere with our calculation, so
    # disable them and later re-enable them
    no strict 'refs';
    my $abr = "$class\::accuracy";
    my $ab = $$abr;
    $$abr = undef;
    my $pbr = "$class\::precision";
    my $pb = $$pbr;
    $$pbr = undef;
    # we also need to disable any set A or P on $x (_find_round_parameters took
    # them already into account), since these would interfere, too
    delete $x->{_a};
    delete $x->{_p};

    my $over = $x * $x;         # X ^ 2
    my $x2 = $over->copy();     # X ^ 2; difference between terms
    my $sign = 1;               # start with -=
    my $below = $class->new(2);
    my $factorial = $class->new(3);
    $x = $x->bone();
    delete $x->{_a};
    delete $x->{_p};

    my $limit = $class->new("1E-". ($scale-1));
    #my $steps = 0;
    while (3 < 5) {
        # we calculate the next term, and add it to the last
        # when the next term is below our limit, it won't affect the outcome
        # anymore, so we stop:
        my $next = $over->copy()->bdiv($below, $scale);
        last if $next->bacmp($limit) <= 0;

        if ($sign == 0) {
            $x = $x->badd($next);
        } else {
            $x = $x->bsub($next);
        }
        $sign = 1-$sign;        # alternate
        # calculate things for the next term
        $over = $over->bmul($x2);                       # $x*$x
        $below = $below->bmul($factorial);              # n*(n+1)
        $factorial = $factorial -> binc();
        $below = $below->bmul($factorial);              # n*(n+1)
        $factorial = $factorial -> binc();
    }

    # shortcut to not run through _find_round_parameters again
    if (defined $params[0]) {
        $x = $x->bround($params[0], $params[2]); # then round accordingly
    } else {
        $x = $x->bfround($params[1], $params[2]); # then round accordingly
    }
    if ($fallback) {
        # clear a/p after round, since user did not request it
        delete $x->{_a};
        delete $x->{_p};
    }
    # restore globals
    $$abr = $ab;
    $$pbr = $pb;

    return $downgrade -> new($x -> bdstr(), @r)
      if defined($downgrade) && $x -> is_int();
    $x;
}

sub batan {
    # Calculate a arcus tangens of x.
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    # taylor:       x^3   x^5   x^7   x^9
    #    atan = x - --- + --- - --- + --- ...
    #                3     5     7     9

    return $x if $x->modify('batan');

    return $x -> bnan(@r) if $x->is_nan();

    # We need to limit the accuracy to protect against overflow.

    my $fallback = 0;
    my ($scale, @params);
    ($x, @params) = $x->_find_round_parameters(@r);

    # Error in _find_round_parameters?

    return $x -> bnan(@r) if $x->is_nan();

    if ($x->{sign} =~ /^[+-]inf\z/) {
        # +inf result is PI/2
        # -inf result is -PI/2
        # calculate PI/2
        my $pi = $class->bpi(@r);
        # modify $x in place
        $x->{_m} = $pi->{_m};
        $x->{_e} = $pi->{_e};
        $x->{_es} = $pi->{_es};
        # -y => -PI/2, +y => PI/2
        $x->{sign} = substr($x->{sign}, 0, 1); # "+inf" => "+"
        $x -> {_m} = $LIB->_div($x->{_m}, $LIB->_new(2));
        return $x;
    }

    return $x->bzero(@r) if $x->is_zero();

    # no rounding at all, so must use fallback
    if (scalar @params == 0) {
        # simulate old behaviour
        $params[0] = $class->div_scale(); # and round to it as accuracy
        $params[1] = undef;               # disable P
        $scale = $params[0]+4;            # at least four more for proper round
        $params[2] = $r[2];               # round mode by caller or undef
        $fallback = 1;                    # to clear a/p afterwards
    } else {
        # the 4 below is empirical, and there might be cases where it is not
        # enough...
        $scale = abs($params[0] || $params[1]) + 4; # take whatever is defined
    }

    # 1 or -1 => PI/4
    # inlined is_one() && is_one('-')
    if ($LIB->_is_one($x->{_m}) && $LIB->_is_zero($x->{_e})) {
        my $pi = $class->bpi($scale - 3);
        # modify $x in place
        $x->{_m} = $pi->{_m};
        $x->{_e} = $pi->{_e};
        $x->{_es} = $pi->{_es};
        # leave the sign of $x alone (+1 => +PI/4, -1 => -PI/4)
        $x->{_m} = $LIB->_div($x->{_m}, $LIB->_new(4));
        return $x;
    }

    # This series is only valid if -1 < x < 1, so for other x we need to
    # calculate PI/2 - atan(1/x):
    my $pi = undef;
    if ($x->bacmp($x->copy()->bone) >= 0) {
        # calculate PI/2
        $pi = $class->bpi($scale - 3);
        $pi->{_m} = $LIB->_div($pi->{_m}, $LIB->_new(2));
        # calculate 1/$x:
        my $x_copy = $x->copy();
        # modify $x in place
        $x = $x->bone();
        $x = $x->bdiv($x_copy, $scale);
    }

    my $fmul = 1;
    foreach (0 .. int($scale / 20)) {
        $fmul *= 2;
        $x = $x->bdiv($x->copy()->bmul($x)->binc()->bsqrt($scale + 4)->binc(),
                      $scale + 4);
    }

    # When user set globals, they would interfere with our calculation, so
    # disable them and later re-enable them.
    no strict 'refs';
    my $abr = "$class\::accuracy";
    my $ab = $$abr;
    $$abr = undef;
    my $pbr = "$class\::precision";
    my $pb = $$pbr;
    $$pbr = undef;
    # We also need to disable any set A or P on $x (_find_round_parameters
    # took them already into account), since these would interfere, too
    delete $x->{_a};
    delete $x->{_p};

    # Disabling upgrading and downgrading is no longer necessary to avoid an
    # infinite recursion, but it avoids unnecessary upgrading and downgrading in
    # the intermediate computations.

    local $Math::BigInt::upgrade = undef;
    local $Math::BigFloat::downgrade = undef;

    my $over = $x * $x;   # X ^ 2
    my $x2 = $over->copy();  # X ^ 2; difference between terms
    $over = $over->bmul($x);         # X ^ 3 as starting value
    my $sign = 1;               # start with -=
    my $below = $class->new(3);
    my $two = $class->new(2);
    delete $x->{_a};
    delete $x->{_p};

    my $limit = $class->new("1E-". ($scale-1));
    #my $steps = 0;
    while (1) {
        # We calculate the next term, and add it to the last. When the next
        # term is below our limit, it won't affect the outcome anymore, so we
        # stop:
        my $next = $over->copy()->bdiv($below, $scale);
        last if $next->bacmp($limit) <= 0;

        if ($sign == 0) {
            $x = $x->badd($next);
        } else {
            $x = $x->bsub($next);
        }
        $sign = 1-$sign;        # alternatex
        # calculate things for the next term
        $over = $over->bmul($x2);    # $x*$x
        $below = $below->badd($two);     # n += 2
    }
    $x = $x->bmul($fmul);

    if (defined $pi) {
        my $x_copy = $x->copy();
        # modify $x in place
        $x->{_m} = $pi->{_m};
        $x->{_e} = $pi->{_e};
        $x->{_es} = $pi->{_es};
        # PI/2 - $x
        $x = $x->bsub($x_copy);
    }

    # Shortcut to not run through _find_round_parameters again.
    if (defined $params[0]) {
        $x = $x->bround($params[0], $params[2]); # then round accordingly
    } else {
        $x = $x->bfround($params[1], $params[2]); # then round accordingly
    }
    if ($fallback) {
        # Clear a/p after round, since user did not request it.
        delete $x->{_a};
        delete $x->{_p};
    }

    # restore globals
    $$abr = $ab;
    $$pbr = $pb;

    return $downgrade -> new($x -> bdstr(), @r)
      if defined($downgrade) && ($x -> is_int() || $x -> is_inf());
    $x;
}

sub batan2 {
    # $y -> batan2($x) returns the arcus tangens of $y / $x.

    # Set up parameters.
    my ($class, $y, $x, @r) = ref($_[0]) && ref($_[0]) eq ref($_[1])
                            ? (ref($_[0]), @_)
                            : objectify(2, @_);

    # Quick exit if $y is read-only.
    return $y if $y -> modify('batan2');

    # Handle all NaN cases.
    return $y -> bnan() if $x->{sign} eq $nan || $y->{sign} eq $nan;

    # We need to limit the accuracy to protect against overflow.
    my $fallback = 0;
    my ($scale, @params);
    ($y, @params) = $y -> _find_round_parameters(@r);

    # Error in _find_round_parameters?
    return $y if $y->is_nan();

    # No rounding at all, so must use fallback.
    if (scalar @params == 0) {
        # Simulate old behaviour
        $params[0] = $class -> div_scale(); # and round to it as accuracy
        $params[1] = undef;                 # disable P
        $scale = $params[0] + 4; # at least four more for proper round
        $params[2] = $r[2];      # round mode by caller or undef
        $fallback = 1;           # to clear a/p afterwards
    } else {
        # The 4 below is empirical, and there might be cases where it is not
        # enough ...
        $scale = abs($params[0] || $params[1]) + 4; # take whatever is defined
    }

    if ($x -> is_inf("+")) {                          # x = inf
        if ($y -> is_inf("+")) {                      #    y = inf
            $y = $y -> bpi($scale) -> bmul("0.25");   #       pi/4
        } elsif ($y -> is_inf("-")) {                 #    y = -inf
            $y = $y -> bpi($scale) -> bmul("-0.25");  #       -pi/4
        } else {                                      #    -inf < y < inf
            return $y -> bzero(@r);                   #       0
        }
    } elsif ($x -> is_inf("-")) {                     # x = -inf
        if ($y -> is_inf("+")) {                      #    y = inf
            $y = $y -> bpi($scale) -> bmul("0.75");   #       3/4 pi
        } elsif ($y -> is_inf("-")) {                 #    y = -inf
            $y = $y -> bpi($scale) -> bmul("-0.75");  #       -3/4 pi
        } elsif ($y >= 0) {                           #    y >= 0
            $y = $y -> bpi($scale);                   #       pi
        } else {                                      #    y < 0
            $y = $y -> bpi($scale) -> bneg();         #       -pi
        }
    } elsif ($x > 0) {                                    # 0 < x < inf
        if ($y -> is_inf("+")) {                          #    y = inf
            $y = $y -> bpi($scale) -> bmul("0.5");        #       pi/2
        } elsif ($y -> is_inf("-")) {                     #    y = -inf
            $y = $y -> bpi($scale) -> bmul("-0.5");       #       -pi/2
        } else {                                          #   -inf < y < inf
            $y = $y -> bdiv($x, $scale) -> batan($scale); #       atan(y/x)
        }
    } elsif ($x < 0) {                                # -inf < x < 0
        my $pi = $class -> bpi($scale);
        if ($y >= 0) {                                #    y >= 0
            $y = $y -> bdiv($x, $scale) -> batan()    #       atan(y/x) + pi
               -> badd($pi);
        } else {                                      #    y < 0
            $y = $y -> bdiv($x, $scale) -> batan()    #       atan(y/x) - pi
               -> bsub($pi);
        }
    } else {                                          # x = 0
        if ($y > 0) {                                 #    y > 0
            $y = $y -> bpi($scale) -> bmul("0.5");    #       pi/2
        } elsif ($y < 0) {                            #    y < 0
            $y = $y -> bpi($scale) -> bmul("-0.5");   #       -pi/2
        } else {                                      #    y = 0
            return $y -> bzero(@r);                   #       0
        }
    }

    $y = $y -> round(@r);

    if ($fallback) {
        delete $y->{_a};
        delete $y->{_p};
    }

    return $y;
}

sub bsqrt {
    # calculate square root
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    return $x if $x->modify('bsqrt');

    # Handle trivial cases.

    return $x -> bnan(@r)      if $x->is_nan();
    return $x -> binf("+", @r) if $x->{sign} eq '+inf';
    return $x -> round(@r)     if $x->is_zero() || $x->is_one();

    # We don't support complex numbers.

    if ($x -> is_neg()) {
        return $upgrade -> bsqrt($x, @r) if defined($upgrade);
        return $x -> bnan(@r);
    }

    # we need to limit the accuracy to protect against overflow
    my $fallback = 0;
    my (@params, $scale);
    ($x, @params) = $x->_find_round_parameters(@r);

    # error in _find_round_parameters?
    return $x -> bnan(@r) if $x->is_nan();

    # no rounding at all, so must use fallback
    if (scalar @params == 0) {
        # simulate old behaviour
        $params[0] = $class->div_scale(); # and round to it as accuracy
        $scale = $params[0]+4;            # at least four more for proper round
        $params[2] = $r[2];               # round mode by caller or undef
        $fallback = 1;                    # to clear a/p afterwards
    } else {
        # the 4 below is empirical, and there might be cases where it is not
        # enough...
        $scale = abs($params[0] || $params[1]) + 4; # take whatever is defined
    }

    # when user set globals, they would interfere with our calculation, so
    # disable them and later re-enable them
    no strict 'refs';
    my $abr = "$class\::accuracy";
    my $ab = $$abr;
    $$abr = undef;
    my $pbr = "$class\::precision";
    my $pb = $$pbr;
    $$pbr = undef;
    # we also need to disable any set A or P on $x (_find_round_parameters took
    # them already into account), since these would interfere, too
    delete $x->{_a};
    delete $x->{_p};

    # Disabling upgrading and downgrading is no longer necessary to avoid an
    # infinite recursion, but it avoids unnecessary upgrading and downgrading in
    # the intermediate computations.

    local $Math::BigInt::upgrade = undef;
    local $Math::BigFloat::downgrade = undef;

    my $i = $LIB->_copy($x->{_m});
    $i = $LIB->_lsft($i, $x->{_e}, 10) unless $LIB->_is_zero($x->{_e});
    my $xas = Math::BigInt->bzero();
    $xas->{value} = $i;

    my $gs = $xas->copy()->bsqrt(); # some guess

    if (($x->{_es} ne '-')           # guess can't be accurate if there are
        # digits after the dot
        && ($xas->bacmp($gs * $gs) == 0)) # guess hit the nail on the head?
    {
        # exact result, copy result over to keep $x
        $x->{_m} = $gs->{value};
        $x->{_e} = $LIB->_zero();
        $x->{_es} = '+';
        $x = $x->bnorm();
        # shortcut to not run through _find_round_parameters again
        if (defined $params[0]) {
            $x = $x->bround($params[0], $params[2]); # then round accordingly
        } else {
            $x = $x->bfround($params[1], $params[2]); # then round accordingly
        }
        if ($fallback) {
            # clear a/p after round, since user did not request it
            delete $x->{_a};
            delete $x->{_p};
        }
        # re-enable A and P, upgrade is taken care of by "local"
        ${"$class\::accuracy"} = $ab;
        ${"$class\::precision"} = $pb;
        return $x;
    }

    # sqrt(2) = 1.4 because sqrt(2*100) = 1.4*10; so we can increase the
    # accuracy of the result by multiplying the input by 100 and then divide the
    # integer result of sqrt(input) by 10. Rounding afterwards returns the real
    # result.

    # The following steps will transform 123.456 (in $x) into 123456 (in $y1)
    my $y1 = $LIB->_copy($x->{_m});

    my $length = $LIB->_len($y1);

    # Now calculate how many digits the result of sqrt(y1) would have
    my $digits = int($length / 2);

    # But we need at least $scale digits, so calculate how many are missing
    my $shift = $scale - $digits;

    # This happens if the input had enough digits
    # (we take care of integer guesses above)
    $shift = 0 if $shift < 0;

    # Multiply in steps of 100, by shifting left two times the "missing" digits
    my $s2 = $shift * 2;

    # We now make sure that $y1 has the same odd or even number of digits than
    # $x had. So when _e of $x is odd, we must shift $y1 by one digit left,
    # because we always must multiply by steps of 100 (sqrt(100) is 10) and not
    # steps of 10. The length of $x does not count, since an even or odd number
    # of digits before the dot is not changed by adding an even number of digits
    # after the dot (the result is still odd or even digits long).
    $s2++ if $LIB->_is_odd($x->{_e});

    $y1 = $LIB->_lsft($y1, $LIB->_new($s2), 10);

    # now take the square root and truncate to integer
    $y1 = $LIB->_sqrt($y1);

    # By "shifting" $y1 right (by creating a negative _e) we calculate the final
    # result, which is than later rounded to the desired scale.

    # calculate how many zeros $x had after the '.' (or before it, depending
    # on sign of $dat, the result should have half as many:
    my $dat = $LIB->_num($x->{_e});
    $dat = -$dat if $x->{_es} eq '-';
    $dat += $length;

    if ($dat > 0) {
        # no zeros after the dot (e.g. 1.23, 0.49 etc)
        # preserve half as many digits before the dot than the input had
        # (but round this "up")
        $dat = int(($dat+1)/2);
    } else {
        $dat = int(($dat)/2);
    }
    $dat -= $LIB->_len($y1);
    if ($dat < 0) {
        $dat = abs($dat);
        $x->{_e} = $LIB->_new($dat);
        $x->{_es} = '-';
    } else {
        $x->{_e} = $LIB->_new($dat);
        $x->{_es} = '+';
    }
    $x->{_m} = $y1;
    $x = $x->bnorm();

    # shortcut to not run through _find_round_parameters again
    if (defined $params[0]) {
        $x = $x->bround($params[0], $params[2]); # then round accordingly
    } else {
        $x = $x->bfround($params[1], $params[2]); # then round accordingly
    }
    if ($fallback) {
        # clear a/p after round, since user did not request it
        delete $x->{_a};
        delete $x->{_p};
    }
    # restore globals
    $$abr = $ab;
    $$pbr = $pb;

    return $downgrade -> new($x -> bdstr(), @r)
      if defined($downgrade) && ($x -> is_int() || $x -> is_inf());
    $x;
}

sub broot {
    # calculate $y'th root of $x

    # set up parameters
    my ($class, $x, $y, @r) = ref($_[0]) && ref($_[0]) eq ref($_[1])
                            ? (ref($_[0]), @_)
                            : objectify(2, @_);

    return $x if $x->modify('broot');

    # Handle trivial cases.

    return $x -> bnan(@r) if $x->is_nan() || $y->is_nan();

    if ($x -> is_neg()) {
        # -27 ** (1/3) = -3
        return $x -> broot($y -> copy() -> bneg(), @r) -> bneg()
          if $x -> is_int() && $y -> is_int() && $y -> is_neg();
        return $upgrade -> broot($x, $y, @r) if defined $upgrade;
        return $x -> bnan(@r);
    }

    # NaN handling: $x ** 1/0, x or y NaN, or y inf/-inf or y == 0
    return $x->bnan() if $x->{sign} !~ /^\+/ || $y->is_zero() ||
      $y->{sign} !~ /^\+$/;

    return $x if $x->is_zero() || $x->is_one() || $x->is_inf() || $y->is_one();

    # we need to limit the accuracy to protect against overflow
    my $fallback = 0;
    my (@params, $scale);
    ($x, @params) = $x->_find_round_parameters(@r);

    return $x if $x->is_nan();  # error in _find_round_parameters?

    # no rounding at all, so must use fallback
    if (scalar @params == 0) {
        # simulate old behaviour
        $params[0] = $class->div_scale(); # and round to it as accuracy
        $scale = $params[0]+4;            # at least four more for proper round
        $params[2] = $r[2];               # round mode by caller or undef
        $fallback = 1;                    # to clear a/p afterwards
    } else {
        # the 4 below is empirical, and there might be cases where it is not
        # enough...
        $scale = abs($params[0] || $params[1]) + 4; # take whatever is defined
    }

    # when user set globals, they would interfere with our calculation, so
    # disable them and later re-enable them
    no strict 'refs';
    my $abr = "$class\::accuracy";
    my $ab = $$abr;
    $$abr = undef;
    my $pbr = "$class\::precision";
    my $pb = $$pbr;
    $$pbr = undef;
    # we also need to disable any set A or P on $x (_find_round_parameters took
    # them already into account), since these would interfere, too
    delete $x->{_a};
    delete $x->{_p};

    # Disabling upgrading and downgrading is no longer necessary to avoid an
    # infinite recursion, but it avoids unnecessary upgrading and downgrading in
    # the intermediate computations.

    local $Math::BigInt::upgrade = undef;
    local $Math::BigFloat::downgrade = undef;

    # remember sign and make $x positive, since -4 ** (1/2) => -2
    my $sign = 0;
    $sign = 1 if $x->{sign} eq '-';
    $x->{sign} = '+';

    my $is_two = 0;
    if ($y->isa('Math::BigFloat')) {
        $is_two = $y->{sign} eq '+' && $LIB->_is_two($y->{_m})
                    && $LIB->_is_zero($y->{_e});
    } else {
        $is_two = $y == 2;
    }

    # normal square root if $y == 2:
    if ($is_two) {
        $x = $x->bsqrt($scale+4);
    } elsif ($y->is_one('-')) {
        # $x ** -1 => 1/$x
        my $u = $class->bone()->bdiv($x, $scale);
        # copy private parts over
        $x->{_m} = $u->{_m};
        $x->{_e} = $u->{_e};
        $x->{_es} = $u->{_es};
    } else {
        # calculate the broot() as integer result first, and if it fits, return
        # it rightaway (but only if $x and $y are integer):

        my $done = 0;           # not yet
        if ($y->is_int() && $x->is_int()) {
            my $i = $LIB->_copy($x->{_m});
            $i = $LIB->_lsft($i, $x->{_e}, 10) unless $LIB->_is_zero($x->{_e});
            my $int = Math::BigInt->bzero();
            $int->{value} = $i;
            $int = $int->broot($y->as_number());
            # if ($exact)
            if ($int->copy()->bpow($y) == $x) {
                # found result, return it
                $x->{_m} = $int->{value};
                $x->{_e} = $LIB->_zero();
                $x->{_es} = '+';
                $x = $x->bnorm();
                $done = 1;
            }
        }
        if ($done == 0) {
            my $u = $class->bone()->bdiv($y, $scale+4);
            delete $u->{_a};
            delete $u->{_p};
            $x = $x->bpow($u, $scale+4);            # el cheapo
        }
    }
    $x = $x->bneg() if $sign == 1;

    # shortcut to not run through _find_round_parameters again
    if (defined $params[0]) {
        $x = $x->bround($params[0], $params[2]); # then round accordingly
    } else {
        $x = $x->bfround($params[1], $params[2]); # then round accordingly
    }
    if ($fallback) {
        # clear a/p after round, since user did not request it
        delete $x->{_a};
        delete $x->{_p};
    }
    # restore globals
    $$abr = $ab;
    $$pbr = $pb;

    return $downgrade -> new($x -> bdstr(), @r)
      if defined($downgrade) && ($x -> is_int() || $x -> is_inf());
    $x;
}

sub bfac {
    # (BFLOAT or num_str, BFLOAT or num_str) return BFLOAT
    # compute factorial number, modifies first argument

    # set up parameters
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    # inf => inf
    return $x if $x->modify('bfac');

    return $x -> bnan(@r)      if $x->is_nan()  || $x->is_inf("-");
    return $x -> binf("+", @r) if $x->is_inf("+");
    return $x -> bone(@r)      if $x->is_zero() || $x->is_one();

    if ($x -> is_neg() || !$x -> is_int()) {
        return $upgrade -> bfac($x, @r) if defined($upgrade);
        return $x -> bnan(@r);
    }

    if (! $LIB->_is_zero($x->{_e})) {
        $x->{_m} = $LIB->_lsft($x->{_m}, $x->{_e}, 10); # change 12e1 to 120e0
        $x->{_e} = $LIB->_zero();           # normalize
        $x->{_es} = '+';
    }
    $x->{_m} = $LIB->_fac($x->{_m});       # calculate factorial

    $x = $x->bnorm()->round(@r);     # norm again and round result

    return $downgrade -> new($x -> bdstr(), @r) if defined($downgrade)
      && ($x -> is_int() || $x -> is_inf());
    $x;
}

sub bdfac {
    # compute double factorial

    # set up parameters
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    return $x if $x->modify('bdfac');

    return $x -> bnan(@r)      if $x->is_nan()  || $x->is_inf("-");
    return $x -> binf("+", @r) if $x->is_inf("+");

    if ($x <= -2 || !$x -> is_int()) {
        return $upgrade -> bdfac($x, @r) if defined($upgrade);
        return $x -> bnan(@r);
    }

    return $x->bone() if $x <= 1;

    croak("bdfac() requires a newer version of the $LIB library.")
        unless $LIB->can('_dfac');

    if (! $LIB->_is_zero($x->{_e})) {
        $x->{_m} = $LIB->_lsft($x->{_m}, $x->{_e}, 10); # change 12e1 to 120e0
        $x->{_e} = $LIB->_zero();           # normalize
        $x->{_es} = '+';
    }
    $x->{_m} = $LIB->_dfac($x->{_m});       # calculate factorial

    $x = $x->bnorm()->round(@r);     # norm again and round result

    return $downgrade -> new($x -> bdstr(), @r)
      if defined($downgrade) && $x -> is_int();
    return $x;
}

sub btfac {
    # compute triple factorial

    # set up parameters
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    return $x if $x->modify('btfac');

    return $x -> bnan(@r)      if $x->is_nan()  || $x->is_inf("-");
    return $x -> binf("+", @r) if $x->is_inf("+");

    if ($x <= -3 || !$x -> is_int()) {
        return $upgrade -> btfac($x, @r) if defined($upgrade);
        return $x -> bnan(@r);
    }

    my $k = $class -> new("3");
    return $x->bnan(@r) if $x <= -$k;

    my $one = $class -> bone();
    return $x->bone(@r) if $x <= $one;

    my $f = $x -> copy();
    while ($f -> bsub($k) > $one) {
        $x = $x -> bmul($f);
    }

    $x = $x->round(@r);

    return $downgrade -> new($x -> bdstr(), @r)
      if defined($downgrade) && $x -> is_int();
    return $x;
}

sub bmfac {
    my ($class, $x, $k, @r) = ref($_[0]) && ref($_[0]) eq ref($_[1])
                            ? (ref($_[0]), @_)
                            : objectify(2, @_);

    return $x if $x->modify('bmfac');

    return $x -> bnan(@r) if $x->is_nan() || $x->is_inf("-") || !$k->is_pos();
    return $x -> binf("+", @r) if $x->is_inf("+");

    if ($x <= -$k || !$x -> is_int() ||
        ($k -> is_finite() && !$k -> is_int()))
    {
        return $upgrade -> bmfac($x, $k, @r) if defined($upgrade);
        return $x -> bnan(@r);
    }

    my $one = $class -> bone();
    return $x->bone(@r) if $x <= $one;

    my $f = $x -> copy();
    while ($f -> bsub($k) > $one) {
        $x = $x -> bmul($f);
    }

    $x = $x->round(@r);

    return $downgrade -> new($x -> bdstr(), @r)
      if defined($downgrade) && $x -> is_int();
    return $x;
}

sub blsft {
    # shift left by $y (multiply by $b ** $y)

    # set up parameters
    my ($class, $x, $y, $b, @r)
      = ref($_[0]) && ref($_[0]) eq ref($_[1]) && ref($_[1]) eq ref($_[2])
      ? (ref($_[0]), @_)
      : objectify(2, @_);

    return $x if $x -> modify('blsft');

    return $x -> bnan(@r) if $x -> is_nan() || $y -> is_nan();

    $b = 2 if !defined $b;
    $b = $class -> new($b) unless ref($b) && $b -> isa($class);
    return $x -> bnan(@r) if $b -> is_nan();

    # There needs to be more checking for special cases here. Fixme!

    # shift by a negative amount?
    return $x -> brsft($y -> copy() -> babs(), $b) if $y -> {sign} =~ /^-/;

    $x = $x -> bmul($b -> bpow($y), $r[0], $r[1], $r[2], $y);

    return $downgrade -> new($x -> bdstr(), @r) if defined($downgrade)
      && ($x -> is_int() || $x -> is_inf() || $x -> is_nan());
    return $x;
}

sub brsft {
    # shift right by $y (divide $b ** $y)

    # set up parameters
    my ($class, $x, $y, $b, @r)
      = ref($_[0]) && ref($_[0]) eq ref($_[1]) && ref($_[1]) eq ref($_[2])
      ? (ref($_[0]), @_)
      : objectify(2, @_);

    return $x if $x -> modify('brsft');

    return $x -> bnan(@r) if $x -> is_nan() || $y -> is_nan();

    # There needs to be more checking for special cases here. Fixme!

    $b = 2 if !defined $b;
    $b = $class -> new($b) unless ref($b) && $b -> isa($class);
    return $x -> bnan(@r) if $b -> is_nan();

    # shift by a negative amount?
    return $x -> blsft($y -> copy() -> babs(), $b) if $y -> {sign} =~ /^-/;

    # call bdiv()
    $x = $x -> bdiv($b -> bpow($y), $r[0], $r[1], $r[2], $y);

    return $downgrade -> new($x -> bdstr(), @r) if defined($downgrade)
      && ($x -> is_int() || $x -> is_inf() || $x -> is_nan());
    return $x;
}

###############################################################################
# Bitwise methods
###############################################################################

sub band {
    my ($class, $x, $y, @r) = ref($_[0]) && ref($_[0]) eq ref($_[1])
                            ? (ref($_[0]), @_)
                            : objectify(2, @_);

    return if $x -> modify('band');

    return $x -> bnan(@r) if $x -> is_nan() || $y -> is_nan();

    my $xtmp = Math::BigInt -> new($x -> bint());   # to Math::BigInt
    $xtmp = $xtmp -> band($y);

    return $xtmp -> round(@r) if defined $downgrade;

    $xtmp = $class -> new($xtmp);                   # back to Math::BigFloat
    $x -> {sign} = $xtmp -> {sign};
    $x -> {_m}   = $xtmp -> {_m};
    $x -> {_es}  = $xtmp -> {_es};
    $x -> {_e}   = $xtmp -> {_e};

    return $x -> round(@r);
}

sub bior {
    my ($class, $x, $y, @r) = ref($_[0]) && ref($_[0]) eq ref($_[1])
                            ? (ref($_[0]), @_)
                            : objectify(2, @_);

    return if $x -> modify('bior');

    return $x -> bnan(@r) if $x -> is_nan() || $y -> is_nan();

    my $xtmp = Math::BigInt -> new($x -> bint());   # to Math::BigInt
    $xtmp = $xtmp -> bior($y);

    return $xtmp -> round(@r) if defined $downgrade;

    $xtmp = $class -> new($xtmp);                   # back to Math::BigFloat
    $x -> {sign} = $xtmp -> {sign};
    $x -> {_m}   = $xtmp -> {_m};
    $x -> {_es}  = $xtmp -> {_es};
    $x -> {_e}   = $xtmp -> {_e};

    return $x -> round(@r);
}

sub bxor {
    my ($class, $x, $y, @r) = ref($_[0]) && ref($_[0]) eq ref($_[1])
                            ? (ref($_[0]), @_)
                            : objectify(2, @_);

    return if $x -> modify('bxor');

    return $x -> bnan(@r) if $x -> is_nan() || $y -> is_nan();

    my $xtmp = Math::BigInt -> new($x -> bint());   # to Math::BigInt
    $xtmp = $xtmp -> bxor($y);

    return $xtmp -> round(@r) if defined $downgrade;

    $xtmp = $class -> new($xtmp);                   # back to Math::BigFloat
    $x -> {sign} = $xtmp -> {sign};
    $x -> {_m}   = $xtmp -> {_m};
    $x -> {_es}  = $xtmp -> {_es};
    $x -> {_e}   = $xtmp -> {_e};

    return $x -> round(@r);
}

sub bnot {
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    return if $x -> modify('bnot');

    return $x -> bnan(@r) if $x -> is_nan();

    my $xtmp = Math::BigInt -> new($x -> bint());   # to Math::BigInt
    $xtmp = $xtmp -> bnot();

    return $xtmp -> round(@r) if defined $downgrade;

    $xtmp = $class -> new($xtmp);                   # back to Math::BigFloat
    $x -> {sign} = $xtmp -> {sign};
    $x -> {_m}   = $xtmp -> {_m};
    $x -> {_es}  = $xtmp -> {_es};
    $x -> {_e}   = $xtmp -> {_e};

    return $x -> round(@r);
}

###############################################################################
# Rounding methods
###############################################################################

sub bround {
    # accuracy: preserve $N digits, and overwrite the rest with 0's

    my ($class, $x, @a) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    if (($a[0] || 0) < 0) {
        croak('bround() needs positive accuracy');
    }

    return $x if $x->modify('bround');

    my ($scale, $mode) = $x->_scale_a(@a);
    if (!defined $scale) {         # no-op
        return $downgrade -> new($x) if defined($downgrade)
          && ($x->is_int() || $x->is_inf() || $x->is_nan());
        return $x;
    }

    # Scale is now either $x->{_a}, $accuracy, or the input argument. Test
    # whether $x already has lower accuracy, do nothing in this case but do
    # round if the accuracy is the same, since a math operation might want to
    # round a number with A=5 to 5 digits afterwards again

    if (defined $x->{_a} && $x->{_a} < $scale) {
        return $downgrade -> new($x) if defined($downgrade)
          && ($x->is_int() || $x->is_inf() || $x->is_nan());
        return $x;
    }

    # scale < 0 makes no sense
    # scale == 0 => keep all digits
    # never round a +-inf, NaN

    if ($scale <= 0 || $x->{sign} !~ /^[+-]$/) {
        return $downgrade -> new($x) if defined($downgrade)
          && ($x->is_int() || $x->is_inf() || $x->is_nan());
        return $x;
    }

    # 1: never round a 0
    # 2: if we should keep more digits than the mantissa has, do nothing
    if ($x->is_zero() || $LIB->_len($x->{_m}) <= $scale) {
        $x->{_a} = $scale if !defined $x->{_a} || $x->{_a} > $scale;
        return $downgrade -> new($x) if defined($downgrade)
          && ($x->is_int() || $x->is_inf() || $x->is_nan());
        return $x;
    }

    # pass sign to bround for '+inf' and '-inf' rounding modes
    my $m = bless { sign => $x->{sign}, value => $x->{_m} }, 'Math::BigInt';

    $m = $m->bround($scale, $mode);     # round mantissa
    $x->{_m} = $m->{value};             # get our mantissa back
    $x->{_a} = $scale;                  # remember rounding
    delete $x->{_p};                    # and clear P

    # bnorm() downgrades if necessary, so no need to check whether to downgrade.
    $x->bnorm();                # del trailing zeros gen. by bround()
}

sub bfround {
    # precision: round to the $Nth digit left (+$n) or right (-$n) from the '.'
    # $n == 0 means round to integer
    # expects and returns normalized numbers!

    my ($class, $x, @p) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    return $x if $x->modify('bfround'); # no-op

    my ($scale, $mode) = $x->_scale_p(@p);
    if (!defined $scale) {
        return $downgrade -> new($x) if defined($downgrade)
          && ($x->is_int() || $x->is_inf() || $x->is_nan());
        return $x;
    }

    # never round a 0, +-inf, NaN

    if ($x->is_zero()) {
        $x->{_p} = $scale if !defined $x->{_p} || $x->{_p} < $scale; # -3 < -2
        return $downgrade -> new($x) if defined($downgrade)
          && ($x->is_int() || $x->is_inf() || $x->is_nan());
        return $x;
    }

    if ($x->{sign} !~ /^[+-]$/) {
        return $downgrade -> new($x) if defined($downgrade)
          && ($x->is_int() || $x->is_inf() || $x->is_nan());
        return $x;
    }

    # don't round if x already has lower precision
    if (defined $x->{_p} && $x->{_p} < 0 && $scale < $x->{_p}) {
        return $downgrade -> new($x) if defined($downgrade)
          && ($x->is_int() || $x->is_inf() || $x->is_nan());
        return $x;
    }

    $x->{_p} = $scale;          # remember round in any case
    delete $x->{_a};            # and clear A
    if ($scale < 0) {
        # round right from the '.'

        if ($x->{_es} eq '+') { # e >= 0 => nothing to round
            return $downgrade -> new($x) if defined($downgrade)
              && ($x->is_int() || $x->is_inf() || $x->is_nan());
            return $x;
        }

        $scale = -$scale;           # positive for simplicity
        my $len = $LIB->_len($x->{_m}); # length of mantissa

        # the following poses a restriction on _e, but if _e is bigger than a
        # scalar, you got other problems (memory etc) anyway
        my $dad = -(0+ ($x->{_es}.$LIB->_num($x->{_e}))); # digits after dot
        my $zad = 0;                                      # zeros after dot
        $zad = $dad - $len if (-$dad < -$len); # for 0.00..00xxx style

        # print "scale $scale dad $dad zad $zad len $len\n";
        # number  bsstr   len zad dad
        # 0.123   123e-3    3   0 3
        # 0.0123  123e-4    3   1 4
        # 0.001   1e-3      1   2 3
        # 1.23    123e-2    3   0 2
        # 1.2345  12345e-4  5   0 4

        # do not round after/right of the $dad

        if ($scale > $dad) { # 0.123, scale >= 3 => exit
            return $downgrade -> new($x) if defined($downgrade)
              && ($x->is_int() || $x->is_inf() || $x->is_nan());
            return $x;
        }

        # round to zero if rounding inside the $zad, but not for last zero like:
        # 0.0065, scale -2, round last '0' with following '65' (scale == zad
        # case)
        if ($scale < $zad) {
            return $downgrade -> new($x) if defined($downgrade)
              && ($x->is_int() || $x->is_inf() || $x->is_nan());
            return $x->bzero();
        }

        if ($scale == $zad) {    # for 0.006, scale -3 and trunc
            $scale = -$len;
        } else {
            # adjust round-point to be inside mantissa
            if ($zad != 0) {
                $scale = $scale-$zad;
            } else {
                my $dbd = $len - $dad;
                $dbd = 0 if $dbd < 0; # digits before dot
                $scale = $dbd+$scale;
            }
        }
    } else {
        # round left from the '.'

        # 123 => 100 means length(123) = 3 - $scale (2) => 1

        my $dbt = $LIB->_len($x->{_m});
        # digits before dot
        my $dbd = $dbt + ($x->{_es} . $LIB->_num($x->{_e}));
        # should be the same, so treat it as this
        $scale = 1 if $scale == 0;
        # shortcut if already integer
        if ($scale == 1 && $dbt <= $dbd) {
            return $downgrade -> new($x) if defined($downgrade)
              && ($x->is_int() || $x->is_inf() || $x->is_nan());
            return $x;
        }
        # maximum digits before dot
        ++$dbd;

        if ($scale > $dbd) {
            # not enough digits before dot, so round to zero
            return $downgrade -> new($x) if defined($downgrade);
            return $x->bzero;
        } elsif ($scale == $dbd) {
            # maximum
            $scale = -$dbt;
        } else {
            $scale = $dbd - $scale;
        }
    }

    # pass sign to bround for rounding modes '+inf' and '-inf'
    my $m = bless { sign => $x->{sign}, value => $x->{_m} }, 'Math::BigInt';
    $m = $m->bround($scale, $mode);
    $x->{_m} = $m->{value};     # get our mantissa back

    # bnorm() downgrades if necessary, so no need to check whether to downgrade.
    $x->bnorm();
}

sub bfloor {
    # round towards minus infinity
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    return $x if $x->modify('bfloor');

    return $x -> bnan(@r) if $x -> is_nan();

    if ($x->{sign} =~ /^[+-]$/) {
        # if $x has digits after dot, remove them
        if ($x->{_es} eq '-') {
            $x->{_m} = $LIB->_rsft($x->{_m}, $x->{_e}, 10);
            $x->{_e} = $LIB->_zero();
            $x->{_es} = '+';
            # increment if negative
            $x->{_m} = $LIB->_inc($x->{_m}) if $x->{sign} eq '-';
        }
        $x = $x->round(@r);
    }
    return $downgrade -> new($x -> bdstr(), @r) if defined($downgrade);
    return $x;
}

sub bceil {
    # round towards plus infinity
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    return $x if $x->modify('bceil');

    return $x -> bnan(@r) if $x -> is_nan();

    # if $x has digits after dot, remove them
    if ($x->{sign} =~ /^[+-]$/) {
        if ($x->{_es} eq '-') {
            $x->{_m} = $LIB->_rsft($x->{_m}, $x->{_e}, 10);
            $x->{_e} = $LIB->_zero();
            $x->{_es} = '+';
            if ($x->{sign} eq '+') {
                $x->{_m} = $LIB->_inc($x->{_m});        # increment if positive
            } else {
                $x->{sign} = '+' if $LIB->_is_zero($x->{_m});   # avoid -0
            }
        }
        $x = $x->round(@r);
    }

    return $downgrade -> new($x -> bdstr(), @r) if defined($downgrade);
    return $x;
}

sub bint {
    # round towards zero
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    return $x if $x->modify('bint');

    return $x -> bnan(@r) if $x -> is_nan();

    if ($x->{sign} =~ /^[+-]$/) {
        # if $x has digits after the decimal point
        if ($x->{_es} eq '-') {
            $x->{_m} = $LIB->_rsft($x->{_m}, $x->{_e}, 10); # remove frac part
            $x->{_e} = $LIB->_zero();                       # truncate/normalize
            $x->{_es} = '+';                                # abs e
            $x->{sign} = '+' if $LIB->_is_zero($x->{_m});   # avoid -0
        }
        $x = $x->round(@r);
    }

    return $downgrade -> new($x -> bdstr(), @r) if defined($downgrade);
    return $x;
}

###############################################################################
# Other mathematical methods
###############################################################################

sub bgcd {
    # (BINT or num_str, BINT or num_str) return BINT
    # does not modify arguments, but returns new object

    # Class::method(...) -> Class->method(...)
    unless (@_ && (defined(blessed($_[0])) && $_[0] -> isa(__PACKAGE__) ||
                   $_[0] =~ /^[a-z]\w*(?:::[a-z]\w*)*$/i))
    {
        #carp "Using ", (caller(0))[3], "() as a function is deprecated;",
        #  " use is as a method instead";
        unshift @_, __PACKAGE__;
    }

    my ($class, @args) = objectify(0, @_);

    my $x = shift @args;
    $x = ref($x) && $x -> isa($class) ? $x -> copy() : $class -> new($x);
    return $class->bnan() unless $x -> is_int();

    while (@args) {
        my $y = shift @args;
        $y = $class->new($y) unless ref($y) && $y -> isa($class);
        return $class->bnan() unless $y -> is_int();

        # greatest common divisor
        while (! $y->is_zero()) {
            ($x, $y) = ($y->copy(), $x->copy()->bmod($y));
        }

        last if $x -> is_one();
    }
    $x = $x -> babs();

    return $downgrade -> new($x)
      if defined $downgrade && $x->is_int();
    return $x;
}

sub blcm {
    # (BFLOAT or num_str, BFLOAT or num_str) return BFLOAT
    # does not modify arguments, but returns new object
    # Least Common Multiple

    # Class::method(...) -> Class->method(...)
    unless (@_ && (defined(blessed($_[0])) && $_[0] -> isa(__PACKAGE__) ||
                   $_[0] =~ /^[a-z]\w*(?:::[a-z]\w*)*$/i))
    {
        #carp "Using ", (caller(0))[3], "() as a function is deprecated;",
        #  " use is as a method instead";
        unshift @_, __PACKAGE__;
    }

    my ($class, @args) = objectify(0, @_);

    my $x = shift @args;
    $x = ref($x) && $x -> isa($class) ? $x -> copy() : $class -> new($x);
    return $class->bnan() if $x->{sign} !~ /^[+-]$/;    # x NaN?

    while (@args) {
        my $y = shift @args;
        $y = $class -> new($y) unless ref($y) && $y -> isa($class);
        return $x->bnan() unless $y -> is_int();
        my $gcd = $x -> bgcd($y);
        $x = $x -> bdiv($gcd) -> bmul($y);
    }

    $x = $x -> babs();

    return $downgrade -> new($x)
      if defined $downgrade && $x->is_int();
    return $x;
}

###############################################################################
# Object property methods
###############################################################################

sub length {
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    return 1 if $LIB->_is_zero($x->{_m});

    my $len = $LIB->_len($x->{_m});
    $len += $LIB->_num($x->{_e}) if $x->{_es} eq '+';
    if (wantarray()) {
        my $t = 0;
        $t = $LIB->_num($x->{_e}) if $x->{_es} eq '-';
        return ($len, $t);
    }
    $len;
}

sub mantissa {
    # return a copy of the mantissa
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    # The following line causes a lot of noise in the test suits for
    # the Math-BigRat and bignum distributions. Fixme!
    #carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    return $x -> bnan(@r) if $x -> is_nan();

    if ($x->{sign} !~ /^[+-]$/) {
        my $s = $x->{sign};
        $s =~ s/^\+//;
        return Math::BigInt->new($s, undef, undef); # -inf, +inf => +inf
    }
    my $m = Math::BigInt->new($LIB->_str($x->{_m}), undef, undef);
    $m = $m->bneg() if $x->{sign} eq '-';
    $m;
}

sub exponent {
    # return a copy of the exponent
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    # The following line causes a lot of noise in the test suits for
    # the Math-BigRat and bignum distributions. Fixme!
    #carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    return $x -> bnan(@r) if $x -> is_nan();

    if ($x->{sign} !~ /^[+-]$/) {
        my $s = $x->{sign};
        $s =~ s/^[+-]//;
        return Math::BigInt->new($s, undef, undef); # -inf, +inf => +inf
    }
    Math::BigInt->new($x->{_es} . $LIB->_str($x->{_e}), undef, undef);
}

sub parts {
    # return a copy of both the exponent and the mantissa
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    if ($x->{sign} !~ /^[+-]$/) {
        my $s = $x->{sign};
        $s =~ s/^\+//;
        my $se = $s;
        $se =~ s/^-//;
        # +inf => inf and -inf, +inf => inf
        return ($class->new($s), $class->new($se));
    }
    my $m = Math::BigInt->bzero();
    $m->{value} = $LIB->_copy($x->{_m});
    $m = $m->bneg() if $x->{sign} eq '-';
    ($m, Math::BigInt->new($x->{_es} . $LIB->_num($x->{_e})));
}

# Parts used for scientific notation with significand/mantissa and exponent as
# integers. E.g., "12345.6789" is returned as "123456789" (mantissa) and "-4"
# (exponent).

sub sparts {
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    # Not-a-number.

    if ($x -> is_nan()) {
        my $mant = $class -> bnan();            # mantissa
        return $mant unless wantarray;          # scalar context
        my $expo = $class -> bnan();            # exponent
        return ($mant, $expo);                  # list context
    }

    # Infinity.

    if ($x -> is_inf()) {
        my $mant = $class -> binf($x->{sign});  # mantissa
        return $mant unless wantarray;          # scalar context
        my $expo = $class -> binf('+');         # exponent
        return ($mant, $expo);                  # list context
    }

    # Finite number.

    my $mant = $x -> copy();
    $mant->{_es} = '+';
    $mant->{_e}  = $LIB->_zero();
    $mant = $downgrade -> new($mant) if defined $downgrade;
    return $mant unless wantarray;

    my $expo = bless { sign => $x -> {_es},
                       _m   => $LIB->_copy($x -> {_e}),
                       _es  => '+',
                       _e   => $LIB->_zero(),
                     }, $class;
    $expo = $downgrade -> new($expo) if defined $downgrade;
    return ($mant, $expo);
}

# Parts used for normalized notation with significand/mantissa as either 0 or a
# number in the semi-open interval [1,10). E.g., "12345.6789" is returned as
# "1.23456789" and "4".

sub nparts {
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    # Not-a-number and Infinity.

    return $x -> sparts() if $x -> is_nan() || $x -> is_inf();

    # Finite number.

    my ($mant, $expo) = $x -> sparts();

    if ($mant -> bcmp(0)) {
        my ($ndigtot, $ndigfrac) = $mant -> length();
        my $expo10adj = $ndigtot - $ndigfrac - 1;

        if ($expo10adj > 0) {          # if mantissa is not an integer
            $mant = $mant -> brsft($expo10adj, 10);
            return $mant unless wantarray;
            $expo = $expo -> badd($expo10adj);
            return ($mant, $expo);
        }
    }

    return $mant unless wantarray;
    return ($mant, $expo);
}

# Parts used for engineering notation with significand/mantissa as either 0 or a
# number in the semi-open interval [1,1000) and the exponent is a multiple of 3.
# E.g., "12345.6789" is returned as "12.3456789" and "3".

sub eparts {
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    # Not-a-number and Infinity.

    return $x -> sparts() if $x -> is_nan() || $x -> is_inf();

    # Finite number.

    my ($mant, $expo) = $x -> nparts();

    my $c = $expo -> copy() -> bmod(3);
    $mant = $mant -> blsft($c, 10);
    return $mant unless wantarray;

    $expo = $expo -> bsub($c);
    return ($mant, $expo);
}

# Parts used for decimal notation, e.g., "12345.6789" is returned as "12345"
# (integer part) and "0.6789" (fraction part).

sub dparts {
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    # Not-a-number.

    if ($x -> is_nan()) {
        my $int = $class -> bnan();
        return $int unless wantarray;
        my $frc = $class -> bzero();    # or NaN?
        return ($int, $frc);
    }

    # Infinity.

    if ($x -> is_inf()) {
        my $int = $class -> binf($x->{sign});
        return $int unless wantarray;
        my $frc = $class -> bzero();
        return ($int, $frc);
    }

    # Finite number.

    my $int = $x -> copy();
    my $frc;

    # If the input is an integer.

    if ($int->{_es} eq '+') {
        $frc = $class -> bzero();
    }

    # If the input has a fraction part

    else {
        $int->{_m} = $LIB -> _rsft($int->{_m}, $int->{_e}, 10);
        $int->{_e} = $LIB -> _zero();
        $int->{_es} = '+';
        $int->{sign} = '+' if $LIB->_is_zero($int->{_m});   # avoid -0
        return $int unless wantarray;
        $frc = $x -> copy() -> bsub($int);
        return ($int, $frc);
    }

    $int = $downgrade -> new($int) if defined $downgrade;
    return $int unless wantarray;
    return $int, $frc;
}

# Fractional parts with the numerator and denominator as integers. E.g.,
# "123.4375" is returned as "1975" and "16".

sub fparts {
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    # NaN => NaN/NaN

    if ($x -> is_nan()) {
        return $class -> bnan() unless wantarray;
        return $class -> bnan(), $class -> bnan();
    }

    # ±Inf => ±Inf/1

    if ($x -> is_inf()) {
        my $numer = $class -> binf($x->{sign});
        return $numer unless wantarray;
        my $denom = $class -> bone();
        return $numer, $denom;
    }

    # Finite number.

    # If we get here, we know that the output is an integer.

    $class = $downgrade if defined $downgrade;

    my @flt_parts = ($x->{sign}, $x->{_m}, $x->{_es}, $x->{_e});
    my @rat_parts = $class -> _flt_lib_parts_to_rat_lib_parts(@flt_parts);
    my $num = $class -> new($LIB -> _str($rat_parts[1]));
    my $den = $class -> new($LIB -> _str($rat_parts[2]));
    $num = $num -> bneg() if $rat_parts[0] eq "-";
    return $num unless wantarray;
    return $num, $den;
}

# Given "123.4375", returns "1975", since "123.4375" is "1975/16".

sub numerator {
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    return $class -> bnan()             if $x -> is_nan();
    return $class -> binf($x -> sign()) if $x -> is_inf();
    return $class -> bzero()            if $x -> is_zero();

    # If we get here, we know that the output is an integer.

    $class = $downgrade if defined $downgrade;

    if ($x -> {_es} eq '-') {                   # exponent < 0
        my $numer_lib = $LIB -> _copy($x -> {_m});
        my $denom_lib = $LIB -> _1ex($x -> {_e});
        my $gcd_lib = $LIB -> _gcd($LIB -> _copy($numer_lib), $denom_lib);
        $numer_lib = $LIB -> _div($numer_lib, $gcd_lib);
        return $class -> new($x -> {sign} . $LIB -> _str($numer_lib));
    }

    elsif (! $LIB -> _is_zero($x -> {_e})) {    # exponent > 0
        my $numer_lib = $LIB -> _copy($x -> {_m});
        $numer_lib = $LIB -> _lsft($numer_lib, $x -> {_e}, 10);
        return $class -> new($x -> {sign} . $LIB -> _str($numer_lib));
    }

    else {                                      # exponent = 0
        return $class -> new($x -> {sign} . $LIB -> _str($x -> {_m}));
    }
}

# Given "123.4375", returns "16", since "123.4375" is "1975/16".

sub denominator {
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    return $class -> bnan() if $x -> is_nan();

    # If we get here, we know that the output is an integer.

    $class = $downgrade if defined $downgrade;

    if ($x -> {_es} eq '-') {                   # exponent < 0
        my $numer_lib = $LIB -> _copy($x -> {_m});
        my $denom_lib = $LIB -> _1ex($x -> {_e});
        my $gcd_lib = $LIB -> _gcd($LIB -> _copy($numer_lib), $denom_lib);
        $denom_lib = $LIB -> _div($denom_lib, $gcd_lib);
        return $class -> new($LIB -> _str($denom_lib));
    }

    else {                                      # exponent >= 0
        return $class -> bone();
    }
}

###############################################################################
# String conversion methods
###############################################################################

sub bstr {
    # (ref to BFLOAT or num_str) return num_str
    # Convert number from internal format to (non-scientific) string format.
    # internal format is always normalized (no leading zeros, "-0" => "+0")
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    # Inf and NaN

    if ($x->{sign} ne '+' && $x->{sign} ne '-') {
        return $x->{sign} unless $x->{sign} eq '+inf';  # -inf, NaN
        return 'inf';                                   # +inf
    }

    # Finite number

    my $es = '0';
    my $len = 1;
    my $cad = 0;
    my $dot = '.';

    # $x is zero?
    my $not_zero = !($x->{sign} eq '+' && $LIB->_is_zero($x->{_m}));
    if ($not_zero) {
        $es = $LIB->_str($x->{_m});
        $len = CORE::length($es);
        my $e = $LIB->_num($x->{_e});
        $e = -$e if $x->{_es} eq '-';
        if ($e < 0) {
            $dot = '';
            # if _e is bigger than a scalar, the following will blow your memory
            if ($e <= -$len) {
                my $r = abs($e) - $len;
                $es = '0.'. ('0' x $r) . $es;
                $cad = -($len+$r);
            } else {
                substr($es, $e, 0) = '.';
                $cad = $LIB->_num($x->{_e});
                $cad = -$cad if $x->{_es} eq '-';
            }
        } elsif ($e > 0) {
            # expand with zeros
            $es .= '0' x $e;
            $len += $e;
            $cad = 0;
        }
    }                           # if not zero

    $es = '-'.$es if $x->{sign} eq '-';
    # if set accuracy or precision, pad with zeros on the right side
    if ((defined $x->{_a}) && ($not_zero)) {
        # 123400 => 6, 0.1234 => 4, 0.001234 => 4
        my $zeros = $x->{_a} - $cad; # cad == 0 => 12340
        $zeros = $x->{_a} - $len if $cad != $len;
        $es .= $dot.'0' x $zeros if $zeros > 0;
    } elsif ((($x->{_p} || 0) < 0)) {
        # 123400 => 6, 0.1234 => 4, 0.001234 => 6
        my $zeros = -$x->{_p} + $cad;
        $es .= $dot.'0' x $zeros if $zeros > 0;
    }
    $es;
}

# Decimal notation, e.g., "12345.6789" (no exponent).

sub bdstr {
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    # Inf and NaN

    if ($x->{sign} ne '+' && $x->{sign} ne '-') {
        return $x->{sign} unless $x->{sign} eq '+inf';  # -inf, NaN
        return 'inf';                                   # +inf
    }

    # Upgrade?

    return $upgrade -> bdstr($x, @r)
      if defined($upgrade) && !$x -> isa($class);

    # Finite number

    my $mant = $LIB->_str($x->{_m});
    my $esgn = $x->{_es};
    my $eabs = $LIB -> _num($x->{_e});

    my $uintmax = ~0;

    my $str = $mant;
    if ($esgn eq '+') {

        croak("The absolute value of the exponent is too large")
          if $eabs > $uintmax;

        $str .= "0" x $eabs;

    } else {
        my $mlen = CORE::length($mant);
        my $c = $mlen - $eabs;

        my $intmax = ($uintmax - 1) / 2;
        croak("The absolute value of the exponent is too large")
          if (1 - $c) > $intmax;

        $str = "0" x (1 - $c) . $str if $c <= 0;
        substr($str, -$eabs, 0) = '.';
    }

    return $x->{sign} eq '-' ? '-' . $str : $str;
}

# Scientific notation with significand/mantissa and exponent as integers, e.g.,
# "12345.6789" is written as "123456789e-4".

sub bsstr {
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    # Inf and NaN

    if ($x->{sign} ne '+' && $x->{sign} ne '-') {
        return $x->{sign} unless $x->{sign} eq '+inf';  # -inf, NaN
        return 'inf';                                   # +inf
    }

    # Upgrade?

    return $upgrade -> bsstr($x, @r)
      if defined($upgrade) && !$x -> isa($class);

    # Finite number

    ($x->{sign} eq '-' ? '-' : '') . $LIB->_str($x->{_m})
      . 'e' . $x->{_es} . $LIB->_str($x->{_e});
}

# Normalized notation, e.g., "12345.6789" is written as "1.23456789e+4".

sub bnstr {
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    # Inf and NaN

    if ($x->{sign} ne '+' && $x->{sign} ne '-') {
        return $x->{sign} unless $x->{sign} eq '+inf';  # -inf, NaN
        return 'inf';                                   # +inf
    }

    # Upgrade?

    return $upgrade -> bnstr($x, @r)
      if defined($upgrade) && !$x -> isa($class);

    # Finite number

    my $str = $x->{sign} eq '-' ? '-' : '';

    # Get the mantissa and the length of the mantissa.

    my $mant = $LIB->_str($x->{_m});
    my $mantlen = CORE::length($mant);

    if ($mantlen == 1) {

        # Not decimal point when the mantissa has length one, i.e., return the
        # number 2 as the string "2", not "2.".

        $str .= $mant . 'e' . $x->{_es} . $LIB->_str($x->{_e});

    } else {

        # Compute new exponent where the original exponent is adjusted by the
        # length of the mantissa minus one (because the decimal point is after
        # one digit).

        my ($eabs, $esgn) = $LIB -> _sadd($LIB -> _copy($x->{_e}), $x->{_es},
                                      $LIB -> _new($mantlen - 1), "+");
        substr $mant, 1, 0, ".";
        $str .= $mant . 'e' . $esgn . $LIB->_str($eabs);

    }

    return $str;
}

# Engineering notation, e.g., "12345.6789" is written as "12.3456789e+3".

sub bestr {
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    # Inf and NaN

    if ($x->{sign} ne '+' && $x->{sign} ne '-') {
        return $x->{sign} unless $x->{sign} eq '+inf';  # -inf, NaN
        return 'inf';                                   # +inf
    }

    # Upgrade?

    return $upgrade -> bestr($x, @r)
      if defined($upgrade) && !$x -> isa($class);

    # Finite number

    my $str = $x->{sign} eq '-' ? '-' : '';

    # Get the mantissa, the length of the mantissa, and adjust the exponent by
    # the length of the mantissa minus 1 (because the dot is after one digit).

    my $mant = $LIB->_str($x->{_m});
    my $mantlen = CORE::length($mant);
    my ($eabs, $esgn) = $LIB -> _sadd($LIB -> _copy($x->{_e}), $x->{_es},
                                  $LIB -> _new($mantlen - 1), "+");

    my $dotpos = 1;
    my $mod = $LIB -> _mod($LIB -> _copy($eabs), $LIB -> _new("3"));
    unless ($LIB -> _is_zero($mod)) {
        if ($esgn eq '+') {
            $eabs = $LIB -> _sub($eabs, $mod);
            $dotpos += $LIB -> _num($mod);
        } else {
            my $delta = $LIB -> _sub($LIB -> _new("3"), $mod);
            $eabs = $LIB -> _add($eabs, $delta);
            $dotpos += $LIB -> _num($delta);
        }
    }

    if ($dotpos < $mantlen) {
        substr $mant, $dotpos, 0, ".";
    } elsif ($dotpos > $mantlen) {
        $mant .= "0" x ($dotpos - $mantlen);
    }

    $str .= $mant . 'e' . $esgn . $LIB->_str($eabs);

    return $str;
}

# Fractional notation, e.g., "123.4375" is written as "1975/16".

sub bfstr {
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), $_[0]) : objectify(1, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    # Inf and NaN

    if ($x->{sign} ne '+' && $x->{sign} ne '-') {
        return $x->{sign} unless $x->{sign} eq '+inf';  # -inf, NaN
        return 'inf';                                   # +inf
    }

    # Upgrade?

    return $upgrade -> bfstr($x, @r)
      if defined($upgrade) && !$x -> isa($class);

    # Finite number

    my $str = $x->{sign} eq '-' ? '-' : '';

    if ($x->{_es} eq '+') {
        $str .= $LIB -> _str($x->{_m}) . ("0" x $LIB -> _num($x->{_e}));
    } else {
        my @flt_parts = ($x->{sign}, $x->{_m}, $x->{_es}, $x->{_e});
        my @rat_parts = $class -> _flt_lib_parts_to_rat_lib_parts(@flt_parts);
        $str = $LIB -> _str($rat_parts[1]) . "/" . $LIB -> _str($rat_parts[2]);
        $str = "-" . $str if $rat_parts[0] eq "-";
    }

    return $str;
}

sub to_hex {
    # return number as hexadecimal string (only for integers defined)
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), $_[0]) : objectify(1, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    # Inf and NaN

    if ($x->{sign} ne '+' && $x->{sign} ne '-') {
        return $x->{sign} unless $x->{sign} eq '+inf';  # -inf, NaN
        return 'inf';                                   # +inf
    }

    # Upgrade?

    return $upgrade -> to_hex($x, @r)
      if defined($upgrade) && !$x -> isa($class);

    # Finite number

    return '0' if $x->is_zero();

    return $nan if $x->{_es} ne '+';    # how to do 1e-1 in hex?

    my $z = $LIB->_copy($x->{_m});
    if (! $LIB->_is_zero($x->{_e})) {   # > 0
        $z = $LIB->_lsft($z, $x->{_e}, 10);
    }
    my $str = $LIB->_to_hex($z);
    return $x->{sign} eq '-' ? "-$str" : $str;
}

sub to_oct {
    # return number as octal digit string (only for integers defined)
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), $_[0]) : objectify(1, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    # Inf and NaN

    if ($x->{sign} ne '+' && $x->{sign} ne '-') {
        return $x->{sign} unless $x->{sign} eq '+inf';  # -inf, NaN
        return 'inf';                                   # +inf
    }

    # Upgrade?

    return $upgrade -> to_hex($x, @r)
      if defined($upgrade) && !$x -> isa($class);

    # Finite number

    return '0' if $x->is_zero();

    return $nan if $x->{_es} ne '+';    # how to do 1e-1 in octal?

    my $z = $LIB->_copy($x->{_m});
    if (! $LIB->_is_zero($x->{_e})) {   # > 0
        $z = $LIB->_lsft($z, $x->{_e}, 10);
    }
    my $str = $LIB->_to_oct($z);
    return $x->{sign} eq '-' ? "-$str" : $str;
}

sub to_bin {
    # return number as binary digit string (only for integers defined)
    my ($class, $x, @r) = ref($_[0]) ? (ref($_[0]), $_[0]) : objectify(1, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    # Inf and NaN

    if ($x->{sign} ne '+' && $x->{sign} ne '-') {
        return $x->{sign} unless $x->{sign} eq '+inf';  # -inf, NaN
        return 'inf';                                   # +inf
    }

    # Upgrade?

    return $upgrade -> to_hex($x, @r)
      if defined($upgrade) && !$x -> isa($class);

    # Finite number

    return '0' if $x->is_zero();

    return $nan if $x->{_es} ne '+';    # how to do 1e-1 in binary?

    my $z = $LIB->_copy($x->{_m});
    if (! $LIB->_is_zero($x->{_e})) {   # > 0
        $z = $LIB->_lsft($z, $x->{_e}, 10);
    }
    my $str = $LIB->_to_bin($z);
    return $x->{sign} eq '-' ? "-$str" : $str;
}

sub to_ieee754 {
    my ($class, $x, $format, @r) = ref($_[0]) ? (ref($_[0]), @_) : objectify(1, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    my $enc;            # significand encoding (applies only to decimal)
    my $k;              # storage width in bits
    my $b;              # base

    if ($format =~ /^binary(\d+)\z/) {
        $k = $1;
        $b = 2;
    } elsif ($format =~ /^decimal(\d+)(dpd|bcd)?\z/) {
        $k = $1;
        $b = 10;
        $enc = $2 || 'dpd';     # default is dencely-packed decimals (DPD)
    } elsif ($format eq 'half') {
        $k = 16;
        $b = 2;
    } elsif ($format eq 'single') {
        $k = 32;
        $b = 2;
    } elsif ($format eq 'double') {
        $k = 64;
        $b = 2;
    } elsif ($format eq 'quadruple') {
        $k = 128;
        $b = 2;
    } elsif ($format eq 'octuple') {
        $k = 256;
        $b = 2;
    } elsif ($format eq 'sexdecuple') {
        $k = 512;
        $b = 2;
    }

    if ($b == 2) {

        # Get the parameters for this format.

        my $p;                      # precision (in bits)
        my $t;                      # number of bits in significand
        my $w;                      # number of bits in exponent

        if ($k == 16) {             # binary16 (half-precision)
            $p = 11;
            $t = 10;
            $w =  5;
        } elsif ($k == 32) {        # binary32 (single-precision)
            $p = 24;
            $t = 23;
            $w =  8;
        } elsif ($k == 64) {        # binary64 (double-precision)
            $p = 53;
            $t = 52;
            $w = 11;
        } else {                    # binaryN (quadruple-precition and above)
            if ($k < 128 || $k != 32 * sprintf('%.0f', $k / 32)) {
                croak "Number of bits must be 16, 32, 64, or >= 128 and",
                  " a multiple of 32";
            }
            $p = $k - sprintf('%.0f', 4 * log($k) / log(2)) + 13;
            $t = $p - 1;
            $w = $k - $t - 1;
        }

        # The maximum exponent, minimum exponent, and exponent bias.

        my $emax = $class -> new(2) -> bpow($w - 1) -> bdec();
        my $emin = 1 - $emax;
        my $bias = $emax;

        # Get numerical sign, exponent, and mantissa/significand for bit
        # string.

        my $sign = 0;
        my $expo;
        my $mant;

        if ($x -> is_nan()) {                   # nan
            $sign = 1;
            $expo = $emax -> copy() -> binc();
            $mant = $class -> new(2) -> bpow($t - 1);
        } elsif ($x -> is_inf()) {              # inf
            $sign = 1 if $x -> is_neg();
            $expo = $emax -> copy() -> binc();
            $mant = $class -> bzero();
        } elsif ($x -> is_zero()) {             # zero
            $expo = $emin -> copy() -> bdec();
            $mant = $class -> bzero();
        } else {                                # normal and subnormal

            $sign = 1 if $x -> is_neg();

            # Now we need to compute the mantissa and exponent in base $b.

            my $binv = $class -> new("0.5");
            my $b    = $class -> new(2);
            my $one  = $class -> bone();

            # We start off by initializing the exponent to zero and the
            # mantissa to the input value. Then we increase the mantissa and
            # decrease the exponent, or vice versa, until the mantissa is in
            # the desired range or we hit one of the limits for the exponent.

            $mant = $x -> copy() -> babs();

            # We need to find the base 2 exponent. First make an estimate of
            # the base 2 exponent, before adjusting it below. We could skip
            # this estimation and go straight to the while-loops below, but the
            # loops are slow, especially when the final exponent is far from
            # zero and even more so if the number of digits is large. This
            # initial estimation speeds up the computation dramatically.
            #
            #   log2($m * 10**$e) = log10($m + 10**$e) * log(10)/log(2)
            #                     = (log10($m) + $e) * log(10)/log(2)
            #                     = (log($m)/log(10) + $e) * log(10)/log(2)

            my ($m, $e) = $x -> nparts();
            my $ms = $m -> numify();
            my $es = $e -> numify();

            my $expo_est = (log(abs($ms))/log(10) + $es) * log(10)/log(2);
            $expo_est = int($expo_est);

            # Limit the exponent.

            if ($expo_est > $emax) {
                $expo_est = $emax;
            } elsif ($expo_est < $emin) {
                $expo_est = $emin;
            }

            # Don't multiply by a number raised to a negative exponent. This
            # will cause a division, whose result is truncated to some fixed
            # number of digits. Instead, multiply by the inverse number raised
            # to a positive exponent.

            $expo = $class -> new($expo_est);
            if ($expo_est > 0) {
                $mant = $mant -> bmul($binv -> copy() -> bpow($expo));
            } elsif ($expo_est < 0) {
                my $expo_abs = $expo -> copy() -> bneg();
                $mant = $mant -> bmul($b -> copy() -> bpow($expo_abs));
            }

            # Final adjustment of the estimate above.

            while ($mant >= $b && $expo <= $emax) {
                $mant = $mant -> bmul($binv);
                $expo = $expo -> binc();
            }

            while ($mant < $one && $expo >= $emin) {
                $mant = $mant -> bmul($b);
                $expo = $expo -> bdec();
            }

            # This is when the magnitude is larger than what can be represented
            # in this format. Encode as infinity.

            if ($expo > $emax) {
                $mant = $class -> bzero();
                $expo = $emax -> copy() -> binc();
            }

            # This is when the magnitude is so small that the number is encoded
            # as a subnormal number.
            #
            # If the magnitude is smaller than that of the smallest subnormal
            # number, and rounded downwards, it is encoded as zero. This works
            # transparently and does not need to be treated as a special case.
            #
            # If the number is between the largest subnormal number and the
            # smallest normal number, and the value is rounded upwards, the
            # value must be encoded as a normal number. This must be treated as
            # a special case.

            elsif ($expo < $emin) {

                # Scale up the mantissa (significand), and round to integer.

                my $const = $class -> new($b) -> bpow($t - 1);
                $mant = $mant -> bmul($const);
                $mant = $mant -> bfround(0);

                # If the mantissa overflowed, encode as the smallest normal
                # number.

                if ($mant == $const -> bmul($b)) {
                    $mant = $mant -> bzero();
                    $expo = $expo -> binc();
                }
            }

            # This is when the magnitude is within the range of what can be
            # encoded as a normal number.

            else {

                # Remove implicit leading bit, scale up the mantissa
                # (significand) to an integer, and round.

                $mant = $mant -> bdec();
                my $const = $class -> new($b) -> bpow($t);
                $mant = $mant -> bmul($const) -> bfround(0);

                # If the mantissa overflowed, encode as the next larger value.
                # This works correctly also when the next larger value is
                # infinity.

                if ($mant == $const) {
                    $mant = $mant -> bzero();
                    $expo = $expo -> binc();
                }
            }
        }

        $expo = $expo -> badd($bias);           # add bias

        my $signbit = "$sign";

        my $mantbits = $mant -> to_bin();
        $mantbits = ("0" x ($t - CORE::length($mantbits))) . $mantbits;

        my $expobits = $expo -> to_bin();
        $expobits = ("0" x ($w - CORE::length($expobits))) . $expobits;

        my $bin = $signbit . $expobits . $mantbits;
        return pack "B*", $bin;
    }

    croak("The format '$format' is not yet supported.");
}

sub as_hex {
    # return number as hexadecimal string (only for integers defined)

    my (undef, $x, @r) = ref($_[0]) ? (undef, @_) : objectify(1, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    return $x->bstr() if $x->{sign} !~ /^[+-]$/; # inf, nan etc
    return '0x0' if $x->is_zero();

    return $nan if $x->{_es} ne '+';    # how to do 1e-1 in hex?

    my $z = $LIB->_copy($x->{_m});
    if (! $LIB->_is_zero($x->{_e})) {   # > 0
        $z = $LIB->_lsft($z, $x->{_e}, 10);
    }
    my $str = $LIB->_as_hex($z);
    return $x->{sign} eq '-' ? "-$str" : $str;
}

sub as_oct {
    # return number as octal digit string (only for integers defined)

    my (undef, $x, @r) = ref($_[0]) ? (undef, @_) : objectify(1, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    return $x->bstr() if $x->{sign} !~ /^[+-]$/; # inf, nan etc
    return '00' if $x->is_zero();

    return $nan if $x->{_es} ne '+';    # how to do 1e-1 in octal?

    my $z = $LIB->_copy($x->{_m});
    if (! $LIB->_is_zero($x->{_e})) {   # > 0
        $z = $LIB->_lsft($z, $x->{_e}, 10);
    }
    my $str = $LIB->_as_oct($z);
    return $x->{sign} eq '-' ? "-$str" : $str;
}

sub as_bin {
    # return number as binary digit string (only for integers defined)

    my (undef, $x, @r) = ref($_[0]) ? (undef, @_) : objectify(1, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    return $x->bstr() if $x->{sign} !~ /^[+-]$/; # inf, nan etc
    return '0b0' if $x->is_zero();

    return $nan if $x->{_es} ne '+';    # how to do 1e-1 in binary?

    my $z = $LIB->_copy($x->{_m});
    if (! $LIB->_is_zero($x->{_e})) {   # > 0
        $z = $LIB->_lsft($z, $x->{_e}, 10);
    }
    my $str = $LIB->_as_bin($z);
    return $x->{sign} eq '-' ? "-$str" : $str;
}

sub numify {
    # Make a Perl scalar number from a Math::BigFloat object.

    my (undef, $x, @r) = ref($_[0]) ? (undef, @_) : objectify(1, @_);

    carp "Rounding is not supported for ", (caller(0))[3], "()" if @r;

    if ($x -> is_nan()) {
        require Math::Complex;
        my $inf = $Math::Complex::Inf;
        return $inf - $inf;
    }

    if ($x -> is_inf()) {
        require Math::Complex;
        my $inf = $Math::Complex::Inf;
        return $x -> is_negative() ? -$inf : $inf;
    }

    # Create a string and let Perl's atoi()/atof() handle the rest.

    return 0 + $x -> bnstr();
}

###############################################################################
# Private methods and functions.
###############################################################################

sub import {
    my $class = shift;
    $IMPORT++;                  # remember we did import()

    my @import = ('objectify');
    my @a;                      # unrecognized arguments

    while (@_) {
        my $param = shift;

        # Enable overloading of constants.

        if ($param eq ':constant') {
            overload::constant

                integer => sub {
                    $class -> new(shift);
                },

                float   => sub {
                    $class -> new(shift);
                },

                binary  => sub {
                    # E.g., a literal 0377 shall result in an object whose value
                    # is decimal 255, but new("0377") returns decimal 377.
                    return $class -> from_oct($_[0]) if $_[0] =~ /^0_*[0-7]/;
                    $class -> new(shift);
                };
            next;
        }

        # Upgrading.

        if ($param eq 'upgrade') {
            $class -> upgrade(shift);
            next;
        }

        # Downgrading.

        if ($param eq 'downgrade') {
            $class -> downgrade(shift);
            next;
        }

        # Accuracy.

        if ($param eq 'accuracy') {
            $class -> accuracy(shift);
            next;
        }

        # Precision.

        if ($param eq 'precision') {
            $class -> precision(shift);
            next;
        }

        # Rounding mode.

        if ($param eq 'round_mode') {
            $class -> round_mode(shift);
            next;
        }

        # Backend library.

        if ($param =~ /^(lib|try|only)\z/) {
            push @import, $param;
            push @import, shift() if @_;
            next;
        }

        if ($param eq 'with') {
            # alternative class for our private parts()
            # XXX: no longer supported
            # $LIB = shift() || 'Calc';
            # carp "'with' is no longer supported, use 'lib', 'try', or 'only'";
            shift;
            next;
        }

        # Unrecognized parameter.

        push @a, $param;
    }

    Math::BigInt -> import(@import);

    # find out which one was actually loaded
    $LIB = Math::BigInt -> config('lib');

    $class->export_to_level(1, $class, @a); # export wanted functions
}

sub _len_to_steps {
    # Given D (digits in decimal), compute N so that N! (N factorial) is
    # at least D digits long. D should be at least 50.
    my $d = shift;

    # two constants for the Ramanujan estimate of ln(N!)
    my $lg2 = log(2 * 3.14159265) / 2;
    my $lg10 = log(10);

    # D = 50 => N => 42, so L = 40 and R = 50
    my $l = 40;
    my $r = $d;

    # Otherwise this does not work under -Mbignum and we do not yet have "no
    # bignum;" :(
    $l = $l->numify if ref($l);
    $r = $r->numify if ref($r);
    $lg2 = $lg2->numify if ref($lg2);
    $lg10 = $lg10->numify if ref($lg10);

    # binary search for the right value (could this be written as the reverse of
    # lg(n!)?)
    while ($r - $l > 1) {
        my $n = int(($r - $l) / 2) + $l;
        my $ramanujan
          = int(($n * log($n) - $n + log($n * (1 + 4*$n*(1+2*$n))) / 6 + $lg2)
                / $lg10);
        $ramanujan > $d ? $r = $n : $l = $n;
    }
    $l;
}

sub _log {
    # internal log function to calculate ln() based on Taylor series.
    # Modifies $x in place.
    my ($x, $scale) = @_;
    my $class = ref $x;

    # in case of $x == 1, result is 0
    return $x->bzero() if $x->is_one();

    # XXX TODO: rewrite this in a similar manner to bexp()

    # http://www.efunda.com/math/taylor_series/logarithmic.cfm?search_string=log

    # u = x-1, v = x+1
    #              _                               _
    # Taylor:     |    u    1   u^3   1   u^5       |
    # ln (x)  = 2 |   --- + - * --- + - * --- + ... |  x > 0
    #             |_   v    3   v^3   5   v^5      _|

    # This takes much more steps to calculate the result and is thus not used
    # u = x-1
    #              _                               _
    # Taylor:     |    u    1   u^2   1   u^3       |
    # ln (x)  = 2 |   --- + - * --- + - * --- + ... |  x > 1/2
    #             |_   x    2   x^2   3   x^3      _|

    my ($limit, $v, $u, $below, $factor, $next, $over, $f);

    $v = $x->copy();
    $v = $v -> binc();                  # v = x+1
    $x = $x->bdec();
    $u = $x->copy();                    # u = x-1; x = x-1
    $x = $x->bdiv($v, $scale);          # first term: u/v
    $below = $v->copy();
    $over = $u->copy();
    $u = $u -> bmul($u);                # u^2
    $v = $v -> bmul($v);                # v^2
    $below = $below->bmul($v);          # u^3, v^3
    $over = $over->bmul($u);
    $factor = $class->new(3);
    $f = $class->new(2);

    $limit = $class->new("1E-". ($scale-1));

    while (3 < 5) {
        # we calculate the next term, and add it to the last
        # when the next term is below our limit, it won't affect the outcome
        # anymore, so we stop

        # calculating the next term simple from over/below will result in quite
        # a time hog if the input has many digits, since over and below will
        # accumulate more and more digits, and the result will also have many
        # digits, but in the end it is rounded to $scale digits anyway. So if we
        # round $over and $below first, we save a lot of time for the division
        # (not with log(1.2345), but try log (123**123) to see what I mean. This
        # can introduce a rounding error if the division result would be f.i.
        # 0.1234500000001 and we round it to 5 digits it would become 0.12346,
        # but if we truncated $over and $below we might get 0.12345. Does this
        # matter for the end result? So we give $over and $below 4 more digits
        # to be on the safe side (unscientific error handling as usual... :+D

        $next = $over->copy()->bround($scale+4)
          ->bdiv($below->copy()->bmul($factor)->bround($scale+4),
                 $scale);

        last if $next->bacmp($limit) <= 0;

        delete $next->{_a};
        delete $next->{_p};
        $x = $x->badd($next);
        # calculate things for the next term
        $over *= $u;
        $below *= $v;
        $factor = $factor->badd($f);
    }
    $x->bmul($f);               # $x *= 2
}

sub _log_10 {
    # Internal log function based on reducing input to the range of 0.1 .. 9.99
    # and then "correcting" the result to the proper one. Modifies $x in place.
    my ($x, $scale) = @_;
    my $class = ref $x;

    # Taking blog() from numbers greater than 10 takes a *very long* time, so we
    # break the computation down into parts based on the observation that:
    #  blog(X*Y) = blog(X) + blog(Y)
    # We set Y here to multiples of 10 so that $x becomes below 1 - the smaller
    # $x is the faster it gets. Since 2*$x takes about 10 times as
    # long, we make it faster by about a factor of 100 by dividing $x by 10.

    # The same observation is valid for numbers smaller than 0.1, e.g. computing
    # log(1) is fastest, and the further away we get from 1, the longer it
    # takes. So we also 'break' this down by multiplying $x with 10 and subtract
    # the log(10) afterwards to get the correct result.

    # To get $x even closer to 1, we also divide by 2 and then use log(2) to
    # correct for this. For instance if $x is 2.4, we use the formula:
    #  blog(2.4 * 2) == blog(1.2) + blog(2)
    # and thus calculate only blog(1.2) and blog(2), which is faster in total
    # than calculating blog(2.4).

    # In addition, the values for blog(2) and blog(10) are cached.

    # Calculate nr of digits before dot. x = 123, dbd = 3; x = 1.23, dbd = 1;
    # x = 0.0123, dbd = -1; x = 0.000123, dbd = -3, etc.

    my $dbd = $LIB->_num($x->{_e});
    $dbd = -$dbd if $x->{_es} eq '-';
    $dbd += $LIB->_len($x->{_m});

    # more than one digit (e.g. at least 10), but *not* exactly 10 to avoid
    # infinite recursion

    my $calc = 1;               # do some calculation?

    # disable the shortcut for 10, since we need log(10) and this would recurse
    # infinitely deep
    if ($x->{_es} eq '+' &&                     # $x == 10
        ($LIB->_is_one($x->{_e}) &&
         $LIB->_is_one($x->{_m})))
    {
        $dbd = 0;               # disable shortcut
        # we can use the cached value in these cases
        if ($scale <= $LOG_10_A) {
            $x = $x->bzero();
            $x = $x->badd($LOG_10); # modify $x in place
            $calc = 0;                      # no need to calc, but round
        }
        # if we can't use the shortcut, we continue normally
    } else {
        # disable the shortcut for 2, since we maybe have it cached
        if (($LIB->_is_zero($x->{_e}) &&        # $x == 2
             $LIB->_is_two($x->{_m})))
        {
            $dbd = 0;           # disable shortcut
            # we can use the cached value in these cases
            if ($scale <= $LOG_2_A) {
                $x = $x->bzero();
                $x = $x->badd($LOG_2); # modify $x in place
                $calc = 0;                     # no need to calc, but round
            }
            # if we can't use the shortcut, we continue normally
        }
    }

    # if $x = 0.1, we know the result must be 0-log(10)
    if ($calc != 0 &&
        ($x->{_es} eq '-' &&                    # $x == 0.1
         ($LIB->_is_one($x->{_e}) &&
          $LIB->_is_one($x->{_m}))))
    {
        $dbd = 0;               # disable shortcut
        # we can use the cached value in these cases
        if ($scale <= $LOG_10_A) {
            $x = $x->bzero();
            $x = $x->bsub($LOG_10);
            $calc = 0;          # no need to calc, but round
        }
    }

    return $x if $calc == 0;    # already have the result

    # default: these correction factors are undef and thus not used
    my $l_10;                   # value of ln(10) to A of $scale
    my $l_2;                    # value of ln(2) to A of $scale

    my $two = $class->new(2);

    # $x == 2 => 1, $x == 13 => 2, $x == 0.1 => 0, $x == 0.01 => -1
    # so don't do this shortcut for 1 or 0
    if (($dbd > 1) || ($dbd < 0)) {
        # convert our cached value to an object if not already (avoid doing this
        # at import() time, since not everybody needs this)
        $LOG_10 = $class->new($LOG_10, undef, undef) unless ref $LOG_10;

        #print "x = $x, dbd = $dbd, calc = $calc\n";
        # got more than one digit before the dot, or more than one zero after
        # the dot, so do:
        #  log(123)    == log(1.23) + log(10) * 2
        #  log(0.0123) == log(1.23) - log(10) * 2

        if ($scale <= $LOG_10_A) {
            # use cached value
            $l_10 = $LOG_10->copy(); # copy for mul
        } else {
            # else: slower, compute and cache result

            # Disabling upgrading and downgrading is no longer necessary to
            # avoid an infinite recursion, but it avoids unnecessary upgrading
            # and downgrading in the intermediate computations.

            local $Math::BigInt::upgrade = undef;
            local $Math::BigFloat::downgrade = undef;

            # shorten the time to calculate log(10) based on the following:
            # log(1.25 * 8) = log(1.25) + log(8)
            #               = log(1.25) + log(2) + log(2) + log(2)

            # first get $l_2 (and possible compute and cache log(2))
            $LOG_2 = $class->new($LOG_2, undef, undef) unless ref $LOG_2;
            if ($scale <= $LOG_2_A) {
                # use cached value
                $l_2 = $LOG_2->copy(); # copy() for the mul below
            } else {
                # else: slower, compute and cache result
                $l_2 = $two->copy();
                $l_2 = $l_2->_log($scale); # scale+4, actually
                $LOG_2 = $l_2->copy(); # cache the result for later
                # the copy() is for mul below
                $LOG_2_A = $scale;
            }

            # now calculate log(1.25):
            $l_10 = $class->new('1.25');
            $l_10 = $l_10->_log($scale); # scale+4, actually

            # log(1.25) + log(2) + log(2) + log(2):
            $l_10 = $l_10->badd($l_2);
            $l_10 = $l_10->badd($l_2);
            $l_10 = $l_10->badd($l_2);
            $LOG_10 = $l_10->copy(); # cache the result for later
            # the copy() is for mul below
            $LOG_10_A = $scale;
        }
        $dbd-- if ($dbd > 1);       # 20 => dbd=2, so make it dbd=1
        $l_10 = $l_10->bmul($class->new($dbd)); # log(10) * (digits_before_dot-1)
        my $dbd_sign = '+';
        if ($dbd < 0) {
            $dbd = -$dbd;
            $dbd_sign = '-';
        }
        ($x->{_e}, $x->{_es}) =
          $LIB -> _ssub($x->{_e}, $x->{_es}, $LIB->_new($dbd), $dbd_sign);
    }

    # Now: 0.1 <= $x < 10 (and possible correction in l_10)

    ### Since $x in the range 0.5 .. 1.5 is MUCH faster, we do a repeated div
    ### or mul by 2 (maximum times 3, since x < 10 and x > 0.1)

    $HALF = $class->new($HALF) unless ref($HALF);

    my $twos = 0;               # default: none (0 times)
    while ($x->bacmp($HALF) <= 0) { # X <= 0.5
        $twos--;
        $x = $x->bmul($two);
    }
    while ($x->bacmp($two) >= 0) { # X >= 2
        $twos++;
        $x = $x->bdiv($two, $scale+4); # keep all digits
    }
    $x = $x->bround($scale+4);
    # $twos > 0 => did mul 2, < 0 => did div 2 (but we never did both)
    # So calculate correction factor based on ln(2):
    if ($twos != 0) {
        $LOG_2 = $class->new($LOG_2, undef, undef) unless ref $LOG_2;
        if ($scale <= $LOG_2_A) {
            # use cached value
            $l_2 = $LOG_2->copy(); # copy() for the mul below
        } else {
            # else: slower, compute and cache result

            # Disabling upgrading and downgrading is no longer necessary to
            # avoid an infinite recursion, but it avoids unnecessary upgrading
            # and downgrading in the intermediate computations.

            local $Math::BigInt::upgrade = undef;
            local $Math::BigFloat::downgrade = undef;

            $l_2 = $two->copy();
            $l_2 = $l_2->_log($scale); # scale+4, actually
            $LOG_2 = $l_2->copy(); # cache the result for later
            # the copy() is for mul below
            $LOG_2_A = $scale;
        }
        $l_2 = $l_2->bmul($twos);      # * -2 => subtract, * 2 => add
    } else {
        undef $l_2;
    }

    $x = $x->_log($scale);       # need to do the "normal" way
    $x = $x->badd($l_10) if defined $l_10; # correct it by ln(10)
    $x = $x->badd($l_2) if defined $l_2;   # and maybe by ln(2)

    # all done, $x contains now the result
    $x;
}

sub _pow {
    # Calculate a power where $y is a non-integer, like 2 ** 0.3
    my ($x, $y, @r) = @_;
    my $class = ref($x);

    # if $y == 0.5, it is sqrt($x)
    $HALF = $class->new($HALF) unless ref($HALF);
    return $x->bsqrt(@r, $y) if $y->bcmp($HALF) == 0;

    # Using:
    # a ** x == e ** (x * ln a)

    # u = y * ln x
    #                _                         _
    # Taylor:       |   u    u^2    u^3         |
    # x ** y  = 1 + |  --- + --- + ----- + ...  |
    #               |_  1    1*2   1*2*3       _|

    # we need to limit the accuracy to protect against overflow
    my $fallback = 0;
    my ($scale, @params);
    ($x, @params) = $x->_find_round_parameters(@r);

    return $x if $x->is_nan();  # error in _find_round_parameters?

    # no rounding at all, so must use fallback
    if (scalar @params == 0) {
        # simulate old behaviour
        $params[0] = $class->div_scale(); # and round to it as accuracy
        $params[1] = undef;               # disable P
        $scale = $params[0]+4;            # at least four more for proper round
        $params[2] = $r[2];               # round mode by caller or undef
        $fallback = 1;                    # to clear a/p afterwards
    } else {
        # the 4 below is empirical, and there might be cases where it is not
        # enough...
        $scale = abs($params[0] || $params[1]) + 4; # take whatever is defined
    }

    # when user set globals, they would interfere with our calculation, so
    # disable them and later re-enable them
    no strict 'refs';
    my $abr = "$class\::accuracy";
    my $ab = $$abr;
    $$abr = undef;
    my $pbr = "$class\::precision";
    my $pb = $$pbr;
    $$pbr = undef;
    # we also need to disable any set A or P on $x (_find_round_parameters took
    # them already into account), since these would interfere, too
    delete $x->{_a};
    delete $x->{_p};

    # Disabling upgrading and downgrading is no longer necessary to avoid an
    # infinite recursion, but it avoids unnecessary upgrading and downgrading in
    # the intermediate computations.

    local $Math::BigInt::upgrade = undef;
    local $Math::BigFloat::downgrade = undef;

    my ($limit, $v, $u, $below, $factor, $next, $over);

    $u = $x->copy()->blog(undef, $scale)->bmul($y);
    my $do_invert = ($u->{sign} eq '-');
    $u = $u->bneg()  if $do_invert;
    $v = $class->bone();        # 1
    $factor = $class->new(2);   # 2
    $x = $x->bone();                 # first term: 1

    $below = $v->copy();
    $over = $u->copy();

    $limit = $class->new("1E-". ($scale-1));
    while (3 < 5) {
        # we calculate the next term, and add it to the last
        # when the next term is below our limit, it won't affect the outcome
        # anymore, so we stop:
        $next = $over->copy()->bdiv($below, $scale);
        last if $next->bacmp($limit) <= 0;
        $x = $x->badd($next);
        # calculate things for the next term
        $over *= $u;
        $below *= $factor;
        $factor = $factor->binc();

        last if $x->{sign} !~ /^[-+]$/;
    }

    if ($do_invert) {
        my $x_copy = $x->copy();
        $x = $x->bone->bdiv($x_copy, $scale);
    }

    # shortcut to not run through _find_round_parameters again
    if (defined $params[0]) {
        $x = $x->bround($params[0], $params[2]); # then round accordingly
    } else {
        $x = $x->bfround($params[1], $params[2]); # then round accordingly
    }
    if ($fallback) {
        # clear a/p after round, since user did not request it
        delete $x->{_a};
        delete $x->{_p};
    }
    # restore globals
    $$abr = $ab;
    $$pbr = $pb;
    $x;
}

# These functions are only provided for backwards compabibility so that old
# version of Math::BigRat etc. don't complain about missing them.

sub _e_add {
    my ($x, $y, $xs, $ys) = @_;
    return $LIB -> _sadd($x, $xs, $y, $ys);
}

sub _e_sub {
    my ($x, $y, $xs, $ys) = @_;
    return $LIB -> _ssub($x, $xs, $y, $ys);
}

1;

__END__

=pod

=head1 NAME

Math::BigFloat - arbitrary size floating point math package

=head1 SYNOPSIS

  use Math::BigFloat;

  # Configuration methods (may be used as class methods and instance methods)

  Math::BigFloat->accuracy();     # get class accuracy
  Math::BigFloat->accuracy($n);   # set class accuracy
  Math::BigFloat->precision();    # get class precision
  Math::BigFloat->precision($n);  # set class precision
  Math::BigFloat->round_mode();   # get class rounding mode
  Math::BigFloat->round_mode($m); # set global round mode, must be one of
                                  # 'even', 'odd', '+inf', '-inf', 'zero',
                                  # 'trunc', or 'common'
  Math::BigFloat->config("lib");  # name of backend math library

  # Constructor methods (when the class methods below are used as instance
  # methods, the value is assigned the invocand)

  $x = Math::BigFloat->new($str);               # defaults to 0
  $x = Math::BigFloat->new('0x123');            # from hexadecimal
  $x = Math::BigFloat->new('0o377');            # from octal
  $x = Math::BigFloat->new('0b101');            # from binary
  $x = Math::BigFloat->from_hex('0xc.afep+3');  # from hex
  $x = Math::BigFloat->from_hex('cafe');        # ditto
  $x = Math::BigFloat->from_oct('1.3267p-4');   # from octal
  $x = Math::BigFloat->from_oct('01.3267p-4');  # ditto
  $x = Math::BigFloat->from_oct('0o1.3267p-4'); # ditto
  $x = Math::BigFloat->from_oct('0377');        # ditto
  $x = Math::BigFloat->from_bin('0b1.1001p-4'); # from binary
  $x = Math::BigFloat->from_bin('0101');        # ditto
  $x = Math::BigFloat->from_ieee754($b, "binary64");  # from IEEE-754 bytes
  $x = Math::BigFloat->bzero();                 # create a +0
  $x = Math::BigFloat->bone();                  # create a +1
  $x = Math::BigFloat->bone('-');               # create a -1
  $x = Math::BigFloat->binf();                  # create a +inf
  $x = Math::BigFloat->binf('-');               # create a -inf
  $x = Math::BigFloat->bnan();                  # create a Not-A-Number
  $x = Math::BigFloat->bpi();                   # returns pi

  $y = $x->copy();        # make a copy (unlike $y = $x)
  $y = $x->as_int();      # return as BigInt
  $y = $x->as_float();    # return as a Math::BigFloat
  $y = $x->as_rat();      # return as a Math::BigRat

  # Boolean methods (these don't modify the invocand)

  $x->is_zero();          # if $x is 0
  $x->is_one();           # if $x is +1
  $x->is_one("+");        # ditto
  $x->is_one("-");        # if $x is -1
  $x->is_inf();           # if $x is +inf or -inf
  $x->is_inf("+");        # if $x is +inf
  $x->is_inf("-");        # if $x is -inf
  $x->is_nan();           # if $x is NaN

  $x->is_positive();      # if $x > 0
  $x->is_pos();           # ditto
  $x->is_negative();      # if $x < 0
  $x->is_neg();           # ditto

  $x->is_odd();           # if $x is odd
  $x->is_even();          # if $x is even
  $x->is_int();           # if $x is an integer

  # Comparison methods

  $x->bcmp($y);           # compare numbers (undef, < 0, == 0, > 0)
  $x->bacmp($y);          # compare absolutely (undef, < 0, == 0, > 0)
  $x->beq($y);            # true if and only if $x == $y
  $x->bne($y);            # true if and only if $x != $y
  $x->blt($y);            # true if and only if $x < $y
  $x->ble($y);            # true if and only if $x <= $y
  $x->bgt($y);            # true if and only if $x > $y
  $x->bge($y);            # true if and only if $x >= $y

  # Arithmetic methods

  $x->bneg();             # negation
  $x->babs();             # absolute value
  $x->bsgn();             # sign function (-1, 0, 1, or NaN)
  $x->bnorm();            # normalize (no-op)
  $x->binc();             # increment $x by 1
  $x->bdec();             # decrement $x by 1
  $x->badd($y);           # addition (add $y to $x)
  $x->bsub($y);           # subtraction (subtract $y from $x)
  $x->bmul($y);           # multiplication (multiply $x by $y)
  $x->bmuladd($y,$z);     # $x = $x * $y + $z
  $x->bdiv($y);           # division (floored), set $x to quotient
                          # return (quo,rem) or quo if scalar
  $x->btdiv($y);          # division (truncated), set $x to quotient
                          # return (quo,rem) or quo if scalar
  $x->bmod($y);           # modulus (x % y)
  $x->btmod($y);          # modulus (truncated)
  $x->bmodinv($mod);      # modular multiplicative inverse
  $x->bmodpow($y,$mod);   # modular exponentiation (($x ** $y) % $mod)
  $x->bpow($y);           # power of arguments (x ** y)
  $x->blog();             # logarithm of $x to base e (Euler's number)
  $x->blog($base);        # logarithm of $x to base $base (e.g., base 2)
  $x->bexp();             # calculate e ** $x where e is Euler's number
  $x->bnok($y);           # x over y (binomial coefficient n over k)
  $x->bsin();             # sine
  $x->bcos();             # cosine
  $x->batan();            # inverse tangent
  $x->batan2($y);         # two-argument inverse tangent
  $x->bsqrt();            # calculate square root
  $x->broot($y);          # $y'th root of $x (e.g. $y == 3 => cubic root)
  $x->bfac();             # factorial of $x (1*2*3*4*..$x)

  $x->blsft($n);          # left shift $n places in base 2
  $x->blsft($n,$b);       # left shift $n places in base $b
                          # returns (quo,rem) or quo (scalar context)
  $x->brsft($n);          # right shift $n places in base 2
  $x->brsft($n,$b);       # right shift $n places in base $b
                          # returns (quo,rem) or quo (scalar context)

  # Bitwise methods

  $x->band($y);           # bitwise and
  $x->bior($y);           # bitwise inclusive or
  $x->bxor($y);           # bitwise exclusive or
  $x->bnot();             # bitwise not (two's complement)

  # Rounding methods
  $x->round($A,$P,$mode); # round to accuracy or precision using
                          # rounding mode $mode
  $x->bround($n);         # accuracy: preserve $n digits
  $x->bfround($n);        # $n > 0: round to $nth digit left of dec. point
                          # $n < 0: round to $nth digit right of dec. point
  $x->bfloor();           # round towards minus infinity
  $x->bceil();            # round towards plus infinity
  $x->bint();             # round towards zero

  # Other mathematical methods

  $x->bgcd($y);            # greatest common divisor
  $x->blcm($y);            # least common multiple

  # Object property methods (do not modify the invocand)

  $x->sign();              # the sign, either +, - or NaN
  $x->digit($n);           # the nth digit, counting from the right
  $x->digit(-$n);          # the nth digit, counting from the left
  $x->length();            # return number of digits in number
  ($xl,$f) = $x->length(); # length of number and length of fraction
                           # part, latter is always 0 digits long
                           # for Math::BigInt objects
  $x->mantissa();          # return (signed) mantissa as BigInt
  $x->exponent();          # return exponent as BigInt
  $x->parts();             # return (mantissa,exponent) as BigInt
  $x->sparts();            # mantissa and exponent (as integers)
  $x->nparts();            # mantissa and exponent (normalised)
  $x->eparts();            # mantissa and exponent (engineering notation)
  $x->dparts();            # integer and fraction part
  $x->fparts();            # numerator and denominator
  $x->numerator();         # numerator
  $x->denominator();       # denominator

  # Conversion methods (do not modify the invocand)

  $x->bstr();         # decimal notation, possibly zero padded
  $x->bsstr();        # string in scientific notation with integers
  $x->bnstr();        # string in normalized notation
  $x->bestr();        # string in engineering notation
  $x->bdstr();        # string in decimal notation
  $x->bfstr();        # string in fractional notation

  $x->as_hex();       # as signed hexadecimal string with prefixed 0x
  $x->as_bin();       # as signed binary string with prefixed 0b
  $x->as_oct();       # as signed octal string with prefixed 0
  $x->to_ieee754($format); # to bytes encoded according to IEEE 754-2008

  # Other conversion methods

  $x->numify();           # return as scalar (might overflow or underflow)

=head1 DESCRIPTION

Math::BigFloat provides support for arbitrary precision floating point.
Overloading is also provided for Perl operators.

All operators (including basic math operations) are overloaded if you
declare your big floating point numbers as

  $x = Math::BigFloat -> new('12_3.456_789_123_456_789E-2');

Operations with overloaded operators preserve the arguments, which is
exactly what you expect.

=head2 Input

Input values to these routines may be any scalar number or string that looks
like a number. Anything that is accepted by Perl as a literal numeric constant
should be accepted by this module.

=over

=item *

Leading and trailing whitespace is ignored.

=item *

Leading zeros are ignored, except for floating point numbers with a binary
exponent, in which case the number is interpreted as an octal floating point
number. For example, "01.4p+0" gives 1.5, "00.4p+0" gives 0.5, but "0.4p+0"
gives a NaN. And while "0377" gives 255, "0377p0" gives 255.

=item *

If the string has a "0x" or "0X" prefix, it is interpreted as a hexadecimal
number.

=item *

If the string has a "0o" or "0O" prefix, it is interpreted as an octal number. A
floating point literal with a "0" prefix is also interpreted as an octal number.

=item *

If the string has a "0b" or "0B" prefix, it is interpreted as a binary number.

=item *

Underline characters are allowed in the same way as they are allowed in literal
numerical constants.

=item *

If the string can not be interpreted, NaN is returned.

=item *

For hexadecimal, octal, and binary floating point numbers, the exponent must be
separated from the significand (mantissa) by the letter "p" or "P", not "e" or
"E" as with decimal numbers.

=back

Some examples of valid string input

    Input string                Resulting value

    123                         123
    1.23e2                      123
    12300e-2                    123

    67_538_754                  67538754
    -4_5_6.7_8_9e+0_1_0         -4567890000000

    0x13a                       314
    0x13ap0                     314
    0x1.3ap+8                   314
    0x0.00013ap+24              314
    0x13a000p-12                314

    0o472                       314
    0o1.164p+8                  314
    0o0.0001164p+20             314
    0o1164000p-10               314

    0472                        472     Note!
    01.164p+8                   314
    00.0001164p+20              314
    01164000p-10                314

    0b100111010                 314
    0b1.0011101p+8              314
    0b0.00010011101p+12         314
    0b100111010000p-3           314

    0x1.921fb5p+1               3.14159262180328369140625e+0
    0o1.2677025p1               2.71828174591064453125
    01.2677025p1                2.71828174591064453125
    0b1.1001p-4                 9.765625e-2

=head2 Output

Output values are usually Math::BigFloat objects.

Boolean operators C<is_zero()>, C<is_one()>, C<is_inf()>, etc. return true or
false.

Comparison operators C<bcmp()> and C<bacmp()>) return -1, 0, 1, or
undef.

=head1 METHODS

Math::BigFloat supports all methods that Math::BigInt supports, except it
calculates non-integer results when possible. Please see L<Math::BigInt> for a
full description of each method. Below are just the most important differences:

=head2 Configuration methods

=over

=item accuracy()

    $x->accuracy(5);           # local for $x
    CLASS->accuracy(5);        # global for all members of CLASS
                               # Note: This also applies to new()!

    $A = $x->accuracy();       # read out accuracy that affects $x
    $A = CLASS->accuracy();    # read out global accuracy

Set or get the global or local accuracy, aka how many significant digits the
results have. If you set a global accuracy, then this also applies to new()!

Warning! The accuracy I<sticks>, e.g. once you created a number under the
influence of C<< CLASS->accuracy($A) >>, all results from math operations with
that number will also be rounded.

In most cases, you should probably round the results explicitly using one of
L<Math::BigInt/round()>, L<Math::BigInt/bround()> or L<Math::BigInt/bfround()>
or by passing the desired accuracy to the math operation as additional
parameter:

    my $x = Math::BigInt->new(30000);
    my $y = Math::BigInt->new(7);
    print scalar $x->copy()->bdiv($y, 2);           # print 4300
    print scalar $x->copy()->bdiv($y)->bround(2);   # print 4300

=item precision()

    $x->precision(-2);        # local for $x, round at the second
                              # digit right of the dot
    $x->precision(2);         # ditto, round at the second digit
                              # left of the dot

    CLASS->precision(5);      # Global for all members of CLASS
                              # This also applies to new()!
    CLASS->precision(-5);     # ditto

    $P = CLASS->precision();  # read out global precision
    $P = $x->precision();     # read out precision that affects $x

Note: You probably want to use L</accuracy()> instead. With L</accuracy()> you
set the number of digits each result should have, with L</precision()> you
set the place where to round!

=back

=head2 Constructor methods

=over

=item from_hex()

    $x -> from_hex("0x1.921fb54442d18p+1");
    $x = Math::BigFloat -> from_hex("0x1.921fb54442d18p+1");

Interpret input as a hexadecimal string.A prefix ("0x", "x", ignoring case) is
optional. A single underscore character ("_") may be placed between any two
digits. If the input is invalid, a NaN is returned. The exponent is in base 2
using decimal digits.

If called as an instance method, the value is assigned to the invocand.

=item from_oct()

    $x -> from_oct("1.3267p-4");
    $x = Math::BigFloat -> from_oct("1.3267p-4");

Interpret input as an octal string. A single underscore character ("_") may be
placed between any two digits. If the input is invalid, a NaN is returned. The
exponent is in base 2 using decimal digits.

If called as an instance method, the value is assigned to the invocand.

=item from_bin()

    $x -> from_bin("0b1.1001p-4");
    $x = Math::BigFloat -> from_bin("0b1.1001p-4");

Interpret input as a hexadecimal string. A prefix ("0b" or "b", ignoring case)
is optional. A single underscore character ("_") may be placed between any two
digits. If the input is invalid, a NaN is returned. The exponent is in base 2
using decimal digits.

If called as an instance method, the value is assigned to the invocand.

=item from_ieee754()

Interpret the input as a value encoded as described in IEEE754-2008.  The input
can be given as a byte string, hex string or binary string. The input is
assumed to be in big-endian byte-order.

        # both $dbl and $mbf are 3.141592...
        $bytes = "\x40\x09\x21\xfb\x54\x44\x2d\x18";
        $dbl = unpack "d>", $bytes;
        $mbf = Math::BigFloat -> from_ieee754($bytes, "binary64");

=item bpi()

    print Math::BigFloat->bpi(100), "\n";

Calculate PI to N digits (including the 3 before the dot). The result is
rounded according to the current rounding mode, which defaults to "even".

This method was added in v1.87 of Math::BigInt (June 2007).

=back

=head2 Arithmetic methods

=over

=item bmuladd()

    $x->bmuladd($y,$z);

Multiply $x by $y, and then add $z to the result.

This method was added in v1.87 of Math::BigInt (June 2007).

=item bdiv()

    $q = $x->bdiv($y);
    ($q, $r) = $x->bdiv($y);

In scalar context, divides $x by $y and returns the result to the given or
default accuracy/precision. In list context, does floored division
(F-division), returning an integer $q and a remainder $r so that $x = $q * $y +
$r. The remainer (modulo) is equal to what is returned by C<< $x->bmod($y) >>.

=item bmod()

    $x->bmod($y);

Returns $x modulo $y. When $x is finite, and $y is finite and non-zero, the
result is identical to the remainder after floored division (F-division). If,
in addition, both $x and $y are integers, the result is identical to the result
from Perl's % operator.

=item bexp()

    $x->bexp($accuracy);            # calculate e ** X

Calculates the expression C<e ** $x> where C<e> is Euler's number.

This method was added in v1.82 of Math::BigInt (April 2007).

=item bnok()

    $x->bnok($y);   # x over y (binomial coefficient n over k)

Calculates the binomial coefficient n over k, also called the "choose"
function. The result is equivalent to:

    ( n )      n!
    | - |  = -------
    ( k )    k!(n-k)!

This method was added in v1.84 of Math::BigInt (April 2007).

=item bsin()

    my $x = Math::BigFloat->new(1);
    print $x->bsin(100), "\n";

Calculate the sinus of $x, modifying $x in place.

This method was added in v1.87 of Math::BigInt (June 2007).

=item bcos()

    my $x = Math::BigFloat->new(1);
    print $x->bcos(100), "\n";

Calculate the cosinus of $x, modifying $x in place.

This method was added in v1.87 of Math::BigInt (June 2007).

=item batan()

    my $x = Math::BigFloat->new(1);
    print $x->batan(100), "\n";

Calculate the arcus tanges of $x, modifying $x in place. See also L</batan2()>.

This method was added in v1.87 of Math::BigInt (June 2007).

=item batan2()

    my $y = Math::BigFloat->new(2);
    my $x = Math::BigFloat->new(3);
    print $y->batan2($x), "\n";

Calculate the arcus tanges of C<$y> divided by C<$x>, modifying $y in place.
See also L</batan()>.

This method was added in v1.87 of Math::BigInt (June 2007).

=item as_float()

This method is called when Math::BigFloat encounters an object it doesn't know
how to handle. For instance, assume $x is a Math::BigFloat, or subclass
thereof, and $y is defined, but not a Math::BigFloat, or subclass thereof. If
you do

    $x -> badd($y);

$y needs to be converted into an object that $x can deal with. This is done by
first checking if $y is something that $x might be upgraded to. If that is the
case, no further attempts are made. The next is to see if $y supports the
method C<as_float()>. The method C<as_float()> is expected to return either an
object that has the same class as $x, a subclass thereof, or a string that
C<ref($x)-E<gt>new()> can parse to create an object.

In Math::BigFloat, C<as_float()> has the same effect as C<copy()>.

=item to_ieee754()

Encodes the invocand as a byte string in the given format as specified in IEEE
754-2008. Note that the encoded value is the nearest possible representation of
the value. This value might not be exactly the same as the value in the
invocand.

    # $x = 3.1415926535897932385
    $x = Math::BigFloat -> bpi(30);

    $b = $x -> to_ieee754("binary64");  # encode as 8 bytes
    $h = unpack "H*", $b;               # "400921fb54442d18"

    # 3.141592653589793115997963...
    $y = Math::BigFloat -> from_ieee754($h, "binary64");

All binary formats in IEEE 754-2008 are accepted. For convenience, som aliases
are recognized: "half" for "binary16", "single" for "binary32", "double" for
"binary64", "quadruple" for "binary128", "octuple" for "binary256", and
"sexdecuple" for "binary512".

See also L<https://en.wikipedia.org/wiki/IEEE_754>.

=back

=head2 ACCURACY AND PRECISION

See also: L<Rounding|/Rounding>.

Math::BigFloat supports both precision (rounding to a certain place before or
after the dot) and accuracy (rounding to a certain number of digits). For a
full documentation, examples and tips on these topics please see the large
section about rounding in L<Math::BigInt>.

Since things like C<sqrt(2)> or C<1 / 3> must presented with a limited
accuracy lest a operation consumes all resources, each operation produces
no more than the requested number of digits.

If there is no global precision or accuracy set, B<and> the operation in
question was not called with a requested precision or accuracy, B<and> the
input $x has no accuracy or precision set, then a fallback parameter will
be used. For historical reasons, it is called C<div_scale> and can be accessed
via:

    $d = Math::BigFloat->div_scale();       # query
    Math::BigFloat->div_scale($n);          # set to $n digits

The default value for C<div_scale> is 40.

In case the result of one operation has more digits than specified,
it is rounded. The rounding mode taken is either the default mode, or the one
supplied to the operation after the I<scale>:

    $x = Math::BigFloat->new(2);
    Math::BigFloat->accuracy(5);              # 5 digits max
    $y = $x->copy()->bdiv(3);                 # gives 0.66667
    $y = $x->copy()->bdiv(3,6);               # gives 0.666667
    $y = $x->copy()->bdiv(3,6,undef,'odd');   # gives 0.666667
    Math::BigFloat->round_mode('zero');
    $y = $x->copy()->bdiv(3,6);               # will also give 0.666667

Note that C<< Math::BigFloat->accuracy() >> and
C<< Math::BigFloat->precision() >> set the global variables, and thus B<any>
newly created number will be subject to the global rounding B<immediately>. This
means that in the examples above, the C<3> as argument to C<bdiv()> will also
get an accuracy of B<5>.

It is less confusing to either calculate the result fully, and afterwards
round it explicitly, or use the additional parameters to the math
functions like so:

    use Math::BigFloat;
    $x = Math::BigFloat->new(2);
    $y = $x->copy()->bdiv(3);
    print $y->bround(5),"\n";               # gives 0.66667

    or

    use Math::BigFloat;
    $x = Math::BigFloat->new(2);
    $y = $x->copy()->bdiv(3,5);             # gives 0.66667
    print "$y\n";

=head2 Rounding

=over

=item bfround ( +$scale )

Rounds to the $scale'th place left from the '.', counting from the dot.
The first digit is numbered 1.

=item bfround ( -$scale )

Rounds to the $scale'th place right from the '.', counting from the dot.

=item bfround ( 0 )

Rounds to an integer.

=item bround  ( +$scale )

Preserves accuracy to $scale digits from the left (aka significant digits) and
pads the rest with zeros. If the number is between 1 and -1, the significant
digits count from the first non-zero after the '.'

=item bround  ( -$scale ) and bround ( 0 )

These are effectively no-ops.

=back

All rounding functions take as a second parameter a rounding mode from one of
the following: 'even', 'odd', '+inf', '-inf', 'zero', 'trunc' or 'common'.

The default rounding mode is 'even'. By using
C<< Math::BigFloat->round_mode($round_mode); >> you can get and set the default
mode for subsequent rounding. The usage of C<$Math::BigFloat::$round_mode> is
no longer supported.
The second parameter to the round functions then overrides the default
temporarily.

The C<as_number()> function returns a BigInt from a Math::BigFloat. It uses
'trunc' as rounding mode to make it equivalent to:

    $x = 2.5;
    $y = int($x) + 2;

You can override this by passing the desired rounding mode as parameter to
C<as_number()>:

    $x = Math::BigFloat->new(2.5);
    $y = $x->as_number('odd');      # $y = 3

=head1 NUMERIC LITERALS

After C<use Math::BigFloat ':constant'> all numeric literals in the given scope
are converted to C<Math::BigFloat> objects. This conversion happens at compile
time.

For example,

    perl -MMath::BigFloat=:constant -le 'print 2e-150'

prints the exact value of C<2e-150>. Note that without conversion of constants
the expression C<2e-150> is calculated using Perl scalars, which leads to an
inaccuracte result.

Note that strings are not affected, so that

    use Math::BigFloat qw/:constant/;

    $y = "1234567890123456789012345678901234567890"
            + "123456789123456789";

does not give you what you expect. You need an explicit Math::BigFloat->new()
around at least one of the operands. You should also quote large constants to
prevent loss of precision:

    use Math::BigFloat;

    $x = Math::BigFloat->new("1234567889123456789123456789123456789");

Without the quotes Perl converts the large number to a floating point constant
at compile time, and then converts the result to a Math::BigFloat object at
runtime, which results in an inaccurate result.

=head2 Hexadecimal, octal, and binary floating point literals

Perl (and this module) accepts hexadecimal, octal, and binary floating point
literals, but use them with care with Perl versions before v5.32.0, because some
versions of Perl silently give the wrong result. Below are some examples of
different ways to write the number decimal 314.

Hexadecimal floating point literals:

    0x1.3ap+8         0X1.3AP+8
    0x1.3ap8          0X1.3AP8
    0x13a0p-4         0X13A0P-4

Octal floating point literals (with "0" prefix):

    01.164p+8         01.164P+8
    01.164p8          01.164P8
    011640p-4         011640P-4

Octal floating point literals (with "0o" prefix) (requires v5.34.0):

    0o1.164p+8        0O1.164P+8
    0o1.164p8         0O1.164P8
    0o11640p-4        0O11640P-4

Binary floating point literals:

    0b1.0011101p+8    0B1.0011101P+8
    0b1.0011101p8     0B1.0011101P8
    0b10011101000p-2  0B10011101000P-2

=head2 Math library

Math with the numbers is done (by default) by a module called
Math::BigInt::Calc. This is equivalent to saying:

    use Math::BigFloat lib => "Calc";

You can change this by using:

    use Math::BigFloat lib => "GMP";

B<Note>: General purpose packages should not be explicit about the library to
use; let the script author decide which is best.

Note: The keyword 'lib' will warn when the requested library could not be
loaded. To suppress the warning use 'try' instead:

    use Math::BigFloat try => "GMP";

If your script works with huge numbers and Calc is too slow for them, you can
also for the loading of one of these libraries and if none of them can be used,
the code will die:

    use Math::BigFloat only => "GMP,Pari";

The following would first try to find Math::BigInt::Foo, then Math::BigInt::Bar,
and when this also fails, revert to Math::BigInt::Calc:

    use Math::BigFloat lib => "Foo,Math::BigInt::Bar";

See the respective low-level library documentation for further details.

See L<Math::BigInt> for more details about using a different low-level library.

=head2 Using Math::BigInt::Lite

For backwards compatibility reasons it is still possible to
request a different storage class for use with Math::BigFloat:

    use Math::BigFloat with => 'Math::BigInt::Lite';

However, this request is ignored, as the current code now uses the low-level
math library for directly storing the number parts.

=head1 EXPORTS

C<Math::BigFloat> exports nothing by default, but can export the C<bpi()>
method:

    use Math::BigFloat qw/bpi/;

    print bpi(10), "\n";

=head1 CAVEATS

Do not try to be clever to insert some operations in between switching
libraries:

    require Math::BigFloat;
    my $matter = Math::BigFloat->bone() + 4;    # load BigInt and Calc
    Math::BigFloat->import( lib => 'Pari' );    # load Pari, too
    my $anti_matter = Math::BigFloat->bone()+4; # now use Pari

This will create objects with numbers stored in two different backend libraries,
and B<VERY BAD THINGS> will happen when you use these together:

    my $flash_and_bang = $matter + $anti_matter;    # Don't do this!

=over

=item stringify, bstr()

Both stringify and bstr() now drop the leading '+'. The old code would return
'+1.23', the new returns '1.23'. See the documentation in L<Math::BigInt> for
reasoning and details.

=item brsft()

The following will probably not print what you expect:

    my $c = Math::BigFloat->new('3.14159');
    print $c->brsft(3,10),"\n";     # prints 0.00314153.1415

It prints both quotient and remainder, since print calls C<brsft()> in list
context. Also, C<< $c->brsft() >> will modify $c, so be careful.
You probably want to use

    print scalar $c->copy()->brsft(3,10),"\n";
    # or if you really want to modify $c
    print scalar $c->brsft(3,10),"\n";

instead.

=item Modifying and =

Beware of:

    $x = Math::BigFloat->new(5);
    $y = $x;

It will not do what you think, e.g. making a copy of $x. Instead it just makes
a second reference to the B<same> object and stores it in $y. Thus anything
that modifies $x will modify $y (except overloaded math operators), and vice
versa. See L<Math::BigInt> for details and how to avoid that.

=item precision() vs. accuracy()

A common pitfall is to use L</precision()> when you want to round a result to
a certain number of digits:

    use Math::BigFloat;

    Math::BigFloat->precision(4);           # does not do what you
                                            # think it does
    my $x = Math::BigFloat->new(12345);     # rounds $x to "12000"!
    print "$x\n";                           # print "12000"
    my $y = Math::BigFloat->new(3);         # rounds $y to "0"!
    print "$y\n";                           # print "0"
    $z = $x / $y;                           # 12000 / 0 => NaN!
    print "$z\n";
    print $z->precision(),"\n";             # 4

Replacing L</precision()> with L</accuracy()> is probably not what you want,
either:

    use Math::BigFloat;

    Math::BigFloat->accuracy(4);          # enables global rounding:
    my $x = Math::BigFloat->new(123456);  # rounded immediately
                                          #   to "12350"
    print "$x\n";                         # print "123500"
    my $y = Math::BigFloat->new(3);       # rounded to "3
    print "$y\n";                         # print "3"
    print $z = $x->copy()->bdiv($y),"\n"; # 41170
    print $z->accuracy(),"\n";            # 4

What you want to use instead is:

    use Math::BigFloat;

    my $x = Math::BigFloat->new(123456);    # no rounding
    print "$x\n";                           # print "123456"
    my $y = Math::BigFloat->new(3);         # no rounding
    print "$y\n";                           # print "3"
    print $z = $x->copy()->bdiv($y,4),"\n"; # 41150
    print $z->accuracy(),"\n";              # undef

In addition to computing what you expected, the last example also does B<not>
"taint" the result with an accuracy or precision setting, which would
influence any further operation.

=back

=head1 BUGS

Please report any bugs or feature requests to
C<bug-math-bigint at rt.cpan.org>, or through the web interface at
L<https://rt.cpan.org/Ticket/Create.html?Queue=Math-BigInt> (requires login).
We will be notified, and then you'll automatically be notified of progress on
your bug as I make changes.

=head1 SUPPORT

You can find documentation for this module with the perldoc command.

    perldoc Math::BigFloat

You can also look for information at:

=over 4

=item * GitHub

L<https://github.com/pjacklam/p5-Math-BigInt>

=item * RT: CPAN's request tracker

L<https://rt.cpan.org/Dist/Display.html?Name=Math-BigInt>

=item * MetaCPAN

L<https://metacpan.org/release/Math-BigInt>

=item * CPAN Testers Matrix

L<http://matrix.cpantesters.org/?dist=Math-BigInt>

=item * CPAN Ratings

L<https://cpanratings.perl.org/dist/Math-BigInt>

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

=head1 SEE ALSO

L<Math::BigInt> and L<Math::BigInt> as well as the backends
L<Math::BigInt::FastCalc>, L<Math::BigInt::GMP>, and L<Math::BigInt::Pari>.

The pragmas L<bignum>, L<bigint> and L<bigrat>.

=head1 AUTHORS

=over 4

=item *

Mark Biggar, overloaded interface by Ilya Zakharevich, 1996-2001.

=item *

Completely rewritten by Tels L<http://bloodgate.com> in 2001-2008.

=item *

Florian Ragwitz E<lt>flora@cpan.orgE<gt>, 2010.

=item *

Peter John Acklam E<lt>pjacklam@gmail.comE<gt>, 2011-.

=back

=cut
