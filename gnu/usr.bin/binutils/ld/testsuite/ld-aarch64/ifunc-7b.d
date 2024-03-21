#source: ifunc-7.s
#ld: -shared
#target: [check_shared_lib_support]
#readelf: -r --wide

Relocation section '.rela.plt' at .*
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_AARCH64_(P32_|)IRELATIV(E|)[ ]+[0-9a-f]*
