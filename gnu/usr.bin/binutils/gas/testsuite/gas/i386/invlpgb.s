# Check INVLPGB instructions

	.text
_start:
        invlpgb
.ifdef x86_64
att64:
        invlpgb %rax, %ecx, %edx
.endif
att32:
        invlpgb %eax, %ecx, %edx
.ifndef x86_64
att16:
        invlpgb %ax, %ecx, %edx
.endif

	.intel_syntax noprefix
.ifdef x86_64
intel64:
        invlpgb rax, ecx, edx
.endif
intel32:
        invlpgb eax, ecx, edx
.ifndef x86_64
intel16:
        invlpgb ax, ecx, edx
.endif
