#source: abs32-reloc.s
#as: -a32
#ld: -melf32ppc -shared --hash-style=sysv --defsym a=1 --defsym 'HIDDEN(b=2)' --defsym c=0x12345678
#readelf: -rW

Relocation section '\.rela\.dyn' at offset .* contains 6 entries:
 Offset     Info    Type                Sym\. Value  Symbol's Name \+ Addend
000101e0  00000016 R_PPC_RELATIVE                    101e0
000101f4  00000016 R_PPC_RELATIVE                    101e0
000101e4  00000401 R_PPC_ADDR32           00000001   a \+ 0
000101fc  00000414 R_PPC_GLOB_DAT         00000001   a \+ 0
000101ec  00000301 R_PPC_ADDR32           12345678   c \+ 0
000101f8  00000314 R_PPC_GLOB_DAT         12345678   c \+ 0
