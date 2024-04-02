# ‏ravynOS چیست؟ [![Build Status](https://api.cirrus-ci.com/github/ravynsoft/ravynos.svg?branch=main)](https://cirrus-ci.com/github/ravynsoft/ravynos) [![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)
### Don't speak Persian? Read this in: [English](README.md)

‏ravynOS یک پروژه‌ٔ سیستم عامل منبع باز جدید است که تلاش می‌کند تا تجربه و برخی سازگاری‌ها را با macOS در دستگاه‌های x86-64 (و سرانجام ARM) داشته باشد. این سیستم عامل، بر مبنای پایه‌های استوار FreeBSD ساخته و در همان فضا، برنامه‌های فعلی آزاد را داشته و کدهای جدیدی را وارد می‌کند تا فضای خالی موجود را پر کند.

اهداف اصلی عبارت‌اند از:
- سازگاری منبع با برنامه‌های macOS (این به این معناست که شما می‌توانید برنامه‌های مک را در ravynOS کامپایل و اجرا کنید)
- حالات رابط گرافیکی مشابه و تجربه‌ی کاربری آشنا (مدیر فایل، اجرا کننده‌ی برنامه، نوار بالایی که برنامه‌ی باز را نشان می‌دهد و غیره)
- سازگار با چیدمان پوشه‌های macOS (/Library، /System، /Users، /Volumes و غیره) و چه بسا filesystemها (HFS+، APFS) همچنین پشتیبانی کامل از ZFS
- برنامه‌های self-contained در [App Bundles](https://developer.apple.com/documentation/foundation/bundle)، [AppDirs](https://github.com/AppImage/AppImageKit/wiki/AppDir) و فایل‌های [AppImage](https://github.com/AppImage) - یک تجربه‌ی بدون نصب برای /Applications
- عمدتاً نگه‌داری سازگاری با پایه‌ی FreeBSD و X11 - یک استاندارد محیط یونیکس
- سازگار با باینری‌های لینوکس از طریق پشتیبانی FreeBSD از آن‌ها
- سازگاری احتمالی با باینری‌های x86-64/arm64 سیستم عامل macOS (Mach-O) و کتابخانه‌ها
- عالی برای استفاده، ایمن، پایدار و قدرتمند

لطفاً برای اطلاعات بیشتر به [ravynos.com](https://ravynos.com/) مراجعه کنید. [اطلاعات نسخه‌ها](https://ravynos.com/releases.html) | [عکس از صفحه‌ها](https://ravynos.com/screenshots.html) | [سؤالات پرتکرار](https://ravynos.com/faq.html)

### به ما بپیوندید!

* آیا می‌توانید به ما در ساخت این رؤیا کمک کنید؟ پروژه‌ها و نیازهای فعلی را در [CONTRIBUTING.md](CONTRIBUTING.md) مشاهده کنید.
* سرور [دیسکورد](https://discord.com/invite/8caJbAGNwY) ما.
* #ravynOS-general:matrix.org - با [Element.io](https://app.element.io/#/room/%23ravynOS-general:matrix.org) به ما بپیوندید.

[![بسته‌ها توسط Cloudsmith میزبانی می‌شوند](https://img.shields.io/badge/OSS%20hosting%20by-cloudsmith-blue?logo=cloudsmith&style=flat-square)](https://cloudsmith.com)

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
