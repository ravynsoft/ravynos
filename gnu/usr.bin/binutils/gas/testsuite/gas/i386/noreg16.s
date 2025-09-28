	.macro pfx insn:vararg
	.ifdef DATA32
	data32 \insn
	.else
	\insn
	.endif
	.endm

	.text
	.code16
noreg:
	pfx adc		$1, (%bx)
	pfx adc		$0x89, (%bx)
	pfx adc		$0x1234, (%bx)
	pfx add		$1, (%bx)
	pfx add		$0x89, (%bx)
	pfx add		$0x1234, (%bx)
	pfx and		$1, (%bx)
	pfx and		$0x89, (%bx)
	pfx and		$0x1234, (%bx)
	pfx bt		$1, (%bx)
	pfx btc		$1, (%bx)
	pfx btr		$1, (%bx)
	pfx bts		$1, (%bx)
	pfx call	*(%bx)
	pfx cmp		$1, (%bx)
	pfx cmp		$0x89, (%bx)
	pfx cmp		$0x1234, (%bx)
	pfx cmps
	pfx cmps	%es:(%di), (%si)
	pfx crc32	(%bx), %eax
	cvtsi2sd	(%bx), %xmm0
	cvtsi2ss	(%bx), %xmm0
	pfx dec		(%bx)
	pfx div		(%bx)
	pfx fadd	(%bx)
	pfx fcom	(%bx)
	pfx fcomp	(%bx)
	pfx fdiv	(%bx)
	pfx fdivr	(%bx)
	pfx fiadd	(%bx)
	pfx ficom	(%bx)
	pfx ficomp	(%bx)
	pfx fidiv	(%bx)
	pfx fidivr	(%bx)
	pfx fild	(%bx)
	pfx fimul	(%bx)
	pfx fist	(%bx)
	pfx fistp	(%bx)
	pfx fisttp	(%bx)
	pfx fisub	(%bx)
	pfx fisubr	(%bx)
	pfx fld		(%bx)
	pfx fmul	(%bx)
	pfx fst		(%bx)
	pfx fstp	(%bx)
	pfx fsub	(%bx)
	pfx fsubr	(%bx)
	pfx idiv	(%bx)
	pfx imul	(%bx)
	pfx in		$0
	pfx in		%dx
	pfx inc		(%bx)
	pfx ins
	pfx ins		%dx, %es:(%di)
	pfx jmp		*(%bx)
	pfx lgdt	(%bx)
	pfx lidt	(%bx)
	pfx lldt	(%bx)
	pfx lmsw	(%bx)
	pfx lods
	pfx lods	(%si)
	pfx ltr		(%bx)
	pfx mov		$0x12, (%bx)
	pfx mov		$0x1234, (%bx)
	pfx mov		%es, (%bx)
	pfx mov		(%bx), %es
	pfx movs
	pfx movs	(%si), %es:(%di)
	pfx movsx	(%bx), %ax
	movsx		(%bx), %eax
	pfx movzx	(%bx), %ax
	movzx		(%bx), %eax
	pfx mul		(%bx)
	pfx neg		(%bx)
	pfx nop		(%bx)
	pfx not		(%bx)
	pfx or		$1, (%bx)
	pfx or		$0x89, (%bx)
	pfx or		$0x1234, (%bx)
	pfx out		$0
	pfx out		%dx
	pfx outs
	pfx outs	(%si), %dx
	pfx pop		(%bx)
	pfx pop		%es
	ptwrite		(%bx)
	pfx push	(%bx)
	pfx push	%es
	pfx rcl		$1, (%bx)
	pfx rcl		$2, (%bx)
	pfx rcl		%cl, (%bx)
	pfx rcl		(%bx)
	pfx rcr		$1, (%bx)
	pfx rcr		$2, (%bx)
	pfx rcr		%cl, (%bx)
	pfx rcr		(%bx)
	pfx rol		$1, (%bx)
	pfx rol		$2, (%bx)
	pfx rol		%cl, (%bx)
	pfx rol		(%bx)
	pfx ror		$1, (%bx)
	pfx ror		$2, (%bx)
	pfx ror		%cl, (%bx)
	pfx ror		(%bx)
	pfx sbb		$1, (%bx)
	pfx sbb		$0x89, (%bx)
	pfx sbb		$0x1234, (%bx)
	pfx scas
	pfx scas	%es:(%di)
	pfx sal		$1, (%bx)
	pfx sal		$2, (%bx)
	pfx sal		%cl, (%bx)
	pfx sal		(%bx)
	pfx sar		$1, (%bx)
	pfx sar		$2, (%bx)
	pfx sar		%cl, (%bx)
	pfx sar		(%bx)
	pfx shl		$1, (%bx)
	pfx shl		$2, (%bx)
	pfx shl		%cl, (%bx)
	pfx shl		(%bx)
	pfx shr		$1, (%bx)
	pfx shr		$2, (%bx)
	pfx shr		%cl, (%bx)
	pfx shr		(%bx)
	pfx stos
	pfx stos	%es:(%di)
	pfx sub		$1, (%bx)
	pfx sub		$0x89, (%bx)
	pfx sub		$0x1234, (%bx)
	pfx test	$0x89, (%bx)
	pfx test	$0x1234, (%bx)
	vcvtsi2sd	(%bx), %xmm0, %xmm0
	{evex} vcvtsi2sd (%bx), %xmm0, %xmm0
	vcvtsi2ss	(%bx), %xmm0, %xmm0
	{evex} vcvtsi2ss (%bx), %xmm0, %xmm0
	vcvtusi2sd	(%bx), %xmm0, %xmm0
	vcvtusi2ss	(%bx), %xmm0, %xmm0
	pfx xor		$1, (%bx)
	pfx xor		$0x89, (%bx)
	pfx xor		$0x1234, (%bx)
