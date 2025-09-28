	lsr   d0, d3, d5
	asr   d1, d4, #13
	asr   d1, d0, #1
	lsl.b d7, (-s), #2
	lsl.w d7, (-s), #2
	lsl.l d7, (-s), #2
	lsr.p d7, (-s), #2
	lsr.w (+y), #2
	lsr.p (d6,x), #2
	asl   d7, #1
	asr   d1, #2
	asl   d6, d6, #17
	asl   d6, d6, #16
