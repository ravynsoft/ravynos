.. _scrolling:

==============================================================================
Scrolling
==============================================================================

libinput supports three different types of scrolling methods:
:ref:`twofinger_scrolling`, :ref:`edge_scrolling` and
:ref:`button_scrolling`. Some devices support multiple methods, though only
one can be enabled at a time. As a general overview:

- touchpad devices with physical buttons below the touchpad support edge and
  two-finger scrolling
- touchpad devices without physical buttons (:ref:`ClickPads <clickpad_softbuttons>`)
  support two-finger scrolling only
- pointing sticks provide on-button scrolling by default
- mice and other pointing devices support on-button scrolling but it is not
  enabled by default

A device may differ from the above based on its capabilities. See
**libinput_device_config_scroll_set_method()** for documentation on how to
switch methods and **libinput_device_config_scroll_get_methods()** for
documentation on how to query a device for available scroll methods.

.. _horizontal_scrolling:

------------------------------------------------------------------------------
Horizontal scrolling
------------------------------------------------------------------------------

Scroll movements provide vertical and horizontal directions, each
scroll event contains both directions where applicable, see
**libinput_event_pointer_get_axis_value()**. libinput does not provide separate
toggles to enable or disable horizontal scrolling. Instead, horizontal
scrolling is always enabled. This is intentional, libinput does not have
enough context to know when horizontal scrolling is appropriate for a given
widget. The task of filtering horizontal movements is up to the caller.

.. _twofinger_scrolling:

------------------------------------------------------------------------------
Two-finger scrolling
------------------------------------------------------------------------------

The default on two-finger capable touchpads (almost all modern touchpads are
capable of detecting two fingers). Scrolling is triggered by two fingers
being placed on the surface of the touchpad, then moving those fingers
vertically or horizontally.

.. figure:: twofinger-scrolling.svg
    :align: center

    Vertical and horizontal two-finger scrolling

For scrolling to trigger, a built-in distance threshold has to be met, but once
engaged, any movement will scroll. In other words: to start scrolling, a
sufficiently large movement is required; once scrolling, tiny amounts of
movements will translate into tiny scroll movements.
Scrolling in both directions at once is possible by meeting the required
distance thresholds to enable each direction separately.

When a scroll gesture remains close to perfectly straight, it will be held to
exact 90-degree angles; but if the gesture moves diagonally, it is free to
scroll in any direction.

Two-finger scrolling requires the touchpad to track both touch points with
reasonable precision. Unfortunately, some so-called "semi-mt" touchpads can
only track the bounding box of the two fingers rather than the actual
position of each finger. In addition, that bounding box usually suffers from
a low resolution, causing jumpy movement during two-finger scrolling.
libinput does not provide two-finger scrolling on those touchpads.

.. _edge_scrolling:

------------------------------------------------------------------------------
Edge scrolling
------------------------------------------------------------------------------

On some touchpads, edge scrolling is available, triggered by moving a single
finger along the right edge (vertical scroll) or bottom edge (horizontal
scroll).

.. figure:: edge-scrolling.svg
    :align: center

    Vertical and horizontal edge scrolling

Due to the layout of the edges, diagonal scrolling is not possible. The
behavior of edge scrolling using both edges at the same time is undefined.

Edge scrolling overlaps with :ref:`clickpad_softbuttons`. A physical click on
a clickpad ends scrolling.

.. _button_scrolling:

------------------------------------------------------------------------------
On-Button scrolling
------------------------------------------------------------------------------

On-button scrolling converts the motion of a device into scroll events while
a designated button is held down. For example, Lenovo devices provide a
`pointing stick <http://en.wikipedia.org/wiki/Pointing_stick>`_ that emulates
scroll events when the trackstick's middle mouse button is held down.

.. note:: On-button scrolling is enabled by default for pointing sticks. This
	prevents middle-button dragging; all motion events while the middle
	button is down are converted to scroll events.

.. figure:: button-scrolling.svg
    :align: center

    Button scrolling

The button may be changed with
**libinput_device_config_scroll_set_button()** but must be on the same device as
the motion events. Cross-device scrolling is not supported but
for one exception: libinput's :ref:`t440_support` enables the use of the middle
button for button scrolling (even when the touchpad is disabled).

If the scroll button lock is enabled (see
**libinput_device_config_scroll_set_button_lock()**), the button does not
need to be held down. Pressing and releasing the button once enables the
button lock, the button is now considered logically held down. Pressing and
releasing the button a second time logically releases the button. While the
button is logically held down, motion events are converted to scroll events.

.. _scroll_sources:

------------------------------------------------------------------------------
Scroll sources
------------------------------------------------------------------------------

.. note:: Scroll sources are deprecated with libinput 1.19. The scroll
   source is now encoded in the event type.

libinput provides a pointer axis *source* for each scroll event. The
source can be obtained with the **libinput_event_pointer_get_axis_source()**
function and is one of **wheel**, **finger**, or **continuous**. The source
information lets a caller decide when to implement kinetic scrolling.
Usually, a caller will process events of source wheel as they come in.
For events of source finger a caller should calculate the velocity of the
scroll motion and upon finger release start a kinetic scrolling motion (i.e.
continue executing a scroll according to some friction factor).
libinput expects the caller to be in charge of widget handling, the source
information is thus enough to provide kinetic scrolling on a per-widget
basis. A caller should cancel kinetic scrolling when the pointer leaves the
current widget or when a key is pressed.

See the **libinput_event_pointer_get_axis_source()** for details on the
behavior of each scroll source.

See also http://who-t.blogspot.com.au/2015/03/libinput-scroll-sources.html

.. _natural_scrolling:

------------------------------------------------------------------------------
Natural scrolling vs. traditional scrolling
------------------------------------------------------------------------------

Natural scrolling is the term (probably) coined by Apple for matching
the motion of the scroll device with the direction of the **content**.

In traditional scrolling, moving the wheel down causes the scroll bar
indicators to move down and the content to move up. In natural scrolling,
moving the wheel down causes the content to move down and the scroll bar
indicators to move up. This method of scrolling matches the interaction
with content on touch screens where a movement down also moves the content
down.

libinput supports natural scrolling for all its scroll methods; it can
be enabled with the
**libinput_device_config_scroll_set_natural_scroll_enabled()** function.
