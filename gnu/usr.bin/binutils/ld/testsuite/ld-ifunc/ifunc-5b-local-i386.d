#source: ifunc-5-local-i386.s
#ld: -shared -m elf_i386 -z nocombreloc
#as: --32
#readelf: -r --wide
#target: x86_64-*-* i?86-*-*
#notarget: *-*-lynxos *-*-nto*

Relocation section '.rel.plt' at .*
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_386_IRELATIVE[ ]*
