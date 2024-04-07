package sort;

use strict;
use warnings;

our $VERSION = '2.05';

sub import {
    shift;
    if (@_ == 0) {
	require Carp;
	Carp::croak("sort pragma requires arguments");
    }
    $^H{sort} //= 0;
    for my $subpragma (@_) {
        next
            if $subpragma eq 'stable' || $subpragma eq 'defaults';
        require Carp;
        Carp::croak("sort: unknown subpragma '$_'");
    }
}

sub unimport {
    shift;
    if (@_ == 0) {
	require Carp;
	Carp::croak("sort pragma requires arguments");
    }
    for my $subpragma (@_) {
        next
            if $subpragma eq 'stable';
        require Carp;
        Carp::croak("sort: unknown subpragma '$_'");
    }
}

sub current {
    warnings::warnif("deprecated", "sort::current is deprecated, and will always return 'stable'");
    return 'stable';
}

1;
__END__

=head1 NAME

sort - perl pragma to control sort() behaviour

=head1 SYNOPSIS

The sort pragma is now a no-op, and its use is discouraged. These three
operations are valid, but have no effect:

    use sort 'stable';		# guarantee stability
    use sort 'defaults';	# revert to default behavior
    no  sort 'stable';		# stability not important

=head1 DESCRIPTION

Historically the C<sort> pragma you can control the behaviour of the builtin
C<sort()> function.

Prior to v5.28.0 there were two other options:

    use sort '_mergesort';
    use sort '_qsort';		# or '_quicksort'

If you try and specify either of these in v5.28+ it will croak.

The default sort has been stable since v5.8.0, and given this consistent
behaviour for almost two decades, everyone has come to assume stability.

Stability will remain the default - hence there is no need for a pragma for
code to opt into stability "just in case" this changes - it won't.

We do not foresee going back to offering multiple implementations of general
purpose sorting - hence there is no future need to offer a pragma to choose
between them.

If you know that you care that much about performance of your sorting, and
that for your use case and your data, it was worth investigating
alternatives, possible to identify an alternative from our default that was
better, and the cost of switching was worth it, then you know more than we
do. Likely whatever choices we can give are not as good as implementing your
own. (For example, a Radix sort can be faster than O(n log n), but can't be
used for all keys and has larger overheads.)

We are not averse to B<changing> the sort algorithm, but we don't see the
benefit in offering the choice of two general purpose implementations.

=head1 CAVEATS

The function C<sort::current()> was provided to report the current state of
the sort pragmata. This function was not exported, and there is no code to
call it on CPAN. It is now deprecated, and will warn by default.

As we no longer store any sort "state", it can no longer return the correct
value, so it will always return the string C<stable>, as this is consistent
with what we actually have implemented.

=cut
