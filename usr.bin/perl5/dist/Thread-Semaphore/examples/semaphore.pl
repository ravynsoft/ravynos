#!/usr/bin/perl

use strict;
use warnings;

use threads;
use Thread::Semaphore;

MAIN:
{
    # Create semaphore with count of 0
    my $s = Thread::Semaphore->new(0);

    # Create detached thread
    threads->create(sub {
            # Thread is blocked until released by main
            $s->down();

            # Thread does work
            # ...

            # Tell main that thread is finished
            $s->up();
    })->detach();

    # Release thread to do work
    $s->up();

    # Wait for thread to finish
    $s->down();
}

exit(0);

# EOF
