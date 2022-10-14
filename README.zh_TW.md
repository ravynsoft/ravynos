# 什麼是 ravynOS? [![目前狀態](https://api.cirrus-ci.com/github/ravynsoft/ravynos.svg?branch=main)](https://cirrus-ci.com/github/ravynsoft/ravynos) [![貢獻者](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)
### Don't speak Chinese（Taiwan）? Read this in: [Italiano](README.IT.md), [Türkçe](README.TR.md), [Deutsch](README.DE.md), [Indonesia](README.ID.md), [Hungarian](README.HU.md)

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

著作權資訊請詳閱 [在這個資料夾內的檔案](COPYRIGHT)
其他著作權資訊也遍佈各個資料夾
這個 Makefile 支援編譯數曾獲所有資料夾
請詳閱 build(7)，config(8)，[FreeBSD 編譯指南](https://docs.freebsd.org/en/books/handbook/cutting-edge/#makeworld)，以及[核心手冊](https://docs.freebsd.org/en/books/handbook/kernelconfig/)來取得更多資訊以及查詢 make(1) 變數。

Source Roadmap:
---------------
| 資料夾 | 資訊 |
| --------- | ----------- |
| bin | 基本指令 |
| cddl | 其他指令以及資料庫 |
| contrib | 第三方套件 |
| crypto | 翻譯器（請詳閱[crypto/README](crypto/README)) |
| etc | 範本檔案 |
| gnu | 符合 GPL 或 LGPL 的指令。請詳閱 [gnu/COPYING](gnu/COPYING) 和 [gnu/COPYING.LIB](gnu/COPYING.LIB)  |
| include | 系統檔案 |
| kerberos5 | Kerberos5 (Heimdal) 套件 |
| lib | 資料庫 |
| libexec | 系統守護進程 |
| release | 釋出 Makefile 和工具 |
| rescue | 系統工具 |
| sbin | 系統指令 |
| secure | 翻譯器指令 |
| share | 分享的資料 |
| stand | 開機管理器資料 |
| sys | 核心資料 |
| sys/`arch`/conf | 核心設定檔案。GENERIC 是最主要的。 NOTES 提供設定指南 |
| tests | 回歸測試的軟體例如 Kyua。請詳閱 [tests/README](tests/README) 取得更多資訊 |
| tools | Utilities for regression testing and miscellaneous tasks. |
| usr.bin | 使用者指令 |
| usr.sbin | 系統權限指令 |

更多資訊 [FreeBSD 手冊](https://docs.freebsd.org/zh-tw/books/handbook/cutting-edge/#current-stable).
