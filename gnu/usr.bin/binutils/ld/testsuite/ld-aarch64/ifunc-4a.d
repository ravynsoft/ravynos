#ld: -s
#readelf: -r --wide
#target: aarch64*-*-*
#source: ifunc-4.s

#...
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_[_0-9A-Z]+_IRELATIVE[ ]*[0-9a-f]*
#pass
