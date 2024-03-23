#!/bin/sh

if test $# -ne 4 ; then
   echo "Usage: $0 <pdf-file> <pdfinfo-output> <pdfinfo-ref> <diff-output>"
   exit 3
fi

# Check for pdfinfo version >= 21.10.00
if pdfinfo -v 2>& 1 | awk '/pdfinfo version/ { split($3,v,/[.]/); if (v[1] > 21 || (v[1] == 21 && v[2] >= 10) ) { print "yes" }  } ' | grep -q  'yes'; then
    pdfinfo -struct-text "$1" > "$2"
    if test -f "$3" ; then
        diff -u "$3" "$2" > "$4"
        # diff exit codes: 0 = match, 1 = different, 2 = error
        exit $?
    else
        exit 3 # missing ref file
    fi
fi

 # pdfinfo missing or wrong version
exit 4
