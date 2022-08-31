# 什麼是 ravynOS? [![目前狀態](https://api.cirrus-ci.com/github/ravynsoft/ravynos.svg?branch=main)](https://cirrus-ci.com/github/ravynsoft/ravynos) [![貢獻者](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)
### Don't speak Chinese(Taiwan)? Read this in: [Italiano](README.IT.md), [Türkçe](README_TR.md), [Deutsch](README.DE.md), [Indonesia](README.ID.md), [Hungarian](README.HU.md)

ravynOS 可說是 macOS＋BSD 的精神。自由又好用，而且開源。

我們的目標：
- 可使用 MacOS 上的應用程式
- 與 MacOS 高度相似
- 檔案管理層與 MacOS 相同（如 /Library、/System、/Users、/Volumes，等等）甚至檔案系統（HFS+，APFS）以及支援 ZFS
- 可使用 [App Bundles](https://developer.apple.com/documentation/foundation/bundle), [AppDirs](https://github.com/AppImage/AppImageKit/wiki/AppDir)，[AppImage](https://github.com/AppImage) 檔案，換句話說就是更簡易的安裝應用程式
- 是基於 FreeBSD 的作業系統和 X11
- 支援 Linux 和 macOS 資料庫
- 簡單易用

請前往 [ravynos.com](https://ravynos.com/) 取得更多資訊：[更新日誌](https://ravynos.com/releases.html) | [截圖](https://ravynos.com/screenshots.html) | [常見問題](https://ravynos.com/faq.html)

### 加入我們！

* 想幫我們實現夢想嗎？請閱讀 [CONTRIBUTING.md](CONTRIBUTING.md)!
* 我們的 [Discord](https://discord.com/invite/8caJbAGNwY) 伺服器。
* `#ravynOS-general:matrix.org` - 用[Element.io](https://app.element.io/#/room/%23ravynOS-general:matrix.org) 加入我們的 Matrix 
* `#airyx` 在 [Libera IRC](https://web.libera.chat/?channel=#airyx)

[![套件主持者：Cloudsmith](https://img.shields.io/badge/OSS%20hosting%20by-cloudsmith-blue?logo=cloudsmith&style=flat-square)](https://cloudsmith.com)

---

FreeBSD 資源：
---------------
這是最高層的 FreeBSD 資料夾

FreeBSD 是一個為個人電腦以及伺服器所設計的作業系統
這個古老的作業系統有著超過三十年的歷史，以及陪伴我們三十年的夥伴。

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
