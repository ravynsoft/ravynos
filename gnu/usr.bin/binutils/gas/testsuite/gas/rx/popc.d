#source: ./popc.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	7e e0                         	popc	psw
   2:	7e e3                         	popc	fpsw
   4:	7e e2                         	popc	usp
   6:	7e ea                         	popc	isp
   8:	7e ec                         	popc	intb
   a:	7e e8                         	popc	bpsw
   c:	7e e9                         	popc	bpc
   e:	7e eb                         	popc	fintv
  10:	7e ed                         	popc	extb

