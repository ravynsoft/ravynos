#name: changelma (pr20659)
#ld: -T changelma.lnk --no-warn-rwx-segments
#objcopy_linked_file: --change-section-lma .dynamic+0x80000000
#readelf: -l --wide
#xfail: rx-*-*
# rx-elf fails since it forces vma equal to lma

#...
  PHDR +0x[0-9a-f]* 0x[0-9a-f]* 0x0*8000[0-9a-f]{4} .*
  LOAD +0x0+ 0x0+ 0x0*80000000 .*
  DYNAMIC +0x[0-9a-f]* 0x[0-9a-f]* 0x0*8000[0-9a-f]{4} .*
#pass
