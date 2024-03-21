#source: ifunc-20.s
#target: [check_shared_lib_support]
#ld: -shared -z nocombreloc
#readelf: -r --wide

Relocation section '.rela.ifunc' at offset 0x[0-9a-f]+ contains 1 entry:
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_AARCH64_ABS64[ ]+ifunc\(\)[ ]+ifunc \+ 0

Relocation section '.rela.plt' at offset 0x[0-9a-f]+ contains 1 entry:
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_AARCH64_JUMP_SLOT[ ]+ifunc\(\)[ ]+ifunc \+ 0
