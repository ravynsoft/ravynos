# What is Airyx?

Airyx is a new open source OS project that aims to provide a similar experience and some compatibiilty with macOS on x86-64 sytems. It builds on the solid foundations of FreeBSD, existing open source packages in the same space, and new code to fill the gaps.

Please visit [airyx.org](https://airyx.org/) or join us on [Libera IRC](https://libera.chat) in `#airyx` for more info!

![](https://api.cirrus-ci.com/github/mszoek/airyx.svg?branch=main&task=airyx)

The main design goals are:
- source compatibility with macOS applications (i.e. you could compile a Mac application on Airyx and run it)
- similar GUI metaphors and familiar UX (file manager, application launcher, top menu bar that reflects the open application, etc)
- compatible with macOS filesystems (HFS+ and APFS) and folder layouts (/Library, /System, /Users, /Volumes, etc)
- self-contained applications in [folders](https://github.com/AppImage/AppImageKit/wiki/AppDir) or a [single file](https://github.com/AppImage) and a (mostly) installer-less experience for /Applications
- mostly maintain compatibility with the FreeBSD base system and X11 - a standard Unix environment under the hood
- compatible with Linux binaries via FreeBSD's Linux support
- eventual compatibility with x86-64 macOS binaries (Mach-O) and libraries
- pleasant to use, secure, stable, and performant

## Acknowledgements
This project would not be possible without the generous support of [Cirrus CI](https://cirrus-ci.org/) for CI/CD and [Cloudsmith](https://cloudsmith.io/) for package hosting! Thanks to these awesome organizations for supporting open source software!

[![Packages hosted by: Cloudsmith](https://img.shields.io/badge/OSS%20hosting%20by-cloudsmith-blue?logo=cloudsmith&style=flat-square)](https://cloudsmith.com)
