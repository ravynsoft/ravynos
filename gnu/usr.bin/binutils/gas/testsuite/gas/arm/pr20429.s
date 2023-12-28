.syntax unified
.arm

.text
	vpush {d0-d31}		// 32 > 16, not catched.
	vpush {d1-d17}		// 17 > 16, not catched.
	vpop {d1-d17}		// 17 > 16, not catched.
	vpop {d0-d31}		// 32 > 16, not catched.

	vpush {q0-q15}		// 32 > 16, not catched.
	vpush {q1-q9}		// 18 > 16, not catched.
	vpop {q1-q9}		// 18 > 16, not catched.
	vpop {q0-q15}		// 32 > 16, not catched.
