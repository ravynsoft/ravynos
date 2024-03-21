#as:-mcpu=arcem -mdpfp
#objdump: -dr -M dpfp
#source: tfpx.s

.*: +file format .*arc.*


Disassembly of section .text:

00000000 <.text>:
   0:	24aa 008c           	lr	r4,\[770\]
   4:	23aa 004c           	lr	r3,\[769\]
   8:	24aa 010c           	lr	r4,\[772\]
   c:	23aa 00cc           	lr	r3,\[771\]
  10:	320c 00c1           	daddh11	r1,r2,r3
  14:	320d 00c1           	daddh12	r1,r2,r3
  18:	320e 00c1           	daddh21	r1,r2,r3
  1c:	320f 00c1           	daddh22	r1,r2,r3
  20:	3218 00c1           	dexcl1	r1,r2,r3
  24:	3219 00c1           	dexcl2	r1,r2,r3
  28:	3208 00c1           	dmulh11	r1,r2,r3
  2c:	3209 00c1           	dmulh12	r1,r2,r3
  30:	320a 00c1           	dmulh21	r1,r2,r3
  34:	320b 00c1           	dmulh22	r1,r2,r3
  38:	3210 00c1           	dsubh11	r1,r2,r3
  3c:	3211 00c1           	dsubh12	r1,r2,r3
  40:	3212 00c1           	dsubh21	r1,r2,r3
  44:	3213 00c1           	dsubh22	r1,r2,r3
