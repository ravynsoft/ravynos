#source: linkinfo1.s
#ld: -shared --hash-style=sysv
#objcopy_linked_file: --strip-debug
#readelf: -SW
#target: x86_64-* i?86-*

#...
  \[[ 0-9]+\] \.rel[a]?\.plt[ \t]+REL[A]?[ \t][ \t0-9a-f]+AI[ \t]+[0-9a-f]+[ \t]+(9|10)[ \t]+[0-9a-f]+
#...
  \[[[ ]*(9|10)\] \.got(|.plt)[ \t]+PROGBITS?[ \t][ \t0-9a-f]+WA[ \t]+[0-9a-f]+[ \t]+0[ \t]+[0-9a-f]+
#pass
