#source: ppc476-shared.s
#as: -a32
#ld: -melf32ppc -q -shared -z common-page-size=0x10000 -z notext --ppc476-workaround -T ppc476-shared.lnk
#objdump: -dr
#target: powerpc*-*-*

.*:     file format .*

Disassembly of section \.text:

0000fffc <\.text>:
    fffc:	(48 03 00 04|04 00 03 48) 	b       40000 .*
   10000:	(38 63 00 00|00 00 63 38) 	addi    r3,r3,0
			1000[02]: R_PPC_ADDR16_LO	\.bss
	\.\.\.
   1fffc:	(48 02 00 14|14 00 02 48) 	b       40010 .*
   20000:	(38 63 00 00|00 00 63 38) 	addi    r3,r3,0
			2000[02]: R_PPC_ADDR16_LO	\.bss
	\.\.\.
   2fffc:	(48 01 00 24|24 00 01 48) 	b       40020 .*
   30000:	(38 63 00 00|00 00 63 38) 	addi    r3,r3,0
			3000[02]: R_PPC_ADDR16_LO	\.bss
	\.\.\.
   3fff0:	(42 9f 00 05|05 00 9f 42) 	bcl     .*
   3fff4:	(7d 28 02 a6|a6 02 28 7d) 	mflr    r9
   3fff8:	(3d 29 00 01|01 00 29 3d) 	addis   r9,r9,1
			3fff[8a]: R_PPC_REL16_HA	\.bss\+0x[46]
   3fffc:	(48 00 00 34|34 00 00 48) 	b       40030 .*
   40000:	(3c 60 00 00|00 00 60 3c) 	lis     r3,0
			4000[02]: R_PPC_ADDR16_HA	\.bss
   40004:	(4b fc ff fc|fc ff fc 4b) 	b       10000 .*
   40008:	(48 00 00 02|02 00 00 48) 	ba      0 .*
   4000c:	(48 00 00 02|02 00 00 48) 	ba      0 .*
   40010:	(3c 60 00 00|00 00 60 3c) 	lis     r3,0
			4001[02]: R_PPC_ADDR16_HA	\.bss
   40014:	(4b fd ff ec|ec ff fd 4b) 	b       20000 .*
   40018:	(48 00 00 02|02 00 00 48) 	ba      0 .*
   4001c:	(48 00 00 02|02 00 00 48) 	ba      0 .*
   40020:	(3c 60 00 00|00 00 60 3c) 	lis     r3,0
			4002[02]: R_PPC_ADDR16_HA	\.bss
   40024:	(4b fe ff dc|dc ff fe 4b) 	b       30000 .*
   40028:	(48 00 00 02|02 00 00 48) 	ba      0 .*
   4002c:	(48 00 00 02|02 00 00 48) 	ba      0 .*
   40030:	(39 29 00 0c|0c 00 29 39) 	addi    r9,r9,12
			4003[02]: R_PPC_REL16_LO	\.bss\+0x3[ce]
   40034:	(4b ff ff cc|cc ff ff 4b) 	b       40000 .*
   40038:	(48 00 00 02|02 00 00 48) 	ba      0 .*
   4003c:	(48 00 00 02|02 00 00 48) 	ba      0 .*
