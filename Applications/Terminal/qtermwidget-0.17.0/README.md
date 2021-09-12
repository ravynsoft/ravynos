# QTermWidget

## Overview

A terminal emulator widget for Qt 5.

QTermWidget is an open-source project originally based on the KDE4 Konsole application, but it took its own direction later on.
The main goal of this project is to provide a unicode-enabled, embeddable Qt widget for using as a built-in console (or terminal emulation widget).

It is compatible with BSD, Linux and OS X.

This project is licensed under the terms of the [GPLv2](https://www.gnu.org/licenses/gpl-2.0.en.html) or any later version. See the LICENSE file for the full text of the license. Some files are published under compatible licenses:
```
Files: example/main.cpp
       lib/TerminalCharacterDecoder.cpp
       lib/TerminalCharacterDecoder.h
       lib/kprocess.cpp
       lib/kprocess.h
       lib/kpty.cpp
       lib/kpty.h
       lib/kpty_p.h
       lib/kptydevice.cpp
       lib/kptydevice.h
       lib/kptyprocess.cpp
       lib/kptyprocess.h
       lib/qtermwidget.cpp
       lib/qtermwidget.h
Copyright: Author Adriaan de Groot <groot@kde.org>
           2010, KDE e.V <kde-ev-board@kde.org>
           2002-2007, Oswald Buddenhagen <ossi@kde.org>
           2006-2008, Robert Knight <robertknight@gmail.com>
           2002, Waldo Bastian <bastian@kde.org>
           2008, e_k <e_k@users.sourceforge.net>
License: LGPL-2+

Files: pyqt/cmake/*
Copyright: 2012, Luca Beltrame <lbeltrame@kde.org>
           2012, Rolf Eike Beer <eike@sf-mail.de>
           2007-2014, Simon Edwards <simon@simonzone.com>
License: BSD-3-clause

Files: cmake/FindUtf8Proc.cmake
Copyright: 2009-2011, Kitware, Inc
           2009-2011, Philip Lowman <philip@yhbt.com>
License: BSD-3-clause

Files: pyqt/cmake/PythonCompile.py
License: public-domain
```

## Installation

### Compiling sources

The only runtime dependency is qtbase ≥ 5.12.0.
Build dependencies are as follows:
- CMake ≥ 3.1.0 serves as the build system and therefore needs to be present to compile.
- The latest [lxqt-build-tools](https://github.com/lxqt/lxqt-build-tools/) is also needed for compilation.
- Git is needed to optionally pull latest VCS checkouts.

Code configuration is handled by CMake. CMake variable `CMAKE_INSTALL_PREFIX` will normally have to be set to `/usr`, depending on the way library paths are dealt with on 64bit systems. Variables like `CMAKE_INSTALL_LIBDIR` may have to be set as well.

To build, run `make`. To install, run `make install` which accepts variable `DESTDIR` as usual.

To build PyQt bindings, specify an additional CMake option `QTERMWIDGET_BUILD_PYTHON_BINDING=ON` when building this library.

### Binary packages

The library is provided by all major Linux distributions. This includes Arch Linux, Debian, Fedora, openSUSE and all of their children, given they use the same package repositories.
Just use the distributions' package managers to search for string `qtermwidget`.


### Translation

Translations can be done in [LXQt-Weblate](https://translate.lxqt-project.org/projects/lxqt-desktop/qtermwidget/)

<a href="https://translate.lxqt-project.org/projects/lxqt-desktop/qtermwidget/">
<img src="https://translate.lxqt-project.org/widgets/lxqt-desktop/-/qtermwidget/multi-auto.svg" alt="Translation status" />
</a>

