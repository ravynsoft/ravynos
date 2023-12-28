# Based on PR 16688 this test checks that bignums are correctly added to the literal pool
	.text

	ldr q0, =0x5a8279995a8279995a8279995a827999
	ldr q1, =0x6ed9eba16ed9eba16ed9eba16ed9eba1
	ldr q2, =0x8f1bbcdc8f1bbcdc8f1bbcdc8f1bbcdc
	ldr q3, =0xca62c1d6ca62c1d6ca62c1d6ca62c1d6
