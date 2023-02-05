Czym jest ravynOS? [![Build Status](https://api.cirrus-ci.com/github/ravynsoft/ravynos.svg?branch=main)](https://cirrus-ci.com/github/ravynsoft/ravynos) [![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)
### Don't speak Polish? Read this in: [Italiano](README.IT.md), [Türkçe](README.TR.md), [Deutsch](README.DE.md), [Indonesia](README.ID.md), [简体中文](README.zh_CN.md), [繁體中文](README.zh_TW.md)

ravynOS jest nowym projektem otwartoźródłowego systemu operacyjnego, który dąży do dostarczania podobnego środowiska i kompatybilności z macOS na architekturze x86-64 (a później ARM). Projekt jest zbudowany na solidnych fundamentach FreeBSD, istniejących pakietów otwartoźródłowych i nowego kodu wypełniającego luki.

Główne założenia:
- Kompatybilność z kodem źródłowym aplikacji macOS (możesz skompilować aplikację macOS na ravynOS i ją uruchomić)
- Podobne metafory GUI i przyjazny UX (menadżer plików, uruchamianie aplikacji, górny pasek menu zależny od otwartej aplikacji, itd.)
- Kompatybilność z układem folderów macOS (/Library, /System, /Users, /Volumes, etc) i prawdopodobnie systemem plików (HFS+, APFS) i pełne wsparcie dla ZFS
- Pełne aplikacje znajdujące się w plikach [App Bundles](https://developer.apple.com/documentation/foundation/bundle), [AppDirs](https://github.com/AppImage/AppImageKit/wiki/AppDir) i [AppImage](https://github.com/AppImage) - brak potrzeby instalowania do /Applications
- W większości wspieranie kompatybilności z systemem bazowym FreeBSD i X11 - standardowym środowiskiem Unix
- Kompatybilność z plikami binarnymi Linux poprzez warstwę kompatybilności z FreeBSD na Linux.
- Ewentualna kompatybilność z plikami binarnymi macOSa dla architektury x86-64/arm64 (Mach-O) i bibliotekami
- Przyjemny w użyciu, bezpieczny, stabilny, i wydajny

Odwiedź stronę [ravynos.com](https://ravynos.com/), aby dowiedzieć się więcej: [Wydania](https://ravynos.com/releases.html) | [Zrzuty ekranu](https://ravynos.com/screenshots.html) | [FAQ](https://ravynos.com/faq.html)

### Dołącz do nas!

* Pomożesz spełnić nasze plany? Sprawdź listę naszych bieżących projektów/potrzeb w [CONTRIBUTING.md](CONTRIBUTING.md)!
* Nasz serwer [Discord](https://discord.com/invite/8caJbAGNwY).
* `#ravynOS-general:matrix.org` - dołącz za pomocą [Element.io](https://app.element.io/#/room/%23ravynOS-general:matrix.org)
* `#airyx` na [Libera IRC](https://web.libera.chat/?channel=#airyx)

[![Packages hosted by: Cloudsmith](https://img.shields.io/badge/OSS%20hosting%20by-cloudsmith-blue?logo=cloudsmith&style=flat-square)](https://cloudsmith.com)

---

Źródło FreeBSD:
---------------
To jest główny katalog kodu źródłowego FreeBSD.

FreeBSD jest systemem operacyjnym używanym, aby zasilić nowoczesne serwery, komputery osobiste i platformy osadzone.
Projekt jest wspierwany przez dużą społeczność od ponad 30 lat.
Jego zaawansowane funkcje sieciowe, bezpieczeństwa i przechowywania sprawiły, że FreeBSD jest platformą obsługującą wiele popularnych stron internetowych i większości wbudowanych urządzeń sieciowych i pamięci masowej.

Informacje o prawach autorskich znajdują się w [pliku COPYRIGHT](COPYRIGHT) w tym katalogu.
Dodaktowe informacje dotyczące praw autorskich można znaleźć w drzewie źródłowym - proszę sprawdzić poszczególne katalogi w celu uzyskania informacji.

Plik Makefile w tym katalogu umożliwia kompilację wielu (lub wszystkich) komponentów znajdujących się w drzewie źródłowym FreeBSD.
Sprawdź build(7), config(8), [rodział podręcznika FreeBSD o kompilacji narzędzi użytkowych](https://docs.freebsd.org/en/books/handbook/cutting-edge/#makeworld) i [rozdział o kompilacji jądra systemu](https://docs.freebsd.org/en/books/handbook/kernelconfig/), aby dowiedzieć się więcej, w tym o ustawianiu zmiennych make(1).

Informacje o architekturach CPU i platformach wspieranych przez FreeBSD, zajrzyj na [stronę
FreeBSD](https://www.freebsd.org/platforms/).

Drzewo źródłowe:
---------------
| Katalog | Opis |
| --------- | ----------- |
| bin | Komendy systemowe/użytkownika. |
| cddl | Róźne komendy i biblioteki na licencji Common Development and Distribution License. |
| contrib | Pakiety dostarczane przez osoby trzecie. |
| crypto | Kryptografia (przeczytaj [crypto/README](crypto/README)). |
| etc | Szablony plików /etc. |
| gnu | Komendy i biblioteki na licencji GNU General Public License (GPL) lub Lesser General Public License (LGPL). Przeczytaj [gnu/COPYING](gnu/COPYING) i [gnu/COPYING.LIB](gnu/COPYING.LIB), aby dowiedzieć się więcej. |
| include | Systemowe pliki nagłówków. |
| kerberos5 | Pakiet Kerberos5 (Heimdal). |
| lib | Biblioteki systemowe. |
| libexec | Systemowe procesy działające w tle. |
| release | Makefile kompilujące wydanie i powiązanie narzędzia. |
| rescue | System budowania statycznie złączonych narzędzi /rescue. |
| sbin | Komendy systemowe. |
| secure | Biblioteki i komendy kryptograficzne. |
| share | Współdzielone zasoby. |
| stand | Źródło programu uruchamiającego. |
| sys | Źródło jądra systemu (przeczytaj [sys/README.md](sys/README.md)). |
| targets | Wsparcie dla eksperymentalnego `DIRDEPS_BUILD` |
| tests | Testy regresji, które mogą być uruchamiane przez Kyua.  Przeczytaj [tests/README](tests/README), aby dowiedzieć się więcej. |
| tools | Narzędzia do testów regresji i innych zadań. |
| usr.bin | Komendy użytkownika. |
| usr.sbin | Komendy administracji systemem. |

Informacje o synchronizowaniu twojego drzewa źródłowego z jedną lub więcej gałęzi deweloperskich Projektu FreeBSD znajdują się w [Podręczniku FreeBSD](https://docs.freebsd.org/en/books/handbook/cutting-edge/#current-stable).
