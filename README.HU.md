# Mi az a ravynOS? [![Build Status](https://api.cirrus-ci.com/github/ravynsoft/ravynos.svg?branch=main)](https://cirrus-ci.com/github/ravynsoft/ravynos) [![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)
### Don't speak Hungarian? Read this in: [Italiano](README.IT.md), [Türkçe](README_TR.md), [Deutsch](README.DE.md), [Indonesia](README.ID.md)

A ravynOS egy új nyilt forrásu operációs rendszer projekt ami arra célzik hogy hasonló élményt és valamenyi kompatibilitást adjon a macOS-el x86-64 (és egyszer majd ARM) rendszereken is. A megbízható FreeBSD alapján épűl, létező nyilt forrású csomagokkal ugyanazon a helyen és új kódot hogy a hibákat befedjük.

Az alap tervezési célpontjaink:
- Fórráskód kompatibilitás a macOS programokkal (példáúl létre tudnál hozni egy Mac programot a ravynOS-en és futtatni lehetne)
- Hasonló felület (fáljkezelő, programinditó, menü a képernyő tetején ami mutatja a kinyitott programot, stb)
- Kompatibilis a macOS mappa felosztásokkal (/Library, /System, /Users, /Volumes, stb) és esetleg fáljrendszerekkel (HFS+, APFS) úgy ahogy még a ZFS-t is támogatja
- Öntárolt alkalmazások [App Bundles](https://developer.apple.com/documentation/foundation/bundle), [AppDirs](https://github.com/AppImage/AppImageKit/wiki/AppDir), és [AppImage](https://github.com/AppImage) fáljok - telepitő-nélküli élmény az /Applications-ben
- Nagyjából megtartja a kompatibilitást a FreeBSD rendszerrel és az X11-el - a standard Unix környezettel amivel működik
- Kompatibilis a Linux alkalmazásokkal a FreeBSD Linuxulator segitségével
- Talán kompatibilitás x86-64/arm64 macOS alkalmazásokkal (Mach-O) és könyvtárakkal 
- Jó élmény használni, biztonságos, stabil, és performáns

Látogasd meg a [ravynos.com](https://ravynos.com/)-ot több informaciókrt: [Release Notes](https://ravynos.com/releases.html) | [Screenshots](https://ravynos.com/screenshots.html) | [FAQ](https://ravynos.com/faq.html)

### Csatlakozz!

* Tudsz segíteni hogy megépitjük az álmot? Nézd meg a jelenlegi projektek/szükségeket a[CONTRIBUTING.md](CONTRIBUTING.md)-ben!
* A [Discord](https://discord.com/invite/8caJbAGNwY) serverünkhez.
* `#ravynOS-general:matrix.org` - csatlakozz az Element segitségével: [Element.io](https://app.element.io/#/room/%23ravynOS-general:matrix.org)
* `#airyx` a [Libera IRC](https://web.libera.chat/?channel=#airyx)-n

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

For information on synchronizing your source tree with one or more of the FreeBSD Project's development branches, please see [FreeBSD Handbook](https://docs.freebsd.org/en/books/handbook/cutting-edge/#current-stable).
