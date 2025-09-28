
.*:     file format elf32-tilepro.*

Contents of section .text:
 10078 .*
 10088 .*
 10098 .*
 100a8 .*
 100b8 .*
 100c8 .*
 100d8 .*
 100e8 .*
 100f8 .*
 10108 .*
 10118 .*
 10128 .*
Contents of section .data:
 20140 20010100 28010100 7a82644b 11773200  .*
 20150 00002e00 2c214398 ba6587dc fe6587dd  .*
 20160 fe000000 00000000 00000000 00000000  .*
 20170 00000000 00000000 00000000 00000000  .*
Disassembly of section .text:

00010078 <_start>:
   10078:	[0-9a-f]* 	{ add r2, zero, zero }
   10080:	[0-9a-f]* 	{ j 10120 <external1> }
   10088:	[0-9a-f]* 	{ add r3, r2, r2 }
   10090:	[0-9a-f]* 	{ bzt zero, 10128 <external2> }
   10098:	[0-9a-f]* 	{ movei r2, 17 ; movei r3, 119 }
   100a0:	[0-9a-f]* 	{ movei r2, 17 ; movei r3, 119 ; lw zero, zero }
   100a8:	[0-9a-f]* 	{ mtspr 17, zero }
   100b0:	[0-9a-f]* 	{ mfspr zero, 17 }
   100b8:	[0-9a-f]* 	{ moveli r2, -32134 ; moveli r3, 19300 }
   100c0:	[0-9a-f]* 	{ moveli r2, 17185 ; moveli r3, -17768 }
   100c8:	[0-9a-f]* 	{ addli r2, r2, -30875 ; addli r3, r3, -292 }
   100d0:	[0-9a-f]* 	{ auli r2, r2, -30875 ; auli r3, r3, -291 }
   100d8:	[0-9a-f]* 	{ swadd r0, r0, 17 }
   100e0:	[0-9a-f]* 	{ mm r2, r3, r4, 19, 31 }
   100e8:	[0-9a-f]* 	{ nop ; mm r5, r6, r7, 19, 31 }
   100f0:	[0-9a-f]* 	{ shli r2, r3, 19 ; shli r4, r5, 31 }
   100f8:	[0-9a-f]* 	{ shli r2, r3, 19 ; shli r4, r5, 31 ; lw zero, zero }
   10100:	[0-9a-f]* 	{ moveli r0, 32 }
   10108:	[0-9a-f]* 	{ moveli r0, 120 }
   10110:	[0-9a-f]* 	{ moveli r0, 1 }
   10118:	[0-9a-f]* 	{ moveli r0, 1 }


00010120 <external1>:
   10120:	[0-9a-f]* 	{ j 10120 <external1> }

00010128 <external2>:
   10128:	[0-9a-f]* 	{ j 10120 <external1> }
