#source: abs32-reloc.s
#as: -a32
#ld: -melf32ppc -static --defsym a=1 --defsym 'HIDDEN(b=2)' --defsym c=0x12345678
#readelf: -rW

There are no relocations in this file.
