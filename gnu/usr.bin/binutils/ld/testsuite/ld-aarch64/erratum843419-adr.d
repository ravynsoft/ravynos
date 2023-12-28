#source: erratum843419-near.s
#as:
#ld: -Ttext=0x400000 --fix-cortex-a53-843419=adr
#objdump: -dr
#...

Disassembly of section \.text:

0*400000 <_start>:
	...
  400ffc:	10038020 	adr	x0, 408000 <_start\+0x8000>
  401000:	f9000042 	str	x2, \[x2\]
  401004:	d2800002 	mov	x2, #0x0                   	// #0
  401008:	f9402001 	ldr	x1, \[x0, #64\]
  40100c:	d503201f 	nop
