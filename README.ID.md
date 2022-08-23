# Apa itu ravynOS? [![Status Pembuatan](https://api.cirrus-ci.com/github/ravynsoft/ravynos.svg?branch=main)](https://cirrus-ci.com/github/ravynsoft/ravynos) [ ![Perjanjian Kontributor](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)
### Don't speak Indonesian? Read this in: [English](README.md), [Italiano](README.IT.md), [Türkçe](README.TR.md)

ravynOS adalah proyek OS open source baru yang bertujuan untuk memberikan pengalaman serupa dan beberapa kompatibilitas dengan macOS pada sistem x86-64 (dan akhirnya ARM). Itu dibangun di atas dasar FreeBSD yang kokoh, paket open source yang ada di ruang yang sama, dan kode baru untuk mengisi kekosongan.

Tujuan utama desain adalah:
- Kompatibilitas sumber dengan aplikasi macOS (yaitu Anda dapat mengkompilasi aplikasi Mac di ravynOS dan menjalankannya)
- Metafora GUI serupa dan UX yang sudah dikenal (manajer file, peluncur aplikasi, bilah menu atas yang mencerminkan aplikasi terbuka, dll)
- Kompatibel dengan tata letak folder macOS (/ Perpustakaan, / Sistem, / Pengguna, / Volume, dll) dan mungkin sistem file (HFS +, APFS) serta sepenuhnya mendukung ZFS
- Aplikasi mandiri di [App Bundle](https://developer.apple.com/documentation/foundation/bundle), [AppDirs](https://github.com/AppImage/AppImageKit/wiki/AppDir), dan file [AppImage](https://github.com/AppImage) - pengalaman tanpa penginstal untuk /Applications
- Sebagian besar mempertahankan kompatibilitas dengan sistem dasar FreeBSD dan X11 - lingkungan Unix standar di bawah tenda
- Kompatibel dengan binari Linux melalui dukungan Linux FreeBSD
- Kompatibilitas akhirnya dengan binari macOS x86-64/arm64 (Mach-O) dan library
- Menyenangkan untuk digunakan, aman, stabil, dan berkinerja

Kunjungi [ravynos.com](https://ravynos.com/) untuk info selengkapnya: [Catatan Rilis](https://ravynos.com/releases.html) | [Screenshot](https://ravynos.com/screenshots.html) | [FAQ](https://ravynos.com/faq.html)

### Bergabunglah dengan kami!

* Dapatkah Anda membantu membangun mimpi? Lihat proyek/kebutuhan saat ini di [CONTRIBUTING.md](CONTRIBUTING.md)!
* Server [Discord](https://discord.com/invite/8caJbAGNwY) kami.
* `#ravynOS-general:matrix.org` - bergabung melalui [Element.io](https://app.element.io/#/room/%23ravynOS-general:matrix.org)
* `#airyx` di [Libera IRC](https://web.libera.chat/?channel=#airyx)

[![Paket dihosting oleh: Cloudsmith](https://img.shields.io/badge/OSS%20hosting%20by-cloudsmith-blue?logo=cloudsmith&style=flat-square)](https://cloudsmith.com)

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
