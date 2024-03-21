.. _touchpad_jumping_cursor:

==============================================================================
Touchpad jumping cursor bugs
==============================================================================

A common bug encountered on touchpads is a cursor jump when alternating
between fingers on a multi-touch-capable touchpad. For example, after moving
the cursor a user may use a second finger in the software button area to
physically click the touchpad. Upon setting the finger down, the cursor
exhibits a jump towards the bottom left or right, depending on the finger
position.

When libinput detects a cursor jump it prints a bug warning to the log with
the text **"Touch jump detected and discarded."** and a link to this page.

.. note:: This warning is ratelimited and will stop appearing after a few
	  times, even if the touchpad jumps continue.

In most cases, this is a bug in the firmware (or kernel driver) and to
libinput it appears that the touch point moves from its previous position.
The pointer jump can usually be seen in the :ref:`libinput record
<libinput-record>` output for the device:

::

      E: 249.206319 0000 0000 0000    # ------------ SYN_REPORT (0) ----------
      E: 249.218008 0003 0035 3764    # EV_ABS / ABS_MT_POSITION_X    3764
      E: 249.218008 0003 0036 2221    # EV_ABS / ABS_MT_POSITION_Y    2221
      E: 249.218008 0003 003a 0065    # EV_ABS / ABS_MT_PRESSURE      65
      E: 249.218008 0003 0000 3764    # EV_ABS / ABS_X                3764
      E: 249.218008 0003 0001 2216    # EV_ABS / ABS_Y                2216
      E: 249.218008 0003 0018 0065    # EV_ABS / ABS_PRESSURE         65
      E: 249.218008 0000 0000 0000    # ------------ SYN_REPORT (0) ----------
      E: 249.230881 0003 0035 3752    # EV_ABS / ABS_MT_POSITION_X    3752
      E: 249.230881 0003 003a 0046    # EV_ABS / ABS_MT_PRESSURE      46
      E: 249.230881 0003 0000 3758    # EV_ABS / ABS_X                3758
      E: 249.230881 0003 0018 0046    # EV_ABS / ABS_PRESSURE         46
      E: 249.230881 0000 0000 0000    # ------------ SYN_REPORT (0) ----------
      E: 249.242648 0003 0035 1640    # EV_ABS / ABS_MT_POSITION_X    1640
      E: 249.242648 0003 0036 4681    # EV_ABS / ABS_MT_POSITION_Y    4681
      E: 249.242648 0003 003a 0025    # EV_ABS / ABS_MT_PRESSURE      25
      E: 249.242648 0003 0000 1640    # EV_ABS / ABS_X                1640
      E: 249.242648 0003 0001 4681    # EV_ABS / ABS_Y                4681
      E: 249.242648 0003 0018 0025    # EV_ABS / ABS_PRESSURE         25
      E: 249.242648 0000 0000 0000    # ------------ SYN_REPORT (0) ----------
      E: 249.254568 0003 0035 1648    # EV_ABS / ABS_MT_POSITION_X    1648
      E: 249.254568 0003 003a 0027    # EV_ABS / ABS_MT_PRESSURE      27
      E: 249.254568 0003 0000 1644    # EV_ABS / ABS_X                1644
      E: 249.254568 0003 0018 0027    # EV_ABS / ABS_PRESSURE         27


In this recording, the pointer jumps from its position 3752/2216 to
1640/4681 within a single frame. On this particular touchpad, this would
represent a physical move of almost 50mm. libinput detects some of these
jumps and discards the movement but otherwise continues as usual.
If your only encounter with these jumps is the warning printed to the log,
libinput functions as intended.

When you encounter the warning in the log, please generate a recording of
your touchpad with :ref:`libinput record <libinput-record>` and file a bug.
See :ref:`reporting_bugs` for more details.

Note that it most cases, libinput cannot actually fix the issue. Filing a
bug is useful to figure out if there are other factors at play or whether
there are heuristics we can employ to reduce the impact.

------------------------------------------------------------------------------
AlpsPS/2 ALPS DualPoint TouchPad jumping to 4095/0
------------------------------------------------------------------------------

A special case of pointer jumps happens on ``AlpsPS/2 ALPS DualPoint TouchPad``
devices found in the Lenovo ThinkPad E465 and E550 and likely others with
the same touchpad hardware. On those devices, the touchpad occasionally
sends an event for the second finger to move to position 4095/0 before
moving back to the original position. libinput detects this movement and
removes it but depending on the interaction this may cause a smaller jump
later when the coordinates reset to the new position of the finger.

Some more information is available in `Gitlab Issue #492 <https://gitlab.freedesktop.org/libinput/libinput/-/issues/492>`__.
