package Thread::Semaphore;

use strict;
use warnings;

our $VERSION = '2.13';
$VERSION = eval $VERSION;

use threads::shared;
use Scalar::Util 1.10 qw(looks_like_number);

# Predeclarations for internal functions
my ($validate_arg);

# Create a new semaphore optionally with specified count (count defaults to 1)
sub new {
    my $class = shift;

    my $val :shared = 1;
    if (@_) {
        $val = shift;
        if (! defined($val) ||
            ! looks_like_number($val) ||
            (int($val) != $val))
        {
            require Carp;
            $val = 'undef' if (! defined($val));
            Carp::croak("Semaphore initializer is not an integer: $val");
        }
    }

    return bless(\$val, $class);
}

# Decrement a semaphore's count (decrement amount defaults to 1)
sub down {
    my $sema = shift;
    my $dec = @_ ? $validate_arg->(shift) : 1;

    lock($$sema);
    cond_wait($$sema) until ($$sema >= $dec);
    $$sema -= $dec;
}

# Decrement a semaphore's count only if count >= decrement value
#  (decrement amount defaults to 1)
sub down_nb {
    my $sema = shift;
    my $dec = @_ ? $validate_arg->(shift) : 1;

    lock($$sema);
    my $ok = ($$sema >= $dec);
    $$sema -= $dec if $ok;
    return $ok;
}

# Decrement a semaphore's count even if the count goes below 0
#  (decrement amount defaults to 1)
sub down_force {
    my $sema = shift;
    my $dec = @_ ? $validate_arg->(shift) : 1;

    lock($$sema);
    $$sema -= $dec;
}

# Decrement a semaphore's count with timeout
#  (timeout in seconds; decrement amount defaults to 1)
sub down_timed {
    my $sema = shift;
    my $timeout = $validate_arg->(shift);
    my $dec = @_ ? $validate_arg->(shift) : 1;

    lock($$sema);
    my $abs = time() + $timeout;
    until ($$sema >= $dec) {
        return if !cond_timedwait($$sema, $abs);
    }
    $$sema -= $dec;
    return 1;
}

# Increment a semaphore's count (increment amount defaults to 1)
sub up {
    my $sema = shift;
    my $inc = @_ ? $validate_arg->(shift) : 1;

    lock($$sema);
    ($$sema += $inc) > 0 and cond_broadcast($$sema);
}

### Internal Functions ###

# Validate method argument
$validate_arg = sub {
    my $arg = shift;

    if (! defined($arg) ||
        ! looks_like_number($arg) ||
        (int($arg) != $arg) ||
        ($arg < 1))
    {
        require Carp;
        my ($method) = (caller(1))[3];
        $method =~ s/Thread::Semaphore:://;
        $arg = 'undef' if (! defined($arg));
        Carp::croak("Argument to semaphore method '$method' is not a positive integer: $arg");
    }

    return $arg;
};

1;

=head1 NAME

Thread::Semaphore - Thread-safe semaphores

=head1 VERSION

This document describes Thread::Semaphore version 2.13

=head1 SYNOPSIS

    use Thread::Semaphore;
    my $s = Thread::Semaphore->new();
    $s->down();   # Also known as the semaphore P operation.
    # The guarded section is here
    $s->up();     # Also known as the semaphore V operation.

    # Decrement the semaphore only if it would immediately succeed.
    if ($s->down_nb()) {
        # The guarded section is here
        $s->up();
    }

    # Forcefully decrement the semaphore even if its count goes below 0.
    $s->down_force();

    # The default value for semaphore operations is 1
    my $s = Thread::Semaphore->new($initial_value);
    $s->down($down_value);
    $s->up($up_value);
    if ($s->down_nb($down_value)) {
        ...
        $s->up($up_value);
    }
    $s->down_force($down_value);

=head1 DESCRIPTION

Semaphores provide a mechanism to regulate access to resources.  Unlike
locks, semaphores aren't tied to particular scalars, and so may be used to
control access to anything you care to use them for.

Semaphores don't limit their values to zero and one, so they can be used to
control access to some resource that there may be more than one of (e.g.,
filehandles).  Increment and decrement amounts aren't fixed at one either,
so threads can reserve or return multiple resources at once.

=head1 METHODS

=over 8

=item ->new()

=item ->new(NUMBER)

C<new> creates a new semaphore, and initializes its count to the specified
number (which must be an integer).  If no number is specified, the
semaphore's count defaults to 1.

=item ->down()

=item ->down(NUMBER)

The C<down> method decreases the semaphore's count by the specified number
(which must be an integer >= 1), or by one if no number is specified.

If the semaphore's count would drop below zero, this method will block
until such time as the semaphore's count is greater than or equal to the
amount you're C<down>ing the semaphore's count by.

This is the semaphore "P operation" (the name derives from the Dutch
word "pak", which means "capture" -- the semaphore operations were
named by the late Dijkstra, who was Dutch).

=item ->down_nb()

=item ->down_nb(NUMBER)

The C<down_nb> method attempts to decrease the semaphore's count by the
specified number (which must be an integer >= 1), or by one if no number
is specified.

If the semaphore's count would drop below zero, this method will return
I<false>, and the semaphore's count remains unchanged.  Otherwise, the
semaphore's count is decremented and this method returns I<true>.

=item ->down_force()

=item ->down_force(NUMBER)

The C<down_force> method decreases the semaphore's count by the specified
number (which must be an integer >= 1), or by one if no number is specified.
This method does not block, and may cause the semaphore's count to drop
below zero.

=item ->down_timed(TIMEOUT)

=item ->down_timed(TIMEOUT, NUMBER)

The C<down_timed> method attempts to decrease the semaphore's count by 1
or by the specified number within the specified timeout period given in
seconds (which must be an integer >= 0).

If the semaphore's count would drop below zero, this method will block
until either the semaphore's count is greater than or equal to the
amount you're C<down>ing the semaphore's count by, or until the timeout is
reached.

If the timeout is reached, this method will return I<false>, and the
semaphore's count remains unchanged.  Otherwise, the semaphore's count is
decremented and this method returns I<true>.

=item ->up()

=item ->up(NUMBER)

The C<up> method increases the semaphore's count by the number specified
(which must be an integer >= 1), or by one if no number is specified.

This will unblock any thread that is blocked trying to C<down> the
semaphore if the C<up> raises the semaphore's count above the amount that
the C<down> is trying to decrement it by.  For example, if three threads
are blocked trying to C<down> a semaphore by one, and another thread C<up>s
the semaphore by two, then two of the blocked threads (which two is
indeterminate) will become unblocked.

This is the semaphore "V operation" (the name derives from the Dutch
word "vrij", which means "release").

=back

=head1 NOTES

Semaphores created by L<Thread::Semaphore> can be used in both threaded and
non-threaded applications.  This allows you to write modules and packages
that potentially make use of semaphores, and that will function in either
environment.

=head1 SEE ALSO

Thread::Semaphore on MetaCPAN:
L<https://metacpan.org/release/Thread-Semaphore>

Code repository for CPAN distribution:
L<https://github.com/Dual-Life/Thread-Semaphore>

L<threads>, L<threads::shared>

Sample code in the I<examples> directory of this distribution on CPAN.

=head1 MAINTAINER

Jerry D. Hedden, S<E<lt>jdhedden AT cpan DOT orgE<gt>>

=head1 LICENSE

This program is free software; you can redistribute it and/or modify it under
the same terms as Perl itself.

=cut
