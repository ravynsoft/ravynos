	.macro pfx insn:vararg
	.ifdef DATA16
	data16 \insn
	.else
	.ifdef REX64
	rex64 \insn
	.else
	\insn
	.endif
	.endif
	.endm

	.macro pfx16 insn:vararg
	.ifndef REX64
	pfx \insn
	.endif
	.endm

	.macro pfx64 insn:vararg
	.ifndef DATA16
	pfx \insn
	.endif
	.endm

	.text
noreg:
	pfx adc		$1, (%rax)
	pfx adc		$0x89, (%rax)
	pfx adc		$0x1234, (%rax)
	pfx adc		$0x12345678, (%rax)
	pfx add		$1, (%rax)
	pfx add		$0x89, (%rax)
	pfx add		$0x1234, (%rax)
	pfx add		$0x12345678, (%rax)
	pfx and		$1, (%rax)
	pfx and		$0x89, (%rax)
	pfx and		$0x1234, (%rax)
	pfx and		$0x12345678, (%rax)
	pfx bt		$1, (%rax)
	pfx btc		$1, (%rax)
	pfx btr		$1, (%rax)
	pfx bts		$1, (%rax)
	pfx call	*(%rax)
	pfx cmp		$1, (%rax)
	pfx cmp		$0x89, (%rax)
	pfx cmp		$0x1234, (%rax)
	pfx cmp		$0x12345678, (%rax)
	pfx cmps
	pfx cmps	%es:(%rdi), (%rsi)
	pfx crc32	(%rax), %eax
	pfx16 crc32	(%rax), %rax
	pfx dec		(%rax)
	pfx div		(%rax)
	pfx fadd	(%rax)
	pfx fcom	(%rax)
	pfx fcomp	(%rax)
	pfx fdiv	(%rax)
	pfx fdivr	(%rax)
	pfx fiadd	(%rax)
	pfx ficom	(%rax)
	pfx ficomp	(%rax)
	pfx fidiv	(%rax)
	pfx fidivr	(%rax)
	pfx fild	(%rax)
	pfx fimul	(%rax)
	pfx fist	(%rax)
	pfx fistp	(%rax)
	pfx fisttp	(%rax)
	pfx fisub	(%rax)
	pfx fisubr	(%rax)
	pfx fld		(%rax)
	pfx fmul	(%rax)
	pfx fst		(%rax)
	pfx fstp	(%rax)
	pfx fsub	(%rax)
	pfx fsubr	(%rax)
	pfx idiv	(%rax)
	pfx imul	(%rax)
	pfx in		$0
	pfx in		%dx
	pfx inc		(%rax)
	pfx ins
	pfx ins		%dx, %es:(%rdi)
	pfx iret
	pfx jmp		*(%rax)
	pfx lcall	*(%rax)
	pfx lgdt	(%rax)
	pfx lidt	(%rax)
	pfx ljmp	*(%rax)
	pfx lldt	(%rax)
	pfx lmsw	(%rax)
	pfx lods
	pfx lods	(%rsi)
	pfx lret
	pfx lret	$4
	pfx ltr		(%rax)
	pfx mov		$0x12, (%rax)
	pfx mov		$0x1234, (%rax)
	pfx mov		$0x12345678, (%rax)
	pfx mov		%es, (%rax)
	pfx mov		(%rax), %es
	pfx movs
	pfx movs	(%rsi), %es:(%rdi)
	pfx64 movsx	(%rax), %ax
	pfx movsx	(%rax), %eax
	pfx16 movsx	(%rax), %rax
	pfx64 movzx	(%rax), %ax
	pfx movzx	(%rax), %eax
	pfx16 movzx	(%rax), %rax
	pfx mul		(%rax)
	pfx neg		(%rax)
	pfx nop		(%rax)
	pfx not		(%rax)
	pfx or		$1, (%rax)
	pfx or		$0x89, (%rax)
	pfx or		$0x1234, (%rax)
	pfx or		$0x12345678, (%rax)
	pfx out		$0
	pfx out		%dx
	pfx outs
	pfx outs	(%rsi), %dx
	pfx pop		(%rax)
	pfx pop		%fs
	pfx64 ptwrite	(%rax)
	pfx push	(%rax)
	pfx push	%fs
	pfx rcl		$1, (%rax)
	pfx rcl		$2, (%rax)
	pfx rcl		%cl, (%rax)
	pfx rcl		(%rax)
	pfx rcr		$1, (%rax)
	pfx rcr		$2, (%rax)
	pfx rcr		%cl, (%rax)
	pfx rcr		(%rax)
	pfx rol		$1, (%rax)
	pfx rol		$2, (%rax)
	pfx rol		%cl, (%rax)
	pfx rol		(%rax)
	pfx ror		$1, (%rax)
	pfx ror		$2, (%rax)
	pfx ror		%cl, (%rax)
	pfx ror		(%rax)
	pfx sbb		$1, (%rax)
	pfx sbb		$0x89, (%rax)
	pfx sbb		$0x1234, (%rax)
	pfx sbb		$0x12345678, (%rax)
	pfx scas
	pfx scas	%es:(%rdi)
	pfx sal		$1, (%rax)
	pfx sal		$2, (%rax)
	pfx sal		%cl, (%rax)
	pfx sal		(%rax)
	pfx sar		$1, (%rax)
	pfx sar		$2, (%rax)
	pfx sar		%cl, (%rax)
	pfx sar		(%rax)
	pfx shl	$1, (%rax)
	pfx shl	$2, (%rax)
	pfx shl	%cl, (%rax)
	pfx shl	(%rax)
	pfx shr	$1, (%rax)
	pfx shr	$2, (%rax)
	pfx shr	%cl, (%rax)
	pfx shr	(%rax)
	pfx stos
	pfx stos	%es:(%rdi)
	pfx sub	$1, (%rax)
	pfx sub	$0x89, (%rax)
	pfx sub	$0x1234, (%rax)
	pfx sub	$0x12345678, (%rax)
	pfx sysexit
	pfx sysret
	pfx test	$0x89, (%rax)
	pfx test	$0x1234, (%rax)
	pfx test	$0x12345678, (%rax)
	pfx xor	$1, (%rax)
	pfx xor	$0x89, (%rax)
	pfx xor	$0x1234, (%rax)
	pfx xor	$0x12345678, (%rax)
