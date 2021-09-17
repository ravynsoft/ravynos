#! /bin/sh
$EXTRACTRC `find . -name \*.ui` >>  rc.cpp
$XGETTEXT *.cpp -o $podir/kate-ctags-plugin.pot
rm -f rc.cpp

