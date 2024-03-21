	.macro pfx insn:vararg
	.ifdef DATA16
	data16 \insn
	.else
	\insn
	.endif
	.endm

	.text
noreg:
	pfx adc		$1, (%eax)
	pfx adc		$0x89, (%eax)
	pfx adc		$0x1234, (%eax)
	pfx adc		$0x12345678, (%eax)
	pfx add		$1, (%eax)
	pfx add		$0x89, (%eax)
	pfx add		$0x1234, (%eax)
	pfx add		$0x12345678, (%eax)
	pfx and		$1, (%eax)
	pfx and		$0x89, (%eax)
	pfx and		$0x1234, (%eax)
	pfx and		$0x12345678, (%eax)
	pfx bt		$1, (%eax)
	pfx btc		$1, (%eax)
	pfx btr		$1, (%eax)
	pfx bts		$1, (%eax)
	pfx call	*(%eax)
	pfx cmp		$1, (%eax)
	pfx cmp		$0x89, (%eax)
	pfx cmp		$0x1234, (%eax)
	pfx cmp		$0x12345678, (%eax)
	pfx cmps
	pfx cmps	%es:(%edi), (%esi)
	pfx crc32	(%eax), %eax
	cvtsi2sd	(%eax), %xmm0
	cvtsi2ss	(%eax), %xmm0
	pfx dec		(%eax)
	pfx div		(%eax)
	pfx fadd	(%eax)
	pfx fcom	(%eax)
	pfx fcomp	(%eax)
	pfx fdiv	(%eax)
	pfx fdivr	(%eax)
	pfx fiadd	(%eax)
	pfx ficom	(%eax)
	pfx ficomp	(%eax)
	pfx fidiv	(%eax)
	pfx fidivr	(%eax)
	pfx fild	(%eax)
	pfx fimul	(%eax)
	pfx fist	(%eax)
	pfx fistp	(%eax)
	pfx fisttp	(%eax)
	pfx fisub	(%eax)
	pfx fisubr	(%eax)
	pfx fld		(%eax)
	pfx fmul	(%eax)
	pfx fst		(%eax)
	pfx fstp	(%eax)
	pfx fsub	(%eax)
	pfx fsubr	(%eax)
	pfx idiv	(%eax)
	pfx imul	(%eax)
	pfx in		$0
	pfx in		%dx
	pfx inc		(%eax)
	pfx ins
	pfx ins		%dx, %es:(%edi)
	pfx jmp		*(%eax)
	pfx lgdt	(%eax)
	pfx lidt	(%eax)
	pfx lldt	(%eax)
	pfx lmsw	(%eax)
	pfx lods
	pfx lods	(%esi)
	pfx ltr		(%eax)
	pfx mov		$0x12, (%eax)
	pfx mov		$0x1234, (%eax)
	pfx mov		$0x12345678, (%eax)
	pfx mov		%es, (%eax)
	pfx mov		(%eax), %es
	pfx movs
	pfx movs	(%esi), %es:(%edi)
	movsx		(%eax), %ax
	pfx movsx	(%eax), %eax
	movzx		(%eax), %ax
	pfx movzx	(%eax), %eax
	pfx mul		(%eax)
	pfx neg		(%eax)
	pfx nop		(%eax)
	pfx not		(%eax)
	pfx or		$1, (%eax)
	pfx or		$0x89, (%eax)
	pfx or		$0x1234, (%eax)
	pfx or		$0x12345678, (%eax)
	pfx out		$0
	pfx out		%dx
	pfx outs
	pfx outs	(%esi), %dx
	pfx pop		(%eax)
	pfx pop		%es
	ptwrite		(%eax)
	pfx push	(%eax)
	pfx push	%es
	pfx rcl		$1, (%eax)
	pfx rcl		$2, (%eax)
	pfx rcl		%cl, (%eax)
	pfx rcl		(%eax)
	pfx rcr		$1, (%eax)
	pfx rcr		$2, (%eax)
	pfx rcr		%cl, (%eax)
	pfx rcr		(%eax)
	pfx rol		$1, (%eax)
	pfx rol		$2, (%eax)
	pfx rol		%cl, (%eax)
	pfx rol		(%eax)
	pfx ror		$1, (%eax)
	pfx ror		$2, (%eax)
	pfx ror		%cl, (%eax)
	pfx ror		(%eax)
	pfx sbb		$1, (%eax)
	pfx sbb		$0x89, (%eax)
	pfx sbb		$0x1234, (%eax)
	pfx sbb		$0x12345678, (%eax)
	pfx scas
	pfx scas	%es:(%edi)
	pfx sal		$1, (%eax)
	pfx sal		$2, (%eax)
	pfx sal		%cl, (%eax)
	pfx sal		(%eax)
	pfx sar		$1, (%eax)
	pfx sar		$2, (%eax)
	pfx sar		%cl, (%eax)
	pfx sar		(%eax)
	pfx shl		$1, (%eax)
	pfx shl		$2, (%eax)
	pfx shl		%cl, (%eax)
	pfx shl		(%eax)
	pfx shr		$1, (%eax)
	pfx shr		$2, (%eax)
	pfx shr		%cl, (%eax)
	pfx shr		(%eax)
	pfx stos
	pfx stos	%es:(%edi)
	pfx sub		$1, (%eax)
	pfx sub		$0x89, (%eax)
	pfx sub		$0x1234, (%eax)
	pfx sub		$0x12345678, (%eax)
	pfx test	$0x89, (%eax)
	pfx test	$0x1234, (%eax)
	pfx test	$0x12345678, (%eax)
	vcvtsi2sd	(%eax), %xmm0, %xmm0
	{evex} vcvtsi2sd (%eax), %xmm0, %xmm0
	vcvtsi2ss	(%eax), %xmm0, %xmm0
	{evex} vcvtsi2ss (%eax), %xmm0, %xmm0
	vcvtusi2sd	(%eax), %xmm0, %xmm0
	vcvtusi2ss	(%eax), %xmm0, %xmm0
	pfx xor		$1, (%eax)
	pfx xor		$0x89, (%eax)
	pfx xor		$0x1234, (%eax)
	pfx xor		$0x12345678, (%eax)
