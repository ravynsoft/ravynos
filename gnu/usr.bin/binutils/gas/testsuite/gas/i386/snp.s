# Check SNP instructions

	.text
att:
        pvalidate
        pvalidate %eax, %ecx, %edx
.ifdef x86_64
        pvalidate %rax, %ecx, %edx
        psmash
        psmash	%rax
        psmash	%eax
        rmpupdate
        rmpupdate %rax, %rcx
        rmpupdate %eax, %rcx
        rmpadjust
        rmpadjust %rax, %rcx, %rdx
        rmpadjust %eax, %rcx, %rdx
.else
        pvalidate %ax, %ecx, %edx
.endif

	.intel_syntax noprefix
intel:
        pvalidate
        pvalidate eax, ecx, edx
.ifdef x86_64
        pvalidate rax, ecx, edx
        psmash
        psmash	rax
        psmash	eax
        rmpupdate
        rmpupdate rax, rcx
        rmpupdate eax, rcx
        rmpadjust
        rmpadjust rax, rcx, rdx
        rmpadjust eax, rcx, rdx
.else
        pvalidate ax, ecx, edx
.endif
