#ld: -shared -melf_x86_64
#readelf: -S --wide
#as: --64

#...
 +\[ *[0-9]+\] \.plt +PROGBITS +[0-9a-f]+ +[0-9a-f]+ +0+10 +.*
#...
 +\[ *[0-9]+\] \.got\.plt +PROGBITS +[0-9a-f]+ +[0-9a-f]+ +0+18 +.*
#pass
