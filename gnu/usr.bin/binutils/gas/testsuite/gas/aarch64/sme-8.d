#name: SME mode selection and state access instructions
#as: -march=armv8-a+sme
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	d53b4240 	mrs	x0, svcr
   4:	d51b4240 	msr	svcr, x0
   8:	d503427f 	smstop	sm
   c:	d503447f 	smstop	za
  10:	d503467f 	smstop
  14:	d503437f 	smstart	sm
  18:	d503457f 	smstart	za
  1c:	d503477f 	smstart
  20:	d503477f 	smstart
  24:	d503437f 	smstart	sm
  28:	d503457f 	smstart	za
  2c:	d503437f 	smstart	sm
  30:	d503457f 	smstart	za
  34:	d503467f 	smstop
  38:	d503427f 	smstop	sm
  3c:	d503447f 	smstop	za
  40:	d503427f 	smstop	sm
  44:	d503447f 	smstop	za
