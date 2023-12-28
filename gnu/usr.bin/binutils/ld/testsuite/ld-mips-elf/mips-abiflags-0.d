#readelf: -x .MIPS.abiflags
#name: MIPS .MIPS.abiflags section size 0
#source: empty.s RUN_OBJCOPY
#objcopy_objects: -R .MIPS.abiflags
#ld: -T mips-abiflags-0.ld

Hex dump of section '\.MIPS\.abiflags':
  0x00000000 ........ ........ ........ ........ .*
  0x00000010 ........ ........                   .*
