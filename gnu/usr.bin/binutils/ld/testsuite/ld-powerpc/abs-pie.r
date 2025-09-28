#source: abs-reloc.s
#as: -a64
#ld: -melf64ppc -pie --hash-style=sysv --defsym a=1 --defsym 'HIDDEN(b=2)' --defsym c=0x123456789abcdef0
#readelf: -rW

Relocation section '\.rela\.dyn' at offset .* contains 1 entry:
 +Offset +Info +Type +Symbol's Value +Symbol's Name \+ Addend
0+10438 +0+16 R_PPC64_RELATIVE +10438
