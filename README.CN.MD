# 什么是 ravynOS？ [![Build Status](https://api.cirrus-ci.com/github/ravynsoft/ravynos.svg?branch=main)](https://cirrus-ci.com/github/ravynsoft/ravynos) [![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)
### Don't speak Chinese? Read this in: [English](README.md), [Italiano](README.IT.md), [Türkçe](README.TR.md), [Deutsch](README.DE.md), [Indonesia](README.ID.md)

ravynOS是一个崭新的开源操作系统项目，旨在x86-64(终将支持ARM)系统上提供类似macOS的体验与其兼容性。它建立于FreeBSD坚实的基础、相同空间中现有开源软件包以及填补空白的新代码之上。

主要设计目标为：
- 与macOS应用程序兼容的资源(即您可以在ravynOS上编译Mac应用程序并运行)
- 类似的图形用户界面与熟悉的用户体验(文件管理、应用启动、顶部菜单栏体现打开的应用等等)
- 兼容macOS文件夹布局(/Library、/System、/Users、/Volumes等等) 与可能还有完全支持ZFS的文件系统(HFS+、APFS)
- [App Bundles](https://developer.apple.com/documentation/foundation/bundle)、[AppDirs](https://github.com/AppImage/AppImageKit/wiki/AppDir) 和 [AppImage](https://github.com/AppImage) 文件中独立完备的应用程序 - 无安装程序体验的 /Applications
- 主要保持与FreeBSD基本系统和X11的兼容性 --- 一个标准的Unix环境
- 通过FreeBSD对Linux的支持与Linux二进制文件兼容
- 最终兼容x86-64/arm64 macOS二进制文件(Mach-O)和库
- 使用愉快、安全、稳定与高性能

请访问[ravynos.com](https://ravynos.com/)获取更多信息：[发行说明](https://ravynos.com/releases.html) | [界面截图](https://ravynos.com/screenshots.html) | [常见问题](https://ravynos.com/faq.html)

### 请加入我们！

* 你能帮助共建梦想吗？ 查看当前项目/需求于[CONTRIBUTING.md](CONTRIBUTING.md)!
* 我们的[Discord](https://discord.com/invite/8caJbAGNwY)社区
* `#ravynOS-general:matrix.org` - 通过[Element.io](https://app.element.io/#/room/%23ravynOS-general:matrix.org)加入
* `#airyx`于[Libera IRC](https://web.libera.chat/?channel=#airyx)

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
