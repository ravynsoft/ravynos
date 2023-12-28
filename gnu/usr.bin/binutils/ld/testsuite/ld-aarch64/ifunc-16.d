#source: ifunc-16.s
#target: [check_shared_lib_support]
#ld: -shared
#readelf: -r --wide

Relocation section '.rela.plt' at .*
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_AARCH64_(P32_|)JUMP_SLO(T|)[ ]+0+[ ]+ifunc \+ 0
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_AARCH64_(P32_|)IRELATIV(E|)[ ]+[0-9a-f]*
