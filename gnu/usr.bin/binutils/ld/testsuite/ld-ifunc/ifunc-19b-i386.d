#source: ifunc-19b.s
#source: ifunc-19a.s
#ld: -shared -m elf_i386 -z nocombreloc
#as: --32
#readelf: -r --wide
#target: x86_64-*-* i?86-*-*
#notarget: *-*-lynxos *-*-nto*

Relocation section '.rel.ifunc' at .*
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_386_IRELATIVE[ ]*

Relocation section '.rel.plt' at .*
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_386_IRELATIVE[ ]*
