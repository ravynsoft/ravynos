# ravynOS란? [![Build Status](https://api.cirrus-ci.com/github/ravynsoft/ravynos.svg?branch=main)](https://cirrus-ci.com/github/ravynsoft/ravynos) [![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)
### Don't speak Korean? Read this in: [Italiano](README.IT.md), [Türkçe](README.TR.md), [Deutsch](README.DE.md), [Indonesia](README.ID.md), [简体中文](README.zh_CN.md), [繁體中文](README.zh_TW.md), [Português do Brasil](README.pt_BR.md), [English](README.md)

ravynOS는 새로운 오픈소스 운영체제 프로젝트로써 x86-64 (추후 ARM도 예정)상에서 macOS와 유사한 사용환경과 더불어 부분적인 호환성을 제공하고자 합니다. FreeBSD를 기반으로 작성되었으며 이미 존재하는 오픈소스 패키지들과 더불어 부족한 부분은 새로 작성되고 있습니다.

디자인 주요 지향점:
- macOS 어플리케이션들과 소스 호환성 (예. ravynOS 에서 mac 어플리케이션을 컴파일 후 실행 가능)
- 유사한 GUI 구성과 익숙한 UX (파일 매니저, 어플리케이션 런쳐, 현재 열린 어플리케이션을 나타내는 상단 메뉴바 등등)
- macOS 폴더 구성(/Library, /System, /Users, /Volumes, etc)과 호환 및 가능하다면 파일시스템 호환(HFS+, APFS), ZFS 완벽 지원
- [App Bundles](https://developer.apple.com/documentation/foundation/bundle), [AppDirs](https://github.com/AppImage/AppImageKit/wiki/AppDir),[AppImage](https://github.com/AppImage) 규격의 독립형 어플리케이션 지원 - /Applications에 대해 설치기가 필요 없는 경험 제공
- 최대한 FreeBSD와 X11 호환성 유지 - 내부적으로는 일반적인 Unix 환경
- FreeBSD의 리눅스 지원을 활용하여 리눅스 바이너리 호환
- 궁극적으로 x86-64/arm64 macOS 바이너리와 (Mach-O) 라이브러리 호환을 목표
- 쾌적한 사용환경, 보안, 안정성, 성능 제공

더 많은 정보를 얻고 싶다면 [ravynos.com](https://ravynos.com/)에 접속해보세요: [릴리즈 노트](https://ravynos.com/releases.html) | [스크린샷](https://ravynos.com/screenshots.html) | [FAQ](https://ravynos.com/faq.html)

### 함께하기!

* 꿈을 이루는 데에 도움을 주시겠나요? [CONTRIBUTING.md](CONTRIBUTING.md)에서 현재 프로젝트 및 요구사항을 확인하세요!
* 우리 [Discord](https://discord.com/invite/8caJbAGNwY) 서버.
* `#ravynOS-general:matrix.org` - [Element.io](https://app.element.io/#/room/%23ravynOS-general:matrix.org)를 통해 접속.
* [Libera IRC](https://web.libera.chat/?channel=#airyx) 에서 `#airyx`.

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
