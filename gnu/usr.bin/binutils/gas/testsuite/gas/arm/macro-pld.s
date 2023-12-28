.macro foo arg, rest:vararg
	\rest
.endm
	foo r0, pld [r0]
	foo r0, push {r0}
