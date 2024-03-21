.text
.nocmp
	nop
	addkpc .S2 f1,b1,0
	addkpc .S2 f2,b1,0
	b .S2 f1
	b .S2 f2
	bdec .S2 f1,b1
	bdec .S2 f2,b1
	bnop .S2 f1,0
	bnop .S2 f2,0
.section .text.1,"ax",@progbits
f1:
	nop
f2:
	nop
