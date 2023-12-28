#source: ifunc-6-x86-64.s
#as: --64
#ld: -melf_x86_64 -shared -z nocombreloc
#readelf: -r --wide
#target: x86_64-*-*

Relocation section '.rela.got' at .*
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_X86_64_GLOB_DAT[ ]+foo\(\)[ ]+foo \+ 0
#...
Relocation section '.rela.plt' at .*
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_X86_64_IRELATIVE[ ]+[0-9a-f]*
