#source: maxpage1.s
#as: --32
#ld: -z max-page-size=0x200000 -z common-page-size=0x100000 -T maxpage4.t
#readelf: -l --wide
#target: x86_64-*-linux*

#...
  LOAD+.*0x200000
#pass
