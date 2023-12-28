	.text
_start:
	dmsetdmrz	0
	dmmr		1,2
	dmxor		2,3
	dmxxextfdmr512	0,2,4,0
	dmxxextfdmr512	4,6,5,1
	dmxxinstdmr512	6,8,10,0
	dmxxinstdmr512	7,8,10,1
	dmxxextfdmr256	12,0,0
	dmxxextfdmr256	14,1,1
	dmxxextfdmr256	16,2,2
	dmxxextfdmr256	18,3,3
	dmxxinstdmr256	4,20,0
	dmxxinstdmr256	5,22,1
	dmxxinstdmr256	6,24,2
	dmxxinstdmr256	7,26,3
	dmxvi4ger8 0,0,1	# VSRs can now overlap the ACCs
