.text
.nocmp
	nop
	bnop .S2 f3,0
	bnop .S2 f4,0
	bdec .S2 f3,b1
	bdec .S2 f4,b1
	b .S2 f3
	b .S2 f4
	addkpc .S2 f3,b1,0
	addkpc .S2 f4,b1,0
.section .text.1,"ax",@progbits
f3:
	nop
f4:
	nop
