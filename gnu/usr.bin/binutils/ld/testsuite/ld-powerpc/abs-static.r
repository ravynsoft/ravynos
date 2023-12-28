#source: abs-reloc.s
#as: -a64
#ld: -melf64ppc -static --defsym a=1 --defsym 'HIDDEN(b=2)' --defsym c=0x123456789abcdef0
#readelf: -rW

There are no relocations in this file.
