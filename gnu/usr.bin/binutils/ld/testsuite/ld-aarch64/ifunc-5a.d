#source: ifunc-5.s
#ld:
#readelf: -r --wide
#target: aarch64*-*-*

Relocation section '.rela.plt' at .*
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_AARCH64_(P32_|)IRELATIV(E|)[ ]+[0-9a-f]*
