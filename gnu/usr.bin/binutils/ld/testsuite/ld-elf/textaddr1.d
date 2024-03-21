#source: maxpage1.s
#ld: -Ttext-segment 0x7000000 -z max-page-size=0x200000 -z noseparate-code
#readelf: -l --wide
#target: *-*-linux-gnu *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: h8300-*-* rx-*-linux*

#...
  LOAD +0x0+ 0x0*7000000 0x0*7000000 0x0*[0-9a-f][0-9a-f][0-9a-f] 0x0*[0-9a-f][0-9a-f][0-9a-f] R E 0x200000
#pass
