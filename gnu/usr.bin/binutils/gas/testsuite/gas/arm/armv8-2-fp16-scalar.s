	.macro f16_sss_arithmetic reg0, reg1, reg2
	.irp op, vdiv.f16, vfma.f16, vfms.f16, vfnma.f16, vfnms.f16, vmaxnm.f16, vminnm.f16, vmla.f16, vmls.f16, vmul.f16, vnmla.f16, vnmls.f16, vnmul.f16, vsub.f16
		\op s\reg0, s\reg1, s\reg2
	.endr
	.endm

	.macro f16_ss_arithmetic reg0, reg1
	.irp op, vabs.f16, vadd.f16, vsqrt.f16, vneg.f16
		\op s\reg0, s\reg1
	.endr
	.endm

	.macro f16_si_cmp reg0, imm
	.irp op, vcmp.f16, vcmpe.f16
		\op s\reg0, \imm
	.endr
	.endm

	.macro f16_ss_cmp reg0, reg1
	.irp op, vcmp.f16, vcmpe.f16
		\op s\reg0, s\reg1
	.endr
	.endm

	.macro f16_sss_vsel reg0, reg1, reg2
	.irp op, vseleq.f16 vselge.f16, vselvs.f16
		\op s\reg0, s\reg1, s\reg2
	.endr
	.endm

	.macro f16_ss_cvt reg0, reg1
	.irp op, vcvt.s32.f16, vcvt.u32.f16, vcvt.f16.s32, vcvt.f16.u32
		\op s\reg0, s\reg1
	.endr
	.endm

	.macro f16_ssi_cvt_imm32 reg0, reg1, imm
	.irp op, vcvt.f16.s32, vcvt.f16.u32, vcvt.s32.f16, vcvt.u32.f16
		\op s\reg0, s\reg1, \imm
	.endr
	.endm

	.macro f16_ss_cvt_amnpr reg0, reg1
	.irp op, vcvta.s32.f16, vcvta.u32.f16, vcvtm.s32.f16, vcvtm.u32.f16, vcvtn.s32.f16, vcvtn.u32.f16, vcvtp.s32.f16, vcvtp.u32.f16, vcvtr.u32.f16, vcvtr.s32.f16
		\op s\reg0, s\reg1
	.endr
	.endm

	.macro f16_ss_vrint reg0, reg1
	.irp op, vrinta.f16, vrintm.f16, vrintn.f16, vrintp.f16, vrintr.f16, vrintx.f16, vrintz.f16
		\op s\reg0, s\reg1
	.endr
	.endm

	.macro f16_ss_mov reg0, reg1
	.irp op, vins.f16, vmovx.f16
		\op s\reg0, s\reg1
	.endr
	.endm

	.text

	vmov.f16 s0, r1
	vmov.f16 r0, s1
	vmov.f16 s0, #2.0
	label:
	.word 0xffe
	vldr.16 s3, label

	vldr.16 s6, [pc, #-4]
	vldr.16 s3, [pc, #4]
	vldr.16 s1, [r0, #4]
	vldr.16 s2, [r0, #-4]
	vstr.16 s6 , [r0, #4]
	vstr.16 s11 , [r0, #-4]

	f16_sss_arithmetic 5, 13, 24
	f16_ss_arithmetic 5, 12
	f16_si_cmp 2, #0.0
	f16_ss_cmp 5, 13
	f16_sss_vsel 5, 13, 23
	f16_ss_cvt 3, 8
	f16_ssi_cvt_imm32 7, 7, #29
	f16_ss_cvt_amnpr 5, 10
	f16_ss_vrint 3, 11
	f16_ss_mov 5, 9
