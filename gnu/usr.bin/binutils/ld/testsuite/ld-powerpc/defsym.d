#source: defsym.s
#as: -a64
#ld: -melf64ppc --defsym bar=foo
#objdump: -Dr

.*:     file format elf64-powerpc.*

Disassembly of section \.text:

0+100000b0 <_start>:
    100000b0:	(48 00 00 15|15 00 00 48) 	bl      100000c4 <(foo|bar)\+0x8>
    100000b4:	(48 00 00 11|11 00 00 48) 	bl      100000c4 <(foo|bar)\+0x8>
    100000b8:	(60 00 00 00|00 00 00 60) 	nop

0+100000bc <(foo|bar)>:
    100000bc:	(3c 40 10 02|02 10 40 3c) 	lis     r2,4098
    100000c0:	(38 42 80 00|00 80 42 38) 	addi    r2,r2,-32768
    100000c4:	(4e 80 00 20|20 00 80 4e) 	blr

Disassembly of section \.data:

0+100100c8 .*:
    100100c8:	(00 00 00 00|bc 00 00 10) 	.*
    100100cc:	(10 00 00 bc|00 00 00 00) 	.*
    100100d0:	(00 00 00 00|bc 00 00 10) 	.*
    100100d4:	(10 00 00 bc|00 00 00 00) 	.*
