.. _trackpoints:

==============================================================================
Trackpoints and Pointing Sticks
==============================================================================

This page provides an overview of trackpoint handling in libinput, also
referred to as Pointing Stick or Trackstick. The device itself is usually a
round plastic stick between the G, H and B keys with a set of buttons below
the space bar.

.. figure:: button-scrolling.svg
    :align: center

    A trackpoint

libinput always treats the buttons below the space bar as the buttons that
belong to the trackpoint even on the few laptops where the buttons are not
physically wired to the trackpoint device anyway, see :ref:`t440_support`.

.. _trackpoint_buttonscroll:

------------------------------------------------------------------------------
Button scrolling on trackpoints
------------------------------------------------------------------------------

Trackpoint devices have :ref:`button_scrolling` enabled by default. This may
interfer with middle-button dragging, if middle-button dragging is required
by a user then button scrolling must be disabled.

.. _trackpoint_range:

------------------------------------------------------------------------------
Motion range on trackpoints
------------------------------------------------------------------------------

It is difficult to associate motion on a trackpoint with a physical
reference. Unlike mice or touchpads where the motion can be
measured in mm, the trackpoint only responds to pressure. Without special
equipment it is impossible to measure identical pressure values across
multiple laptops.

The values provided by a trackpoint are motion deltas, usually corresponding
to the pressure applied to the trackstick. For example, pressure towards the
screen on a laptop provides negative y deltas. The reporting rate increases
as the pressure increases and once events are reported at the maximum rate,
the delta values increase. The figure below shows a rough illustration of
this concept. As the pressure
decreases, the delta decrease first, then the reporting rate until the
trackpoint is in a neutral state and no events are reported. Trackpoint data
is hard to generalize, see
`Observations on trackpoint input data
<https://who-t.blogspot.com/2018/06/observations-on-trackpoint-input-data.html>`_
for more details.

.. figure:: trackpoint-delta-illustration.svg
    :align: center

    Illustration of the relationship between reporting rate and delta values on a trackpoint

The delta range itself can vary greatly between laptops, some devices send a
maximum delta value of 30, others can go beyond 100. However, the useful
delta range is a fraction of the maximum range. It is uncomfortable to exert
sufficient pressure to even get close to the maximum ranges.

libinput provides a :ref:`Magic Trackpoint Multiplier
<trackpoint_multiplier>` to normalize the trackpoint input data.

