#target: *-*-linux* *-*-gnu* *-*-vxworks arm*-*-uclinuxfdpiceabi
#source: seg.s
#ld: -T seg.t -z max-page-size=0x1000 -z common-page-size=0x1000
#readelf: -l --wide

#...
Program Headers:
  Type           Offset   VirtAddr.*
# On MIPS, the first segment is for .reginfo.
#...
  LOAD           .*
  LOAD           0x0*001000 0xf*fffff000 0xf*fffff000 0x0*1000 0x0*1000 .*
# FRV adds a PT_GNU_STACK header
#...
 Section to Segment mapping:
  Segment Sections...
   00     .*
# On MIPS, the first segment is for .reginfo.
#...
   0.     reset boot 
#pass
