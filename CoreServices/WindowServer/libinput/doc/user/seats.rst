.. _seats:

==============================================================================
Seats
==============================================================================

Each device in libinput is assigned to one seat.
A seat has two identifiers, the physical name and the logical name. The
physical name is summarized as the list of devices a process on the same
physical seat has access to. The logical seat name is the seat name for a
logical group of devices. A compositor may use that to create additional
seats as independent device sets. Alternatively, a compositor may limit
itself to a single logical seat, leaving a second compositor to manage
devices on the other logical seats.

.. _seats_overview:

------------------------------------------------------------------------------
Overview
------------------------------------------------------------------------------

Below is an illustration of how physical seats and logical seats interact:

.. graphviz:: seats-sketch.gv

The devices "Foo", "Bar" and "Spam" share the same physical seat and are
thus available in the same libinput context. Only "Foo" and "Bar" share the
same logical seat. The device "Egg" is not available in the libinput context
associated with the physical seat 0.

The above graph is for illustration purposes only. In libinput, a struct
**libinput_seat** comprises both physical seat and logical seat. From a
caller's point-of-view the above device layout is presented as:

.. graphviz:: seats-sketch-libinput.gv

Thus, devices "Foo" and "Bar" both reference the same struct
**libinput_seat**, all other devices reference their own respective seats.

.. _seats_and_features:

------------------------------------------------------------------------------
The effect of seat assignment
------------------------------------------------------------------------------

A logical set is interpreted as a group of devices that usually belong to a
single user that interacts with a computer. Thus, the devices are
semantically related. This means for devices within the same logical seat:

- if the same button is pressed on different devices, the button should only
  be considered logically pressed once.
- if the same button is released on one device, the button should be
  considered logically down if still down on another device.
- if two different buttons or keys are pressed on different devices, the
  logical state is that of both buttons/keys down.
- if a button is pressed on one device and another device moves, this should
  count as dragging.
- if two touches are down on different devices, the logical state is that of
  two touches down.

libinput provides functions to aid with the above:
**libinput_event_pointer_get_seat_button_count()**,
**libinput_event_keyboard_get_seat_key_count()**, and
**libinput_event_touch_get_seat_slot()**.

Internally, libinput counts devices within the same logical seat as related.
Cross-device features only activate if all required devices are in the same
logical seat. For example, libinput will only activate the top software
buttons (see :ref:`t440_support`) if both trackstick and touchpad are assigned
to the same logical seat.


.. _changing_seats:

------------------------------------------------------------------------------
Changing seats
------------------------------------------------------------------------------

A device may change the logical seat it is assigned to at runtime with
**libinput_device_set_seat_logical_name()**. The physical seat is immutable and
may not be changed.

Changing the logical seat for a device is equivalent to unplugging the
device and plugging it back in with the new logical seat. No device state
carries over across a logical seat change.
