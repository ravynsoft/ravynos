# ravynOS Nedir? [![Build Status](https://api.cirrus-ci.com/github/ravynsoft/ravynos.svg?branch=main)](https://cirrus-ci.com/github/ravynsoft/ravynos) [![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)
### Don't speak you Turkish? Read this in: [Italiano](README.IT.md), [English](README.md)

ravynOS macOS ile x86-64(ve ARM) mimarisinde uyumluluk sağlamayı ve benzer bir deneyim sunmayı amaçlayan açık kaynaklı bir işletim sistemidir. FreeBSD'nin sağlam temelleri üzerine inşa edilmiş olup hali hazırda bulunan açık kaynaklı paketleri ve boşlukları doldurmak için kendi kodunu kullanır.

Projenin temel amaçları:
- macOS programları ile kaynak kodu uyumluluğu (Örn. bir Mac programını rayvnOS üzerinde derleyip onu sorunsuz bir şekilde çalıştırabilirsiniz)
- Benzer GUI metaforları ve benzer kullanıcı deneyimi (dosya yöneticisi, uygulama başlatıcısı, açık olan uygulama ile çalışan menü çubuğu)
- macOS dosya düzeni ile uyumluluk (/Library, /System, /Users, /Volumes, vb) ve muhtemel dosya sistemleri (HFS+, APFS) ilave olarakta ZFS dosya sistemi ile uyumluluk
- [App Bundles](https://developer.apple.com/documentation/foundation/bundle), [AppDirs](https://github.com/AppImage/AppImageKit/wiki/AppDir) ve [AppImage](https://github.com/AppImage) formatlarında bağımsız ve /Applications için kuruluma ihtiyaç duymayan bir deneyim 
- FreeBSD ve X11 ile uyumluluğu korumak - kaputun altında standart bir Unix ortamı
- FreeBSD'nin Linux desteği sayesinde Linux programları ile uyumludur
- x86-64/arm64 macOS programları (Mach-O) ve kütüphaneleri ile nihai uyumluluk
- Kullanımı keyifli, güvenli, kararlı ve performanslı

Daha fazla bilgi için lütfen [ravynos.com](https://ravynos.com/) adresini ziyaret edin ziyaret edin:
[Release Notes](https://ravynos.com/releases.html) | [Screenshots](https://ravynos.com/screenshots.html) | [FAQ](https://ravynos.com/faq.html)

### Bize katılın!

* Hayalimdekini gerçekleştirmede bize yardım edebilirmisin? Projenin şu anki ihtiyaç duyduğu şeyleri [CONTRIBUTING.md](CONTRIBUTING.md) üzerinde bulabilirsin!
* [Discord](https://discord.com/invite/8caJbAGNwY) sunucumuz.
* `#airyx:matrix.org` - [Element.io](https://app.element.io/#/room/#airyx:matrix.org) ile katılın
* [Libera IRC](https://web.libera.chat/?channel=#airyx)'de `#airyx`

_(not: matrix.org üzerindeki `#airyx` ve IRC kanalları Discord sunucumuzdaki `#general` kanalı ile bağlantılıdır.)_

Yeni logo tasarımı & sanat eseri [nayaabkhan](https://nayaabkhan.me) tarafından yapılmıştır. 
Palmiye ağacı konsepti için [llui85](https://github.com/llui85) saygılar. 


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
