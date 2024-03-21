#as: -mcpu=archs
#objdump: -d

.*: +file format .*arc.*


Disassembly of section .text:

00000000 <.text>:
   0:	226f 003f           	swi
   4:	7ae0                	swi_s
   6:	276f 003f           	clri	0
   a:	266f 003f           	seti	0
   e:	246f 003f           	rtie
  12:	216f 003f           	sleep	0
  16:	226f 103f           	dsync
  1a:	264a 7000           	nop
  1e:	78e0                	nop_s
  20:	256f 003f           	brk
  24:	236f 003f           	sync
  28:	7fff                	brk_s
  2a:	79e0                	unimp_s
