#PROG: objcopy
#name: objcopy add-empty-section
#source: empty.s
#objcopy: --add-section NEW=$srcdir/$subdir/empty-file
#readelf: -S --wide

#...
  \[[ 0-9]+\] NEW[ \t]+PROGBITS[ \t]+[0-9a-f]+[ \t]+[0-9a-f]+[ \t]+0+[ \t]+[ \t0-9a-f]+
#...
