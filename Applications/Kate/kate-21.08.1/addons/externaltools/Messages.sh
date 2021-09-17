#! /bin/sh
$EXTRACTRC *.rc *.ui >> rc.cpp
grep -n -e '^ *name=' defaultexternaltoolsrc | sed 's!^\(.*\):.*name= *\(.*\) *$!// i18n: file: \1\ni18nc("External tool name", "\2");!' | sed 's/ \+")/")/' >>rc.cpp || exit 13
grep -n -e '^ *category=' defaultexternaltoolsrc | sed 's!^\(.*\):.*category= *\(.*\) *$!// i18n: file: \1\ni18nc("External tool category", "\2");!' | sed 's/ \+")/")/' >>rc.cpp || exit 13
$XGETTEXT *.cpp  -o $podir/kateexternaltoolsplugin.pot
