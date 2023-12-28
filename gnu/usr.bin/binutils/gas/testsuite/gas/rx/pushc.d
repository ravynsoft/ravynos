#source: ./pushc.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	7e c0                         	pushc	psw
   2:	7e c3                         	pushc	fpsw
   4:	7e c2                         	pushc	usp
   6:	7e ca                         	pushc	isp
   8:	7e cc                         	pushc	intb
   a:	7e c8                         	pushc	bpsw
   c:	7e c9                         	pushc	bpc
   e:	7e cb                         	pushc	fintv
  10:	7e cd                         	pushc	extb

