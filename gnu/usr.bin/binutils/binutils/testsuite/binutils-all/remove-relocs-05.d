#PROG: objcopy
#source: remove-relocs-01.s
#objcopy: --remove-section=.rela.data.relocs.01 --remove-section=.rel.data.relocs.01
#readelf: -r
#notarget: mips64*-*-openbsd* mips64*-*-*-gnuabi64

Relocation section '\.rela?\.data\.relocs\.02' at offset 0x[0-9a-f]+ contains 3 entries:
.*
.*
.*
.*

Relocation section '\.rela?\.data\.relocs\.03' at offset 0x[0-9a-f]+ contains 3 entries:
.*
.*
.*
.*

