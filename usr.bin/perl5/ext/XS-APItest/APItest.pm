package XS::APItest;

use strict;
use warnings;
use Carp;

our $VERSION = '1.32';

require XSLoader;

# Export everything since these functions are only used by a test script
# Export subpackages too - in effect, export all their routines into us, then
# export everything from us.
sub import {
    my $package = shift;
    croak ("Can't export for '$package'") unless $package eq __PACKAGE__;
    my $exports;
    @{$exports}{@_} = () if @_;

    my $callpkg = caller;

    my @stashes = ('XS::APItest::', \%XS::APItest::);
    while (my ($stash_name, $stash) = splice @stashes, 0, 2) {
	while (my ($sym_name, $glob) = each %$stash) {
	    if ($sym_name =~ /::$/) {
		# Skip any subpackages that are clearly OO
		next if *{$glob}{HASH}{'new'};
		# and any that have AUTOLOAD
		next if *{$glob}{HASH}{AUTOLOAD};
		push @stashes, "$stash_name$sym_name", *{$glob}{HASH};
	    } elsif (ref $glob eq 'SCALAR' || *{$glob}{CODE}) {
		if ($exports) {
		    next if !exists $exports->{$sym_name};
		    delete $exports->{$sym_name};
		}
		no strict 'refs';
		*{"$callpkg\::$sym_name"} = \&{"$stash_name$sym_name"};
	    }
	}
    }
    foreach (keys %{$exports||{}}) {
	next unless /\A(?:rpn|calcrpn|stufftest|swaptwostmts|looprest|scopelessblock|stmtasexpr|stmtsasexpr|loopblock|blockasexpr|swaplabel|labelconst|arrayfullexpr|arraylistexpr|arraytermexpr|arrayarithexpr|arrayexprflags|subsignature|DEFSV|with_vars|join_with_space)\z/;
	$^H{"XS::APItest/$_"} = 1;
	delete $exports->{$_};
    }
    if ($exports) {
	my @carp = keys %$exports;
	if (@carp) {
	    croak(join '',
		  (map "\"$_\" is not exported by the $package module\n", sort @carp),
		  "Can't continue after import errors");
	}
    }
}

use vars '$WARNINGS_ON_BOOTSTRAP';
use vars map "\$${_}_called_PP", qw(BEGIN UNITCHECK CHECK INIT END);

# Do these here to verify that XS code and Perl code get called at the same
# times
BEGIN {
    $BEGIN_called_PP++;
}
UNITCHECK {
    $UNITCHECK_called_PP++;
};
{
    # Need $W false by default, as some tests run under -w, and under -w we
    # can get warnings about "Too late to run CHECK" block (and INIT block)
    no warnings 'void';
    CHECK {
	$CHECK_called_PP++;
    }
    INIT {
	$INIT_called_PP++;
    }
}
END {
    $END_called_PP++;
}

if ($WARNINGS_ON_BOOTSTRAP) {
    XSLoader::load();
} else {
    # More CHECK and INIT blocks that could warn:
    local $^W;
    XSLoader::load();
}

# This XS function needs the lvalue attr applied.
eval 'use attributes __PACKAGE__, \\&lv_temp_object, "lvalue"; 1' or die;

1;
__END__

=head1 NAME

XS::APItest - Test the perl C API

=head1 SYNOPSIS

  use XS::APItest;
  print_double(4);

  use XS::APItest qw(rpn calcrpn);
  $triangle = rpn($n $n 1 + * 2 /);
  calcrpn $triangle { $n $n 1 + * 2 / }

=head1 ABSTRACT

This module tests the perl C API. Also exposes various bit of the perl
internals for the use of core test scripts.

=head1 DESCRIPTION

This module can be used to check that the perl C API is behaving
correctly. This module provides test functions and an associated
test script that verifies the output.

This module is not meant to be installed.

=head2 EXPORT

Exports all the test functions:

=over 4

=item B<print_double>

Test that a double-precision floating point number is formatted
correctly by C<printf>.

  print_double( $val );

Output is sent to STDOUT.

=item B<print_long_double>

Test that a C<long double> is formatted correctly by
C<printf>. Takes no arguments - the test value is hard-wired
into the function (as "7").

  print_long_double();

Output is sent to STDOUT.

=item B<have_long_double>

Determine whether a C<long double> is supported by Perl.  This should
be used to determine whether to test C<print_long_double>.

  print_long_double() if have_long_double;

=item B<print_nv>

Test that an C<NV> is formatted correctly by
C<printf>.

  print_nv( $val );

Output is sent to STDOUT.

=item B<print_iv>

Test that an C<IV> is formatted correctly by
C<printf>.

  print_iv( $val );

Output is sent to STDOUT.

=item B<print_uv>

Test that an C<UV> is formatted correctly by
C<printf>.

  print_uv( $val );

Output is sent to STDOUT.

=item B<print_int>

Test that an C<int> is formatted correctly by
C<printf>.

  print_int( $val );

Output is sent to STDOUT.

=item B<print_long>

Test that an C<long> is formatted correctly by
C<printf>.

  print_long( $val );

Output is sent to STDOUT.

=item B<print_float>

Test that a single-precision floating point number is formatted
correctly by C<printf>.

  print_float( $val );

Output is sent to STDOUT.

=item B<filter>

Installs a source filter that substitutes "e" for "o" (witheut regard fer
what it might be medifying).

=item B<call_sv>, B<call_pv>, B<call_method>

These exercise the C calls of the same names. Everything after the flags
arg is passed as the args to the called function. They return whatever
the C function itself pushed onto the stack, plus the return value from
the function; for example

    call_sv( sub { @_, 'c' }, G_LIST,  'a', 'b');
    # returns 'a', 'b', 'c', 3
    call_sv( sub { @_ },      G_SCALAR, 'a', 'b');
    # returns 'b', 1

=item B<eval_sv>

Evaluates the passed SV. Result handling is done the same as for
C<call_sv()> etc.

=item B<eval_pv>

Exercises the C function of the same name in scalar context. Returns the
same SV that the C function returns.

=item B<require_pv>

Exercises the C function of the same name. Returns nothing.

=back

=head1 KEYWORDS

These are not supplied by default, but must be explicitly imported.
They are lexically scoped.

=over

=item DEFSV

Behaves like C<$_>.

=item rpn(EXPRESSION)

This construct is a Perl expression.  I<EXPRESSION> must be an RPN
arithmetic expression, as described below.  The RPN expression is
evaluated, and its value is returned as the value of the Perl expression.

=item calcrpn VARIABLE { EXPRESSION }

This construct is a complete Perl statement.  (No semicolon should
follow the closing brace.)  I<VARIABLE> must be a Perl scalar C<my>
variable, and I<EXPRESSION> must be an RPN arithmetic expression as
described below.  The RPN expression is evaluated, and its value is
assigned to the variable.

=back

=head2 RPN expression syntax

Tokens of an RPN expression may be separated by whitespace, but such
separation is usually not required.  It is required only where unseparated
tokens would look like a longer token.  For example, C<12 34 +> can be
written as C<12 34+>, but not as C<1234 +>.

An RPN expression may be any of:

=over

=item C<1234>

A sequence of digits is an unsigned decimal literal number.

=item C<$foo>

An alphanumeric name preceded by dollar sign refers to a Perl scalar
variable.  Only variables declared with C<my> or C<state> are supported.
If the variable's value is not a native integer, it will be converted
to an integer, by Perl's usual mechanisms, at the time it is evaluated.

=item I<A> I<B> C<+>

Sum of I<A> and I<B>.

=item I<A> I<B> C<->

Difference of I<A> and I<B>, the result of subtracting I<B> from I<A>.

=item I<A> I<B> C<*>

Product of I<A> and I<B>.

=item I<A> I<B> C</>

Quotient when I<A> is divided by I<B>, rounded towards zero.
Division by zero generates an exception.

=item I<A> I<B> C<%>

Remainder when I<A> is divided by I<B> with the quotient rounded towards zero.
Division by zero generates an exception.

=back

Because the arithmetic operators all have fixed arity and are postfixed,
there is no need for operator precedence, nor for a grouping operator
to override precedence.  This is half of the point of RPN.

An RPN expression can also be interpreted in another way, as a sequence
of operations on a stack, one operation per token.  A literal or variable
token pushes a value onto the stack.  A binary operator pulls two items
off the stack, performs a calculation with them, and pushes the result
back onto the stack.  The stack starts out empty, and at the end of the
expression there must be exactly one value left on the stack.

=head1 SEE ALSO

L<XS::Typemap>, L<perlapi>.

=head1 AUTHORS

Tim Jenness, E<lt>t.jenness@jach.hawaii.eduE<gt>,
Christian Soeller, E<lt>csoelle@mph.auckland.ac.nzE<gt>,
Hugo van der Sanden E<lt>hv@crypt.compulink.co.ukE<gt>,
Andrew Main (Zefram) <zefram@fysh.org>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2002,2004 Tim Jenness, Christian Soeller, Hugo van der Sanden.
All Rights Reserved.

Copyright (C) 2009 Andrew Main (Zefram) <zefram@fysh.org>

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself. 

=cut
