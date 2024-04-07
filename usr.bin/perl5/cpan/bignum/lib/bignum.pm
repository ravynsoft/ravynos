package bignum;

use strict;
use warnings;

use Carp qw< carp croak >;

our $VERSION = '0.66';

use Exporter;
our @ISA            = qw( Exporter );
our @EXPORT_OK      = qw( PI e bpi bexp hex oct );
our @EXPORT         = qw( inf NaN );

use overload;

# Defaults: When a constant is an integer, Inf or NaN, it is converted to an
# object of class $int_class. When a constant is a finite non-integer, it is
# converted to an object of class $float_class.

my $int_class = 'Math::BigInt';
my $float_class = 'Math::BigFloat';

##############################################################################

sub accuracy {
    shift;
    $int_class -> accuracy(@_);
    $float_class -> accuracy(@_);
}

sub precision {
    shift;
    $int_class -> precision(@_);
    $float_class -> precision(@_);
}

sub round_mode {
    shift;
    $int_class -> round_mode(@_);
    $float_class -> round_mode(@_);
}

sub div_scale {
    shift;
    $int_class -> div_scale(@_);
    $float_class -> div_scale(@_);
}

sub upgrade {
    shift;
    $int_class -> upgrade(@_);
}

sub downgrade {
    shift;
    $float_class -> downgrade(@_);
}

sub in_effect {
    my $level = shift || 0;
    my $hinthash = (caller($level))[10];
    $hinthash->{bignum};
}

sub _float_constant {
    my $str = shift;

    # See if we can convert the input string to a string using a normalized form
    # consisting of the significand as a signed integer, the character "e", and
    # the exponent as a signed integer, e.g., "+0e+0", "+314e-2", and "-1e+3".

    my $nstr;

    if (
        # See if it is an octal number. An octal number like '0377' is also
        # accepted by the functions parsing decimal and hexadecimal numbers, so
        # handle octal numbers before decimal and hexadecimal numbers.

        $str =~ /^0(?:[Oo]|_*[0-7])/ and
        $nstr = Math::BigInt -> oct_str_to_dec_flt_str($str)

          or

        # See if it is decimal number.

        $nstr = Math::BigInt -> dec_str_to_dec_flt_str($str)

          or

        # See if it is a hexadecimal number. Every hexadecimal number has a
        # prefix, but the functions parsing numbers don't require it, so check
        # to see if it actually is a hexadecimal number.

        $str =~ /^0[Xx]/ and
        $nstr = Math::BigInt -> hex_str_to_dec_flt_str($str)

          or

        # See if it is a binary numbers. Every binary number has a prefix, but
        # the functions parsing numbers don't require it, so check to see if it
        # actually is a binary number.

        $str =~ /^0[Bb]/ and
        $nstr = Math::BigInt -> bin_str_to_dec_flt_str($str))
    {
        my $pos      = index($nstr, 'e');
        my $expo_sgn = substr($nstr, $pos + 1, 1);
        my $sign     = substr($nstr, 0, 1);
        my $mant     = substr($nstr, 1, $pos - 1);
        my $mant_len = CORE::length($mant);
        my $expo     = substr($nstr, $pos + 2);

        # The number is a non-integer if and only if the exponent is negative.

        if ($expo_sgn eq '-') {
            return $float_class -> new($str);

            my $upgrade = $int_class -> upgrade();
            return $upgrade -> new($nstr) if defined $upgrade;

            if ($mant_len <= $expo) {
                return $int_class -> bzero();                   # underflow
            } else {
                $mant = substr $mant, 0, $mant_len - $expo;     # truncate
                return $int_class -> new($sign . $mant);
            }
        } else {
            $mant .= "0" x $expo;                               # pad with zeros
            return $int_class -> new($sign . $mant);
        }
    }

    # If we get here, there is a bug in the code above this point.

    warn "Internal error: unable to handle literal constant '$str'.",
      " This is a bug, so please report this to the module author.";
    return $int_class -> bnan();
}

#############################################################################
# the following two routines are for "use bignum qw/hex oct/;":

use constant LEXICAL => $] > 5.009004;

# Internal function with the same semantics as CORE::hex(). This function is
# not used directly, but rather by other front-end functions.

sub _hex_core {
    my $str = shift;

    # Strip off, clean, and parse as much as we can from the beginning.

    my $x;
    if ($str =~ s/ ^ ( 0? [xX] )? ( [0-9a-fA-F]* ( _ [0-9a-fA-F]+ )* ) //x) {
        my $chrs = $2;
        $chrs =~ tr/_//d;
        $chrs = '0' unless CORE::length $chrs;
        $x = $int_class -> from_hex($chrs);
    } else {
        $x = $int_class -> bzero();
    }

    # Warn about trailing garbage.

    if (CORE::length($str)) {
        require Carp;
        Carp::carp(sprintf("Illegal hexadecimal digit '%s' ignored",
                           substr($str, 0, 1)));
    }

    return $x;
}

# Internal function with the same semantics as CORE::oct(). This function is
# not used directly, but rather by other front-end functions.

sub _oct_core {
    my $str = shift;

    $str =~ s/^\s*//;

    # Hexadecimal input.

    return _hex_core($str) if $str =~ /^0?[xX]/;

    my $x;

    # Binary input.

    if ($str =~ /^0?[bB]/) {

        # Strip off, clean, and parse as much as we can from the beginning.

        if ($str =~ s/ ^ ( 0? [bB] )? ( [01]* ( _ [01]+ )* ) //x) {
            my $chrs = $2;
            $chrs =~ tr/_//d;
            $chrs = '0' unless CORE::length $chrs;
            $x = $int_class -> from_bin($chrs);
        }

        # Warn about trailing garbage.

        if (CORE::length($str)) {
            require Carp;
            Carp::carp(sprintf("Illegal binary digit '%s' ignored",
                               substr($str, 0, 1)));
        }

        return $x;
    }

    # Octal input. Strip off, clean, and parse as much as we can from the
    # beginning.

    if ($str =~ s/ ^ ( 0? [oO] )? ( [0-7]* ( _ [0-7]+ )* ) //x) {
        my $chrs = $2;
        $chrs =~ tr/_//d;
        $chrs = '0' unless CORE::length $chrs;
        $x = $int_class -> from_oct($chrs);
    }

    # Warn about trailing garbage. CORE::oct() only warns about 8 and 9, but it
    # is more helpful to warn about all invalid digits.

    if (CORE::length($str)) {
        require Carp;
        Carp::carp(sprintf("Illegal octal digit '%s' ignored",
                           substr($str, 0, 1)));
    }

    return $x;
}

{
    my $proto = LEXICAL ? '_' : ';$';
    eval '
sub hex(' . $proto . ') {' . <<'.';
    my $str = @_ ? $_[0] : $_;
    _hex_core($str);
}
.

    eval '
sub oct(' . $proto . ') {' . <<'.';
    my $str = @_ ? $_[0] : $_;
    _oct_core($str);
}
.
}

#############################################################################
# the following two routines are for Perl 5.9.4 or later and are lexical

my ($prev_oct, $prev_hex, $overridden);

if (LEXICAL) { eval <<'.' }
sub _hex(_) {
    my $hh = (caller 0)[10];
    return $$hh{bignum} ? bignum::_hex_core($_[0])
         : $$hh{bigrat} ? bigrat::_hex_core($_[0])
         : $$hh{bigint} ? bigint::_hex_core($_[0])
         : $prev_hex    ? &$prev_hex($_[0])
         : CORE::hex($_[0]);
}

sub _oct(_) {
    my $hh = (caller 0)[10];
    return $$hh{bignum} ? bignum::_oct_core($_[0])
         : $$hh{bigrat} ? bigrat::_oct_core($_[0])
         : $$hh{bigint} ? bigint::_oct_core($_[0])
         : $prev_oct    ? &$prev_oct($_[0])
         : CORE::oct($_[0]);
}
.

sub _override {
    return if $overridden;
    $prev_oct = *CORE::GLOBAL::oct{CODE};
    $prev_hex = *CORE::GLOBAL::hex{CODE};
    no warnings 'redefine';
    *CORE::GLOBAL::oct = \&_oct;
    *CORE::GLOBAL::hex = \&_hex;
    $overridden = 1;
}

sub unimport {
    $^H{bignum} = undef;        # no longer in effect
    overload::remove_constant('binary', '', 'float', '', 'integer');
}

sub import {
    my $class = shift;

    $^H{bignum} = 1;                    # we are in effect
    $^H{bigint} = undef;
    $^H{bigrat} = undef;

    # for newer Perls always override hex() and oct() with a lexical version:
    if (LEXICAL) {
        _override();
    }

    my @import     = ();                        # common options
    my @int_import = (upgrade => $float_class); # int class only options
    my @flt_import = (downgrade => $int_class); # float class only options
    my @a = ();                                 # unrecognized arguments
    my $ver;                                    # display version info?

    while (@_) {
        my $param = shift;

        # Upgrading.

        if ($param eq 'upgrade') {
            my $arg = shift;
            $float_class = $arg if defined $arg;
            push @int_import, 'upgrade', $arg;
            next;
        }

        # Downgrading.

        if ($param eq 'downgrade') {
            my $arg = shift;
            $int_class = $arg if defined $arg;
            push @flt_import, 'downgrade', $arg;
            next;
        }

        # Accuracy.

        if ($param =~ /^a(ccuracy)?$/) {
            push @import, 'accuracy', shift();
            next;
        }

        # Precision.

        if ($param =~ /^p(recision)?$/) {
            push @import, 'precision', shift();
            next;
        }

        # Rounding mode.

        if ($param eq 'round_mode') {
            push @import, 'round_mode', shift();
            next;
        }

        # Backend library.

        if ($param =~ /^(l|lib|try|only)$/) {
            push @import, $param eq 'l' ? 'lib' : $param;
            push @import, shift() if @_;
            next;
        }

        if ($param =~ /^(v|version)$/) {
            $ver = 1;
            next;
        }

        if ($param =~ /^(PI|e|bexp|bpi|hex|oct)\z/) {
            push @a, $param;
            next;
        }

        croak("Unknown option '$param'");
    }

    eval "require $int_class";
    die $@ if $@;
    $int_class -> import(@int_import, @import);

    eval "require $float_class";
    die $@ if $@;
    $float_class -> import(@flt_import, @import);

    if ($ver) {
        printf "%-31s v%s\n", $class, $class -> VERSION();
        printf " lib => %-23s v%s\n",
          $int_class -> config("lib"), $int_class -> config("lib_version");
        printf "%-31s v%s\n", $int_class, $int_class -> VERSION();
        exit;
    }

    $class -> export_to_level(1, $class, @a);   # export inf, NaN, etc.

    overload::constant

        # This takes care each number written as decimal integer and within the
        # range of what perl can represent as an integer, e.g., "314", but not
        # "3141592653589793238462643383279502884197169399375105820974944592307".

        integer => sub {
            #printf "Value '%s' handled by the 'integer' sub.\n", $_[0];
            my $str = shift;
            return $int_class -> new($str);
        },

        # This takes care of each number written with a decimal point and/or
        # using floating point notation, e.g., "3.", "3.0", "3.14e+2" (decimal),
        # "0b1.101p+2" (binary), "03.14p+2" and "0o3.14p+2" (octal), and
        # "0x3.14p+2" (hexadecimal).

        float => sub {
            #printf "# Value '%s' handled by the 'float' sub.\n", $_[0];
            _float_constant(shift);
        },

        # Take care of each number written as an integer (no decimal point or
        # exponent) using binary, octal, or hexadecimal notation, e.g., "0b101"
        # (binary), "0314" and "0o314" (octal), and "0x314" (hexadecimal).

        binary => sub {
            #printf "# Value '%s' handled by the 'binary' sub.\n", $_[0];
            my $str = shift;
            return $int_class -> new($str) if $str =~ /^0[XxBb]/;
            $int_class -> from_oct($str);
        };
}

sub inf () { $int_class -> binf(); }
sub NaN () { $int_class -> bnan(); }

# This should depend on the current accuracy/precision. Fixme!
sub PI  () { $float_class -> new('3.141592653589793238462643383279502884197'); }
sub e   () { $float_class -> new('2.718281828459045235360287471352662497757'); }

sub bpi ($) {
    my $up = Math::BigFloat -> upgrade();   # get current upgrading, if any ...
    Math::BigFloat -> upgrade(undef);       # ... and disable
    my $x = Math::BigFloat -> bpi(@_);
    Math::BigFloat -> upgrade($up);         # reset the upgrading
    return $x;
}

sub bexp ($$) {
    my $up = Math::BigFloat -> upgrade();   # get current upgrading, if any ...
    Math::BigFloat -> upgrade(undef);       # ... and disable
    my $x = Math::BigFloat -> new(shift) -> bexp(@_);
    Math::BigFloat -> upgrade($up);         # reset the upgrading
    return $x;
}

1;

__END__

=pod

=head1 NAME

bignum - transparent big number support for Perl

=head1 SYNOPSIS

    use bignum;

    $x = 2 + 4.5;                       # Math::BigFloat 6.5
    print 2 ** 512 * 0.1;               # Math::BigFloat 134...09.6
    print 2 ** 512;                     # Math::BigInt 134...096
    print inf + 42;                     # Math::BigInt inf
    print NaN * 7;                      # Math::BigInt NaN
    print hex("0x1234567890123490");    # Perl v5.10.0 or later

    {
        no bignum;
        print 2 ** 256;                 # a normal Perl scalar now
    }

    # for older Perls, import into current package:
    use bignum qw/hex oct/;
    print hex("0x1234567890123490");
    print oct("01234567890123490");

=head1 DESCRIPTION

=head2 Literal numeric constants

By default, every literal integer becomes a Math::BigInt object, and literal
non-integer becomes a Math::BigFloat object. Whether a numeric literal is
considered an integer or non-integers depends only on the value of the constant,
not on how it is represented. For instance, the constants 3.14e2 and 0x1.3ap8
become Math::BigInt objects, because they both represent the integer value
decimal 314.

The default C<use bignum;> is equivalent to

    use bignum downgrade => "Math::BigInt", upgrade => "Math::BigFloat";

The classes used for integers and non-integers can be set at compile time with
the C<downgrade> and C<upgrade> options, for example

    # use Math::BigInt for integers and Math::BigRat for non-integers
    use bignum upgrade => "Math::BigRat";

Note that disabling downgrading and upgrading does not affect how numeric
literals are converted to objects

    # disable both downgrading and upgrading
    use bignum downgrade => undef, upgrade => undef;
    $x = 2.4;       # becomes 2.4 as a Math::BigFloat
    $y = 2;         # becomes 2 as a Math::BigInt

=head2 Upgrading and downgrading

By default, when the result of a computation is an integer, an Inf, or a NaN,
the result is downgraded even when all the operands are instances of the upgrade
class.

    use bignum;
    $x = 2.4;       # becomes 2.4 as a Math::BigFloat
    $y = 1.2;       # becomes 1.2 as a Math::BigFloat
    $z = $x / $y;   # becomes 2 as a Math::BigInt due to downgrading

Equivalently, by default, when the result of a computation is a finite
non-integer, the result is upgraded even when all the operands are instances of
the downgrade class.

    use bignum;
    $x = 7;         # becomes 7 as a Math::BigInt
    $y = 2;         # becomes 2 as a Math::BigInt
    $z = $x / $y;   # becomes 3.5 as a Math::BigFloat due to upgrading

The classes used for downgrading and upgrading can be set at runtime with the
L</downgrade()> and L</upgrade()> methods, but see L</CAVEATS> below.

The upgrade and downgrade classes don't have to be Math::BigInt and
Math::BigFloat. For example, to use Math::BigRat as the upgrade class, use

    use bignum upgrade => "Math::BigRat";
    $x = 2;         # becomes 2 as a Math::BigInt
    $y = 3.6;       # becomes 18/5 as a Math::BigRat

The upgrade and downgrade classes can be modified at runtime

    use bignum;
    $x = 3;         # becomes 3 as a Math::BigInt
    $y = 2;         # becomes 2 as a Math::BigInt
    $z = $x / $y;   # becomes 1.5 as a Math::BigFlaot

    bignum -> upgrade("Math::BigRat");
    $w = $x / $y;   # becomes 3/2 as a Math::BigRat

Disabling downgrading doesn't change the fact that literal constant integers are
converted to the downgrade class, it only prevents downgrading as a result of a
computation. E.g.,

    use bignum downgrade => undef;
    $x = 2;         # becomes 2 as a Math::BigInt
    $y = 2.4;       # becomes 2.4 as a Math::BigFloat
    $z = 1.2;       # becomes 1.2 as a Math::BigFloat
    $w = $x / $y;   # becomes 2 as a Math::BigFloat due to no downgrading

If you want all numeric literals, both integers and non-integers, to become
Math::BigFloat objects, use the L<bigfloat> pragma.

Equivalently, disabling upgrading doesn't change the fact that literal constant
non-integers are converted to the upgrade class, it only prevents upgrading as a
result of a computation. E.g.,

    use bignum upgrade => undef;
    $x = 2.5;       # becomes 2.5 as a Math::BigFloat
    $y = 7;         # becomes 7 as a Math::BigInt
    $z = 2;         # becomes 2 as a Math::BigInt
    $w = $x / $y;   # becomes 3 as a Math::BigInt due to no upgrading

If you want all numeric literals, both integers and non-integers, to become
Math::BigInt objects, use the L<bigint> pragma.

You can even do

    use bignum upgrade => "Math::BigRat", upgrade => undef;

which converts all integer literals to Math::BigInt objects and all non-integer
literals to Math::BigRat objects. However, when the result of a computation
involving two Math::BigInt objects results in a non-integer (e.g., 7/2), the
result will be truncted to a Math::BigInt rather than being upgraded to a
Math::BigRat, since upgrading is disabled.

=head2 Overloading

Since all numeric literals become objects, you can call all the usual methods
from Math::BigInt and Math::BigFloat on them. This even works to some extent on
expressions:

    perl -Mbignum -le '$x = 1234; print $x->bdec()'
    perl -Mbignum -le 'print 1234->copy()->binc();'
    perl -Mbignum -le 'print 1234->copy()->binc()->badd(6);'

=head2 Options

C<bignum> recognizes some options that can be passed while loading it via via
C<use>. The following options exist:

=over 4

=item a or accuracy

This sets the accuracy for all math operations. The argument must be greater
than or equal to zero. See Math::BigInt's bround() method for details.

    perl -Mbignum=a,50 -le 'print sqrt(20)'

Note that setting precision and accuracy at the same time is not possible.

=item p or precision

This sets the precision for all math operations. The argument can be any
integer. Negative values mean a fixed number of digits after the dot, while a
positive value rounds to this digit left from the dot. 0 means round to integer.
See Math::BigInt's bfround() method for details.

    perl -Mbignum=p,-50 -le 'print sqrt(20)'

Note that setting precision and accuracy at the same time is not possible.

=item l, lib, try, or only

Load a different math lib, see L<Math Library>.

    perl -Mbignum=l,GMP -e 'print 2 ** 512'
    perl -Mbignum=lib,GMP -e 'print 2 ** 512'
    perl -Mbignum=try,GMP -e 'print 2 ** 512'
    perl -Mbignum=only,GMP -e 'print 2 ** 512'

=item hex

Override the built-in hex() method with a version that can handle big numbers.
This overrides it by exporting it to the current package. Under Perl v5.10.0 and
higher, this is not so necessary, as hex() is lexically overridden in the
current scope whenever the C<bignum> pragma is active.

=item oct

Override the built-in oct() method with a version that can handle big numbers.
This overrides it by exporting it to the current package. Under Perl v5.10.0 and
higher, this is not so necessary, as oct() is lexically overridden in the
current scope whenever the C<bignum> pragma is active.

=item v or version

this prints out the name and version of the modules and then exits.

    perl -Mbignum=v

=back

=head2 Math Library

Math with the numbers is done (by default) by a backend library module called
Math::BigInt::Calc. The default is equivalent to saying:

    use bignum lib => 'Calc';

you can change this by using:

    use bignum lib => 'GMP';

The following would first try to find Math::BigInt::Foo, then Math::BigInt::Bar,
and if this also fails, revert to Math::BigInt::Calc:

    use bignum lib => 'Foo,Math::BigInt::Bar';

Using c<lib> warns if none of the specified libraries can be found and
L<Math::BigInt> and L<Math::BigFloat> fell back to one of the default
libraries. To suppress this warning, use C<try> instead:

    use bignum try => 'GMP';

If you want the code to die instead of falling back, use C<only> instead:

    use bignum only => 'GMP';

Please see respective module documentation for further details.

=head2 Method calls

Since all numbers are now objects, you can use the methods that are part of the
Math::BigInt and Math::BigFloat API.

But a warning is in order. When using the following to make a copy of a number,
only a shallow copy will be made.

    $x = 9; $y = $x;
    $x = $y = 7;

Using the copy or the original with overloaded math is okay, e.g., the following
work:

    $x = 9; $y = $x;
    print $x + 1, " ", $y,"\n";     # prints 10 9

but calling any method that modifies the number directly will result in B<both>
the original and the copy being destroyed:

    $x = 9; $y = $x;
    print $x->badd(1), " ", $y,"\n";        # prints 10 10

    $x = 9; $y = $x;
    print $x->binc(1), " ", $y,"\n";        # prints 10 10

    $x = 9; $y = $x;
    print $x->bmul(2), " ", $y,"\n";        # prints 18 18

Using methods that do not modify, but test that the contents works:

    $x = 9; $y = $x;
    $z = 9 if $x->is_zero();                # works fine

See the documentation about the copy constructor and C<=> in overload, as well
as the documentation in Math::BigFloat for further details.

=head2 Methods

=over 4

=item inf()

A shortcut to return C<inf> as an object. Useful because Perl does not always
handle bareword C<inf> properly.

=item NaN()

A shortcut to return C<NaN> as an object. Useful because Perl does not always
handle bareword C<NaN> properly.

=item e

    # perl -Mbignum=e -wle 'print e'

Returns Euler's number C<e>, aka exp(1) (= 2.7182818284...).

=item PI

    # perl -Mbignum=PI -wle 'print PI'

Returns PI (= 3.1415926532..).

=item bexp()

    bexp($power, $accuracy);

Returns Euler's number C<e> raised to the appropriate power, to the wanted
accuracy.

Example:

    # perl -Mbignum=bexp -wle 'print bexp(1,80)'

=item bpi()

    bpi($accuracy);

Returns PI to the wanted accuracy.

Example:

    # perl -Mbignum=bpi -wle 'print bpi(80)'

=item accuracy()

Set or get the accuracy.

=item precision()

Set or get the precision.

=item round_mode()

Set or get the rounding mode.

=item div_scale()

Set or get the division scale.

=item upgrade()

Set or get the class that the downgrade class upgrades to, if any. Set the
upgrade class to C<undef> to disable upgrading. See C</CAVEATS> below.

=item downgrade()

Set or get the class that the upgrade class downgrades to, if any. Set the
downgrade class to C<undef> to disable upgrading. See L</CAVEATS> below.

=item in_effect()

    use bignum;

    print "in effect\n" if bignum::in_effect;       # true
    {
        no bignum;
        print "in effect\n" if bignum::in_effect;   # false
    }

Returns true or false if C<bignum> is in effect in the current scope.

This method only works on Perl v5.9.4 or later.

=back

=head1 CAVEATS

=over 4

=item The upgrade() and downgrade() methods

Note that setting both the upgrade and downgrade classes at runtime with the
L</upgrade()> and L</downgrade()> methods, might not do what you expect:

    # Assuming that downgrading and upgrading hasn't been modified so far, so
    # the downgrade and upgrade classes are Math::BigInt and Math::BigFloat,
    # respectively, the following sets the upgrade class to Math::BigRat, i.e.,
    # makes Math::BigInt upgrade to Math::BigRat:

    bignum -> upgrade("Math::BigRat");

    # The following sets the downgrade class to Math::BigInt::Lite, i.e., makes
    # the new upgrade class Math::BigRat downgrade to Math::BigInt::Lite

    bignum -> downgrade("Math::BigInt::Lite");

    # Note that at this point, it is still Math::BigInt, not Math::BigInt::Lite,
    # that upgrades to Math::BigRat, so to get Math::BigInt::Lite to upgrade to
    # Math::BigRat, we need to do the following (again):

    bignum -> upgrade("Math::BigRat");

A simpler way to do this at runtime is to use import(),

    bignum -> import(upgrade => "Math::BigRat",
                     downgrade => "Math::BigInt::Lite");

=item Hexadecimal, octal, and binary floating point literals

Perl (and this module) accepts hexadecimal, octal, and binary floating point
literals, but use them with care with Perl versions before v5.32.0, because some
versions of Perl silently give the wrong result.

=item Operator vs literal overloading

C<bigrat> works by overloading handling of integer and floating point literals,
converting them to L<Math::BigRat> objects.

This means that arithmetic involving only string values or string literals are
performed using Perl's built-in operators.

For example:

    use bigrat;
    my $x = "900000000000000009";
    my $y = "900000000000000007";
    print $x - $y;

outputs C<0> on default 32-bit builds, since C<bignum> never sees the string
literals. To ensure the expression is all treated as C<Math::BigFloat> objects,
use a literal number in the expression:

    print +(0+$x) - $y;

=item Ranges

Perl does not allow overloading of ranges, so you can neither safely use ranges
with C<bignum> endpoints, nor is the iterator variable a C<Math::BigFloat>.

    use 5.010;
    for my $i (12..13) {
      for my $j (20..21) {
        say $i ** $j;  # produces a floating-point number,
                       # not an object
      }
    }

=item in_effect()

This method only works on Perl v5.9.4 or later.

=item hex()/oct()

C<bignum> overrides these routines with versions that can also handle big
integer values. Under Perl prior to version v5.9.4, however, this will not
happen unless you specifically ask for it with the two import tags "hex" and
"oct" - and then it will be global and cannot be disabled inside a scope with
C<no bignum>:

    use bignum qw/hex oct/;

    print hex("0x1234567890123456");
    {
        no bignum;
        print hex("0x1234567890123456");
    }

The second call to hex() will warn about a non-portable constant.

Compare this to:

    use bignum;

    # will warn only under Perl older than v5.9.4
    print hex("0x1234567890123456");

=back

=head1 EXAMPLES

Some cool command line examples to impress the Python crowd ;)

    perl -Mbignum -le 'print sqrt(33)'
    perl -Mbignum -le 'print 2**255'
    perl -Mbignum -le 'print 4.5+2**255'
    perl -Mbignum -le 'print 3/7 + 5/7 + 8/3'
    perl -Mbignum -le 'print 123->is_odd()'
    perl -Mbignum -le 'print log(2)'
    perl -Mbignum -le 'print exp(1)'
    perl -Mbignum -le 'print 2 ** 0.5'
    perl -Mbignum=a,65 -le 'print 2 ** 0.2'
    perl -Mbignum=l,GMP -le 'print 7 ** 7777'

=head1 BUGS

Please report any bugs or feature requests to
C<bug-bignum at rt.cpan.org>, or through the web interface at
L<https://rt.cpan.org/Ticket/Create.html?Queue=bignum> (requires login).
We will be notified, and then you'll automatically be notified of
progress on your bug as I make changes.

=head1 SUPPORT

You can find documentation for this module with the perldoc command.

    perldoc bignum

You can also look for information at:

=over 4

=item * GitHub

L<https://github.com/pjacklam/p5-bignum>

=item * RT: CPAN's request tracker

L<https://rt.cpan.org/Dist/Display.html?Name=bignum>

=item * MetaCPAN

L<https://metacpan.org/release/bignum>

=item * CPAN Testers Matrix

L<http://matrix.cpantesters.org/?dist=bignum>

=item * CPAN Ratings

L<https://cpanratings.perl.org/dist/bignum>

=back

=head1 LICENSE

This program is free software; you may redistribute it and/or modify it under
the same terms as Perl itself.

=head1 SEE ALSO

L<bigint> and L<bigrat>.

L<Math::BigInt>, L<Math::BigFloat>, L<Math::BigRat> and L<Math::Big> as well as
L<Math::BigInt::FastCalc>, L<Math::BigInt::Pari> and L<Math::BigInt::GMP>.

=head1 AUTHORS

=over 4

=item *

(C) by Tels L<http://bloodgate.com/> in early 2002 - 2007.

=item *

Maintained by Peter John Acklam E<lt>pjacklam@gmail.comE<gt>, 2014-.

=back

=cut
