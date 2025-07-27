# ravynOS 是什么？ [![Build Status](https://api.cirrus-ci.com/github/ravynsoft/ravynos.svg?branch=main)](https://cirrus-ci.com/github/ravynsoft/ravynos) [![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)
### Don't speak Chinese (China)? Read this in: [Italiano](README.IT.md), [Türkçe](README.TR.md), [Deutsch](README.DE.md), [Indonesia](README.ID.md), [Português do Brasil](README.pt_BR.md), [한국어](README.KR.md), [فارسی](README.FA.md)

ravynOS 是一个新型的操作系统项目，致力于在 x86-64（乃至于 ARM）平台上提供与 macOS 类似的体验和兼容性。它基于坚如磐石的 FreeBSD、现有的开源代码和锦上添花的新代码构建。

主要设计目标：
- 源码级兼容 macOS 应用程序（比如你可以在 ravynOS 上编译和运行 macOS 应用）
- 相似的 GUI 体验和 UX（文件管理器、程序启动器、顶部菜单之类）
- 兼容 macOS 的目录层次（/Library、/System、/Users、/Volumes 等等），也许支持 HFS+ 和 APFS 文件系统，并完全支持 ZFS 文件系统
- 支持 [App Bundles](https://developer.apple.com/documentation/foundation/bundle)、[AppDirs](https://github.com/AppImage/AppImageKit/wiki/AppDir) 以及 [AppImage](https://github.com/AppImage) 自封装格式的应用——摆脱使用安装器的烦恼
- 尽量保持与 FreeBSD 基本系统和 X11 的兼容性——本质上仍是标准的 Unix 环境
- 藉着 FreeBSD 的 Linux 兼容层实现对 Linux 应用程序的支持
- 终极目标是实现与 x86-64/arm64 macOS 应用程序（Mach-O）和库的兼容
- 易用、安全、稳定、高效

请访问 [ravynos.com](https://ravynos.com/) 以了解更多信息：[发行说明](https://ravynos.com/releases.html) | [屏幕截图](https://ravynos.com/screenshots.html) | [FAQ](https://ravynos.com/faq.html)

### 加入我们！

* 想帮我们圆梦吗？看一下 [CONTRIBUTING.md](CONTRIBUTING.md) 了解当下项目及需求！
* 我们的 [Discord](https://discord.com/invite/8caJbAGNwY) 服务器。
* `#ravynOS-general:matrix.org` - 通过 [Element.io](https://app.element.io/#/room/%23ravynOS-general:matrix.org) 加入。

[![Packages hosted by: Cloudsmith](https://img.shields.io/badge/OSS%20hosting%20by-cloudsmith-blue?logo=cloudsmith&style=flat-square)](https://cloudsmith.com)

---

FreeBSD 源代码:
---------------
这是 FreeBSD 源代码的顶层目录。

FreeBSD 是一款操作系统，为现代服务器、桌面计算机和嵌入式平台赋能，由一家迄今 30 余年的大型社区持续推进开发。先进的网络、安全和存储功能使其成为许多流行网站、嵌入式网络和存储设备的首选平台。

关于版权信息，请查阅此路径下的 [COPYRIGHT 文件](COPYRIGHT)。此源码树中的部分源码还包含其他的版权信息，请查阅相关源代码目录。

此目录中的 Makefile 支持多个目标，用于构建 FreeBSD 源代码树的各个组件（或全部组件）。请参阅 build(7)、config(8)、[关于构建用户空间的 FreeBSD 手册](https://docs.freebsd.org/zh_CN/books/handbook/cutting-edge/#makeworld) 和 [内核手册](https://docs.freebsd.org/zh_CN/books/handbook/kernelconfig/) 获取更多信息，如设置 make(1) 变量。

有关 FreeBSD 支持的 CPU 架构和平台的信息，请参见 [FreeBSD 网站的 Platforms 页面](https://www.freebsd.org/platforms/)。

要获取官方 FreeBSD 可启动镜像，请访问[版本发布页面](https://download.freebsd.org/ftp/releases/ISO-IMAGES/)。

源码开发路径：
---------------
| 目录 | 说明                                                         |
| --------- | ----------- |
| bin | 系统和用户命令 |
| cddl | 在 CDDL 许可证下发表的各种命令和库 |
| contrib | 第三方软件包 |
| crypto | 加密相关（请参阅 [crypto/README](crypto/README)） |
| etc | /etc 的模板文件 |
| gnu | 在 GNU 通用公共许可证（GPL）或 GNU 宽通用公共许可证（LGPL）下发布的命令和库。请参阅 [gnu/COPYING](gnu/COPYING) 和 [gnu/COPYING.LIB](gnu/COPYING.LIB) 了解更多信息。 |
| include | 系统级的 include 文件 |
| kerberos5 | Kerberos5（Heimdal）软件包 |
| lib | 系统库文件 |
| libexec | 系统守护进程 |
| release | 构建发布版本的 Makefile 文件和相关工具 |
| rescue | 静态链接 /rescue 实用工具的构建系统 |
| sbin | 系统命令 |
| secure | 加密库及命令行工具 |
| share | 共享的资源 |
| stand | 引导程序源代码 |
| sys | 内核源代码 |
| targets | 对实验性的 `DIRDEPS_BUILD` 功能的支持 |
| tests | 可以由 Kyua 运行的回归测试。请参阅 [tests/README](tests/README) 获取更多信息。 |
| tools | 用于回归测试和其他任务的实用程序 |
| usr.bin | 用户命令 |
| usr.sbin | 系统级管理命令 |

有关将您的源代码树同 FreeBSD 项目的一个或多个开发版本分支同步的信息，请参阅 [FreeBSD 手册](https://docs.freebsd.org/zh_CN/books/handbook/cutting-edge/#current-stable)。
