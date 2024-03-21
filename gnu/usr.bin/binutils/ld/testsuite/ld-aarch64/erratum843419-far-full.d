#source: erratum843419-far.s
#as:
#ld: -Ttext=0x400000 --fix-cortex-a53-843419=full
#objdump: -dr
#...

Disassembly of section \.text:

0*400000 <_start>:
	...
  400ffc:	90400000 	adrp	x0, 80400000 <__bss_end__\+0x7ffedff0>
  401000:	f9000042 	str	x2, \[x2\]
  401004:	d2800002 	mov	x2, #0x0                   	// #0
  401008:	14000004 	b	401018 <e843419@0002_00000010_1008>
  40100c:	d503201f 	nop
  401010:	14000400 	b	402010 <e843419@0002_00000010_1008\+0xff8>
  401014:	d503201f 	nop

0*401018 <e843419@0002_00000010_1008>:
  401018:	f9402001 	ldr	x1, \[x0, #64\]
  40101c:	17fffffc 	b	40100c <_start\+0x100c>
	...
