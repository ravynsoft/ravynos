
.*:     file format .*tilegx.*

Contents of section .text:
 100b0 .*
 100c0 .*
 100d0 .*
 100e0 .*
 100f0 .*
 10100 .*
 10110 .*
 10120 .*
 10130 .*
 10140 .*
 10150 .*
 10160 .*
 10170 .*
 10180 .*
 10190 .*
 101a0 .*
 101b0 .*
 101c0 .*
Contents of section .data:
 201e0 000101b8 000101c0 827a4b64 11770000  .*
 201f0 0032002e 2c827a12 34567812 3456789a  .*
 20200 bc123456 789abcde f0000000 00000000  .*
 20210 00000000 00000000 00000000 00000000  .*

Disassembly of section .text:

00000000000100b0 <_start>:
   100b0:	[0-9a-f]* 	{ add r2, zero, zero }
   100b8:	[0-9a-f]* 	{ j 101b8 <external1> }
   100c0:	[0-9a-f]* 	{ add r3, r2, r2 }
   100c8:	[0-9a-f]* 	{ beqzt zero, 101c0 <external2> }
   100d0:	[0-9a-f]* 	{ movei r2, 17 ; movei r3, 119 }
   100d8:	[0-9a-f]* 	{ movei r2, 17 ; movei r3, 119 ; ld zero, zero }
   100e0:	[0-9a-f]* 	{ mtspr 17, zero }
   100e8:	[0-9a-f]* 	{ mfspr zero, 17 }
   100f0:	[0-9a-f]* 	{ moveli r2, -32134 ; moveli r3, 19300 }
   100f8:	[0-9a-f]* 	{ moveli r2, 4660 ; moveli r3, -30293 }
   10100:	[0-9a-f]* 	{ shl16insli r2, r2, 22136 ; shl16insli r3, r3, -12816 }
   10108:	[0-9a-f]* 	{ moveli r2, 4660 ; moveli r3, 30292 }
   10110:	[0-9a-f]* 	{ shl16insli r2, r2, 22136 ; shl16insli r3, r3, 12816 }
   10118:	[0-9a-f]* 	{ shl16insli r2, r2, -25924 ; shl16insli r3, r3, -292 }
   10120:	[0-9a-f]* 	{ moveli r2, 4660 ; moveli r3, -292 }
   10128:	[0-9a-f]* 	{ shl16insli r2, r2, 22136 ; shl16insli r3, r3, -17768 }
   10130:	[0-9a-f]* 	{ shl16insli r2, r2, -25924 ; shl16insli r3, r3, 30292 }
   10138:	[0-9a-f]* 	{ shl16insli r2, r2, -8464 ; shl16insli r3, r3, 12816 }
   10140:	[0-9a-f]* 	{ ld_add r0, r0, 17 }
   10148:	[0-9a-f]* 	{ st_add r0, r0, 17 }
   10150:	[0-9a-f]* 	{ mm r2, r3, 19, 31 }
   10158:	[0-9a-f]* 	{ shli r2, r3, 19 ; shli r4, r5, 31 }
   10160:	[0-9a-f]* 	{ shli r2, r3, 19 ; shli r4, r5, 31 ; ld zero, zero }
   10168:	[0-9a-f]* 	{ moveli r0, 80 ; moveli r1, 80 }
   10170:	[0-9a-f]* 	{ moveli r0, 1 ; moveli r1, 1 }
   10178:	[0-9a-f]* 	{ moveli r0, 168 ; moveli r1, 168 }
   10180:	[0-9a-f]* 	{ moveli r0, 4096 ; moveli r1, 4096 }
   10188:	[0-9a-f]* 	{ moveli r0, 1 ; moveli r1, 1 }
   10190:	[0-9a-f]* 	{ moveli r0, 144 ; moveli r1, 144 }
   10198:	[0-9a-f]* 	{ moveli r0, 4096 ; moveli r1, 4096 }
   101a0:	[0-9a-f]* 	{ moveli r0, 0 ; moveli r1, 0 }
   101a8:	[0-9a-f]* 	{ moveli r0, 1 ; moveli r1, 1 }
   101b0:	[0-9a-f]* 	{ moveli r0, 112 ; moveli r1, 112 }

00000000000101b8 <external1>:
   101b8:	[0-9a-f]* 	{ j 101b8 <external1> }

00000000000101c0 <external2>:
   101c0:	[0-9a-f]* 	{ j 101b8 <external1> }
