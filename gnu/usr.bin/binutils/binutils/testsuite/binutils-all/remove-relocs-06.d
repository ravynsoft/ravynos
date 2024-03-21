#PROG: objcopy
#source: remove-relocs-01.s
#objcopy: --remove-relocations=.data.relocs.* --remove-relocations=!.data.relocs.02
#readelf: -r
#notarget: mips64*-*-openbsd* mips64*-*-*-gnuabi64

Relocation section '\.rela?\.data\.relocs\.02' at offset 0x[0-9a-f]+ contains 3 entries:
.*
.*
.*
.*

