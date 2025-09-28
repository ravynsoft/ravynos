# Check RMPQUERY instruction

	.text
att:
        rmpquery
        rmpquery %rax, %rcx, %rdx
        rmpquery %eax, %rcx, %rdx

	.intel_syntax noprefix
intel:
        rmpquery
        rmpquery rax, rcx, rdx
        rmpquery eax, rcx, rdx
