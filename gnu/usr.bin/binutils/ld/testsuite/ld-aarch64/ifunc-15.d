#source: ifunc-15.s
#target: [check_shared_lib_support]
#ld: -shared -z nocombreloc
#readelf: -r --wide

Relocation section '.rela.got' at offset 0x[0-9a-f]+ contains 1 entry:
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_AARCH64_(P32_|)GLOB_DAT[ ]+ifunc\(\)[ ]+ifunc \+ 0

Relocation section '.rela.plt' at offset 0x[0-9a-f]+ contains 1 entry:
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_AARCH64_(P32_|)JUMP_SLO(T|)[ ]+ifunc\(\)[ ]+ifunc \+ 0
