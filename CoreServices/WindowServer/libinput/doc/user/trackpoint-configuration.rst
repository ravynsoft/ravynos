.. _trackpoint_configuration:

==============================================================================
Trackpoint configuration
==============================================================================

The sections below describe the trackpoint magic multiplier and how to apply
it to your local device. See :ref:`trackpoint_range` for an explanation on
why this multiplier is needed.

.. note:: The magic trackpoint multiplier **is not user visible configuration**. It is
           part of the :ref:`device-quirks` system and provided once per device.

User-specific preferences can be adjusted with the
:ref:`config_pointer_acceleration` setting.

.. _trackpoint_multiplier:

------------------------------------------------------------------------------
The magic trackpoint multiplier
------------------------------------------------------------------------------

To accommodate for the wildly different input data on trackpoint, libinput
uses a multiplier that is applied to input deltas. Trackpoints that send
comparatively high deltas can be "slowed down", trackpoints that send low
deltas can be "sped up" to match the expected range. The actual acceleration
profile is applied to these pre-multiplied deltas.

Given a trackpoint delta ``(dx, dy)``, a multiplier ``M`` and a pointer acceleration
function ``f(dx, dy) → (dx', dy')``, the algorithm is effectively:

::

     f(M * dx, M * dy) → (dx', dy')

.. _trackpoint_multiplier_adjustment:

..............................................................................
Adjusting the magic trackpoint multiplier
..............................................................................

This section only applies if:

- the trackpoint default speed (speed setting 0) is unusably slow or
  unusably fast, **and**
- the lowest speed setting (-1) is still too fast **or** the highest speed
  setting is still too slow, **and**
- the :ref:`device-quirks` for this device do not list a trackpoint multiplier
  (see :ref:`device-quirks-debugging`)

If the only satisfactory speed settings are less than -0.75 or greater than
0.75, a multiplier *may* be required.

A specific multiplier will apply to **all users with the same laptop
model**, so proceed with caution. You must be capable/willing to adjust
device quirks, build libinput from source and restart the session frequently
to adjust the multiplier. If this does not apply, wait for someone else with
the same hardware to do this.

Finding the correct multiplier is difficult and requires some trial and
error. The default multiplier is always 1.0. A value between 0.0 and 1.0
slows the trackpoint down, a value above 1.0 speeds the trackpoint up.
Values below zero are invalid.

.. warning:: The multiplier is not a configuration to adjust to personal
	preferences. The multiplier normalizes the input data into a range that
	can then be configured with the speed setting.

To adjust the local multiplier, first
:ref:`build libinput from git master <building_libinput>`. It is not
required to install libinput from git. The below assumes that all
:ref:`building_dependencies` are already
installed.


::

     $ cd path/to/libinput.git

     # Use an approximate multiplier in the quirks file
     $ cat > quirks/99-trackpoint-override.quirks <<EOF
     [Trackpoint Override]
     MatchUdevType=pointingstick
     AttrTrackpointMultiplier=1.0
     EOF

     # Use your trackpoint's event node. If the Attr does not show up
     # then the quirk does not apply to your trackpoint.
     $ ./builddir/libinput quirks list /dev/input/event18
     AttrTrackpointMultiplier=1.0

     # Now start a GUI program to debug the trackpoint speed.
     # ESC closes the debug GUI
     $ sudo ./builddir/libinput debug-gui


Replace the multiplier with an approximate value and the event node with
your trackpoint's event node. Try to use trackpoint and verify the
multiplier is good enough. If not, adjust the ``.quirks`` file and re-run the
``libinput debug-gui``.  Note that the ``libinput debug-gui`` always feels
less responsive than libinput would behave in a normal install.

Once the trackpoint behaves correctly you are ready to test the system
libinput:


::

     $ sudo cp quirks/99-trackpoint-override.quirks /etc/libinput/local-overrides.quirks


Now verify the override is seen by the system libinput

::

     $ libinput quirks list
     AttrTrackpointMultiplier=1.0


If the multiplier is listed, restart your Wayland session or X server. The
new multiplier is now applied to your trackpoint.

If the trackpoint behavior is acceptable, you are ready to submit this file
upstream. First, find add a more precise match for the device so it only
applies to the built-in trackpoint on your laptop model. Usually a
variation of the following is sufficient:


::

     [Trackpoint Override]
     MatchUdevType=pointingstick
     MatchName=*TPPS/2 IBM TrackPoint*
     MatchDMIModalias=dmi:*svnLENOVO:*:pvrThinkPadT440p*
     AttrTrackpointMultiplier=1.0


Look at your ``/sys/class/dmi/id/modalias`` file for the values to add. Verify
that ``libinput quirks list`` still shows the ``AttrTrackpointMultiplier``. If
it does, then you should :ref:`report a bug <reporting_bugs>` with the contents of
the file. Alternatively, file a merge request with the data added.


.. _trackpoint_range_measure:

------------------------------------------------------------------------------
Measuring the trackpoint range
------------------------------------------------------------------------------

This section only applied to libinput version 1.9.x, 1.10.x, and 1.11.x and
has been removed. See :ref:`trackpoint_multiplier` for versions 1.12.x and later.

If using libinput version 1.11.x or earlier, please see
`the 1.11.0 documentation <https://wayland.freedesktop.org/libinput/doc/1.11.0/trackpoints.html#trackpoint_range_measure>`_

