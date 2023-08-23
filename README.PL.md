# Co to ravynOS? [![Build Status](https://api.cirrus-ci.com/github/ravynsoft/ravynos.svg?branch=main)](https://cirrus-ci.com/github/ravynsoft/ravynos) [![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)
### Don't speak Polish? Read this in: [English](README.md), [Italiano](README.IT.md), [Türkçe](README.TR.md), [Deutsch](README.DE.md), [Indonesia](README.ID.md), [简体中文](README.zh_CN.md), [繁體中文](README.zh_TW.md), [Português do Brasil](README.pt_BR.md)

ravynOS jest nowym i otwartoźródłowym systemem operacyjnym, którego celem jest zapewnienie podobnego doświadczenia i trochę kompatybilności z systemem macOS na platformie x86-64 (oraz ewentualnie ARM). Opiera się na solidnych podstawach FreeBSD, istniejącym oprogramowaniu open source w tym samym miejscu a także nowym kodzie, który ma zapełnić pustkę.

Głównymi celami ravynOS są:
- Źródłowa kompatybilność z aplikacjami macOS (np. możesz skompilować aplikację macOS na ravynOS i ją uruchomić)
- Podobne metafory GUI i rodzime UX (menedżer plików, launcher aplikacji, górny pasek menu, itp)
- Kompatybilność z strukturą folderów macOS (/Library, /System, /Users, /Volumes, etc) i być może systemami plików (HFS+, APFS) a także pełne wsparcie ZFS
- Samodzielne aplikacje w plikach [App Bundles](https://developer.apple.com/documentation/foundation/bundle), [AppDirs](https://github.com/AppImage/AppImageKit/wiki/AppDir), oraz [AppImage](https://github.com/AppImage) - bezinstalacyjne doświadczenia dla /Applications
- Przeważnie utrzymana zgodność z podstawowym systemem FreeBSD i X11 - standardowym środowiskiem Unix pod maską
- Kompatybilność z aplikacjami Linuxa przez obsługę Linuxa z FreeBSD
- Ewentualna kompatybilność z aplikacjami i bibliotekami macOS pod platformy x86-64/arm64
- Przyjemny w użyciu, bezpieczny, stabilny i wydajny

Odwiedź [ravynos.com](https://ravynos.com/) po więcej informacji: [Notes wydań](https://ravynos.com/releases.html) | [Zrzuty ekranu](https://ravynos.com/screenshots.html) | [FAQ](https://ravynos.com/faq.html)

### Dołącz do nas!

* Pomożesz zbudować marzenie? Zobacz jak pomóc w [CONTRIBUTING.md](CONTRIBUTING.md)!
* Nasz serwer [Discord](https://discord.com/invite/8caJbAGNwY).
* `#ravynOS-general:matrix.org` - dołącz za pomocą [Element.io](https://app.element.io/#/room/%23ravynOS-general:matrix.org)
* `#airyx` na [Libera IRC](https://web.libera.chat/?channel=#airyx)

[![Packages hosted by: Cloudsmith](https://img.shields.io/badge/OSS%20hosting%20by-cloudsmith-blue?logo=cloudsmith&style=flat-square)](https://cloudsmith.com)

---

FreeBSD Source:
---------------
This is the top level of the FreeBSD source directory.

FreeBSD is an operating system used to power modern servers, desktops, and embedded platforms.
A large community has continually developed it for more than thirty years.
Its advanced networking, security, and storage features have made FreeBSD the platform of choice for many of the busiest web sites and most pervasive embedded networking and storage devices.

For copyright information, please see [the file COPYRIGHT](COPYRIGHT) in this directory.
Additional copyright information also exists for some sources in this tree - please see the specific source directories for more information.

The Makefile in this directory supports a number of targets for building components (or all) of the FreeBSD source tree.
See build(7), config(8), [FreeBSD handbook on building userland](https://docs.freebsd.org/en/books/handbook/cutting-edge/#makeworld), and [Handbook for kernels](https://docs.freebsd.org/en/books/handbook/kernelconfig/) for more information, including setting make(1) variables.

For information on the CPU architectures and platforms supported by FreeBSD, see the [FreeBSD
website's Platforms page](https://www.freebsd.org/platforms/).

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
| sys | Kernel sources (see [sys/README.md](sys/README.md)). |
| targets | Support for experimental `DIRDEPS_BUILD` |
| tests | Regression tests which can be run by Kyua.  See [tests/README](tests/README) for additional information. |
| tools | Utilities for regression testing and miscellaneous tasks. |
| usr.bin | User commands. |
| usr.sbin | System administration commands. |

For information on synchronizing your source tree with one or more of the FreeBSD Project's development branches, please see [FreeBSD Handbook](https://docs.freebsd.org/en/books/handbook/cutting-edge/#current-stable).
