package constant;
use 5.008;
use strict;
use warnings::register;

our $VERSION = '1.33';
our %declared;

#=======================================================================

# Some names are evil choices.
my %keywords = map +($_, 1), qw{ BEGIN INIT CHECK END DESTROY AUTOLOAD };
$keywords{UNITCHECK}++ if $] > 5.009;

my %forced_into_main = map +($_, 1),
    qw{ STDIN STDOUT STDERR ARGV ARGVOUT ENV INC SIG };

my %forbidden = (%keywords, %forced_into_main);

my $normal_constant_name = qr/^_?[^\W_0-9]\w*\z/;
my $tolerable = qr/^[A-Za-z_]\w*\z/;
my $boolean = qr/^[01]?\z/;

BEGIN {
    # We'd like to do use constant _CAN_PCS => $] > 5.009002
    # but that's a bit tricky before we load the constant module :-)
    # By doing this, we save several run time checks for *every* call
    # to import.
    my $const = $] > 5.009002;
    my $downgrade = $] < 5.015004; # && $] >= 5.008
    my $constarray = exists &_make_const;
    if ($const) {
	Internals::SvREADONLY($const, 1);
	Internals::SvREADONLY($downgrade, 1);
	$constant::{_CAN_PCS}   = \$const;
	$constant::{_DOWNGRADE} = \$downgrade;
	$constant::{_CAN_PCS_FOR_ARRAY} = \$constarray;
    }
    else {
	no strict 'refs';
	*{"_CAN_PCS"}   = sub () {$const};
	*{"_DOWNGRADE"} = sub () { $downgrade };
	*{"_CAN_PCS_FOR_ARRAY"} = sub () { $constarray };
    }
}

#=======================================================================
# import() - import symbols into user's namespace
#
# What we actually do is define a function in the caller's namespace
# which returns the value. The function we create will normally
# be inlined as a constant, thereby avoiding further sub calling 
# overhead.
#=======================================================================
sub import {
    my $class = shift;
    return unless @_;			# Ignore 'use constant;'
    my $constants;
    my $multiple  = ref $_[0];
    my $caller = caller;
    my $flush_mro;
    my $symtab;

    if (_CAN_PCS) {
	no strict 'refs';
	$symtab = \%{$caller . '::'};
    };

    if ( $multiple ) {
	if (ref $_[0] ne 'HASH') {
	    require Carp;
	    Carp::croak("Invalid reference type '".ref(shift)."' not 'HASH'");
	}
	$constants = shift;
    } else {
	unless (defined $_[0]) {
	    require Carp;
	    Carp::croak("Can't use undef as constant name");
	}
	$constants->{+shift} = undef;
    }

    foreach my $name ( keys %$constants ) {
	my $pkg;
	my $symtab = $symtab;
	my $orig_name = $name;
	if ($name =~ s/(.*)(?:::|')(?=.)//s) {
	    $pkg = $1;
	    if (_CAN_PCS && $pkg ne $caller) {
		no strict 'refs';
		$symtab = \%{$pkg . '::'};
	    }
	}
	else {
	    $pkg = $caller;
	}

	# Normal constant name
	if ($name =~ $normal_constant_name and !$forbidden{$name}) {
	    # Everything is okay

	# Name forced into main, but we're not in main. Fatal.
	} elsif ($forced_into_main{$name} and $pkg ne 'main') {
	    require Carp;
	    Carp::croak("Constant name '$name' is forced into main::");

	# Starts with double underscore. Fatal.
	} elsif ($name =~ /^__/) {
	    require Carp;
	    Carp::croak("Constant name '$name' begins with '__'");

	# Maybe the name is tolerable
	} elsif ($name =~ $tolerable) {
	    # Then we'll warn only if you've asked for warnings
	    if (warnings::enabled()) {
		if ($keywords{$name}) {
		    warnings::warn("Constant name '$name' is a Perl keyword");
		} elsif ($forced_into_main{$name}) {
		    warnings::warn("Constant name '$name' is " .
			"forced into package main::");
		}
	    }

	# Looks like a boolean
	# use constant FRED == fred;
	} elsif ($name =~ $boolean) {
            require Carp;
	    if (@_) {
		Carp::croak("Constant name '$name' is invalid");
	    } else {
		Carp::croak("Constant name looks like boolean value");
	    }

	} else {
	   # Must have bad characters
            require Carp;
	    Carp::croak("Constant name '$name' has invalid characters");
	}

	{
	    no strict 'refs';
	    my $full_name = "${pkg}::$name";
	    $declared{$full_name}++;
	    if ($multiple || @_ == 1) {
		my $scalar = $multiple ? $constants->{$orig_name} : $_[0];

		if (_DOWNGRADE) { # for 5.8 to 5.14
		    # Work around perl bug #31991: Sub names (actually glob
		    # names in general) ignore the UTF8 flag. So we have to
		    # turn it off to get the "right" symbol table entry.
		    utf8::is_utf8 $name and utf8::encode $name;
		}

		# The constant serves to optimise this entire block out on
		# 5.8 and earlier.
		if (_CAN_PCS) {
		    # Use a reference as a proxy for a constant subroutine.
		    # If this is not a glob yet, it saves space.  If it is
		    # a glob, we must still create it this way to get the
		    # right internal flags set, as constants are distinct
		    # from subroutines created with sub(){...}.
		    # The check in Perl_ck_rvconst knows that inlinable
		    # constants from cv_const_sv are read only. So we have to:
		    Internals::SvREADONLY($scalar, 1);
		    if (!exists $symtab->{$name}) {
			$symtab->{$name} = \$scalar;
			++$flush_mro->{$pkg};
		    }
		    else {
			local $constant::{_dummy} = \$scalar;
			*$full_name = \&{"_dummy"};
		    }
		} else {
		    *$full_name = sub () { $scalar };
		}
	    } elsif (@_) {
		my @list = @_;
		if (_CAN_PCS_FOR_ARRAY) {
		    _make_const($list[$_]) for 0..$#list;
		    _make_const(@list);
		    if (!exists $symtab->{$name}) {
			$symtab->{$name} = \@list;
			$flush_mro->{$pkg}++;
		    }
		    else {
			local $constant::{_dummy} = \@list;
			*$full_name = \&{"_dummy"};
		    }
		}
		else { *$full_name = sub () { @list }; }
	    } else {
		*$full_name = sub () { };
	    }
	}
    }
    # Flush the cache exactly once if we make any direct symbol table changes.
    if (_CAN_PCS && $flush_mro) {
	mro::method_changed_in($_) for keys %$flush_mro;
    }
}

1;

__END__

=head1 NAME

constant - Perl pragma to declare constants

=head1 SYNOPSIS

    use constant PI    => 4 * atan2(1, 1);
    use constant DEBUG => 0;

    print "Pi equals ", PI, "...\n" if DEBUG;

    use constant {
        SEC   => 0,
        MIN   => 1,
        HOUR  => 2,
        MDAY  => 3,
        MON   => 4,
        YEAR  => 5,
        WDAY  => 6,
        YDAY  => 7,
        ISDST => 8,
    };

    use constant WEEKDAYS => qw(
        Sunday Monday Tuesday Wednesday Thursday Friday Saturday
    );

    print "Today is ", (WEEKDAYS)[ (localtime)[WDAY] ], ".\n";

=head1 DESCRIPTION

This pragma allows you to declare constants at compile-time.

When you declare a constant such as C<PI> using the method shown
above, each machine your script runs upon can have as many digits
of accuracy as it can use.  Also, your program will be easier to
read, more likely to be maintained (and maintained correctly), and
far less likely to send a space probe to the wrong planet because
nobody noticed the one equation in which you wrote C<3.14195>.

When a constant is used in an expression, Perl replaces it with its
value at compile time, and may then optimize the expression further.
In particular, any code in an C<if (CONSTANT)> block will be optimized
away if the constant is false.

=head1 NOTES

As with all C<use> directives, defining a constant happens at
compile time.  Thus, it's probably not correct to put a constant
declaration inside of a conditional statement (like C<if ($foo)
{ use constant ... }>).

Constants defined using this module cannot be interpolated into
strings like variables.  However, concatenation works just fine:

    print "Pi equals PI...\n";        # WRONG: does not expand "PI"
    print "Pi equals ".PI."...\n";    # right

Even though a reference may be declared as a constant, the reference may
point to data which may be changed, as this code shows.

    use constant ARRAY => [ 1,2,3,4 ];
    print ARRAY->[1];
    ARRAY->[1] = " be changed";
    print ARRAY->[1];

Constants belong to the package they are defined in.  To refer to a
constant defined in another package, specify the full package name, as
in C<Some::Package::CONSTANT>.  Constants may be exported by modules,
and may also be called as either class or instance methods, that is,
as C<< Some::Package->CONSTANT >> or as C<< $obj->CONSTANT >> where
C<$obj> is an instance of C<Some::Package>.  Subclasses may define
their own constants to override those in their base class.

As of version 1.32 of this module, constants can be defined in packages
other than the caller, by including the package name in the name of the
constant:

    use constant "OtherPackage::FWIBBLE" => 7865;
    constant->import("Other::FWOBBLE",$value); # dynamically at run time

The use of all caps for constant names is merely a convention,
although it is recommended in order to make constants stand out
and to help avoid collisions with other barewords, keywords, and
subroutine names.  Constant names must begin with a letter or
underscore.  Names beginning with a double underscore are reserved.  Some
poor choices for names will generate warnings, if warnings are enabled at
compile time.

=head2 List constants

Constants may be lists of more (or less) than one value.  A constant
with no values evaluates to C<undef> in scalar context.  Note that
constants with more than one value do I<not> return their last value in
scalar context as one might expect.  They currently return the number
of values, but B<this may change in the future>.  Do not use constants
with multiple values in scalar context.

B<NOTE:> This implies that the expression defining the value of a
constant is evaluated in list context.  This may produce surprises:

    use constant TIMESTAMP => localtime;                # WRONG!
    use constant TIMESTAMP => scalar localtime;         # right

The first line above defines C<TIMESTAMP> as a 9-element list, as
returned by C<localtime()> in list context.  To set it to the string
returned by C<localtime()> in scalar context, an explicit C<scalar>
keyword is required.

List constants are lists, not arrays.  To index or slice them, they
must be placed in parentheses.

    my @workdays = WEEKDAYS[1 .. 5];            # WRONG!
    my @workdays = (WEEKDAYS)[1 .. 5];          # right

=head2 Defining multiple constants at once

Instead of writing multiple C<use constant> statements, you may define
multiple constants in a single statement by giving, instead of the
constant name, a reference to a hash where the keys are the names of
the constants to be defined.  Obviously, all constants defined using
this method must have a single value.

    use constant {
        FOO => "A single value",
        BAR => "This", "won't", "work!",        # Error!
    };

This is a fundamental limitation of the way hashes are constructed in
Perl.  The error messages produced when this happens will often be
quite cryptic -- in the worst case there may be none at all, and
you'll only later find that something is broken.

When defining multiple constants, you cannot use the values of other
constants defined in the same declaration.  This is because the
calling package doesn't know about any constant within that group
until I<after> the C<use> statement is finished.

    use constant {
        BITMASK => 0xAFBAEBA8,
        NEGMASK => ~BITMASK,                    # Error!
    };

=head2 Magic constants

Magical values and references can be made into constants at compile
time, allowing for way cool stuff like this.  (These error numbers
aren't totally portable, alas.)

    use constant E2BIG => ($! = 7);
    print   E2BIG, "\n";        # something like "Arg list too long"
    print 0+E2BIG, "\n";        # "7"

You can't produce a tied constant by giving a tied scalar as the
value.  References to tied variables, however, can be used as
constants without any problems.

=head1 TECHNICAL NOTES

In the current implementation, scalar constants are actually
inlinable subroutines.  As of version 5.004 of Perl, the appropriate
scalar constant is inserted directly in place of some subroutine
calls, thereby saving the overhead of a subroutine call.  See
L<perlsub/"Constant Functions"> for details about how and when this
happens.

In the rare case in which you need to discover at run time whether a
particular constant has been declared via this module, you may use
this function to examine the hash C<%constant::declared>.  If the given
constant name does not include a package name, the current package is
used.

    sub declared ($) {
        use constant 1.01;              # don't omit this!
        my $name = shift;
        $name =~ s/^::/main::/;
        my $pkg = caller;
        my $full_name = $name =~ /::/ ? $name : "${pkg}::$name";
        $constant::declared{$full_name};
    }

=head1 CAVEATS

List constants are not inlined unless you are using Perl v5.20 or higher.
In v5.20 or higher, they are still not read-only, but that may change in
future versions.

It is not possible to have a subroutine or a keyword with the same
name as a constant in the same package.  This is probably a Good Thing.

A constant with a name in the list C<STDIN STDOUT STDERR ARGV ARGVOUT
ENV INC SIG> is not allowed anywhere but in package C<main::>, for
technical reasons. 

Unlike constants in some languages, these cannot be overridden
on the command line or via environment variables.

You can get into trouble if you use constants in a context which
automatically quotes barewords (as is true for any subroutine call).
For example, you can't say C<$hash{CONSTANT}> because C<CONSTANT> will
be interpreted as a string.  Use C<$hash{CONSTANT()}> or
C<$hash{+CONSTANT}> to prevent the bareword quoting mechanism from
kicking in.  Similarly, since the C<< => >> operator quotes a bareword
immediately to its left, you have to say C<< CONSTANT() => 'value' >>
(or simply use a comma in place of the big arrow) instead of
C<< CONSTANT => 'value' >>.

=head1 SEE ALSO

L<Readonly> - Facility for creating read-only scalars, arrays, hashes.

L<Attribute::Constant> - Make read-only variables via attribute

L<Scalar::Readonly> - Perl extension to the C<SvREADONLY> scalar flag

L<Hash::Util> - A selection of general-utility hash subroutines (mostly
to lock/unlock keys and values)

=head1 BUGS

Please report any bugs or feature requests via the perlbug(1) utility.

=head1 AUTHORS

Tom Phoenix, E<lt>F<rootbeer@redcat.com>E<gt>, with help from
many other folks.

Multiple constant declarations at once added by Casey West,
E<lt>F<casey@geeknest.com>E<gt>.

Documentation mostly rewritten by Ilmari Karonen,
E<lt>F<perl@itz.pp.sci.fi>E<gt>.

This program is maintained by the Perl 5 Porters. 
The CPAN distribution is maintained by SE<eacute>bastien Aperghis-Tramoni
E<lt>F<sebastien@aperghis.net>E<gt>.

=head1 COPYRIGHT & LICENSE

Copyright (C) 1997, 1999 Tom Phoenix

This module is free software; you can redistribute it or modify it
under the same terms as Perl itself.

=cut
