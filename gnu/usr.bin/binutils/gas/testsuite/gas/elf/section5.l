.*: Assembler messages:
.*:7: Error: .*
.*:7: Error: .*
.*:10: Error: .*
.*:16: Error: .*
.*:18: Warning: .*
.*:20: Warning: .*
.*:22: Warning: .*
.*:24: Warning: .*
4 errors, 4 warnings, generating bad object file
.*GAS.*


[ 	]+[0-9]+[ 	]+.section[ 	]+.test0[ 	]*
[ 	]+[0-9]+[ 	]+.section[ 	]+.test1,[ 	]*"",[ 	]*%progbits[ 	]*
[ 	]+[0-9]+[ 	]+.section[ 	]+.test2[ 	]*
[ 	]+[0-9]+[ 	]+.section[ 	]+.test3,[ 	]*"aw"[ 	]*
[ 	]+[0-9]+[ 	]+.section[ 	]+.test4,[ 	]*"aw",[ 	]*%nobits[ 	]*
[ 	]+[0-9]+[ 	]*
[ 	]+[0-9]+[ 	]+.section[ 	]+.test1,[ 	]*"aw",[ 	]*%nobits[ 	]*
[ 	]+[0-9]+[ 	]+.*
[ 	]+[0-9]+[ 	]*
[ 	]+[0-9]+[ 	]+.section[ 	]+.test2,[ 	]*"w"[ 	]*
[ 	]+[0-9]+[ 	]+.*
[ 	]+[0-9]+[ 	]*
[ 	]+[0-9]+[ 	]+.section[ 	]+.test3,[ 	]*"aw",[ 	]*%progbits[ 	]*
[ 	]+[0-9]+[ 	]+.*
[ 	]+[0-9]+[ 	]*
[ 	]+[0-9]+[ 	]+.section[ 	]+.test4,[ 	]*"aw"[ 	]*
[ 	]+[0-9]+[ 	]*
[ 	]+[0-9]+[ 	]+.section[ 	]+.data,[ 	]*"a"[ 	]*
[ 	]+[0-9]+[ 	]*
[ 	]+[0-9]+[ 	]+.section[ 	]+.bss,[ 	]*"a"[ 	]*
[ 	]+[0-9]+[ 	]*
[ 	]+[0-9]+[ 	]+.section[ 	]+.data,[ 	]*"aw",[ 	]*%nobits[ 	]*
[ 	]+[0-9]+[ 	]*
[ 	]+[0-9]+[ 	]+.section[ 	]+.bss,[ 	]*"aw",[ 	]*%progbits[ 	]*
