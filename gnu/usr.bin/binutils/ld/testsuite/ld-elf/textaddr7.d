#source: maxpage1.s
#ld: -n -z max-page-size=0x200000 -Ttext-segment 0x10000
#readelf: -l --wide
#target: *-*-linux-gnu *-*-gnu* arm*-*-uclinuxfdpiceabi

#...
  LOAD .*
#pass
