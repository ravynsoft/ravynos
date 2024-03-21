	.arch armv8.5-a+memtag

	msr TCO, #-1
	msr TCO, #2
	msr TCO, #15
	msr TCO, #0x100000000

	msr daifclr, #-1
	msr daifclr, #16
	msr daifclr, #0x200000000

	msr daifset, #-1
	msr daifset, #16
	msr daifset, #0x200000000
