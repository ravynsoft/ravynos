# as: -EB
# objdump: -dr

.* file format .*

Disassembly of section \.text:

0+000000 <f1>:
   0:	4a01      	addiu	v0,1
   2:	6500      	nop
   4:	6500      	nop
   6:	6500      	nop
   8:	4b01      	addiu	v1,1
   a:	6500      	nop

0+00000c <f2>:
   c:	24420001 	addiu	v0,v0,1
  10:	24630001 	addiu	v1,v1,1
	\.\.\.
  20:	24840001 	addiu	a0,a0,1
  24:	00000000 	nop

0+000028 <f3>:
  28:	4001      	addiu	s0,s0,1
  2a:	6500      	nop
  2c:	6500      	nop
  2e:	6500      	nop
  30:	6500      	nop
  32:	6500      	nop
  34:	6500      	nop
  36:	6500      	nop
  38:	6500      	nop
  3a:	6500      	nop
  3c:	6500      	nop
  3e:	6500      	nop

Disassembly of section \.text\.a:

0+000000 <f4>:
   0:	24a50001 	addiu	a1,a1,1
	\.\.\.
