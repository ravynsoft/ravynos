.globl s7a
.globl s7b
.globl s21a
.globl s21b
.globl s10a
.globl s10b
.globl s12a
.globl s12b
.text
.nocmp
	addkpc .S2 s7a,b1,0
	addkpc .S2 s7b,b1,0
	b .S2 s21a
	b .S2 s21b
	bdec .S2 s10a,b1
	bdec .S2 s10b,b1
	bnop .S2 s12a,0
	bnop .S2 s12b,0
