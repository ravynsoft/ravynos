// Test -march=armv8.2-a+fp16+nofp to disable fp, fp16 and simd.
.text
	fneg s0, s1
	fneg h0, h1
	fneg v0.4s, v1.4s
	fneg v0.8h, v1.8h
	neg v0.16b, v1.16b
