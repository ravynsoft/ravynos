#!/bin/sh

LC_ALL=C
export LC_ALL

test -z "$srcdir" && srcdir=.
test -z "$top_srcdir" && top_srcdir=$srcdir/../..

SGML_DOCS=true
FILES=`echo $srcdir/tmpl/*.sgml`

. "$top_srcdir/src/check-doc-syntax.sh"
