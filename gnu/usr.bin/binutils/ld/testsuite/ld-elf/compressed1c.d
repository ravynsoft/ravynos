#source: compress1.s
#as: --compress-debug-sections=zlib-gabi
#ld: -shared --compress-debug-sections=none
#readelf: -t
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: ![check_shared_lib_support] 

#failif
#...
  .*COMPRESSED.*
#...
