# Cos'è ravynOS? [![Build Status](https://api.cirrus-ci.com/github/mszoek/airyx.svg?branch=main&task=airyx)](https://cirrus-ci.com/github/mszoek/airyx) [![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)
### Don't you speak italian? Read this in: [English](README.md)
ravynOS è un nuovo sistema operativo open source che cerca di fornire un'esperienza simile a quella di macOS cercando di fornire anche la possibilità di eseguire le sue applicazioni per i sistemi basati su x86-64. È basato sulla fondazione solida di FreeBSD, con nuovo codice e con pacchetti open source già esistenti.

Gli obiettivi principali sono:

- Compatibiltà a livello source con le applicazioni macOS (e.s. puoi compilare un'applicazione per macOS su ravynOS ed eseguirla)
- Mantenere una filosofia dell'inferfaccia utente simile (gestore file, launcher delle applicazioni, barra superiore che cambia rispetto all'applicazione che si sta usando, etc)
- Compatibilità con la struttura delle cartelle di macOS (/Library, /System, /Users, /Volumes, etc) e anche dei suoi filesystem (HFS+, APFS) mantenendo anche la compatibilità con ZFS
- Applicazioni auto contenute in [App Bundles](https://developer.apple.com/documentation/foundation/bundle), [AppDirs](https://github.com/AppImage/AppImageKit/wiki/AppDir), e [AppImage](https://github.com/AppImage) con un'esperienza senza installer per /Applications
- Mantenere per il più possibile la commpatibilità con FreeBSD e X11 - lasciando come base un ambiente standard Unix 
- Compatibilità con i file binari di Linux tramite il supporto Linux di FreeBSD
- Eventuale compatibilità con i file binari x86-64 di macOS (Mach-O) e delle sue librerie
- Piacevole da usare, sicuro, stabile e performante

Visita [ravynos.com](https://ravynos.com/) per più informazioni (in inglese): [Note di rilascio](https://ravynos.com/releases.html) | [Screenshots](https://ravynos.com/screenshots.html) | [FAQ](https://ravynos.com/faq.html)
### Vieni a far parte del progetto!

* Il nostro server [Discord](https://discord.com/invite/8caJbAGNwY)
* `#airyx:matrix.org` - entra tramite [Element.io](https://app.element.io/#/room/#airyx:matrix.org)
* `#airyx` su [Libera IRC](https://web.libera.chat/?channel=#airyx)

_(note: `#airyx` su matrix.org e IRC sono collegate col canale `#general` su Discord, solo in lingua inglese)_

Logo e design sono stati creati da [nayaabkhan](https://nayaabkhan.me). Si ringrazia [llui85](https://github.com/llui85) per l'albero di palme.

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
