# libxkbcommon

libxkbcommon is a keyboard keymap compiler and support library which
processes a reduced subset of keymaps as defined by the [XKB] \(X Keyboard
Extension) specification.  It also contains a module for handling Compose
and dead keys and a separate library for listing available keyboard layouts.

[XKB]: doc/introduction-to-xkb.md

## Quick Guide

See [Introduction to XKB][XKB] to learn the essentials of XKB.

See [Quick Guide](doc/quick-guide.md) for an introduction on how to use this
library.

## Building

libxkbcommon is built with [Meson](http://mesonbuild.com/):

    meson setup build
    meson compile -C build
    meson test -C build # Run the tests.

To build for use with Wayland, you can disable X11 support while still
using the X11 keyboard configuration resource files thusly:

    meson setup build \
        -Denable-x11=false \
        -Dxkb-config-root=/usr/share/X11/xkb \
        -Dx-locale-root=/usr/share/X11/locale
    meson compile -C build

## API

While libxkbcommon's API is somewhat derived from the classic XKB API as found
in `X11/extensions/XKB.h` and friends, it has been substantially reworked to
expose fewer internal details to clients.

See the [API Documentation](https://xkbcommon.org/doc/current/modules.html).

## Dataset

libxkbcommon does not distribute a keymap dataset itself, other than for
testing purposes.  The most common dataset is xkeyboard-config, which is used
by all current distributions for their X11 XKB data.  More information on
xkeyboard-config is available here:
    https://www.freedesktop.org/wiki/Software/XKeyboardConfig

The dataset for Compose is distributed in libX11, as part of the X locale
data.

## Relation to X11

See [Compatibility](doc/compatibility.md) notes.

## Development

An extremely rudimentary homepage can be found at
    https://xkbcommon.org

xkbcommon is maintained in git at
    https://github.com/xkbcommon/libxkbcommon

Patches are always welcome, and may be sent to either
    <xorg-devel@lists.x.org> or <wayland-devel@lists.freedesktop.org>
or in a [GitHub](https://github.com/xkbcommon/libxkbcommon) pull request.

Bug reports (and usage questions) are also welcome, and may be filed at
[GitHub](https://github.com/xkbcommon/libxkbcommon/issues).

The maintainers are
- Daniel Stone <daniel@fooishbar.org>
- Ran Benita <ran@unusedvar.com>

## Credits

Many thanks are due to Dan Nicholson for his heroic work in getting xkbcommon
off the ground initially.
