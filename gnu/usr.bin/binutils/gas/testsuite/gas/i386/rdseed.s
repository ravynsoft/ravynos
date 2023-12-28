# Check RdSeed instruction.

	.text
foo:
        rdseed    %ax
        rdseed    %eax

	.intel_syntax noprefix
        rdseed    bx
        rdseed    ebx
