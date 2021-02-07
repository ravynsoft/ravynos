# Wayland backend for GNUstep

This directory (along with `Headers/wayland/`, and a file in `Source/cairo/`
and `Headers/cairo`) contains the Wayland backend for GNUstep GUI. This
display server backend depends on Cairo graphics backend; it will currently
not work with any other combination.

As of April 2020, it is incomplete and broken. Help getting it functional will
be appreciated.

## Known issues

Last updated 25 April 2020:

*   Under Weston, some backing Cairo surfaces, which should not be visible in
    the compositor, are nonetheless drawn randomly onto the screen.
*   Backing view surfaces may never get blitted onto the main window.
*   After a while, Weston assumes that the application is not responding (there
    is a spinner when hovering over the windows, and the surfaces can be
    rotated by holding the right mouse button). Some events still get
    delivered, visible in the debug output, but otherwise the application
    appears frozen.

## Use on Debian

As of April 2020, it requires the stable XDG Shell protocol to be available in
the compositor you may be using. Weston included in Debian buster does _not_
include the stable XDG Shell protocol; this was only checked into Weston in
February 2019, and seemingly released with Weston 6, which does not ship in
Debian buster.

## Regenerating protocol files

To regenerate protocol sources from protocol IDLs in XML format, please use
`wayland-regenerate.sh`. Paths to the XML files are hardcoded to the values on
a Debian testing system.

