#objdump: -dr
#as: -mfix-24k -32 -EB
#name: 24K: Triple Store (Store Macro Check)
#source: 24k-triple-stores-6.s

.*: +file format .*mips.*

Disassembly of section \.text:

[0-9a-f]+ <.*>:
 *[0-9a-f]+:	63fd 8050 	swl	ra,80\(sp\)
 *[0-9a-f]+:	63fd 9053 	swr	ra,83\(sp\)
 *[0-9a-f]+:	627d 8058 	swl	s3,88\(sp\)
 *[0-9a-f]+:	627d 905b 	swr	s3,91\(sp\)
 *[0-9a-f]+:	63dd 8060 	swl	s8,96\(sp\)
 *[0-9a-f]+:	63dd 9063 	swr	s8,99\(sp\)
 *[0-9a-f]+:	4680      	break
 *[0-9a-f]+:	1bfd 0051 	sb	ra,81\(sp\)
 *[0-9a-f]+:	003f 4040 	srl	at,ra,0x8
 *[0-9a-f]+:	183d 0050 	sb	at,80\(sp\)
 *[0-9a-f]+:	1a7d 0059 	sb	s3,89\(sp\)
 *[0-9a-f]+:	0033 4040 	srl	at,s3,0x8
 *[0-9a-f]+:	183d 0058 	sb	at,88\(sp\)
 *[0-9a-f]+:	1bdd 0061 	sb	s8,97\(sp\)
 *[0-9a-f]+:	003e 4040 	srl	at,s8,0x8
 *[0-9a-f]+:	183d 0060 	sb	at,96\(sp\)
 *[0-9a-f]+:	4680      	break
 *[0-9a-f]+:	981d 0050 	swc1	\$f0,80\(sp\)
 *[0-9a-f]+:	985d 0058 	swc1	\$f2,88\(sp\)
 *[0-9a-f]+:	989d 0060 	swc1	\$f4,96\(sp\)
 *[0-9a-f]+:	4680      	break
 *[0-9a-f]+:	b81d 0050 	sdc1	\$f0,80\(sp\)
 *[0-9a-f]+:	b85d 0058 	sdc1	\$f2,88\(sp\)
 *[0-9a-f]+:	b89d 0060 	sdc1	\$f4,96\(sp\)
 *[0-9a-f]+:	4680      	break
	\.\.\.
