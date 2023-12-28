# Check 64bit PREFETCHI instructions

	.allow_index_reg
	.text
_start:

        prefetchit0     0x12345678(%rip)
        prefetchit1     0x12345678(%rip)

        .intel_syntax noprefix

        prefetchit0     BYTE PTR [rip+0x12345678]
        prefetchit1     BYTE PTR [rip+0x12345678]

