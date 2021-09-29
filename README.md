# What is Airyx? ![](https://api.cirrus-ci.com/github/mszoek/airyx.svg?branch=main&task=airyx) [![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)

Airyx is a new open source OS project that aims to provide a similar experience and some compatibility with macOS on x86-64 sytems. It builds on the solid foundations of FreeBSD, existing open source packages in the same space, and new code to fill the gaps.

The main design goals are:
- source compatibility with macOS applications (i.e. you could compile a Mac application on Airyx and run it)
- similar GUI metaphors and familiar UX (file manager, application launcher, top menu bar that reflects the open application, etc)
- compatible with macOS filesystems (HFS+ and APFS) and folder layouts (/Library, /System, /Users, /Volumes, etc)
- self-contained applications in [folders](https://github.com/AppImage/AppImageKit/wiki/AppDir) or a [single file](https://github.com/AppImage) and a (mostly) installer-less experience for /Applications
- mostly maintain compatibility with the FreeBSD base system and X11 - a standard Unix environment under the hood
- compatible with Linux binaries via FreeBSD's Linux support
- eventual compatibility with x86-64 macOS binaries (Mach-O) and libraries
- pleasant to use, secure, stable, and performant

Please visit [airyx.org](https://airyx.org/) for more info: [Release Notes](https://airyx.org/releases.html) | [Screenshots](https://airyx.org/screenshots.html) | [FAQ](https://airyx.org/faq.html)

### Join us!

* Our [Discord](https://discord.com/invite/8caJbAGNwY) server.
* `#airyx:matrix.org` - join via [Element.io](https://app.element.io/#/room/#airyx:matrix.org)
* `#airyx` on [Libera IRC](https://web.libera.chat/?channel=#airyx)

_(note: `#airyx` on matrix.org and IRC are bridged with `#general` on Discord)_

## Acknowledgements
This project would not be possible without the generous support of [Cirrus CI](https://cirrus-ci.org/) for CI/CD and [Cloudsmith](https://cloudsmith.io/) for package hosting! Thanks to these awesome organizations for supporting open source software!

New logo designs & artwork by [nayaabkhan](https://nayaabkhan.me). Hat tip for the palm tree concept to [llui85](https://github.com/llui85).

[![Packages hosted by: Cloudsmith](https://img.shields.io/badge/OSS%20hosting%20by-cloudsmith-blue?logo=cloudsmith&style=flat-square)](https://cloudsmith.com)
