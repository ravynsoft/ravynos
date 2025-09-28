#source: thumb-plt.s
#name: Thumb only PLT and GOT LSB Symbol
#ld: -shared -e0 -z max-page-size=0x10000
#readelf: -rx .got
#skip: *-*-pe *-*-wince *-*-vxworks armeb-*-* *-*-gnueabihf

Relocation section '.rel.plt' at offset 0x108 contains 1 entry:
 Offset     Info    Type            Sym.Value  Sym. Name
000101.+  00000116 R_ARM_JUMP_SLOT   00000000   foo

Hex dump of section '.got':
 NOTE: This section has relocations against it, but these have NOT been applied to this dump.
  0x000101.+ 40010100 00000000 00000000 11010000 @...............

