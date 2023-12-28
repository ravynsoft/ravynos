#objdump: -d

.*: +file format .*arc.*


Disassembly of section .text:

00000000 <.text>:
   0:	246f 003f           	rtie
   4:	216f 003f           	sleep	0
   8:	78e0                	nop_s
   a:	256f 003f           	brk
   e:	236f 003f           	sync
  12:	226f 003f           	trap0
  16:	7fff                	brk_s
  18:	79e0                	unimp_s
  1a:	366f 701a           	rtsc	0,0
