#as:-mcpu=arcem -mfpuda
#objdump: -dr -M fpuda
#source: tfpx.s

.*: +file format .*arc.*


Disassembly of section .text:

00000000 <.text>:
   0:	24aa 00cc           	lr	r4,\[771\]
   4:	23aa 008c           	lr	r3,\[770\]
   8:	24aa 014c           	lr	r4,\[773\]
   c:	23aa 010c           	lr	r3,\[772\]
  10:	3234 00c1           	daddh11	r1,r2,r3
  14:	3235 00c1           	daddh12	r1,r2,r3
  18:	3236 00c1           	daddh21	r1,r2,r3
  1c:	3237 00c1           	daddh22	r1,r2,r3
  20:	323c 00c1           	dexcl1	r1,r2,r3
  24:	323d 00c1           	dexcl2	r1,r2,r3
  28:	3230 00c1           	dmulh11	r1,r2,r3
  2c:	3231 00c1           	dmulh12	r1,r2,r3
  30:	3232 00c1           	dmulh21	r1,r2,r3
  34:	3233 00c1           	dmulh22	r1,r2,r3
  38:	3238 00c1           	dsubh11	r1,r2,r3
  3c:	3239 00c1           	dsubh12	r1,r2,r3
  40:	323a 00c1           	dsubh21	r1,r2,r3
  44:	323b 00c1           	dsubh22	r1,r2,r3
