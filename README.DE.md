# Was ist ravynOS? [![Build Status](https://api.cirrus-ci.com/github/ravynsoft/ravynos.svg?branch=main)](https://cirrus-ci.com/github/ravynsoft/ravynos) [![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)
### Don't you speak German? Read this in: [English](README.md), [Italiano](README.IT.md), [Türkçe](README_TR.md)

ravynOS ist ein neues Open-Source-Betriebssystemprojekt, das darauf abzielt, eine ähnliche Erfahrung und eine gewisse Kompatibilität mit macOS auf x86-64-Systemen (und eventuell ARM) zu bieten. Es baut auf den soliden Grundlagen von FreeBSD, bestehenden Open-Source-Paketen im gleichen Bereich und neuem Code auf, um die Lücken zu schließen.

Die wichtigsten Designziele sind:
- Quellkompatibilität mit macOS-Anwendungen (d.h. man kann eine Mac-Anwendung unter ravynOS kompilieren und ausführen)
- Ähnliche GUI-Metaphern und vertraute UX (Dateimanager, Programmstarter, obere Menüleiste, die die geöffnete Anwendung widerspiegelt, usw.)
- Kompatibel mit macOS-Ordnerlayouts (/Library, /System, /Users, /Volumes, etc.) und möglicherweise Dateisystemen (HFS+, APFS) sowie mit vollständiger Unterstützung von ZFS
- Eigenständige Anwendungen in [App Bundles](https://developer.apple.com/documentation/foundation/bundle), [AppDirs](https://github.com/AppImage/AppImageKit/wiki/AppDir), und [AppImage](https://github.com/AppImage) Dateien - eine Erfahrung ohne Installationsprogramm für /Anwendungen
- Weitgehende Kompatibilität mit dem FreeBSD-Basissystem und X11 - eine Standard-Unix-Umgebung unter der Haube
- Kompatibilität mit Linux-Binärdateien durch FreeBSDs Linux-Unterstützung
- Eventuelle Kompatibilität mit x86-64/arm64 macOS-Binärdateien (Mach-O) und -Bibliotheken
- Angenehm zu benutzen, sicher, stabil und performant

Bitte besuch [ravynos.com](https://ravynos.com/) für mehr Informationen: [Release Notes](https://ravynos.com/releases.html) | [Screenshots](https://ravynos.com/screenshots.html) | [FAQ](https://ravynos.com/faq.html)

### Tritt uns bei!

* Können Sie helfen, den Traum zu verwirklichen? Siehe die aktuellen Projekte/Bedarfe in [CONTRIBUTING.md](CONTRIBUTING.md)!
* Unser [Discord](https://discord.com/invite/8caJbAGNwY) Server.
* `#ravynOS-general:matrix.org` - tritt bei via [Element.io](https://app.element.io/#/room/%23ravynOS-general:matrix.org)
* `#airyx` auf [Libera IRC](https://web.libera.chat/?channel=#airyx)

[![Packages hosted by: Cloudsmith](https://img.shields.io/badge/OSS%20hosting%20by-cloudsmith-blue?logo=cloudsmith&style=flat-square)](https://cloudsmith.com)

---

FreeBSD Source:
---------------
Dies ist die oberste Ebene des FreeBSD-Quellverzeichnisses.

FreeBSD ist ein Betriebssystem, das auf modernen Servern, Desktops und eingebetteten Plattformen läuft.
Eine große Gemeinschaft entwickelt es seit mehr als dreißig Jahren kontinuierlich weiter.
Seine fortschrittlichen Netzwerk-, Sicherheits- und Speicherfunktionen haben FreeBSD zur bevorzugten Plattform für viele der meistgenutzten Websites und der am weitesten verbreiteten eingebetteten Netzwerk- und Speichergeräte gemacht.

Für Urheberrecht Informationen, bitte schau [die COPYRIGHT Datei](COPYRIGHT) in diesem Ordner an.
Für einige Quellen in diesem Baum gibt es auch zusätzliche Copyright-Informationen - weitere Informationen finden Sie in den jeweiligen Quellenverzeichnissen.

Das Makefile in diesem Verzeichnis unterstützt eine Reihe von Zielen für die Erstellung von Komponenten (oder des gesamten FreeBSD-Quellbaums).
Sieh build(7), config(8), [FreeBSD handbook on building userland](https://docs.freebsd.org/en/books/handbook/cutting-edge/#makeworld), und [Handbook for kernels](https://docs.freebsd.org/en/books/handbook/kernelconfig/) für mehr Informationen, inklusive Einstellungen make(1) Variablen.

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
