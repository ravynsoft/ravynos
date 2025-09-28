	.set noreorder
	.set noat

	.globl text_label .text
text_label:

	# VU Macromode instruction set
	vabs.xyzw	$vf0xyzw,$vf31xyzw
	vadd.xyzw	$vf10xyzw,$vf31xyzw,$vf0xyzw
	vaddi.xyzw	$vf10xyzw,$vf31xyzw,$I
	vaddq.xyzw	$vf10xyzw,$vf31xyzw,$Q
	vaddw.xyzw	$vf10xyzw,$vf31xyzw,$vf1w
	vaddx.xyzw	$vf10xyzw,$vf31xyzw,$vf1x
	vaddy.xyzw	$vf10xyzw,$vf31xyzw,$vf1y
	vaddz.xyzw	$vf10xyzw,$vf31xyzw,$vf1z
	vadda.xyzw	$ACCxyzw,$vf0xyzw,$vf31xyzw
	vaddai.xyzw	$ACCxyzw,$vf31xyzw,$I
	vaddaq.xyzw	$ACCxyzw,$vf31xyzw,$Q
	vaddaw.xyzw	$ACCxyzw,$vf31xyzw,$vf1w
	vaddax.xyzw	$ACCxyzw,$vf31xyzw,$vf1x
	vadday.xyzw	$ACCxyzw,$vf31xyzw,$vf1y
	vaddaz.xyzw	$ACCxyzw,$vf31xyzw,$vf1z
	vcallms	0x0
	vcallms	0x340
	vcallms	0xff8
	vcallmsr	$vi27
	vclipw.xyz	$vf31xyz,$vf1w
	vclipw		$vf31xyz,$vf1w
	vdiv		$Q,$vf1y,$vf11x
	vftoi0.xyzw	$vf0xyzw,$vf31xyzw
	vftoi4.xyzw	$vf0xyzw,$vf31xyzw
	vftoi12.xyzw	$vf0xyzw,$vf31xyzw
	vftoi15.xyzw	$vf0xyzw,$vf31xyzw
	viadd	$vi1,$vi15,$vi0
	viaddi	$vi0,$vi15,-1
	viand	$vi1,$vi15,$vi0
	vilwr.w	$vi0,($vi15)
	vilwr.x	$vi0,($vi15)
	vilwr.y	$vi0,($vi15)
	vilwr.z	$vi0,($vi15)
	vior	$vi1,$vi15,$vi0
	viswr.w	$vi0,($vi15)
	viswr.x	$vi0,($vi15)
	viswr.y	$vi0,($vi15)
	viswr.z	$vi0,($vi15)
	visub	$vi1,$vi15,$vi0
	vitof0.xyzw	$vf0xyzw,$vf31xyzw
	vitof4.xyzw	$vf0xyzw,$vf31xyzw
	vitof12.xyzw	$vf0xyzw,$vf31xyzw
	vitof15.xyzw	$vf0xyzw,$vf31xyzw
	vlqd.xyzw	$vf0xyzw,(--$vi15)
	vlqi.xyzw	$vf0xyzw,($vi15++)
	vmadd.xyzw	$vf10xyzw,$vf31xyzw,$vf0xyzw
	vmaddi.xyzw	$vf10xyzw,$vf31xyzw,$I
	vmaddq.xyzw	$vf10xyzw,$vf31xyzw,$Q
	vmaddw.xyzw	$vf10xyzw,$vf31xyzw,$vf1w
	vmaddx.xyzw	$vf10xyzw,$vf31xyzw,$vf1x
	vmaddy.xyzw	$vf10xyzw,$vf31xyzw,$vf1y
	vmaddz.xyzw	$vf10xyzw,$vf31xyzw,$vf1z
	vmaddz		$vf6, $vf3, $vf5
	vmadda.xyzw	$ACCxyzw,$vf31xyzw,$vf0xyzw
	vmaddai.xyzw	$ACCxyzw,$vf31xyzw,$I
	vmaddaq.xyzw	$ACCxyzw,$vf31xyzw,$Q
	vmaddaw.xyzw	$ACCxyzw,$vf31xyzw,$vf1w
	vmaddax.xyzw	$ACCxyzw,$vf31xyzw,$vf1x
	vmaddax		$ACC, $vf1, $vf5
	vmadday.xyzw	$ACCxyzw,$vf31xyzw,$vf1y
	vmadday		$ACC, $vf2, $vf5
	vmaddaz.xyzw	$ACCxyzw,$vf31xyzw,$vf1z
	vmax.xyzw	$vf10xyzw,$vf31xyzw,$vf0xyzw
	vmaxi.xyzw	$vf10xyzw,$vf31xyzw,$I
	vmaxw.xyzw	$vf10xyzw,$vf31xyzw,$vf1w
	vmaxx.xyzw	$vf10xyzw,$vf31xyzw,$vf1x
	vmaxy.xyzw	$vf10xyzw,$vf31xyzw,$vf1y
	vmaxz.xyzw	$vf10xyzw,$vf31xyzw,$vf1z
	vmfir.xyzw	$vf0xyzw,$vi15
	vmini.xyzw	$vf10xyzw,$vf31xyzw,$vf0xyzw
	vminii.xyzw	$vf10xyzw,$vf31xyzw,$I
	vminiw.xyzw	$vf10xyzw,$vf31xyzw,$vf1w
	vminix.xyzw	$vf10xyzw,$vf31xyzw,$vf1x
	vminiy.xyzw	$vf10xyzw,$vf31xyzw,$vf1y
	vminiz.xyzw	$vf10xyzw,$vf31xyzw,$vf1z
	vmove.xyzw	$vf0xyzw,$vf31xyzw
	vmr32.xyzw	$vf0xyzw,$vf31xyzw
	vmsub.xyzw	$vf10xyzw,$vf31xyzw,$vf0xyzw
	vmsubi.xyzw	$vf10xyzw,$vf31xyzw,$I
	vmsubq.xyzw	$vf10xyzw,$vf31xyzw,$Q
	vmsubw.xyzw	$vf10xyzw,$vf31xyzw,$vf1w
	vmsubx.xyzw	$vf10xyzw,$vf31xyzw,$vf1x
	vmsuby.xyzw	$vf10xyzw,$vf31xyzw,$vf1y
	vmsubz.xyzw	$vf10xyzw,$vf31xyzw,$vf1z
	vmsuba.xyzw	$ACCxyzw,$vf0xyzw,$vf31xyzw
	vmsubai.xyzw	$ACCxyzw,$vf31xyzw,$I
	vmsubaq.xyzw	$ACCxyzw,$vf31xyzw,$Q
	vmsubaw.xyzw	$ACCxyzw,$vf31xyzw,$vf1w
	vmsubax.xyzw	$ACCxyzw,$vf31xyzw,$vf1x
	vmsubay.xyzw	$ACCxyzw,$vf31xyzw,$vf1y
	vmsubaz.xyzw	$ACCxyzw,$vf31xyzw,$vf1z
	vmtir	$vi0,$vf1z
	vmul.xyzw	$vf10xyzw,$vf31xyzw,$vf0xyzw
	vmuli.xyzw	$vf10xyzw,$vf31xyzw,$I
	vmulq.xyzw	$vf10xyzw,$vf31xyzw,$Q
	vmulw.xyzw	$vf10xyzw,$vf31xyzw,$vf1w
	vmulx.xyzw	$vf10xyzw,$vf31xyzw,$vf1x
	vmuly.xyzw	$vf10xyzw,$vf31xyzw,$vf1y
	vmulz.xyzw	$vf10xyzw,$vf31xyzw,$vf1z
	vmula.xyzw	$ACCxyzw,$vf31xyzw,$vf0xyzw
	vmulai.xyzw	$ACCxyzw,$vf31xyzw,$I
	vmulaq.xyzw	$ACCxyzw,$vf31xyzw,$Q
	vmulaw.xyzw	$ACCxyzw,$vf31xyzw,$vf1w
	vmulaw		$ACC, $vf4, $vf0
	vmulax.xyzw	$ACCxyzw,$vf31xyzw,$vf1x
	vmulay.xyzw	$ACCxyzw,$vf31xyzw,$vf1y
	vmulaz.xyzw	$ACCxyzw,$vf31xyzw,$vf1z
	vnop
	vopmula.xyz	$ACCxyz,$vf31xyz,$vf0xyz
	vopmsub.xyz	$vf10xyz,$vf31xyz,$vf0xyz
	vrget.xyzw	$vf0xyzw,$R
	vrinit	$R,$vf1w
	vrnext.xyzw	$vf0xyzw,$R
	vrsqrt	$Q,$vf1w,$vf11x
	vrxor	$R,$vf1x
	vsqd.xyzw	$vf31xyzw,(--$vi0)
	vsqi.xyzw	$vf31xyzw,($vi0++)
	vsqrt	$Q,$vf11z
	vsub.xyzw	$vf10xyzw,$vf31xyzw,$vf0xyzw
	vsubi.xyzw	$vf10xyzw,$vf31xyzw,$I
	vsubq.xyzw	$vf10xyzw,$vf31xyzw,$Q
	vsubw.xyzw	$vf10xyzw,$vf31xyzw,$vf1w
	vsubx.xyzw	$vf10xyzw,$vf31xyzw,$vf1x
	vsuby.xyzw	$vf10xyzw,$vf31xyzw,$vf1y
	vsubz.xyzw	$vf10xyzw,$vf31xyzw,$vf1z
	vsuba.xyzw	$ACCxyzw,$vf31xyzw,$vf0xyzw
	vsubai.xyzw	$ACCxyzw,$vf31xyzw,$I
	vsubaq.xyzw	$ACCxyzw,$vf31xyzw,$Q
	vsubaw.xyzw	$ACCxyzw,$vf31xyzw,$vf1w
	vsubax.xyzw	$ACCxyzw,$vf31xyzw,$vf1x
	vsubay.xyzw	$ACCxyzw,$vf31xyzw,$vf1y
	vsubaz.xyzw	$ACCxyzw,$vf31xyzw,$vf1z
	vwaitq

	# Implicit suffixes
	vadd.xyzw	$vf10,$vf31,$vf0
	vadd.xy	$vf10,$vf31,$vf0
	vadd.xyzw	$vf10,$vf31,$vf0
	vlqi.xy	$vf0,($vi15++)

	# VU floating point registers
	vadd.xyzw	$vf0,$vf1,$vf2
	vadd.xyzw	$vf3,$vf4,$vf5
	vadd.xyzw	$vf6,$vf7,$vf8
	vadd.xyzw	$vf9,$vf10,$vf11
	vadd.xyzw	$vf12,$vf13,$vf14
	vadd.xyzw	$vf15,$vf16,$vf17
	vadd.xyzw	$vf18,$vf19,$vf20
	vadd.xyzw	$vf21,$vf22,$vf23
	vadd.xyzw	$vf24,$vf25,$vf26
	vadd.xyzw	$vf27,$vf28,$vf29
	vadd.xyzw	$vf30,$vf31,$vf0

	# VU integer registers
	viadd	$vi0,$vi1,$vi2
	viadd	$vi3,$vi4,$vi5
	viadd	$vi6,$vi7,$vi8
	viadd	$vi9,$vi10,$vi11
	viadd	$vi12,$vi13,$vi14
	viadd	$vi15,$vi16,$vi17
	viadd	$vi18,$vi19,$vi20
	viadd	$vi21,$vi22,$vi23
	viadd	$vi24,$vi25,$vi26
	viadd	$vi27,$vi28,$vi29
	viadd	$vi30,$vi31,$vi0

	# Floating point transfer to VU
	lqc2	$0,0($0)
	lqc2	$1, 0x7fff($1)
	lqc2	$8, -0x8000($8)
	lqc2	$31, -1($31)

	# Floating point transfer from VU
	sqc2	$0,0($0)
	sqc2	$1, 0x7fff($1)
	sqc2	$8, -0x8000($8)
	sqc2	$31, -1($31)

	# Integer transfer from VU
	cfc2	$0,$0
	cfc2	$0,$31
	cfc2.i	$0,$0
	cfc2.i	$0,$31
	cfc2.ni	$0,$0
	cfc2.ni	$0,$31

	# Integer transfer to VU
	ctc2	$0,$0
	ctc2	$0,$31
	ctc2.i	$0,$0
	ctc2.i	$0,$31
	ctc2.ni	$0,$0
	ctc2.ni	$0,$31

	# Floating point transfer from VU
	qmfc2	$0,$0
	qmfc2	$0,$31
	qmfc2.i	$0,$0
	qmfc2.i	$0,$31
	qmfc2.ni	$0,$0
	qmfc2.ni	$0,$31

	# Floating point transfer to VU
	qmtc2	$0,$0
	qmtc2	$0,$31
	qmtc2.i	$0,$0
	qmtc2.i	$0,$31
	qmtc2.ni	$0,$0
	qmtc2.ni	$0,$31

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
      .space  8
