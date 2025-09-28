#!/usr/bin/env perl

use strict;
use warnings;

use threads;
use Thread::Queue 3.01;

# Create a work queue for sending data to a 'worker' thread
#   Prepopulate it with a few work items
my $work_q = Thread::Queue->new(qw/foo bar baz/);

# Create a status queue to get reports from the thread
my $status_q = Thread::Queue->new();

# Create a detached thread to process items from the queue
threads->create(sub {
                    # Keep grabbing items off the work queue
                    while (defined(my $item = $work_q->dequeue())) {
                        # Process the item from the queue
                        print("Thread got '$item'\n");

                        # Ask for more work when the queue is empty
                        if (! $work_q->pending()) {
                            print("\nThread waiting for more work\n\n");
                            $status_q->enqueue('more');
                        }
                    }

                    # Final report
                    print("Thread done\n");
                    $status_q->enqueue('done');

                })->detach();

# More work for the thread
my @work = (
    [ 'bippity', 'boppity', 'boo' ],
    [ 'ping', 'pong' ],
    [ 'dit', 'dot', 'dit' ],
);

# Send work to the thread
while ($status_q->dequeue() eq 'more') {
    last if (! @work);   # No more work
    $work_q->enqueue(@{shift(@work)});
}

# Signal that there is no more work
$work_q->end();
# Wait for thread to terminate
$status_q->dequeue();
# Good-bye
print("Done\n");

# EOF
