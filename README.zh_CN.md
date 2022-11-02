# ravynOS 是什么？ [![Build Status](https://api.cirrus-ci.com/github/ravynsoft/ravynos.svg?branch=main)](https://cirrus-ci.com/github/ravynsoft/ravynos) [![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)
### Don't speak Chinese (China)? Read this in: [English](README.md), [Italiano](README.IT.md), [Türkçe](README.TR.md), [Deutsch](README.DE.md), [Indonesia](README.ID.md), [繁體中文](README.zh_TW.md)

ravynOS 是一个新型的操作系统项目，致力于在 x86-64（终极目标是同时实现 ARM）平台上提供与 macOS 类似的体验和兼容性。它基于坚若磐石的 FreeBSD、现有的开源代码和锦上添花的新代码构建。

主要设计目标：
- 与 macOS 应用程序的源码级兼容（比如你可以在本系统上编译和运行 macOS 应用）
- 相似的 GUI 和 UX（文件管理器、程序启动器、顶部菜单之类）
- 与 macOS 兼容的目录层次（/Library、/System、/Users、/Volumes 之类），可能计划支持 HFS+ 和 APFS 文件系统，计划完全支持 ZFS 文件系统
- 支持 [App Bundles](https://developer.apple.com/documentation/foundation/bundle)、[AppDirs](https://github.com/AppImage/AppImageKit/wiki/AppDir) 以及 [AppImage](https://github.com/AppImage) 自封装格式的应用 —— 摆脱使用安装器的烦恼
- 保持与 FreeBSD 基本系统和 X11 主要的兼容性 —— 深层标准 Unix 环境
- 藉着 FreeBSD 的 Linux 支持实现与 Linux 应用程序的二进制兼容
- 终极目标是实现与 x86-64/arm64 macOS 应用程序（Mach-O）和库的兼容
- 易用、稳定、安全、高效

请访问 [ravynos.com](https://ravynos.com/) 以了解更多信息：[发行说明](https://ravynos.com/releases.html) | [屏幕截图](https://ravynos.com/screenshots.html) | [FAQ](https://ravynos.com/faq.html)

### 加入我们！

* 想帮我们圆梦吗？看一下 [CONTRIBUTING.md](CONTRIBUTING.md) ！
* 我们的 [Discord](https://discord.com/invite/8caJbAGNwY) 服务器。
* `#ravynOS-general:matrix.org` - 通过 [Element.io](https://app.element.io/#/room/%23ravynOS-general:matrix.org) 加入。
* `#airyx` on [Libera IRC](https://web.libera.chat/?channel=#airyx)

[![Packages hosted by: Cloudsmith](https://img.shields.io/badge/OSS%20hosting%20by-cloudsmith-blue?logo=cloudsmith&style=flat-square)](https://cloudsmith.com)

---

FreeBSD 源代码:
---------------
这是 FreeBSD 源代码的顶层文件夹。

FreeBSD 是一款操作系统，为现代服务器、桌面计算机和嵌入式平台提供动力，并拥有一个迄今已发展 30 年的大型社区。高级的网络、 安全和存储特性使得它成为许多常用的网站、嵌入式网络和存储设备的首选平台。

关于著作权信息，请查阅[本文件夹内的 COPYRIGHT](COPYRIGHT) 文件。

此源码树中的部分源码还包含其他的著作权信息，请翻阅特定源码的目录查看。

此文件夹中的 Makefile 文件支持许多用于构建 FreeBSD 源代码树的部分或全部组件的目标文件。请参阅 build(7)、config(8)、[关于构建用户空间的 FreeBSD 手册](https://docs.freebsd.org/zh_CN/books/handbook/cutting-edge/#makeworld) 和 [内核手册](https://docs.freebsd.org/zh_CN/books/handbook/kernelconfig/) 获取更多信息，包括设置 make(1) 变量。

源码指南：
---------------
| 文件夹 | 描述                                                         |
| --------- | ----------- |
| bin | 系统和用户命令 |
| cddl | 在 CDDL 许可证下发表的各种命令和库 |
| contrib | 第三方软件包 |
| crypto | 加密相关（请参阅 [crypto/README](crypto/README)） |
| etc | /etc 的模板文件 |
| gnu | 在 GNU 通用公共许可证（GPL）或 GNU 宽通用公共许可证（LGPL）下发布的命令和库。请参阅 [gnu/COPYING](gnu/COPYING) 和 [gnu/COPYING.LIB](gnu/COPYING.LIB) 了解更多信息。 |
| include | 系统级的 include 文件 |
| kerberos5 | Kerberos5（Heimdal）软件包 |
| lib | 库文件 |
| libexec | 系统守护进程 |
| release | 构建发布版本的 Makefile 文件和相关工具 |
| rescue | 静态链接 /rescue 实用工具的构建系统 |
| sbin | 系统命令 |
| secure | 安全有关文件和命令 |
| share | 共享的源代码 |
| stand | 引导程序源代码 |
| sys | 内核源代码 |
| sys/`arch`/conf | 内核配置文件。GENERIC 是用于发布版本的配置文件。NOTES 包含可能所有条目的文档。 |
| tests | 可以由 Kyua 运行的回归测试。请参阅 [tests/README](tests/README) 获取更多信息。 |
| tools | 用于回归测试和其他任务的实用程序 |
| usr.bin | 用户命令 |
| usr.sbin | 系统级管理命令 |

有关将您的源代码树与 FreeBSD 项目的一个或多个开发版本分支同步的信息，请参阅 [FreeBSD 手册](https://docs.freebsd.org/zh_CN/books/handbook/cutting-edge/#current-stable)。
