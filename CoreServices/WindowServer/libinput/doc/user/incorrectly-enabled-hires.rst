.. _incorrectly_enabled_hires:

==============================================================================
Incorrectly enabled high-resolution scroll
==============================================================================

Some devices might announce support for high-resolution scroll wheel by enabling
``REL_WHEEL_HI_RES`` and/or ``REL_HWHEEL_HI_RES`` but never send a
high-resolution scroll event.

When the first low-resolution scroll event is received without any previous
high-resolution event, libinput prints a bug warning with the text **"device
supports high-resolution scroll but only low-resolution events have been
received"** and a link to this page.

.. note:: This warning will be printed only once

In most cases this is a bug on the device firmware, the kernel driver or in a
software used to create user-space devices through uinput.

Once the bug is detected, libinput will start emulating high-resolution scroll
events.

------------------------------------------------------------------------------
Detecting and fixing the issue
------------------------------------------------------------------------------

Events sent by a buggy device can be shown in the
:ref:`libinput record <libinput-record>` output for the device. Notice that
``REL_WHEEL_HI_RES`` and ``REL_HWHEEL_HI_RES`` are set but only ``REL_WHEEL``
events are sent: ::

    # Supported Events:
    # Event type 0 (EV_SYN)
    # Event type 1 (EV_KEY)
    #   Event code 272 (BTN_LEFT)
    # Event type 2 (EV_REL)
    #   Event code 0 (REL_X)
    #   Event code 1 (REL_Y)
    #   Event code 6 (REL_HWHEEL)
    #   Event code 8 (REL_WHEEL)
    #   Event code 11 (REL_WHEEL_HI_RES)
    #   Event code 12 (REL_HWHEEL_HI_RES)
    [...]
    quirks:
    events:
    - evdev:
        - [  0,      0,   2,   8,       1] # EV_REL / REL_WHEEL                 1
        - [  0,      0,   0,   0,       0] # ------------ SYN_REPORT (0) ---------- +0ms
    - evdev:
        - [  0,  15126,   2,   8,       1] # EV_REL / REL_WHEEL                 1
        - [  0,  15126,   0,   0,       0] # ------------ SYN_REPORT (0) ---------- +15ms
    - evdev:
        - [  0,  30250,   2,   8,       1] # EV_REL / REL_WHEEL                 1
        - [  0,  30250,   0,   0,       0] # ------------ SYN_REPORT (0) ---------- +15ms

The issue can be fixed by adding a quirk to unset the ``REL_WHEEL_HI_RES`` and
``REL_HWHEEL_HI_RES`` event codes: ::

    AttrEventCode=-REL_WHEEL_HI_RES;-REL_HWHEEL_HI_RES;

Please see :ref:`device-quirks` for details.
