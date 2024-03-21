#source: ifunc-5-local.s
#ld: -r
#readelf: -r --wide
#target: aarch64*-*-*

Relocation section '.rela.text' at .*
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_AARCH64_(P32_|)CALL26[ ]+foo\(\)[ ]+foo \+ 0
[0-9a-f]+[ ]+[0-9a-f]+[ ]+.*ADR_GOT.*foo\(\)[ 	]+foo \+ 0
[0-9a-f]+[ ]+[0-9a-f]+[ ]+.*LD(32|64)_GOT.*foo\(\)[ ]+foo \+ 0
