#readelf: -x .reginfo
#name: MIPS .reginfo section size 2
#source: reginfo-2.s RUN_OBJCOPY
#objcopy_objects: -R .reginfo
#ld: -T reginfo-1.ld

Hex dump of section '\.reginfo':
  0x00000000 01020304 05060708 090a0b0c 0d0e0f10 .*
  0x00000010 11121314 00000000                   .*
