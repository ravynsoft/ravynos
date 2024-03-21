#source: abs32-reloc.s
#as: -a32
#ld: -melf32ppc -pie --hash-style=sysv --defsym a=1 --defsym 'HIDDEN(b=2)' --defsym c=0x12345678
#readelf: -rW

Relocation section '\.rela\.dyn' at offset .* contains 2 entries:
 Offset     Info    Type                Sym\. Value  Symbol's Name \+ Addend
000101c8  00000016 R_PPC_RELATIVE                    101c8
000101dc  00000016 R_PPC_RELATIVE                    101c8
