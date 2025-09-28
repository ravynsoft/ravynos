#source: loadaddr.s
#ld: -T loadaddr2.t -T loadaddr.t -z max-page-size=0x200000 -z noseparate-code
#readelf: -l --wide
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: h8300-*-* rx-*-linux*

#...
  LOAD +0x000000 0xf*80000000 0xf*80000000 0x100050 0x100050 RWE 0x200000
  LOAD +0x[13]10000 0xf*80110000 0xf*80101000 0x0*10 0x0*10 R E 0x200000
  LOAD +0x[35]02000 0xf*80302000 0xf*80302000 0x0*10 0x0*10 RW  0x200000
#pass
