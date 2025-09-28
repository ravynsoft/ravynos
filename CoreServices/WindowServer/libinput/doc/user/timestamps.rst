
.. _timestamps:

==============================================================================
Timestamps
==============================================================================

.. _event_timestamps:

------------------------------------------------------------------------------
Event timestamps
------------------------------------------------------------------------------

Most libinput events provide a timestamp in millisecond and/or microsecond
resolution. These timestamp usually increase monotonically, but libinput
does not guarantee that this always the case. In other words, it is possible
to receive an event with a timestamp earlier than the previous event.

For example, if a touchpad has :ref:`tapping` enabled, a button event may have a
lower timestamp than an event from a different device. Tapping requires the
use of timeouts to detect multi-finger taps and/or :ref:`tapndrag`.

Consider the following event sequences from a touchpad and a mouse:


::

     Time      Touchpad      Mouse
     ---------------------------------
     t1       finger down
     t2        finger up
     t3                     movement
     t4       tap timeout


For this event sequence, the first event to be sent to a caller is in
response to the mouse movement: an event of type
**LIBINPUT_EVENT_POINTER_MOTION** with the timestamp t3.
Once the timeout expires at t4, libinput generates an event of
**LIBINPUT_EVENT_POINTER_BUTTON** (press) with a timestamp t1 and an event
**LIBINPUT_EVENT_POINTER_BUTTON** (release) with a timestamp t2.

Thus, the caller gets events with timestamps in the order t3, t1, t2,
despite t3 > t2 > t1.

libinput timestamps use **CLOCK_MONOTONIC**.
