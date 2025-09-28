	.text
	.set noreorder

simd_insns:
	packsshb	$f0, $f1, $f2
	packsswh	$f3, $f4, $f5
	packushb	$f6, $f7, $f8
	paddb		$f9, $f10, $f11
	paddh		$f12, $f13, $f14
	paddw		$f15, $f16, $f17
	paddd		$f18, $f19, $f20
	paddsb		$f21, $f22, $f23
	paddsh		$f24, $f25, $f26
	paddusb		$f27, $f28, $f29
	paddush		$f0, $f1, $f2
	pandn		$f3, $f4, $f5
	pavgb		$f6, $f7, $f8
	pavgh		$f9, $f10, $f11
	pcmpeqb		$f12, $f13, $f14
	pcmpeqh		$f15, $f16, $f17
	pcmpeqw		$f18, $f19, $f20
	pcmpgtb		$f21, $f22, $f23
	pcmpgth		$f24, $f25, $f26
	pcmpgtw		$f27, $f28, $f29
	pextrh		$f0, $f1, $f2
	pinsrh_0	$f3, $f4, $f5
	pinsrh_1	$f6, $f7, $f8
	pinsrh_2	$f9, $f10, $f11
	pinsrh_3	$f12, $f13, $f14
	pmaddhw		$f15, $f16, $f17
	pmaxsh		$f18, $f19, $f20
	pmaxub		$f21, $f22, $f23
	pminsh		$f24, $f25, $f26
	pminub		$f27, $f28, $f29
	pmovmskb	$f0, $f1
	pmulhuh		$f2, $f3, $f4
	pmulhh		$f5, $f6, $f7
	pmullh		$f8, $f9, $f10
	pmuluw		$f11, $f12, $f13
	pasubub		$f14, $f15, $f16
	biadd		$f17, $f18
	pshufh		$f19, $f20, $f21
	psllh		$f22, $f23, $f24
	psllw		$f25, $f26, $f27
	psrah		$f28, $f29, $f30
	psraw		$f0, $f1, $f2
	psrlh		$f3, $f4, $f5
	psrlw		$f6, $f7, $f8
	psubb		$f9, $f10, $f11
	psubh		$f12, $f13, $f14
	psubw		$f15, $f16, $f17
	psubd		$f18, $f19, $f20
	psubsb		$f21, $f22, $f23
	psubsh		$f24, $f25, $f26
	psubusb		$f27, $f28, $f29
	psubush		$f0, $f1, $f2
	punpckhbh	$f3, $f4, $f5
	punpckhhw	$f6, $f7, $f8
	punpckhwd	$f9, $f10, $f11
	punpcklbh	$f12, $f13, $f14
	punpcklhw	$f15, $f16, $f17
	punpcklwd	$f18, $f19, $f20

fixed_point_insns:
	add		$f0, $f1, $f2
	addu		$f3, $f4, $f5
	dadd		$f6, $f7, $f8
	sub		$f9, $f10, $f11
	subu		$f12, $f13, $f14
	dsub		$f15, $f16, $f17
	or		$f18, $f19, $f20
	sll		$f21, $f22, $f23
	dsll		$f24, $f25, $f26
	xor		$f27, $f28, $f29
	nor		$f0, $f1, $f2
	and		$f3, $f4, $f5
	srl		$f6, $f7, $f8
	dsrl		$f9, $f10, $f11
	sra		$f12, $f13, $f14
	dsra		$f15, $f16, $f17
	sequ		$f18, $f19
	sltu		$f20, $f21
	sleu		$f22, $f23
	seq		$f24, $f25
	slt		$f26, $f27
	sle		$f28, $f29
