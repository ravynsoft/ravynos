Interoperation with X11
=======================

*This document is non-normative.*

The former
`X11 Startup notification protocol <https://cgit.freedesktop.org/startup-notification/tree/doc/startup-notification.txt>`_
defines the use of the ``DESKTOP_STARTUP_ID`` environment variable to propagate
startup sequences ("activation tokens" in this protocol) between launcher and
launchee.

These startup sequence IDs are defined as a globally unique string with a
``[unique]_TIME[timestamp]`` format, where the ID as a whole is used for startup
notification and the timestamp is used for focus requests and focus stealing
prevention.

In order to observe mixed usage scenarios where Wayland and X11 clients might
be launching each other, it is possible for a compositor to manage a shared
pool of activation tokens.

Scenario 1. Wayland client spawns X11 client
--------------------------------------------

1. Wayland client requests token.
2. Wayland client spawns X11 client, sets ``$DESKTOP_STARTUP_ID`` in its
   environment with the token string.
3. X11 client starts.
4. X11 client sends startup-notification ``remove`` message with the activation
   ``$DESKTOP_STARTUP_ID`` content.
5. Compositor receives startup notification message, matches ID with
   the common pool.
6. The startup feedback is finished.
7. X11 client requests focus.
8. Compositor applies internal policies to allow/deny focus switch.

Scenario 2. X11 client spawns Wayland client
--------------------------------------------

1. X11 client builds a "globally unique" ID
2. X11 client sends startup-notification ``new`` message with the ID.
3. Compositor receives startup notification message, adds the ID to
   the common pool.
4. X11 client spawns Wayland client, sets ``$DESKTOP_STARTUP_ID`` in its
   environment.
5. Wayland client starts.
6. Wayland client requests surface activation with the activation token,
   as received from ``$DESKTOP_STARTUP_ID``.
7. Compositor receives the request, matches ID with the common pool
8. The startup feedback is finished.
9. Compositor applies internal policies to allow/deny focus switch.

Caveats
-------

- For legacy reasons, the usage of ``$DESKTOP_STARTUP_ID`` (even if as a
  fallback) should be observed in compositors and clients that are
  concerned with X11 interoperation.

- Depending on the X11 startup-notification implementation in use by the
  compositor, the usage of the ``_TIME[timestamp]`` suffix may be mandatory
  for its correct behavior in the first scenario, the startup-notification
  reference library is one such implementation. Compositors may work
  this around by adding a matching suffix to the generated activation tokens.
