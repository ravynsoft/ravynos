#source: maxpage1.s
#ld: -shared -z max-page-size=0x10000 -Ttext-segment 0x10000 -z noseparate-code
#readelf: -l --wide
#target: *-*-linux-gnu *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: ![check_shared_lib_support] 

#...
  LOAD +0x0+ 0x0*10000 0x0*10000 0x0*[0-9a-f]+ 0x0*[0-9a-f]+ R[ W]E 0x10000
#pass
