package bigfloat;

use strict;
use warnings;

use Carp qw< carp croak >;

our $VERSION = '0.66';

use Exporter;
our @ISA            = qw( Exporter );
our @EXPORT_OK      = qw( PI e bpi bexp hex oct );
our @EXPORT         = qw( inf NaN );

use overload;

my $obj_class = "Math::BigFloat";

##############################################################################

sub accuracy {
    my $self = shift;
    $obj_class -> accuracy(@_);
}

sub precision {
    my $self = shift;
    $obj_class -> precision(@_);
}

sub round_mode {
    my $self = shift;
    $obj_class -> round_mode(@_);
}

sub div_scale {
    my $self = shift;
    $obj_class -> div_scale(@_);
}

sub upgrade {
    my $self = shift;
    $obj_class -> upgrade(@_);
}

sub downgrade {
    my $self = shift;
    $obj_class -> downgrade(@_);
}

sub in_effect {
    my $level = shift || 0;
    my $hinthash = (caller($level))[10];
    $hinthash->{bigfloat};
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
        return $obj_class -> new($nstr);
    }

    # If we get here, there is a bug in the code above this point.

    warn "Internal error: unable to handle literal constant '$str'.",
      " This is a bug, so please report this to the module author.";
    return $obj_class -> bnan();
}

#############################################################################
# the following two routines are for "use bigfloat qw/hex oct/;":

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
        $x = $obj_class -> from_hex($chrs);
    } else {
        $x = $obj_class -> bzero();
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
            $x = $obj_class -> from_bin($chrs);
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
        $x = $obj_class -> from_oct($chrs);
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
    return $$hh{bigfloat} ? bigfloat::_hex_core($_[0])
         : $$hh{bigrat} ? bigrat::_hex_core($_[0])
         : $$hh{bigint} ? bigint::_hex_core($_[0])
         : $prev_hex    ? &$prev_hex($_[0])
         : CORE::hex($_[0]);
}

sub _oct(_) {
    my $hh = (caller 0)[10];
    return $$hh{bigfloat} ? bigfloat::_oct_core($_[0])
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
    $^H{bigfloat} = undef;        # no longer in effect
    overload::remove_constant('binary', '', 'float', '', 'integer');
}

sub import {
    my $class = shift;

    $^H{bigfloat} = 1;                  # we are in effect
    $^H{bigint} = undef;
    $^H{bigrat} = undef;

    # for newer Perls always override hex() and oct() with a lexical version:
    if (LEXICAL) {
        _override();
    }

    my @import = ();
    my @a = ();                         # unrecognized arguments
    my $ver;                            # version?

    while (@_) {
        my $param = shift;

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

        if ($param =~ /^(t|trace)$/) {
            $obj_class .= "::Trace";
            eval "require $obj_class";
            die $@ if $@;
            next;
        }

        if ($param =~ /^(PI|e|bexp|bpi|hex|oct)\z/) {
            push @a, $param;
            next;
        }

        croak("Unknown option '$param'");
    }

    eval "require $obj_class";
    die $@ if $@;
    $obj_class -> import(@import);

    if ($ver) {
        printf "%-31s v%s\n", $class, $class -> VERSION();
        printf " lib => %-23s v%s\n",
          $obj_class -> config("lib"), $obj_class -> config("lib_version");
        printf "%-31s v%s\n", $obj_class, $obj_class -> VERSION();
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
            return $obj_class -> new($str);
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
            return $obj_class -> new($str) if $str =~ /^0[XxBb]/;
            $obj_class -> from_oct($str);
        };
}

sub inf () { $obj_class -> binf(); }
sub NaN () { $obj_class -> bnan(); }

# This should depend on the current accuracy/precision. Fixme!
sub PI  () { $obj_class -> new('3.141592653589793238462643383279502884197'); }
sub e   () { $obj_class -> new('2.718281828459045235360287471352662497757'); }

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

    my $x = Math::BigFloat -> new(shift);
    $x -> bexp(@_);

    Math::BigFloat -> upgrade($up);         # reset the upgrading

    return $x;
}

1;

__END__

=pod

=head1 NAME

bigfloat - transparent big floating point number support for Perl

=head1 SYNOPSIS

    use bigfloat;

    $x = 2 + 4.5;                       # Math::BigFloat 6.5
    print 2 ** 512 * 0.1;               # Math::BigFloat 134...09.6
    print inf + 42;                     # Math::BigFloat inf
    print NaN * 7;                      # Math::BigFloat NaN
    print hex("0x1234567890123490");    # Perl v5.10.0 or later

    {
        no bigfloat;
        print 2 ** 256;                 # a normal Perl scalar now
    }

    # for older Perls, import into current package:
    use bigfloat qw/hex oct/;
    print hex("0x1234567890123490");
    print oct("01234567890123490");

=head1 DESCRIPTION

All numeric literals in the given scope are converted to Math::BigFloat objects.

All operators (including basic math operations) except the range operator C<..>
are overloaded.

So, the following:

    use bigfloat;
    $x = 1234;

creates a Math::BigFloat and stores a reference to in $x. This happens
transparently and behind your back, so to speak.

You can see this with the following:

    perl -Mbigfloat -le 'print ref(1234)'

Since numbers are actually objects, you can call all the usual methods from
Math::BigFloat on them. This even works to some extent on expressions:

    perl -Mbigfloat -le '$x = 1234; print $x->bdec()'
    perl -Mbigfloat -le 'print 1234->copy()->binc();'
    perl -Mbigfloat -le 'print 1234->copy()->binc->badd(6);'
    perl -Mbigfloat -le 'print +(1234)->copy()->binc()'

(Note that print doesn't do what you expect if the expression starts with
'(' hence the C<+>)

You can even chain the operations together as usual:

    perl -Mbigfloat -le 'print 1234->copy()->binc->badd(6);'
    1241

Please note the following does not work as expected (prints nothing), since
overloading of '..' is not yet possible in Perl (as of v5.8.0):

    perl -Mbigfloat -le 'for (1..2) { print ref($_); }'

=head2 Options

C<bigfloat> recognizes some options that can be passed while loading it via via
C<use>. The following options exist:

=over 4

=item a or accuracy

This sets the accuracy for all math operations. The argument must be greater
than or equal to zero. See Math::BigInt's bround() method for details.

    perl -Mbigfloat=a,50 -le 'print sqrt(20)'

Note that setting precision and accuracy at the same time is not possible.

=item p or precision

This sets the precision for all math operations. The argument can be any
integer. Negative values mean a fixed number of digits after the dot, while a
positive value rounds to this digit left from the dot. 0 means round to integer.
See Math::BigInt's bfround() method for details.

    perl -Mbigfloat=p,-50 -le 'print sqrt(20)'

Note that setting precision and accuracy at the same time is not possible.

=item t or trace

This enables a trace mode and is primarily for debugging.

=item l, lib, try, or only

Load a different math lib, see L<Math Library>.

    perl -Mbigfloat=l,GMP -e 'print 2 ** 512'
    perl -Mbigfloat=lib,GMP -e 'print 2 ** 512'
    perl -Mbigfloat=try,GMP -e 'print 2 ** 512'
    perl -Mbigfloat=only,GMP -e 'print 2 ** 512'

=item hex

Override the built-in hex() method with a version that can handle big numbers.
This overrides it by exporting it to the current package. Under Perl v5.10.0 and
higher, this is not so necessary, as hex() is lexically overridden in the
current scope whenever the C<bigfloat> pragma is active.

=item oct

Override the built-in oct() method with a version that can handle big numbers.
This overrides it by exporting it to the current package. Under Perl v5.10.0 and
higher, this is not so necessary, as oct() is lexically overridden in the
current scope whenever the C<bigfloat> pragma is active.

=item v or version

this prints out the name and version of the modules and then exits.

    perl -Mbigfloat=v

=back

=head2 Math Library

Math with the numbers is done (by default) by a backend library module called
Math::BigInt::Calc. The default is equivalent to saying:

    use bigfloat lib => 'Calc';

you can change this by using:

    use bigfloat lib => 'GMP';

The following would first try to find Math::BigInt::Foo, then Math::BigInt::Bar,
and if this also fails, revert to Math::BigInt::Calc:

    use bigfloat lib => 'Foo,Math::BigInt::Bar';

Using c<lib> warns if none of the specified libraries can be found and
L<Math::BigInt> fell back to one of the default libraries. To suppress this
warning, use c<try> instead:

    use bigfloat try => 'GMP';

If you want the code to die instead of falling back, use C<only> instead:

    use bigfloat only => 'GMP';

Please see respective module documentation for further details.

=head2 Method calls

Since all numbers are now objects, you can use all methods that are part of the
Math::BigFloat API.

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

A shortcut to return Math::BigFloat->binf(). Useful because Perl does not always
handle bareword C<inf> properly.

=item NaN()

A shortcut to return Math::BigFloat->bnan(). Useful because Perl does not always
handle bareword C<NaN> properly.

=item e

    # perl -Mbigfloat=e -wle 'print e'

Returns Euler's number C<e>, aka exp(1)

=item PI

    # perl -Mbigfloat=PI -wle 'print PI'

Returns PI.

=item bexp()

    bexp($power, $accuracy);

Returns Euler's number C<e> raised to the appropriate power, to the wanted
accuracy.

Example:

    # perl -Mbigfloat=bexp -wle 'print bexp(1,80)'

=item bpi()

    bpi($accuracy);

Returns PI to the wanted accuracy.

Example:

    # perl -Mbigfloat=bpi -wle 'print bpi(80)'

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
upgrade class to C<undef> to disable upgrading.

Upgrading is disabled by default.

=item downgrade()

Set or get the class that the upgrade class downgrades to, if any. Set the
downgrade class to C<undef> to disable upgrading.

Downgrading is disabled by default.

=item in_effect()

    use bigfloat;

    print "in effect\n" if bigfloat::in_effect;       # true
    {
        no bigfloat;
        print "in effect\n" if bigfloat::in_effect;   # false
    }

Returns true or false if C<bigfloat> is in effect in the current scope.

This method only works on Perl v5.9.4 or later.

=back

=head1 CAVEATS

=over 4

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

outputs C<0> on default 32-bit builds, since C<bigfloat> never sees the string
literals. To ensure the expression is all treated as C<Math::BigFloat> objects,
use a literal number in the expression:

    print +(0+$x) - $y;

=item Ranges

Perl does not allow overloading of ranges, so you can neither safely use ranges
with C<bigfloat> endpoints, nor is the iterator variable a C<Math::BigFloat>.

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

C<bigfloat> overrides these routines with versions that can also handle big
integer values. Under Perl prior to version v5.9.4, however, this will not
happen unless you specifically ask for it with the two import tags "hex" and
"oct" - and then it will be global and cannot be disabled inside a scope with
C<no bigfloat>:

    use bigfloat qw/hex oct/;

    print hex("0x1234567890123456");
    {
        no bigfloat;
        print hex("0x1234567890123456");
    }

The second call to hex() will warn about a non-portable constant.

Compare this to:

    use bigfloat;

    # will warn only under Perl older than v5.9.4
    print hex("0x1234567890123456");

=back

=head1 EXAMPLES

Some cool command line examples to impress the Python crowd ;)

    perl -Mbigfloat -le 'print sqrt(33)'
    perl -Mbigfloat -le 'print 2**255'
    perl -Mbigfloat -le 'print 4.5+2**255'
    perl -Mbigfloat -le 'print 3/7 + 5/7 + 8/3'
    perl -Mbigfloat -le 'print 123->is_odd()'
    perl -Mbigfloat -le 'print log(2)'
    perl -Mbigfloat -le 'print exp(1)'
    perl -Mbigfloat -le 'print 2 ** 0.5'
    perl -Mbigfloat=a,65 -le 'print 2 ** 0.2'
    perl -Mbigfloat=l,GMP -le 'print 7 ** 7777'

=head1 BUGS

Please report any bugs or feature requests to
C<bug-bignum at rt.cpan.org>, or through the web interface at
L<https://rt.cpan.org/Ticket/Create.html?Queue=bignum> (requires login).
We will be notified, and then you'll automatically be notified of
progress on your bug as I make changes.

=head1 SUPPORT

You can find documentation for this module with the perldoc command.

    perldoc bigfloat

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
