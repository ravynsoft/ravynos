:orphan:

.. _touchpads:

==============================================================================
Touchpads
==============================================================================

This page provides an outline of touchpad devices. Touchpads aren't simply
categorised into a single type, instead they have a set of properties, a
combination of number of physical buttons, multitouch support abilities and
other properties.

.. _touchpads_buttons:

------------------------------------------------------------------------------
Number of buttons
------------------------------------------------------------------------------

.. _touchapds_buttons_phys:

..............................................................................
Physically separate buttons
..............................................................................

Touchpads with physical buttons usually provide two buttons, left and right.
A few touchpads with three buttons exist, and Apple used to have touchpads
with a single physical buttons until ca 2008. Touchpads with only two
buttons require the software stack to emulate a middle button. libinput does
this when both buttons are pressed simultaneously.

Note that many Lenovo laptops provide a pointing stick above the touchpad.
This pointing stick has a set of physical buttons just above the touchpad.
While many users use those as substitute touchpad buttons, they logically
belong to the pointing stick. The \*40 and \*50 series are an exception here,
the former had no physical buttons on the touchpad and required the top
section of the pad to emulate pointing stick buttons, the \*50 series has
physical buttons but they are wired to the touchpads. The kernel re-routes
those buttons through the trackstick device. See :ref:`t440_support` for more
information.

.. _touchpads_buttons_clickpads:

..............................................................................
Clickpads
..............................................................................

Clickpads are the most common type of touchpads these days. A Clickpad has
no separate physical buttons, instead the touchpad itself is clickable as a
whole, i.e. a user presses down on the touch area and triggers a physical
click. Clickpads thus only provide a single button, everything else needs to
be software-emulated. See :ref:`clickpad_softbuttons` for more information.

Clickpads are labelled by the kernel with the **INPUT_PROP_BUTTONPAD** input
property.

.. _touchpads_buttons_forcepads:

..............................................................................
Forcepads
..............................................................................

Forcepads are Clickpads without a physical button underneath the hardware.
They provide pressure and may have a vibration element that is
software-controlled. This element can simulate the feel of a physical
click or be co-opted for other tasks.


.. _touchpads_touch:

------------------------------------------------------------------------------
Touch capabilities
------------------------------------------------------------------------------

Virtually all touchpads available now can **detect** multiple fingers on
the touchpad, i.e. provide information on how many fingers are on the
touchpad. The touch capabilities described here specify how many fingers a
device can **track**, i.e. provide reliable positional information for.
In the kernel each finger is tracked in a so-called "slot", the number of
slots thus equals the number of simultaneous touches a device can track.

.. _touchapds_touch_st:

..............................................................................
Single-touch touchpads
..............................................................................

Single-finger touchpads can track a single touchpoint. Most single-touch
touchpads can also detect three fingers on the touchpad, but no positional
information is provided for those. In libinput, these touches are termed
"fake touches". The kernel sends **BTN_TOOL_DOUBLETAP**,
**BTN_TOOL_TRIPLETAP**, **BTN_TOOL_QUADTAP** and **BTN_TOOL_QUINTTAP**
events when multiple fingers are detected.

.. _touchpads_touch_mt:

..............................................................................
Pure multi-touch touchpads
..............................................................................

Pure multi-touch touchpads are those that can track, i.e. identify the
location of all fingers on the touchpad. Apple's touchpads support 16
touches, others support 5 touches like the Synaptics touchpads when using
SMBus.

These touchpads usually also provide extra information. Apple touchpads
provide an ellipse and the orientation of the ellipse for each touch point.
Other touchpads provide a pressure value for each touch point (see
:ref:`touchpads_pressure_handling`).

Note that the kernel sends **BTN_TOOL_DOUBLETAP**,
**BTN_TOOL_TRIPLETAP**, **BTN_TOOL_QUADTAP** and **BTN_TOOL_QUINTTAP**
events for all touches for backwards compatibility. libinput ignores these
events if the touchpad can track touches correctly.

.. _touchpads_touch_partial_mt:

..............................................................................
Partial multi-touch touchpads
..............................................................................

The vast majority of touchpads fall into this category, the half-way
point between single-touch and pure multi-touch. These devices can track N
fingers, but detect more than N. For example, when using the serial
protocol, Synaptics touchpads can track two fingers but may detect up to
five.

The number of slots may limit which features are available in libinput.
Any device with two slots can support two-finger scrolling, but
:ref:`thumb-detection` or :ref:`palm_detection` may be limited if only two
slots are available.

.. _touchpads_touch_semi_mt:

..............................................................................
Semi-mt touchpads
..............................................................................

A sub-class of partial multi-touch touchpads. These touchpads can
technically detect two fingers but the location of both is limited to the
bounding box, i.e. the first touch is always the top-left one and the second
touch is the bottom-right one. Coordinates jump around as fingers move past
each other.

Many semi-mt touchpads also have a lower resolution for the second touch, or
both touches. This may limit some features such as :ref:`gestures` or
:ref:`scrolling`.

Semi-mt are labelled by the kernel with the **INPUT_PROP_SEMI_MT** input
property.

.. _touchpads_mis:

------------------------------------------------------------------------------
Other touchpad properties
------------------------------------------------------------------------------

.. _touchpads_external:

..............................................................................
External touchpads
..............................................................................

External touchpads are USB or Bluetooth touchpads not in a laptop chassis,
e.g. Apple Magic Trackpad or the Logitech T650. These are usually
:ref:`touchpads_buttons_clickpads` the biggest difference is that they can be
removed or added at runtime.

One interaction method that is only possible on external touchpads is a
thumb resting on the very edge/immediately next to the touchpad. On the far
edge, touchpads don't always detect the finger location so clicking with a
thumb barely touching the edge makes it hard or impossible to figure out
which software button area the finger is on.

These touchpads also don't need :ref:`palm_detection` - since they're not
located underneath the keyboard, accidental palm touches are a non-issue.

.. _touchpads_pressure_handling:

..............................................................................
Touchpads pressure handling
..............................................................................

Pressure is usually directly related to contact area. Human fingers flatten
out as the pressure on the pad increases, resulting in a bigger contact area
and the firmware then calculates that back into a pressure reading.

libinput uses pressure to detect accidental palm contact and thumbs, though
pressure data is often device-specific and unreliable.

.. _touchpads_circular:

..............................................................................
Circular touchpads
..............................................................................

Only listed for completeness, circular touchpads have not been used in
laptops for a number of years. These touchpad shaped in an ellipse or
straight.

.. _touchpads_tablets:

..............................................................................
Graphics tablets
..............................................................................

Touch-capable graphics tablets are effectively external touchpads, with two
differentiators: they are larger than normal touchpads and they have no
regular touchpad buttons. They either work like a
:ref:`touchpads_buttons_forcepads` Forcepad, or rely on interaction methods that
don't require buttons (like :ref:`tapping`). Since the physical device is
shared with the pen input, some touch arbitration is required to avoid touch
input interfering when the pen is in use.

.. _touchpads_edge_zone:

..............................................................................
Dedicated edge scroll area
..............................................................................

Before :ref:`twofinger_scrolling` became the default scroll method, some
touchpads provided a marking on the touch area that designates the
edge to be used for scrolling. A finger movement in that edge zone should
trigger vertical motions. Some touchpads had markers for a horizontal
scroll area too at the bottom of the touchpad.
