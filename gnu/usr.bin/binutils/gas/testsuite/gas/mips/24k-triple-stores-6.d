#objdump: -dr
#as: -mfix-24k -32 -EB
#name: 24K: Triple Store (Store Macro Check)

.*: +file format .*mips.*

Disassembly of section .text:
0+ <.*>:
   0:	abbf0050 	swl	ra,80\(sp\)
   4:	bbbf0053 	swr	ra,83\(sp\)
   8:	abb30058 	swl	s3,88\(sp\)
   c:	bbb3005b 	swr	s3,91\(sp\)
  10:	abbe0060 	swl	s8,96\(sp\)
  14:	bbbe0063 	swr	s8,99\(sp\)
  18:	0000000d 	break
  1c:	a3bf0051 	sb	ra,81\(sp\)
  20:	001f0a02 	srl	at,ra,0x8
  24:	a3a10050 	sb	at,80\(sp\)
  28:	a3b30059 	sb	s3,89\(sp\)
  2c:	00130a02 	srl	at,s3,0x8
  30:	a3a10058 	sb	at,88\(sp\)
  34:	a3be0061 	sb	s8,97\(sp\)
  38:	001e0a02 	srl	at,s8,0x8
  3c:	a3a10060 	sb	at,96\(sp\)
  40:	0000000d 	break
  44:	e7a00050 	swc1	\$f0,80\(sp\)
  48:	e7a20058 	swc1	\$f2,88\(sp\)
  4c:	00000000 	nop
  50:	e7a40060 	swc1	\$f4,96\(sp\)
  54:	0000000d 	break
  58:	f7a00050 	sdc1	\$f0,80\(sp\)
  5c:	f7a20058 	sdc1	\$f2,88\(sp\)
  60:	00000000 	nop
  64:	f7a40060 	sdc1	\$f4,96\(sp\)
  68:	0000000d 	break
	\.\.\.
