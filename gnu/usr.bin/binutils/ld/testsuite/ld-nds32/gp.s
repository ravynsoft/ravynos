.data
.global mydata
mydata:
	.word 0x11

.text
.global	_start
_start:
	addi.gp $r0, mydata
	lbi.gp $r0, [+mydata]
	lbsi.gp $r0, [+mydata]
	lhi.gp $r0, [+mydata]
	lhsi.gp $r0, [+mydata]
	lwi.gp $r0, [+mydata]
	sbi.gp $r0, [+mydata]
	shi.gp $r0, [+mydata]
	swi.gp $r0, [+mydata]

