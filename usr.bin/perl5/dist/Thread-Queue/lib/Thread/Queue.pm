package Thread::Queue;

use strict;
use warnings;

our $VERSION = '3.14';          # remember to update version in POD!
$VERSION = eval $VERSION;

use threads::shared 1.21;
use Scalar::Util 1.10 qw(looks_like_number blessed reftype refaddr);

# Carp errors from threads::shared calls should complain about caller
our @CARP_NOT = ("threads::shared");

# Create a new queue possibly pre-populated with items
sub new
{
    my $class = shift;
    my @queue :shared = map { shared_clone($_) } @_;
    my %self :shared = ( 'queue' => \@queue );
    return bless(\%self, $class);
}

# Add items to the tail of a queue
sub enqueue
{
    my $self = shift;
    lock(%$self);

    if ($$self{'ENDED'}) {
        require Carp;
        Carp::croak("'enqueue' method called on queue that has been 'end'ed");
    }

    # Block if queue size exceeds any specified limit
    my $queue = $$self{'queue'};
    cond_wait(%$self) while ($$self{'LIMIT'} && (@$queue >= $$self{'LIMIT'}));

    # Add items to queue, and then signal other threads
    push(@$queue, map { shared_clone($_) } @_)
        and cond_signal(%$self);
}

# Set or return the max. size for a queue
sub limit : lvalue
{
    my $self = shift;
    lock(%$self);
    $$self{'LIMIT'};
}

# Return a count of the number of items on a queue
sub pending
{
    my $self = shift;
    lock(%$self);
    return if ($$self{'ENDED'} && ! @{$$self{'queue'}});
    return scalar(@{$$self{'queue'}});
}

# Indicate that no more data will enter the queue
sub end
{
    my $self = shift;
    lock(%$self);
    # No more data is coming
    $$self{'ENDED'} = 1;

    cond_signal(%$self);  # Unblock possibly waiting threads
}

# Return 1 or more items from the head of a queue, blocking if needed
sub dequeue
{
    my $self = shift;
    lock(%$self);
    my $queue = $$self{'queue'};

    my $count = @_ ? $self->_validate_count(shift) : 1;

    # Wait for requisite number of items
    cond_wait(%$self) while ((@$queue < $count) && ! $$self{'ENDED'});

    # If no longer blocking, try getting whatever is left on the queue
    return $self->dequeue_nb($count) if ($$self{'ENDED'});

    # Return single item
    if ($count == 1) {
        my $item = shift(@$queue);
        cond_signal(%$self);  # Unblock possibly waiting threads
        return $item;
    }

    # Return multiple items
    my @items;
    push(@items, shift(@$queue)) for (1..$count);
    cond_signal(%$self);  # Unblock possibly waiting threads
    return @items;
}

# Return items from the head of a queue with no blocking
sub dequeue_nb
{
    my $self = shift;
    lock(%$self);
    my $queue = $$self{'queue'};

    my $count = @_ ? $self->_validate_count(shift) : 1;

    # Return single item
    if ($count == 1) {
        my $item = shift(@$queue);
        cond_signal(%$self);  # Unblock possibly waiting threads
        return $item;
    }

    # Return multiple items
    my @items;
    for (1..$count) {
        last if (! @$queue);
        push(@items, shift(@$queue));
    }
    cond_signal(%$self);  # Unblock possibly waiting threads
    return @items;
}

# Return items from the head of a queue, blocking if needed up to a timeout
sub dequeue_timed
{
    my $self = shift;
    lock(%$self);
    my $queue = $$self{'queue'};

    # Timeout may be relative or absolute
    my $timeout = @_ ? $self->_validate_timeout(shift) : -1;
    # Convert to an absolute time for use with cond_timedwait()
    if ($timeout < 32000000) {   # More than one year
        $timeout += time();
    }

    my $count = @_ ? $self->_validate_count(shift) : 1;

    # Wait for requisite number of items, or until timeout
    while ((@$queue < $count) && ! $$self{'ENDED'}) {
        last if (! cond_timedwait(%$self, $timeout));
    }

    # Get whatever we need off the queue if available
    return $self->dequeue_nb($count);
}

# Return an item without removing it from a queue
sub peek
{
    my $self = shift;
    lock(%$self);
    my $index = @_ ? $self->_validate_index(shift) : 0;
    return $$self{'queue'}[$index];
}

# Insert items anywhere into a queue
sub insert
{
    my $self = shift;
    lock(%$self);

    if ($$self{'ENDED'}) {
        require Carp;
        Carp::croak("'insert' method called on queue that has been 'end'ed");
    }

    my $queue = $$self{'queue'};

    my $index = $self->_validate_index(shift);

    return if (! @_);   # Nothing to insert

    # Support negative indices
    if ($index < 0) {
        $index += @$queue;
        if ($index < 0) {
            $index = 0;
        }
    }

    # Dequeue items from $index onward
    my @tmp;
    while (@$queue > $index) {
        unshift(@tmp, pop(@$queue))
    }

    # Add new items to the queue
    push(@$queue, map { shared_clone($_) } @_);

    # Add previous items back onto the queue
    push(@$queue, @tmp);

    cond_signal(%$self);  # Unblock possibly waiting threads
}

# Remove items from anywhere in a queue
sub extract
{
    my $self = shift;
    lock(%$self);
    my $queue = $$self{'queue'};

    my $index = @_ ? $self->_validate_index(shift) : 0;
    my $count = @_ ? $self->_validate_count(shift) : 1;

    # Support negative indices
    if ($index < 0) {
        $index += @$queue;
        if ($index < 0) {
            $count += $index;
            return if ($count <= 0);           # Beyond the head of the queue
            return $self->dequeue_nb($count);  # Extract from the head
        }
    }

    # Dequeue items from $index+$count onward
    my @tmp;
    while (@$queue > ($index+$count)) {
        unshift(@tmp, pop(@$queue))
    }

    # Extract desired items
    my @items;
    unshift(@items, pop(@$queue)) while (@$queue > $index);

    # Add back any removed items
    push(@$queue, @tmp);

    cond_signal(%$self);  # Unblock possibly waiting threads

    # Return single item
    return $items[0] if ($count == 1);

    # Return multiple items
    return @items;
}

### Internal Methods ###

# Check value of the requested index
sub _validate_index
{
    my $self = shift;
    my $index = shift;

    if (! defined($index) ||
        ! looks_like_number($index) ||
        (int($index) != $index))
    {
        require Carp;
        my ($method) = (caller(1))[3];
        my $class_name = ref($self);
        $method =~ s/$class_name\:://;
        $index = 'undef' if (! defined($index));
        Carp::croak("Invalid 'index' argument ($index) to '$method' method");
    }

    return $index;
};

# Check value of the requested count
sub _validate_count
{
    my $self = shift;
    my $count = shift;

    if (! defined($count) ||
        ! looks_like_number($count) ||
        (int($count) != $count) ||
        ($count < 1) ||
        ($$self{'LIMIT'} && $count > $$self{'LIMIT'}))
    {
        require Carp;
        my ($method) = (caller(1))[3];
        my $class_name = ref($self);
        $method =~ s/$class_name\:://;
        $count = 'undef' if (! defined($count));
        if ($$self{'LIMIT'} && $count > $$self{'LIMIT'}) {
            Carp::croak("'count' argument ($count) to '$method' method exceeds queue size limit ($$self{'LIMIT'})");
        } else {
            Carp::croak("Invalid 'count' argument ($count) to '$method' method");
        }
    }

    return $count;
};

# Check value of the requested timeout
sub _validate_timeout
{
    my $self = shift;
    my $timeout = shift;

    if (! defined($timeout) ||
        ! looks_like_number($timeout))
    {
        require Carp;
        my ($method) = (caller(1))[3];
        my $class_name = ref($self);
        $method =~ s/$class_name\:://;
        $timeout = 'undef' if (! defined($timeout));
        Carp::croak("Invalid 'timeout' argument ($timeout) to '$method' method");
    }

    return $timeout;
};

1;

=head1 NAME

Thread::Queue - Thread-safe queues

=head1 VERSION

This document describes Thread::Queue version 3.14

=head1 SYNOPSIS

    use strict;
    use warnings;

    use threads;
    use Thread::Queue;

    my $q = Thread::Queue->new();    # A new empty queue

    # Worker thread
    my $thr = threads->create(
        sub {
            # Thread will loop until no more work
            while (defined(my $item = $q->dequeue())) {
                # Do work on $item
                ...
            }
        }
    );

    # Send work to the thread
    $q->enqueue($item1, ...);
    # Signal that there is no more work to be sent
    $q->end();
    # Join up with the thread when it finishes
    $thr->join();

    ...

    # Count of items in the queue
    my $left = $q->pending();

    # Non-blocking dequeue
    if (defined(my $item = $q->dequeue_nb())) {
        # Work on $item
    }

    # Blocking dequeue with 5-second timeout
    if (defined(my $item = $q->dequeue_timed(5))) {
        # Work on $item
    }

    # Set a size for a queue
    $q->limit = 5;

    # Get the second item in the queue without dequeuing anything
    my $item = $q->peek(1);

    # Insert two items into the queue just behind the head
    $q->insert(1, $item1, $item2);

    # Extract the last two items on the queue
    my ($item1, $item2) = $q->extract(-2, 2);

=head1 DESCRIPTION

This module provides thread-safe FIFO queues that can be accessed safely by
any number of threads.

Any data types supported by L<threads::shared> can be passed via queues:

=over

=item Ordinary scalars

=item Array refs

=item Hash refs

=item Scalar refs

=item Objects based on the above

=back

Ordinary scalars are added to queues as they are.

If not already thread-shared, the other complex data types will be cloned
(recursively, if needed, and including any C<bless>ings and read-only
settings) into thread-shared structures before being placed onto a queue.

For example, the following would cause L<Thread::Queue> to create a empty,
shared array reference via C<&shared([])>, copy the elements 'foo', 'bar'
and 'baz' from C<@ary> into it, and then place that shared reference onto
the queue:

 my @ary = qw/foo bar baz/;
 $q->enqueue(\@ary);

However, for the following, the items are already shared, so their references
are added directly to the queue, and no cloning takes place:

 my @ary :shared = qw/foo bar baz/;
 $q->enqueue(\@ary);

 my $obj = &shared({});
 $$obj{'foo'} = 'bar';
 $$obj{'qux'} = 99;
 bless($obj, 'My::Class');
 $q->enqueue($obj);

See L</"LIMITATIONS"> for caveats related to passing objects via queues.

=head1 QUEUE CREATION

=over

=item ->new()

Creates a new empty queue.

=item ->new(LIST)

Creates a new queue pre-populated with the provided list of items.

=back

=head1 BASIC METHODS

The following methods deal with queues on a FIFO basis.

=over

=item ->enqueue(LIST)

Adds a list of items onto the end of the queue.

=item ->dequeue()

=item ->dequeue(COUNT)

Removes the requested number of items (default is 1) from the head of the
queue, and returns them.  If the queue contains fewer than the requested
number of items, then the thread will be blocked until the requisite number
of items are available (i.e., until other threads C<enqueue> more items).

=item ->dequeue_nb()

=item ->dequeue_nb(COUNT)

Removes the requested number of items (default is 1) from the head of the
queue, and returns them.  If the queue contains fewer than the requested
number of items, then it immediately (i.e., non-blocking) returns whatever
items there are on the queue.  If the queue is empty, then C<undef> is
returned.

=item ->dequeue_timed(TIMEOUT)

=item ->dequeue_timed(TIMEOUT, COUNT)

Removes the requested number of items (default is 1) from the head of the
queue, and returns them.  If the queue contains fewer than the requested
number of items, then the thread will be blocked until the requisite number of
items are available, or until the timeout is reached.  If the timeout is
reached, it returns whatever items there are on the queue, or C<undef> if the
queue is empty.

The timeout may be a number of seconds relative to the current time (e.g., 5
seconds from when the call is made), or may be an absolute timeout in I<epoch>
seconds the same as would be used with
L<cond_timedwait()|threads::shared/"cond_timedwait VARIABLE, ABS_TIMEOUT">.
Fractional seconds (e.g., 2.5 seconds) are also supported (to the extent of
the underlying implementation).

If C<TIMEOUT> is missing, C<undef>, or less than or equal to 0, then this call
behaves the same as C<dequeue_nb>.

=item ->pending()

Returns the number of items still in the queue.  Returns C<undef> if the queue
has been ended (see below), and there are no more items in the queue.

=item ->limit

Sets the size of the queue.  If set, calls to C<enqueue()> will block until
the number of pending items in the queue drops below the C<limit>.  The
C<limit> does not prevent enqueuing items beyond that count:

 my $q = Thread::Queue->new(1, 2);
 $q->limit = 4;
 $q->enqueue(3, 4, 5);   # Does not block
 $q->enqueue(6);         # Blocks until at least 2 items are
                         # dequeued
 my $size = $q->limit;   # Returns the current limit (may return
                         # 'undef')
 $q->limit = 0;          # Queue size is now unlimited

Calling any of the dequeue methods with C<COUNT> greater than a queue's
C<limit> will generate an error.

=item ->end()

Declares that no more items will be added to the queue.

All threads blocking on C<dequeue()> calls will be unblocked with any
remaining items in the queue and/or C<undef> being returned.  Any subsequent
calls to C<dequeue()> will behave like C<dequeue_nb()>.

Once ended, no more items may be placed in the queue.

=back

=head1 ADVANCED METHODS

The following methods can be used to manipulate items anywhere in a queue.

To prevent the contents of a queue from being modified by another thread
while it is being examined and/or changed, L<lock|threads::shared/"lock
VARIABLE"> the queue inside a local block:

 {
     lock($q);   # Keep other threads from changing the queue's contents
     my $item = $q->peek();
     if ($item ...) {
         ...
     }
 }
 # Queue is now unlocked

=over

=item ->peek()

=item ->peek(INDEX)

Returns an item from the queue without dequeuing anything.  Defaults to the
head of queue (at index position 0) if no index is specified.  Negative
index values are supported as with L<arrays|perldata/"Subscripts"> (i.e., -1
is the end of the queue, -2 is next to last, and so on).

If no items exists at the specified index (i.e., the queue is empty, or the
index is beyond the number of items on the queue), then C<undef> is returned.

Remember, the returned item is not removed from the queue, so manipulating a
C<peek>ed at reference affects the item on the queue.

=item ->insert(INDEX, LIST)

Adds the list of items to the queue at the specified index position (0
is the head of the list).  Any existing items at and beyond that position are
pushed back past the newly added items:

 $q->enqueue(1, 2, 3, 4);
 $q->insert(1, qw/foo bar/);
 # Queue now contains:  1, foo, bar, 2, 3, 4

Specifying an index position greater than the number of items in the queue
just adds the list to the end.

Negative index positions are supported:

 $q->enqueue(1, 2, 3, 4);
 $q->insert(-2, qw/foo bar/);
 # Queue now contains:  1, 2, foo, bar, 3, 4

Specifying a negative index position greater than the number of items in the
queue adds the list to the head of the queue.

=item ->extract()

=item ->extract(INDEX)

=item ->extract(INDEX, COUNT)

Removes and returns the specified number of items (defaults to 1) from the
specified index position in the queue (0 is the head of the queue).  When
called with no arguments, C<extract> operates the same as C<dequeue_nb>.

This method is non-blocking, and will return only as many items as are
available to fulfill the request:

 $q->enqueue(1, 2, 3, 4);
 my $item  = $q->extract(2)     # Returns 3
                                # Queue now contains:  1, 2, 4
 my @items = $q->extract(1, 3)  # Returns (2, 4)
                                # Queue now contains:  1

Specifying an index position greater than the number of items in the
queue results in C<undef> or an empty list being returned.

 $q->enqueue('foo');
 my $nada = $q->extract(3)      # Returns undef
 my @nada = $q->extract(1, 3)   # Returns ()

Negative index positions are supported.  Specifying a negative index position
greater than the number of items in the queue may return items from the head
of the queue (similar to C<dequeue_nb>) if the count overlaps the head of the
queue from the specified position (i.e. if queue size + index + count is
greater than zero):

 $q->enqueue(qw/foo bar baz/);
 my @nada = $q->extract(-6, 2);  # Returns ()      - (3+(-6)+2) <= 0
 my @some = $q->extract(-6, 4);  # Returns (foo)   - (3+(-6)+4) > 0
                                 # Queue now contains:  bar, baz
 my @rest = $q->extract(-3, 4);  # Returns (bar, baz) -
                                 #                   (2+(-3)+4) > 0

=back

=head1 NOTES

Queues created by L<Thread::Queue> can be used in both threaded and
non-threaded applications.

=head1 LIMITATIONS

Passing objects on queues may not work if the objects' classes do not support
sharing.  See L<threads::shared/"BUGS AND LIMITATIONS"> for more.

Passing array/hash refs that contain objects may not work for Perl prior to
5.10.0.

=head1 SEE ALSO

Thread::Queue on MetaCPAN:
L<https://metacpan.org/release/Thread-Queue>

Code repository for CPAN distribution:
L<https://github.com/Dual-Life/Thread-Queue>

L<threads>, L<threads::shared>

Sample code in the I<examples> directory of this distribution on CPAN.

=head1 MAINTAINER

Jerry D. Hedden, S<E<lt>jdhedden AT cpan DOT orgE<gt>>

=head1 LICENSE

This program is free software; you can redistribute it and/or modify it under
the same terms as Perl itself.

=cut
