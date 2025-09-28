package vars;

use 5.006;

our $VERSION = '1.05';

use warnings::register;
use strict qw(vars subs);

sub import {
    my $callpack = caller;
    my (undef, @imports) = @_;
    my ($sym, $ch);
    foreach (@imports) {
        if (($ch, $sym) = /^([\$\@\%\*\&])(.+)/) {
	    if ($sym =~ /\W/) {
		# time for a more-detailed check-up
		if ($sym =~ /^\w+[[{].*[]}]$/) {
		    require Carp;
		    Carp::croak("Can't declare individual elements of hash or array");
		} elsif (warnings::enabled() and length($sym) == 1 and $sym !~ tr/a-zA-Z//) {
		    warnings::warn("No need to declare built-in vars");
		} elsif  (($^H & strict::bits('vars'))) {
		    require Carp;
		    Carp::croak("'$_' is not a valid variable name under strict vars");
		}
	    }
	    $sym = "${callpack}::$sym" unless $sym =~ /::/;
	    *$sym =
		(  $ch eq "\$" ? \$$sym
		 : $ch eq "\@" ? \@$sym
		 : $ch eq "\%" ? \%$sym
		 : $ch eq "\*" ? \*$sym
		 : $ch eq "\&" ? \&$sym 
		 : do {
		     require Carp;
		     Carp::croak("'$_' is not a valid variable name");
		 });
	} else {
	    require Carp;
	    Carp::croak("'$_' is not a valid variable name");
	}
    }
};

1;
__END__

=head1 NAME

vars - Perl pragma to predeclare global variable names

=head1 SYNOPSIS

    use vars qw($frob @mung %seen);

=head1 DESCRIPTION

NOTE: For use with variables in the current package for a single scope, the
functionality provided by this pragma has been superseded by C<our>
declarations, available in Perl v5.6.0 or later, and use of this pragma is
discouraged.  See L<perlfunc/our>.

This pragma will predeclare all the variables whose names are
in the list, allowing you to use them under C<use strict>, and
disabling any typo warnings for them.

Unlike pragmas that affect the C<$^H> hints variable, the C<use vars> and
C<use subs> declarations are not lexically scoped to the block they appear
in: they affect
the entire package in which they appear.  It is not possible to rescind these
declarations with C<no vars> or C<no subs>.

Packages such as the B<AutoLoader> and B<SelfLoader> that delay
loading of subroutines within packages can create problems with
package lexicals defined using C<my()>. While the B<vars> pragma
cannot duplicate the effect of package lexicals (total transparency
outside of the package), it can act as an acceptable substitute by
pre-declaring global symbols, ensuring their availability to the
later-loaded routines.

See L<perlmodlib/Pragmatic Modules>.

=cut
