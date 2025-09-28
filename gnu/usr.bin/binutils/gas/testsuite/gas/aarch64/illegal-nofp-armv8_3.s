// Test -march=armv8.3-a+nofp to disable fp-dependent ARMv8.3 instructions.
.text
	neg w0, w1
	fneg s0, s1
	pacia x0, x1
	fjcvtzs w0, d1
