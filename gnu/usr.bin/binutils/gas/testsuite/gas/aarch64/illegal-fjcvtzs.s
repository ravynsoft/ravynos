// Test illegal ARMv8.3 FJCVTZS instructions
.text

	// Good.
	fjcvtzs w0, d1

	// Bad.
	fjcvtzs d0, d1
	fjcvtzs s0, d1
	fjcvtzs x0, d1
	fjcvtzs w0, s1
	fjcvtzs w0, h1
	fjcvtzs w0, q1
	fjcvtzs w0, x1
