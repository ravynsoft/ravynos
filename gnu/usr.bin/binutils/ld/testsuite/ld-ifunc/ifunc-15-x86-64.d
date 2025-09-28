#source: ifunc-15-x86-64.s
#ld: -shared -m elf_x86_64 -z nocombreloc
#as: --64
#readelf: -r --wide
#target: x86_64-*-*

Relocation section '.rela.got' at offset 0x[0-9a-f]+ contains 1 entry:
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_X86_64_GLOB_DAT[ ]+ifunc\(\)[ ]+ifunc \+ 0
