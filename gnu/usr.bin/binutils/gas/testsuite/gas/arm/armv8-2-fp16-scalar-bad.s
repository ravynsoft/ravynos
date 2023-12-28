	.macro f16_sss_arithmetic reg0, reg1, reg2
	.irp op, vdiv, vfma, vfms, vfnma, vfnms, vmla, vmls, vmul, vnmla, vnmls, vnmul, vsub
	  .irp cond, eq.f16, ne.f16, ge.f16, lt.f16, gt.f16, le.f16
		\op\cond s\reg0, s\reg1, s\reg2
	  .endr
	.endr
	.endm

	.macro f16_ss_arithmetic reg0, reg1
	.irp op, vabs, vadd, vsqrt, vneg
	  .irp cond, eq.f16, ne.f16, ge.f16, lt.f16, gt.f16, le.f16
		\op\cond s\reg0, s\reg1
	  .endr
	.endr
	.endm

	.macro f16_si_cmp reg0, imm
	.irp op, vcmp, vcmpe
	  .irp cond, eq.f16, ne.f16, ge.f16, lt.f16, gt.f16, le.f16
		\op\cond s\reg0, \imm
	  .endr
	.endr
	.endm

	.macro f16_ss_cmp reg0, reg1
	.irp op, vcmp, vcmpe
	  .irp cond, eq.f16, ne.f16, ge.f16, lt.f16, gt.f16, le.f16
		\op\cond s\reg0, s\reg1
	  .endr
	.endr
	.endm

	.macro f16_ss_cvt reg0, reg1
	.irp cond, eq, ne, ge, lt, gt, le
	  .irp mode, .s32.f16, .u32.f16, .f16.s32, .f16.u32
		vcvt\cond\mode s\reg0, s\reg1
	  .endr
	.endr
	.endm

	.macro f16_ssi_cvt_imm32 reg0, reg1, imm
	.irp cond, eq, ne, ge, lt, gt, le
	  .irp mode, .s32.f16, .u32.f16, .f16.s32, .f16.u32
		vcvt\cond\mode s\reg0, s\reg1, \imm
	  .endr
	.endr
	.endm

	.macro f16_ss_cvt_r reg0, reg1
	.irp cond, eq, ne, ge, lt, gt, le
	  .irp mode, .s32.f16, .u32.f16
		vcvtr\cond\mode s\reg0, s\reg1
	  .endr
	.endr
	.endm

	.macro f16_ss_vrint reg0, reg1
	.irp op, vrintr, vrintx, vrintz
	  .irp cond, eq.f16, ne.f16, ge.f16, lt.f16, gt.f16, le.f16
		\op\cond s\reg0, s\reg1
	  .endr
	.endr
	.endm

	.macro f16_ss_mov reg0, reg1
	.irp op, vins, vmovx
	  .irp cond, eq.f16, ne.f16, ge.f16, lt.f16, gt.f16, le.f16
		\op\cond s\reg0, s\reg1
	  .endr
	.endr
	.endm

	.macro t_f16_ss_mov reg0, reg1
	.irp op, vins, vmovx
	  .irp cond, eq, ne, ge, lt, gt, le
	    .irp mode, .f16
		it \cond
		\op\cond\mode s\reg0, s\reg1
	    .endr
	  .endr
	.endr
	.endm

	.text

	@ invalied immediate range
	vldr.16 s6, [pc, #-511]
	vldr.16 s6, [pc, #111]
	vldr.16 s3, [pc, #511]

	@ invalid immediate range
	vcvt.s32.f16 s11, s11, #33
	vcvt.u32.f16 s11, s11, #0
	vcvt.f16.s32 s12, s12, #34
	vcvt.f16.u32 s12, s12, #-1

	@ armv8.2 fp16 scalar instruction cannot be conditional
	f16_sss_arithmetic 0, 1, 2
	f16_ss_arithmetic 0, 1
	f16_si_cmp 2, #0.0
	f16_ss_cmp 0, 1
	f16_ss_cvt 1, 8
	f16_ssi_cvt_imm32 2, 2, #29
	f16_ss_cvt_r 0, 10
	f16_ss_vrint 3, 11
	f16_ss_mov 0, 1

	.syntax unified
	.thumb
	t_f16_ss_mov 0, 1
