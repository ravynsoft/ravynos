#source: ifunc-16-x86.s
#as: --64
#ld: -shared -melf_x86_64
#readelf: -r --wide
#target: x86_64-*-*

Relocation section '.rela.plt' at .*
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_X86_64_JUMP_SLOT[ ]+0+[ ]+ifunc \+ 0
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_X86_64_IRELATIVE[ ]+[0-9a-f]*
