#!/usr/bin/perl

# Simplified example illustrating event handling and callback threads

# Callback threads register their queues with the event handler thread.
# Events are passed to the event handler via a queue.
# The event handler then disseminates the event to the appropriately
#   registered thread.

use strict;
use warnings;

use threads;
use Thread::Queue;

MAIN:
{
    # Queue for registering callbacks
    my $regis_q = Thread::Queue->new();

    # Queue for disseminating events
    my $event_q = Thread::Queue->new();

    # Create callback threads
    threads->create('CallBack', 'USR1', $regis_q)->detach();
    threads->create('CallBack', 'USR2', $regis_q)->detach();
    threads->create('CallBack', 'HUP', $regis_q)->detach();
    threads->create('CallBack', 'ALRM', $regis_q)->detach();

    # Create event handler thread
    threads->create('EventHandler', $regis_q, $event_q)->detach();

    # Capture SIGUSR1 events
    $SIG{'USR1'} = sub {
        $event_q->enqueue('USR1');  # Send to event handler
    };

    # Capture SIGUSR1 events
    $SIG{'USR2'} = sub {
        $event_q->enqueue('USR2');  # Send to event handler
    };

    # Capture SIGHUP events
    $SIG{'HUP'} = sub {
        $event_q->enqueue('HUP');  # Send to event handler
    };

    # Capture SIGHUP events
    $SIG{'ALRM'} = sub {
        $event_q->enqueue('ALRM');  # Send to event handler
        alarm(5);                   # Reset alarm
    };

    # Ready
    print(<<_MSG_);
Send signals to PID = $$
  (e.g., 'kill -USR1 $$')
Use ^C (or 'kill -INT $$') to terminate
_MSG_

    # Set initial alarm
    alarm(5);

    # Just hang around
    while (1) {
        sleep(10);
    }
}

### Subroutines ###

sub EventHandler
{
    my ($regis_q, $event_q) = @_;

    my %callbacks;   # Registered callback queues

    while (1) {
        # Check for any registrations
        while (my ($event_type, $q) = $regis_q->dequeue_nb(2)) {
            if ($q) {
                $callbacks{$event_type} = $q;
            } else {
                warn("BUG: Bad callback registration for event type $event_type\n");
            }
        }

        # Wait for event
        if (my $event = $event_q->dequeue()) {
            # Send event to appropriate queue
            if (exists($callbacks{$event})) {
                $callbacks{$event}->enqueue($event);
            } else {
                warn("WARNING: No callback for event type $event\n");
            }
        }
    }
}


sub CallBack
{
    my $event_type = shift;   # The type of event I'm handling
    my $regis_q    = shift;

    # Announce registration
    my $tid = threads->tid();
    print("Callback thread $tid registering for $event_type events\n");

    # Register my queue for my type of event
    my $q = Thread::Queue->new();
    $regis_q->enqueue($event_type, $q);

    # Process loop
    while (1) {
        # Wait for event callback
        my $item = $q->dequeue();
        # Process event
        print("Callback thread $tid notified of $item event\n") if $item;
    }
}

# EOF
