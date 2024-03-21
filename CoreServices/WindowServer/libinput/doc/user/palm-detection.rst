.. _palm_detection:

==============================================================================
Palm detection
==============================================================================

Palm detection tries to identify accidental touches while typing, while
using the trackpoint and/or during general use of the touchpad area.

On most laptops typing on the keyboard generates accidental touches on the
touchpad with the palm (usually the area below the thumb). This can lead to
cursor jumps or accidental clicks. On large touchpads, the palm may also
touch the bottom edges of the touchpad during normal interaction.

Interference from a palm depends on the size of the touchpad and the position
of the user's hand. Data from touchpads showed that almost all palm events
during tying on a Lenovo T440 happened in the left-most and right-most 5% of
the touchpad. The T440 series has one of the largest touchpads, other
touchpads are less affected by palm touches.

libinput has multiple ways of detecting a palm, each of which depends on
hardware-specific capabilities.

- :ref:`palm_tool`
- :ref:`palm_pressure`
- :ref:`palm_touch_size`
- :ref:`palm_exclusion_zones`
- :ref:`trackpoint-disabling`
- :ref:`disable-while-typing`
- :ref:`disable-while-trackpointing`
- :ref:`stylus-touch-arbitration`

Palm detection is always enabled, with the exception of
disable-while-typing.

.. _palm_tool:

------------------------------------------------------------------------------
Palm detection based on firmware labelling
------------------------------------------------------------------------------

Some devices provide palm detection in the firmware, forwarded by the kernel
as the ``EV_ABS/ABS_MT_TOOL`` axis with a value of ``MT_TOOL_PALM``
(whenever a palm is detected). libinput honors that value and switches that
touch to a palm.

.. _palm_pressure:

------------------------------------------------------------------------------
Palm detection based on pressure
------------------------------------------------------------------------------

The simplest form of palm detection labels a touch as palm when the pressure
value goes above a certain threshold. This threshold is usually high enough
that it cannot be triggered by a finger movement. One a touch is labelled as
palm based on pressure, it will remain so even if the pressure drops below
the threshold again. This ensures that a palm remains a palm even when the
pressure changes as the user is typing.

For some information on how to detect pressure on a touch and debug the
pressure ranges, see :ref:`touchpad_pressure`.

.. _palm_touch_size:

------------------------------------------------------------------------------
Palm detection based on touch size
------------------------------------------------------------------------------

On touchpads that support the ``ABS_MT_TOUCH_MAJOR`` axes, libinput can perform
palm detection based on the size of the touch ellipse. This works similar to
the pressure-based palm detection in that a touch is labelled as palm when
it exceeds the (device-specific) touch size threshold.

For some information on how to detect the size of a touch and debug the
touch size ranges, see :ref:`touchpad_pressure`.

.. _palm_exclusion_zones:

------------------------------------------------------------------------------
Palm exclusion zones
------------------------------------------------------------------------------

libinput enables palm detection on the left, right and top edges of the
touchpad. Two exclusion zones are defined  on the left and right edge of the
touchpad. If a touch starts in the exclusion zone, it is considered a palm
and the touch point is ignored. However, for fast cursor movements across
the screen, it is common for a finger to start inside an exclusion zone and
move rapidly across the touchpad. libinput detects such movements and avoids
palm detection on such touch sequences.

Another exclusion zone is defined on the top edge of the touchpad. As with
the edge zones, libinput detects vertical movements out of the edge zone and
avoids palm detection on such touch sequences.

A touch starting in the exclusion zone does not trigger a tap (see
:ref:`tapping`).

In the diagram below, the exclusion zones are painted red.
Touch 'A' starts inside the exclusion zone and moves
almost vertically. It is considered a palm and ignored for cursor movement,
despite moving out of the exclusion zone.

Touch 'B' starts inside the exclusion zone but moves horizontally out of the
zone. It is considered a valid touch and controls the cursor.

Touch 'C' occurs in the exclusion zone. Despite being a tapping motion, it does
not generate an emulated button event.

.. figure:: palm-detection.svg
    :align: center

.. _trackpoint-disabling:

------------------------------------------------------------------------------
Palm detection during trackpoint use
------------------------------------------------------------------------------

If a device provides a
`trackpoint <http://en.wikipedia.org/wiki/Pointing_stick>`_, it is
usually located above the touchpad. This increases the likelihood of
accidental touches whenever the trackpoint is used.

libinput disables the touchpad whenever it detects trackpoint activity for a
certain timeout until after trackpoint activity stops. Touches generated
during this timeout will not move the pointer, and touches started during
this timeout will likewise not move the pointer (allowing for a user to rest
the palm on the touchpad while using the trackstick).
If the touchpad is disabled, the :ref:`top software buttons <t440_support>`
remain enabled.

.. _disable-while-typing:

------------------------------------------------------------------------------
Disable-while-typing
------------------------------------------------------------------------------

libinput automatically disables the touchpad for a timeout after a key
press, a feature traditionally referred to as "disable while typing" and
previously available through the
`syndaemon(1) <http://linux.die.net/man/1/syndaemon>`_ command. libinput does
not require an external command and the feature is currently enabled for all
touchpads but will be reduced in the future to only apply to touchpads where
finger width or pressure data is unreliable.

Notable behaviors of libinput's disable-while-typing feature:

- Two different timeouts are used, after a single key press the timeout is
  short to ensure responsiveness. After multiple key events, the timeout is
  longer to avoid accidental pointer manipulation while typing.
- Some keys do not trigger the timeout, specifically some modifier keys
  (Ctrl, Alt, Shift, and Fn). Actions such as Ctrl + click thus stay
  responsive.
- Touches started while typing do not control the cursor even after typing
  has stopped, it is thus possible to rest the palm on the touchpad while
  typing.
- Physical buttons work even while the touchpad is disabled. This includes
  :ref:`software-emulated buttons <t440_support>`.
- libinput pairs touchpads and keyboards for the disable-while-typing
  feature. In the most common case, the internal touchpad is paired only
  with the internal keyboard. Typing on an external keyboard will thus not
  disable the touchpad. Some devices require a :ref:`quirk <device-quirks>`
  to be correctly paired.

Disable-while-typing can be enabled and disabled by calling
**libinput_device_config_dwt_set_enabled()**.

.. _disable-while-trackpointing:

------------------------------------------------------------------------------
Disable-while-trackpointing
------------------------------------------------------------------------------

libinput automatically disables the touchpad for a timeout after the trackpoint
is moved, a feature referred to as "disable while trackpointing". libinput does
not require an external command and the feature is currently enabled for all
touchpads.

Disable-while-trackpointing can be enabled and disabled by calling
**libinput_device_config_dwtp_set_enabled()**.

.. _stylus-touch-arbitration:

------------------------------------------------------------------------------
Stylus-touch arbitration
------------------------------------------------------------------------------

A special case of palm detection is touch arbitration on devices that
support styli. When interacting with a stylus on the screen, parts of the
hand may touch the surface and trigger touches. As the user is currently
interacting with the stylus, these touches would interfer with the correct
working of the stylus.

libinput employs a method similar to :ref:`disable-while-typing` to detect
these touches and disables the touchpad accordingly.

.. _thumb-detection:

------------------------------------------------------------------------------
Thumb detection
------------------------------------------------------------------------------

Many users rest their thumb on the touchpad while using the index finger to
move the finger around. For clicks, often the thumb is used rather than the
finger. The thumb should otherwise be ignored as a touch, i.e. it should not
count towards :ref:`clickfinger` and it should not cause a single-finger
movement to trigger :ref:`twofinger_scrolling`.

libinput uses two triggers for thumb detection: pressure and
location. A touch exceeding a pressure threshold is considered a thumb if it
is within the thumb detection zone.

.. note:: "Pressure" on touchpads is synonymous with "contact area." A large touch
	surface area has a higher pressure and thus hints at a thumb or palm
	touching the surface.

Pressure readings are unreliable at the far bottom of the touchpad as a
thumb hanging mostly off the touchpad will have a small surface area.
libinput has a definitive thumb zone where any touch is considered a resting
thumb.

.. figure:: thumb-detection.svg
    :align: center

The picture above shows the two detection areas. In the larger (light red)
area, a touch is labelled as thumb when it exceeds a device-specific
pressure threshold. In the lower (dark red) area, a touch is labelled as
thumb if it remains in that area for a time without moving outside.
