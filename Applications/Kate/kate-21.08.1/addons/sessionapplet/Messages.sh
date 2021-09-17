#! /usr/bin/env bash
$XGETTEXT `find . -name \*.qml -o -name \*.cpp` -o $podir/plasma_applet_org.kde.plasma.katesessions.pot
