#! /bin/sh
$EXTRACTRC `find . -name \*.ui` >>  rc.cpp
$XGETTEXT *.cpp -o $podir/kate-replicode-plugin.pot
rm -f rc.cpp

