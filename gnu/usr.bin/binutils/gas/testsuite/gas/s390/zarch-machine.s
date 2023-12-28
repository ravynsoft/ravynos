.text
foo:
	ag	%r9,4095(%r5,%r10)
.machine push
.machine z10
	asi	5555(%r6),-42
.machine pop
	agf	%r9,4095(%r5,%r10)
