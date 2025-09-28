#source: maxpage1.s
#ld: -z max-page-size=0x10000 -Ttext-segment 0x10000 -z noseparate-code
#readelf: -l --wide
#target: *-*-linux-gnu *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: h8300-*-* rx-*-linux*

#...
  LOAD +0x0+ 0x0*10000 0x0*10000 0x0*[0-9a-f][0-9a-f][0-9a-f] 0x0*[0-9a-f][0-9a-f][0-9a-f] R E 0x10000
#pass
