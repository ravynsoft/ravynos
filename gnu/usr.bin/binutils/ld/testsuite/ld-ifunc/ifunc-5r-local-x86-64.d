#source: ifunc-5-local-x86-64.s
#as: --64 -mrelax-relocations=yes
#ld: -r -melf_x86_64
#readelf: -r --wide
#target: x86_64-*-*

Relocation section '.rela.text' at .*
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_X86_64_PLT32[ ]+foo\(\)[ ]+foo - 4
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_X86_64_REX_GOTPCRELX[ ]+foo\(\)[ ]+foo - 4
