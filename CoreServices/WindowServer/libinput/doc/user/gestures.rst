.. _gestures:

==============================================================================
Gestures
==============================================================================

libinput supports :ref:`gestures_pinch` and :ref:`gestures_swipe` on most
modern touchpads and other indirect touch devices. Note that libinput **does
not** support gestures on touchscreens, see :ref:`gestures_touchscreens`.

.. _gestures_lifetime:

-----------------------------------------------------------------------------
Lifetime of a gesture
-----------------------------------------------------------------------------

A gesture starts when the finger position and/or finger motion is
unambiguous as to what gesture to trigger and continues until the first
finger belonging to this gesture is lifted.

A single gesture cannot change the finger count. For example, if a user
puts down a fourth finger during a three-finger swipe gesture, libinput will
end (cancel) the three-finger gesture and, if applicable, start a
four-finger swipe gesture. A caller may however decide that those gestures
are semantically identical and continue the two gestures as one single
gesture.

.. _gestures_pinch:

------------------------------------------------------------------------------
Pinch gestures
------------------------------------------------------------------------------

Pinch gestures are executed when two or more fingers are located on the
touchpad and are either changing the relative distance to each other
(pinching) or are changing the relative angle (rotate). Pinch gestures may
change both rotation and distance at the same time. For such gestures,
libinput calculates a logical center for the gestures and provides the
caller with the delta x/y coordinates of that center, the relative angle of
the fingers compared to the previous event, and the absolute scale compared
to the initial finger position.

.. figure:: pinch-gestures.svg
    :align: center

    The pinch and rotate gestures

The illustration above shows a basic pinch in the left image and a rotate in
the right angle. Not shown is a movement of the logical center if the
fingers move unevenly. Such a movement is supported by libinput, it is
merely left out of the illustration.

Note that while position and angle is relative to the previous event, the
scale is always absolute and a multiplier of the initial finger position's
scale.

.. _gestures_swipe:

------------------------------------------------------------------------------
Swipe gestures
------------------------------------------------------------------------------

Swipe gestures are executed when three or more fingers are moved
synchronously in the same direction. libinput provides x and y coordinates
in the gesture and thus allows swipe gestures in any direction, including
the tracing of complex paths. It is up to the caller to interpret the
gesture into an action or limit a gesture to specific directions only.

.. figure:: swipe-gestures.svg
    :align: center

    The swipe gestures

The illustration above shows a vertical three-finger swipe. The coordinates
provided during the gesture are the movements of the logical center.

.. _gestures_hold:

------------------------------------------------------------------------------
Hold gestures
------------------------------------------------------------------------------

A hold gesture is one where the user places one or more fingers on the
device without significant movement. The exact conditions when a hold gesture
transitions to pointer motion, scrolling or other gestures
are implementation-defined.

The hold gesture is intended to allow for the implementation of two specific
features:

- where a two-finger scrolling starts kinetic scrolling in the caller, a
  subsequent hold gesture can be used to stop that kinetic scroll motion,
  and
- hold-to-trigger interactions where the interaction could be a click, a
  context menu, or some other context-specific interaction.

Hold gestures have three potential logical states:

- **begin**: one or more fingers are placed on the device at the same time
- **end**: all fingers are removed and the device enters a neutral logical state
- **end(cancelled)**: all fingers are part of a known interaction and the
  currenthold gesture is no longer active. This may also occurs when
  switching between hold gestures with different finger counts.

.. note:: By definition, a hold gesture does not move and thus no coordinate
          updates are available.

For example, a user that puts one finger, then a second finger down and
releases them later may trigger the following event sequence:

=============  ============  ============
Action         Event         Finger count
=============  ============  ============
Finger 1 down  <no event>
Finger 2 down  **begin**     2
Finger 2 up    **end**       2
Finger 1 up    <no event>
=============  ============  ============

A hold gesture may by be **cancelled**. This occurs
when the hold gesture changes into some other interaction and should no
longer be considered the current hold gesture. A **end(cancelled)** event
applies to the whole gesture (all fingers). For example, a pointer motion on
a touchpad may trigger this sequence:

+-------------------+-----------------------+
| Action            |  Event                |
+===================+=======================+
| | Finger 1 down   | | **hold begin**      |
+-------------------+-----------------------+
| | Finger 1 motion | | **hold cancel**     |
| |                 | | **pointer motion**  |
+-------------------+-----------------------+
| | Finger 1 motion | | **pointer motion**  |
+-------------------+-----------------------+
| | Finger 1 up     | | *no event*          |
+-------------------+-----------------------+

.. note:: Many interactions with a touchpad will start with a hold
          gesture that is then cancelled as that gesture turns into e.g.
          pointer motion. A caller **must** handle hold gesture
          cancellations correctly.

A two-finger scroll motion on a touchpad may trigger this sequence:

+------------------------+---------------------+--------------+
| Action                 |  Event              | Finger count |
+========================+=====================+==============+
| | Finger 1 down        | | **hold begin**    | | 1          |
+------------------------+---------------------+--------------+
| | Finger 2 down        | | **hold cancel**   | | 1          |
| |                      | | **hold begin**    | | 2          |
+------------------------+---------------------+--------------+
| | Finger 1+2 motion    | | **hold cancel**   | | 2          |
| |                      | | **pointer axis**  | |            |
+------------------------+---------------------+--------------+
| | Finger 1+2 motion    | | **pointer axis**  |              |
+------------------------+---------------------+--------------+
| | Finger 1 up          | | **pointer axis**  |              |
| | Finger 2 up          | | (scroll stop)     |              |
+------------------------+---------------------+--------------+

A three-finger-swipe on a touchpad may trigger this sequence:

+---------------------+---------------------+--------------+
| Action              |  Event              | Finger count |
+=====================+=====================+==============+
| | Finger 1 down     |  | **hold begin**   | | 1          |
+---------------------+---------------------+--------------+
| | Finger 2 down     | | **hold cancel**   | | 1          |
| |                   | | **hold begin**    | | 2          |
+---------------------+---------------------+--------------+
| | Finger 3 down     | | **hold cancel**   | | 2          |
| |                   | | **hold begin**    | | 3          |
+---------------------+---------------------+--------------+
| | Finger motion     | | **hold cancel**   | | 3          |
| |                   | | **swipe begin**   | | 3          |
+---------------------+---------------------+--------------+
| | Finger motion     | | **swipe update**  | | 3          |
+---------------------+---------------------+--------------+
| | Finger 1 up       | | **swipe end**     | | 3          |
| | Finger 2 up       | |                   | |            |
| | Finger 3 up       | |                   | |            |
+---------------------+---------------------+--------------+

Single-finger hold gestures
...........................

libinput uses implementation-defined timeouts based on other interactions
to determine whether a single-finger hold gestures should start. In other
words, a caller **must not** rely on a hold gesture always being triggered
as soon as a single finger is placed on the touchpad. This is true for any
hold gesture but especially so for single-finger hold gestures.

Hold gestures with a single finger are prone to being extremely short-lived.
On many devices it is impossible to hold a finger still enough for there to
be no pointer motion events, even if those deltas are miniscule. Changing
movement thresholds to rely on hold gestures would reduce device
responsiveness.

It is thus the responsibility of the caller to determine where hold gestures
transition in and out of other interactions. For example, a two-finger hold
may produce a cancelled single-finger hold gesture first:

+--------------------+----------------------+--------------+--------------+
| Action             |  Event               | Finger count | Notes        |
+====================+======================+==============+==============+
| | Finger 1 down    | | **hold begin**     | | 1          |              |
+--------------------+----------------------+--------------+--------------+
| | Finger 1 motion  | | **hold cancel**    | | 1          | | tiny deltas|
| |                  | | **pointer motion** | |            | |            |
+--------------------+----------------------+--------------+--------------+
| | Finger 2 down    | | **hold begin**     | | 2          |              |
+--------------------+----------------------+--------------+--------------+
| | Finger 1 up      | | **hold end**       | |            |              |
| | Finger 2 up      | |                    | |            |              |
+--------------------+----------------------+--------------+--------------+

Note how the second hold gesture started with a finger count of 2 - without
the user ever lifting the first finger. Cancellation of hold gesture does
not imply the user has lifted a finger.

A hold gesture may start after a previous gesture completed. For example, a
single finger move-and-hold may trigger different sequences for the same
user interaction:

+--------------------+---------------------+-------------------+--------------+
| Action             |  Device 1           | Device 2          | Notes        |
+====================+=====================+===================+==============+
| | Finger 1 down    | | **hold begin**    |  | **hold begin** |              |
+--------------------+---------------------+-------------------+--------------+
| | Finger 1 motion  | | **hold cancel**   |                   | | tiny deltas|
|                    | | **pointer motion**|                   | |            |
+--------------------+---------------------+-------------------+--------------+
|                    | |  **hold begin**   |                   |              |
+--------------------+---------------------+-------------------+--------------+
| |  Finger 1 up     | |  **hold end**     | |  **hold end**   |              |
+--------------------+---------------------+-------------------+--------------+

A caller that wants to use hold gestures must thus be able to infer the same
interaction based on a stream of pointer motion events with small deltas.

libinput may start a new hold begin gesture once the pointer stops moving.
The time between the last pointer motion event and the hold begin event is
implementation-defined.


Hold gestures and thumb/palm detection
......................................

Thumb and palm detection effectively remove touches from being counted
towards an interaction, see :ref:`thumb_detection` and
:ref:`palm_detection` for details.

In the context of hold gestures, thumbs and palms are treated by libinput as
if the finger was removed from the device. Where other non-thumb/non-palm
fingers remain on the device, an **hold update** event is sent. Otherwise,
the hold gesture terminates with a **hold cancel** event.

Notably, libinput's thumb and palm detection is not a simple boolean per
touch but specific to the state of that touch in the overall context. For
example, a touch may be a thumb for tapping but not for clickfinger
interactions. A caller must not infer the number of physical fingers from
the hold gesture.

Likewise, libinput may classify a finger as thumb in the same hardware event
as a new finger is placed on the touchpad. In that case, the hold gesture
**may** continue as one-finger gesture despite there being two physical
touch points.

Information to determine whether a touch is a thumb or a palm may not be
available until some time into an interaction. Thus very short brushes
of the touchpad by a palm may trigger a **hold begin** followed by an
immediate **hold end** as libinput lacks sufficient information to identify
the touch as thumb/palm and send the corresponding **hold cancel**
event. A caller must not assume that a hold gesture always represents a
valid finger down.

Hold gestures and tap-to-click
..............................

:ref:`tapping` is the feature that enables short-lived touches to trigger
button presses.

.. warning:: Summary: do not use hold gestures to do your own tap-to-click
             implementation

In the context of hold gestures, tap-to-click cancels current hold gestures
and a finger dragging (see :ref:`tapndrag`) does not begin a hold
gesture. Where tap-to-click is disabled a tap-like gesture may create
**hold begin** followed by a **hold end** event. Callers **must not** use
hold gestures for their own tap-to-click implementation as the data is not
reliable enough. libinput may change internal timeouts and thresholds
depending on whether tap-to-click is enabled and the hold gesture event may
not match touch sequences that a user would expect to be a tap-to-click
interaction.

.. _gestures_touchscreens:

------------------------------------------------------------------------------
Touchscreen gestures
------------------------------------------------------------------------------

Touchscreen gestures are **not** interpreted by libinput. Rather, any touch
point is passed to the caller and any interpretation of gestures is up to
the caller or, eventually, the X or Wayland client.

Interpreting gestures on a touchscreen requires context that libinput does
not have, such as the location of windows and other virtual objects on the
screen as well as the context of those virtual objects:

.. figure:: touchscreen-gestures.svg
    :align: center

    Context-sensitivity of touchscreen gestures

In the above example, the finger movements are identical but in the left
case both fingers are located within the same window, thus suggesting an
attempt to zoom. In the right case  both fingers are located on a window
border, thus suggesting a window movement. libinput has no knowledge of the
window coordinates and thus cannot differentiate the two.

.. _gestures_softbuttons:

------------------------------------------------------------------------------
Gestures with enabled software buttons
------------------------------------------------------------------------------

If the touchpad device is a :ref:`Clickpad <touchpads_buttons_clickpads>`, it
is recommended that a caller switches to :ref:`clickfinger`.
Usually fingers placed in a :ref:`software button area <software_buttons>`
are not considered for gestures, resulting in some gestures to be
interpreted as pointer motion or two-finger scroll events.

.. figure:: pinch-gestures-softbuttons.svg
    :align: center

    Interference of software buttons and pinch gestures

In the example above, the software button area is highlighted in red. The
user executes a three-finger pinch gesture, with the thumb remaining in the
software button area. libinput ignores fingers within the software button
areas, the movement of the remaining fingers is thus interpreted as a
two-finger scroll motion.

.. _gestures_twofinger_touchpads:

------------------------------------------------------------------------------
Gestures on two-finger touchpads
------------------------------------------------------------------------------

As of kernel 4.2, many :ref:`touchpads_touch_partial_mt` provide only two
slots. This affects how gestures can be interpreted. Touchpads with only two
slots can identify two touches by position but can usually tell that there
is a third (or fourth) finger down on the touchpad - without providing
positional information for that finger.

Touchpoints are assigned in sequential order and only the first two touch
points are trackable. For libinput this produces an ambiguity where it is
impossible to detect whether a gesture is a pinch gesture or a swipe gesture
whenever a user puts the index and middle finger down first. Since the third
finger does not have positional information, it's location cannot be
determined.

.. figure:: gesture-2fg-ambiguity.svg
    :align: center

    Ambiguity of three-finger gestures on two-finger touchpads

The image above illustrates this ambiguity. The index and middle finger are
set down first, the data stream from both finger positions looks identical.
In this case, libinput assumes the fingers are in a horizontal arrangement
(the right image above) and use a swipe gesture.
