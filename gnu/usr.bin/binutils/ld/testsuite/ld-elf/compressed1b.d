#source: compress1.s
#as: --compress-debug-sections=zlib-gabi
#ld: -r --compress-debug-sections=none
#readelf: -t
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi

#failif
#...
  .*COMPRESSED.*
#...
