# What is Airyx? [![Build Status](https://api.cirrus-ci.com/github/mszoek/airyx.svg?branch=main&task=airyx)](https://cirrus-ci.com/github/mszoek/airyx) [![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)

Airyx is a new open source OS project that aims to provide a similar experience and some compatibility with macOS on x86-64 sytems. It builds on the solid foundations of FreeBSD, existing open source packages in the same space, and new code to fill the gaps.

The main design goals are:
- Source compatibility with macOS applications (i.e. you could compile a Mac application on airyxOS and run it)
- Similar GUI metaphors and familiar UX (file manager, application launcher, top menu bar that reflects the open application, etc)
- Compatible with macOS folder layouts (/Library, /System, /Users, /Volumes, etc) and perhaps filesystems (HFS+, APFS) as well as fully supporting ZFS
- Self-contained applications in [App Bundles](https://developer.apple.com/documentation/foundation/bundle), [AppDirs](https://github.com/AppImage/AppImageKit/wiki/AppDir), and [AppImage](https://github.com/AppImage) files - an installer-less experience for /Applications
- Mostly maintain compatibility with the FreeBSD base system and X11 - a standard Unix environment under the hood
- Compatible with Linux binaries via FreeBSD's Linux support
- Eventual compatibility with x86-64 macOS binaries (Mach-O) and libraries
- Pleasant to use, secure, stable, and performant

Please visit [airyx.org](https://airyx.org/) for more info: [Release Notes](https://airyx.org/releases.html) | [Screenshots](https://airyx.org/screenshots.html) | [FAQ](https://airyx.org/faq.html)

### Join us!

* Our [Discord](https://discord.com/invite/8caJbAGNwY) server.
* `#airyx:matrix.org` - join via [Element.io](https://app.element.io/#/room/#airyx:matrix.org)
* `#airyx` on [Libera IRC](https://web.libera.chat/?channel=#airyx)

_(note: `#airyx` on matrix.org and IRC are bridged with `#general` on Discord)_

New logo designs & artwork by [nayaabkhan](https://nayaabkhan.me). Hat tip for the palm tree concept to [llui85](https://github.com/llui85).

[![Packages hosted by: Cloudsmith](https://img.shields.io/badge/OSS%20hosting%20by-cloudsmith-blue?logo=cloudsmith&style=flat-square)](https://cloudsmith.com)

---

FreeBSD Source:
---------------
This is the top level of the FreeBSD source directory.  This file
was last revised on:
$FreeBSD$

FreeBSD is an operating system used to power modern servers,
desktops, and embedded platforms. A large community has
continually developed it for more than thirty years. Its
advanced networking, security, and storage features have
made FreeBSD the platform of choice for many of the
busiest web sites and most pervasive embedded networking
and storage devices.

For copyright information, please see the file COPYRIGHT in this
directory. Additional copyright information also exists for some
sources in this tree - please see the specific source directories for
more information.

The Makefile in this directory supports a number of targets for
building components (or all) of the FreeBSD source tree.  See build(7), config(8),
https://www.freebsd.org/doc/en_US.ISO8859-1/books/handbook/makeworld.html, and
https://www.freebsd.org/doc/en_US.ISO8859-1/books/handbook/kernelconfig.html
for more information, including setting make(1) variables.

Source Roadmap:
---------------
| Directory | Description |
| --------- | ----------- |
| bin | System/user commands. |
| cddl | Various commands and libraries under the Common Development and Distribution License. |
| contrib | Packages contributed by 3rd parties. |
| crypto | Cryptography stuff (see [crypto/README](crypto/README)). |
| etc | Template files for /etc. |
| gnu | Commands and libraries under the GNU General Public License (GPL) or Lesser General Public License (LGPL). Please see [gnu/COPYING](gnu/COPYING) and [gnu/COPYING.LIB](gnu/COPYING.LIB) for more information. |
| include | System include files. |
| kerberos5 | Kerberos5 (Heimdal) package. |
| lib | System libraries. |
| libexec | System daemons. |
| release | Release building Makefile & associated tools. |
| rescue | Build system for statically linked /rescue utilities. |
| sbin | System commands. |
| secure | Cryptographic libraries and commands. |
| share | Shared resources. |
| stand | Boot loader sources. |
| sys | Kernel sources. |
| sys/`arch`/conf | Kernel configuration files. GENERIC is the configuration used in release builds. NOTES contains documentation of all possible entries. |
| tests | Regression tests which can be run by Kyua.  See [tests/README](tests/README) for additional information. |
| tools | Utilities for regression testing and miscellaneous tasks. |
| usr.bin | User commands. |
| usr.sbin | System administration commands. |

For information on synchronizing your source tree with one or more of
the FreeBSD Project's development branches, please see:

  https://www.freebsd.org/doc/en_US.ISO8859-1/books/handbook/current-stable.html
