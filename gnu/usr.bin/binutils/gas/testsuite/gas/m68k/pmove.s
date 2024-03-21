# Test handling of the 68030/68851 pmove instructions.
	pmove %psr,%a0@
	pmove %a1@,%psr
	pmove %pcsr,%a2@
	pmove %bad0,%a3@
	pmove %a4@,%bad1
