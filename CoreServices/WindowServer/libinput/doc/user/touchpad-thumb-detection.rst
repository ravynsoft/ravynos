.. _thumb_detection:

==============================================================================
Thumb detection
==============================================================================

Thumb detection tries to identify touches triggered by a thumb rather than a
pointer-moving finger. This is necessary on :ref:`touchpads_buttons_clickpads`
as a finger pressing a button always creates a new touch, causing
misinterpretation of gestures. Click-and-drag with two fingers (one holding
the button, one moving) would be interpreted as two-finger scrolling without
working thumb detection.

libinput has built-in thumb detection, partially dependent on
hardware-specific capabilities.

- :ref:`thumb_pressure`
- :ref:`thumb_areas`
- :ref:`thumb_speed`

Thumb detection uses multiple approaches and the final decision on whether
to ignore a thumb depends on the interaction at the time.

.. _thumb_pressure:

------------------------------------------------------------------------------
Thumb detection based on pressure or size
------------------------------------------------------------------------------

The simplest form of thumb detection identifies a touch as thumb when the
pressure value goes above a certain threshold. This threshold is usually
high enough that it cannot be triggered by a finger movement.

On touchpads that support the ``ABS_MT_TOUCH_MAJOR`` axes, libinput can perform
thumb detection based on the size of the touch ellipse. This works similar to
the pressure-based palm detection in that a touch is labelled as palm when
it exceeds the (device-specific) touch size threshold.

Pressure- and size-based thumb detection depends on the location of the
thumb and usually only applies within the :ref:`thumb_areas`.

For some information on how to detect pressure on a touch and debug the
pressure ranges, see :ref:`touchpad_pressure`. Pressure- and size-based
thumb detection require thresholds set in the :ref:`device-quirks`.

.. _thumb_areas:

------------------------------------------------------------------------------
Thumb detection areas
------------------------------------------------------------------------------

Pressure and size readings are unreliable at the far bottom of the touchpad.
A thumb hanging mostly off the touchpad will have a small surface area.
libinput has a definitive thumb zone where any touch is considered a
thumb. Immediately above that area is the area where libinput will label a
thumb as such if the pressure or size thresholds are exceeded.


.. figure:: thumb-detection.svg
    :align: center

The picture above shows the two detection areas. In the larger (light red)
area, a touch is labelled as thumb when it exceeds a device-specific
pressure threshold. In the lower (dark red) area, a touch is always labelled
as thumb.

Moving outside the areas generally releases the thumb from being a thumb.

.. _thumb_speed:

------------------------------------------------------------------------------
Thumb movement based on speed
------------------------------------------------------------------------------

Regular interactions with thumbs do not usually move the thumb. When fingers
are moving across the touchpad and a thumb is dropped, this can cause
erroneous scroll motion or similar issues. libinput observes the finger
motion speed for all touches - where a finger has been moving a newly
dropped finger is more likely to be labeled as thumb.

------------------------------------------------------------------------------
Thumb detection based on finger positions
------------------------------------------------------------------------------

The shape of the human hand and the interactions that usually involve a
thumb imply that a thumb is situated in a specific position relative to
other fingers (usually to the side and below). This is used by libinput to
detect thumbs during some interactions that do not implicitly require a
thumb (e.g. pinch-and-rotate).
