#source: ifunc-3-x86.s
#ld: -shared
#readelf: -r --wide
#target: x86_64-*-* i?86-*-*

#...
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_[_0-9A-Z]+_IRELATIVE[ ]*[0-9a-f]*
#pass
