	/* ARMv8.1 System instructions.  */
	.text

	/* RAS Extension.  */
	esb
	hint #0x10

	/* Statistical profiling.  */
	psb csync
	psb CSYNC
	hint #0x11

	/* Trace sync barrier.  */
	tsb csync
	tsb CSYNC
	hint #0x12
