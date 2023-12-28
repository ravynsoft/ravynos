#source: noload-1.s
#ld: -T noload-1.t -z max-page-size=0x200000
#readelf: -Sl --wide
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi

#...
 +LOAD +0x.00000 +0x0+ +0x0+ +0x0+.. +0x0+.. +RW +0x200000
#pass
