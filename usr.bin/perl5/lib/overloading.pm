package overloading;
use warnings;

our $VERSION = '0.02';

my $HINT_NO_AMAGIC = 0x01000000; # see perl.h

require 5.010001;

sub _ops_to_nums {
    require overload::numbers;

    map { exists $overload::numbers::names{"($_"}
	? $overload::numbers::names{"($_"}
	: do { require Carp; Carp::croak("'$_' is not a valid overload") }
    } @_;
}

sub import {
    my ( $class, @ops ) = @_;

    if ( @ops ) {
	if ( $^H{overloading} ) {
	    vec($^H{overloading} , $_, 1) = 0 for _ops_to_nums(@ops);
	}

	if ( $^H{overloading} !~ /[^\0]/ ) {
	    delete $^H{overloading};
	    $^H &= ~$HINT_NO_AMAGIC;
	}
    } else {
	delete $^H{overloading};
	$^H &= ~$HINT_NO_AMAGIC;
    }
}

sub unimport {
    my ( $class, @ops ) = @_;

    if ( exists $^H{overloading} or not $^H & $HINT_NO_AMAGIC ) {
	if ( @ops ) {
	    vec($^H{overloading} ||= '', $_, 1) = 1 for _ops_to_nums(@ops);
	} else {
	    delete $^H{overloading};
	}
    }

    $^H |= $HINT_NO_AMAGIC;
}

1;
__END__

=head1 NAME

overloading - perl pragma to lexically control overloading

=head1 SYNOPSIS

    {
	no overloading;
	my $str = "$object"; # doesn't call stringification overload
    }

    # it's lexical, so this stringifies:
    warn "$object";

    # it can be enabled per op
    no overloading qw("");
    warn "$object";

    # and also reenabled
    use overloading;

=head1 DESCRIPTION

This pragma allows you to lexically disable or enable overloading.

=over 6

=item C<no overloading>

Disables overloading entirely in the current lexical scope.

=item C<no overloading @ops>

Disables only specific overloads in the current lexical scope.

=item C<use overloading>

Reenables overloading in the current lexical scope.

=item C<use overloading @ops>

Reenables overloading only for specific ops in the current lexical scope.

=back

=cut
