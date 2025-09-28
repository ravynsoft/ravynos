#readelf: -x .reginfo
#name: MIPS .reginfo section size 1 (relocatable)
#source: reginfo-1.s RUN_OBJCOPY
#objcopy_objects: -R .reginfo
#ld: -r -T reginfo-1.ld

Hex dump of section '\.reginfo':
  0x00000000 01020304 05060708 090a0b0c 0d0e0f10 .*
  0x00000010 00000000 ........                   .*
