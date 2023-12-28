#name: MIPS rel32 o32
#source: rel32.s
#as: -KPIC
#readelf: -x .text -r
#ld: -shared

Relocation section '.rel.dyn' at offset .* contains 2 entries:
 Offset     Info    Type            Sym.Value  Sym. Name
[0-9a-f ]+R_MIPS_NONE      
[0-9a-f ]+R_MIPS_REL32     

Hex dump of section '.text':
  0x00000230 00000000 00000000 00000000 00000000 .*
  0x00000240 00000240 00000000 00000000 00000000 .*
  0x00000250 00000000 00000000 00000000 00000000 .*
