#source: ifunc-5-local-i386.s
#ld: -r -m elf_i386
#as: --32
#readelf: -r --wide
#target: x86_64-*-* i?86-*-*
#notarget: *-*-lynxos *-*-nto*

Relocation section '.rel.text' at .*
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_386_GOTPC[ ]+0+[ ]+_GLOBAL_OFFSET_TABLE_[ ]*
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_386_PLT32[ ]+foo\(\)[ ]+foo[ ]*
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_386_GOT32[ ]+foo\(\)[ ]+foo[ ]*
