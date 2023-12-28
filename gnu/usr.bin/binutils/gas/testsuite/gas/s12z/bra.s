L1:	bra L4
L2:	bsr L1
	bhi L1
L3:	bls L3
	bcc L1
L4:	bcs L2
	bne L3
	beq L4
	bvc L4
	bvs L2
	bpl L1
	bmi L2
	bge L1
	blt L4
	bgt L3
	ble L1
	bhs L2
	blo L2

	bra *+2
	bra *-4
	
