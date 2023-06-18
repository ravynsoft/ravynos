# O que é o ravynOS? [![Status de Construção](https://api.cirrus-ci.com/github/ravynsoft/ravynos.svg?branch=main)](https://cirrus-ci.com/github/ravynsoft/ravynos) [![Pacto de Contribuinte](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)
### Don't you speak portuguese? Read this in: [English](README.md)
ravynOS é um projeto de um novo sistema operacional de código aberto que procura dar uma experiência parecida e alguma compatibilidade com macOS em sistemas de arquitetura x86_64 (e eventualmente ARM). O sistema se baseia nas fundações sólidas do FreeBSD, na existência de pacotes de código aberto no mesmo espaço, e código novo para preencher o que falta.

Os objetivos principais são:

- Compatibilidade com código de programas do macOS (ex. você poderia compilar um programa do Mac no ravynOS e o executar)
- Filosofia de interface gráfica parecida e experiência de usuário familiar (gerenciador de arquivos, launcher de aplicativos, barra de menu no topo que reflita a aplicação aberta, etc)
- Compatível com planos de pastas parecidos do macOS (/Library, /System, /Users, /Volumes, etc) e talves sistemas de arquivos (HFS+, APFS) além de total suporte para ZFS
- Aplicações independentes em arquivos [App Bundles](https://developer.apple.com/documentation/foundation/bundle), [AppDirs](https://github.com/AppImage/AppImageKit/wiki/AppDir), e [AppImage](https://github.com/AppImage) - uma experiência livre de instaladores para /Applications
- Na maior parte manter compatibilidade com o sistema-base FreeBSD e X11 - um espaço Unix padrão no seu fundo
- Compatibilidade com binários do Linux com o suporte para Linux de FreeBSD
- Eventual compatibilidade com binários x86_64/arm64 do macOS (Mach-O) e suas bibliotecas
- Prazeroso de usar, seguro, estável e performante

Visite [ravynos.com](https://ravynos.com/) para mais informações (em inglês): [Notas de Lançamento](https://ravynos.com/releases.html) | [Prints](https://ravynos.com/screenshots.html) | [Perguntas Frequentes](https://ravynos.com/faq.html)
### Junte-se a nós!

* Consegue ajudar a construir o sonho? Veja os projetos/necessidades em [CONTRIBUTING.md](CONTRIBUTING.md)!
* Nosso servidor [Discord](https://discord.com/invite/8caJbAGNwY)
* `#airyx:matrix.org` - entre pelo [Element.io](https://app.element.io/#/room/#airyx:matrix.org)
* `#airyx` em [Libera IRC](https://web.libera.chat/?channel=#airyx)

_(nota: `#airyx` em matrix.org e IRC são conectados `#general` no Discord, apenas em inglês)_

[![Sediamento de pacotes OSS: Cloudsmith](https://img.shields.io/badge/OSS%20hosting%20by-cloudsmith-blue?logo=cloudsmith&style=flat-square)](https://cloudsmith.com)

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
