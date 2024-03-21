#ld: -shared -melf_i386
#readelf: -S --wide
#as: --32

#...
 +\[ *[0-9]+\] \.plt +PROGBITS +[0-9a-f]+ +[0-9a-f]+ +0+10 +.*
#...
 +\[ *[0-9]+\] \.got\.plt +PROGBITS +[0-9a-f]+ +[0-9a-f]+ +0+c +.*
#pass
