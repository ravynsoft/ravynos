package Math::BigInt::FastCalc;

use 5.006001;
use strict;
use warnings;

use Carp qw< carp croak >;

use Math::BigInt::Calc 1.999801;

BEGIN {
    our @ISA = qw< Math::BigInt::Calc >;
}

our $VERSION = '0.5013';

my $MAX_EXP_F;      # the maximum possible base 10 exponent with "no integer"
my $MAX_EXP_I;      # the maximum possible base 10 exponent with "use integer"
my $BASE_LEN;       # the current base exponent in use
my $USE_INT;        # whether "use integer" is used in the computations

sub _base_len {
    my $class = shift;

    if (@_) {                           # if called as setter ...
        my ($base_len, $use_int) = @_;

        croak "The base length must be a positive integer"
          unless defined($base_len) && $base_len == int($base_len)
                 && $base_len > 0;

        if ( $use_int && ($base_len > $MAX_EXP_I) ||
            !$use_int && ($base_len > $MAX_EXP_F))
        {
            croak "The maximum base length (exponent) is $MAX_EXP_I with",
              " 'use integer' and $MAX_EXP_F without 'use integer'. The",
              " requested settings, a base length of $base_len ",
              $use_int ? "with" : "without", " 'use integer', is invalid.";
        }

        return $class -> SUPER::_base_len($base_len, $use_int);
    }

    return $class -> SUPER::_base_len();
}

BEGIN {

    my @params = Math::BigInt::FastCalc -> SUPER::_base_len();
    $BASE_LEN  = $params[0];
    $MAX_EXP_F = $params[8];
    $MAX_EXP_I = $params[9];

    # With quadmath support it should work with a base length of 17, because the
    # maximum intermediate value used in the computations is less than 2**113.
    # However, for some reason a base length of 17 doesn't work, but trial and
    # error shows that a base length of 15 works for all methods except
    # _is_odd() and _is_even(). These two methods determine whether the least
    # significand component is odd or even by converting it to a UV and do a
    # bitwise & operation. Because of this, we need to limit the base length to
    # what fits inside an UV.

    require Config;
    my $max_exp_i = int(8 * $Config::Config{uvsize} * log(2) / log(10));
    $MAX_EXP_I = $max_exp_i if $max_exp_i < $MAX_EXP_I;
    $MAX_EXP_F = $MAX_EXP_I if $MAX_EXP_I < $MAX_EXP_F;

    ($BASE_LEN, $USE_INT) = $MAX_EXP_I > $MAX_EXP_F ? ($MAX_EXP_I, 1)
                                                    : ($MAX_EXP_F, 0);

    Math::BigInt::FastCalc -> SUPER::_base_len($BASE_LEN, $USE_INT);
}

##############################################################################
# global constants, flags and accessory

# Announce that we are compatible with MBI v1.83 and up. This method has been
# made redundant. Each backend is now a subclass of Math::BigInt::Lib, which
# provides the methods not present in the subclasses.

sub api_version () { 2; }

require XSLoader;
XSLoader::load(__PACKAGE__, $VERSION, Math::BigInt::Calc->_base_len());

##############################################################################

1;

__END__

=pod

=head1 NAME

Math::BigInt::FastCalc - Math::BigInt::Calc with some XS for more speed

=head1 SYNOPSIS

    # to use it with Math::BigInt
    use Math::BigInt lib => 'FastCalc';

    # to use it with Math::BigFloat
    use Math::BigFloat lib => 'FastCalc';

    # to use it with Math::BigRat
    use Math::BigRat lib => 'FastCalc';

=head1 DESCRIPTION

Math::BigInt::FastCalc inherits from Math::BigInt::Calc.

Provides support for big integer calculations. Not intended to be used by
other modules. Other modules which sport the same functions can also be used
to support Math::BigInt, like L<Math::BigInt::GMP> or L<Math::BigInt::Pari>.

In order to allow for multiple big integer libraries, Math::BigInt was
rewritten to use library modules for core math routines. Any module which
follows the same API as this can be used instead by using the following:

    use Math::BigInt lib => 'libname';

'libname' is either the long name ('Math::BigInt::Pari'), or only the short
version like 'Pari'. To use this library:

    use Math::BigInt lib => 'FastCalc';

The default behaviour is to chose the best internal representation of big
integers, but the base length used in the internal representation can be
specified explicitly. Note that this must be done before Math::BigInt is loaded.
For example,

    use Math::BigInt::FastCalc base_len => 3;
    use Math::BigInt lib => 'FastCalc';

=head1 STORAGE

Math::BigInt::FastCalc works exactly like Math::BigInt::Calc. Numbers are
stored in decimal form chopped into parts.

=head1 METHODS

The following functions are now implemented in FastCalc.xs:

    _is_odd         _is_even        _is_one         _is_zero
    _is_two         _is_ten
    _zero           _one            _two            _ten
    _acmp           _len
    _inc            _dec
    __strip_zeros   _copy

=head1 BUGS

Please report any bugs or feature requests to
C<bug-math-bigint-fastcalc at rt.cpan.org>, or through the web interface at
L<https://rt.cpan.org/Ticket/Create.html?Queue=Math-BigInt-FastCalc>
(requires login). We will be notified, and then you'll automatically be
notified of progress on your bug as I make changes.

=head1 SUPPORT

After installing, you can find documentation for this module with the perldoc
command.

    perldoc Math::BigInt::FastCalc

You can also look for information at:

=over 4

=item GitHub

L<https://github.com/pjacklam/p5-Math-BigInt-FastCalc>

=item RT: CPAN's request tracker

L<https://rt.cpan.org/Dist/Display.html?Name=Math-BigInt-FastCalc>

=item MetaCPAN

L<https://metacpan.org/release/Math-BigInt-FastCalc>

=item CPAN Testers Matrix

L<http://matrix.cpantesters.org/?dist=Math-BigInt-FastCalc>

=item CPAN Ratings

L<https://cpanratings.perl.org/dist/Math-BigInt-FastCalc>

=back

=head1 LICENSE

This program is free software; you may redistribute it and/or modify it under
the same terms as Perl itself.

=head1 AUTHORS

Original math code by Mark Biggar, rewritten by Tels L<http://bloodgate.com/>
in late 2000.

Separated from Math::BigInt and shaped API with the help of John Peacock.

Fixed, sped-up and enhanced by Tels http://bloodgate.com 2001-2003.
Further streamlining (api_version 1 etc.) by Tels 2004-2007.

Maintained by Peter John Acklam E<lt>pjacklam@gmail.comE<gt> 2010-2021.

=head1 SEE ALSO

L<Math::BigInt::Lib> for a description of the API.

Alternative libraries L<Math::BigInt::Calc>, L<Math::BigInt::GMP>, and
L<Math::BigInt::Pari>.

Some of the modules that use these libraries L<Math::BigInt>,
L<Math::BigFloat>, and L<Math::BigRat>.

=cut
