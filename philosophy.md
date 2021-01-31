# Key Technologies

- Base system, kernel, drivers: FreeBSD 12
- Graphics system: X11, Qt
- Desktop environment: probably based on KDE Plasma

- Apple Open Source (https://opensource.apple.com/source/)
- GNUstep (http://www.gnustep.org)
- Darling (https://darlinghq.org)
- Apportable Foundation (https://github.com/apportable/Foundation)
- The Cocotron (https://www.cocotron.org)


# General approach

## Software Compatibility

The general idea is to create an emulation of the Mach-specific bits and a shim layer that adapts Darwin libraries/frameworks to sit on top of regular BSD libraries and services. Additional frameworks which are not available as open source can be written from API specs in the same way. This should let source be built against the Darwin/macOS frameworks as a regular BSD executable (like [winelib](https://wiki.winehq.org/Winelib_User%27s_Guide#What_is_Winelib.3F)).

In addition, the standard macOS file system layout will be present, and expected services like launchd will be implemented. Eventually, a dyld and Mach-O loader could be written (or adapted from Darling) to handle x86-64 Mac executables as well, although this may be less important since new macOS software will target the M1 chip.

## Look and Feel

Many projects emulate components like the Dock or Finder. Icon sets, high-quality fonts, and other elements are easy to get. Many window decoration themes exist for KDE, GNOME, and other desktop environments. The biggest challenge will be finding the best of breed and bringing it all together coherently. Support for the menu bar needs to be added.

Most desktop environments now have the same basic functionality, including features like "night mode" color shifting, system-wide index and search, notification area, widgets for battery life, sound volume, etc.

A large part of the "feel" will come from supporting AppDirs and AppImages, DMG-style archives for installing software, a consistent set of key bindings, and familiar metaphors like dragging to trash, multi-finger gestures for various functions, and so on.