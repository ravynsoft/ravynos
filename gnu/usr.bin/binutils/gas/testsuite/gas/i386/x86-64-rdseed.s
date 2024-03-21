# Check 64-bit new RdSeed instruction.

	.text
foo:
        rdseed    %ax
        rdseed    %eax
        rdseed    %rax
        rdseed    %r11w
        rdseed    %r11d
        rdseed    %r11

	.intel_syntax noprefix
        rdseed    bx
        rdseed    ebx
        rdseed    rbx
        rdseed    r11w
        rdseed    r11d
        rdseed    r11
