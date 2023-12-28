#source: pltgot-1.s
#ld: -shared -melf_x86_64
#readelf: -d --wide
#as: --64

#failif
#...
 +0x[0-9a-f]+ +\(PLTREL.*
#...
