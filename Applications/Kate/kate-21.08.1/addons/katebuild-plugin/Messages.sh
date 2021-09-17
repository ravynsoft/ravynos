#! /bin/sh
$EXTRACTRC `find . -name \*.ui -o -name \*.rc` >>  rc.cpp
$XGETTEXT *.cpp -o $podir/katebuild-plugin.pot
rm -f rc.cpp

