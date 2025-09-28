#source: pr19162a.s
#source: pr19162b.s
#as: --64
#ld: -melf_x86_64 -shared -z noseparate-code -z max-page-size=0x200000 -z common-page-size=0x1000 --hash-style=sysv $NO_DT_RELR_LDFLAGS
#readelf: -l --wide
#target: x86_64-*-linux*

#...
  DYNAMIC        0x000118 0x0000000000200118 0x0000000000200118 0x0000b0 0x0000b0 RW  0x8
#pass
