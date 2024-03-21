	.macro f16_dq_ifsu reg0 reg1 reg2
	.irp op, vabd.f16, vmax.f16, vmin.f16
		\op d\reg0, d\reg1, d\reg2
		\op q\reg0, q\reg1, q\reg2
	.endr
	.endm

	.macro f16_q_ifsu reg0 reg1 reg2
	.irp op, vabdq.f16, vmaxq.f16, vminq.f16
		\op q\reg0, q\reg1, q\reg2
	.endr
	.endm

	.macro f16_dq_abs_neg reg0 reg1
	.irp op, vabs.f16, vneg.f16
		\op d\reg0, d\reg1
		\op q\reg0, q\reg1
	.endr
	.endm

	.macro f16_q_abs_neg reg0 reg1
	.irp op, vabsq.f16, vnegq.f16
		\op q\reg0, q\reg1
	.endr
	.endm

	.macro f16_dq_fcmp reg0 reg1 reg2
	.irp op, vacge.f16, vacgt.f16, vaclt.f16, vacle.f16, vceq.f16, vcge.f16, vcgt.f16, vcle.f16, vclt.f16
		\op d\reg0, d\reg1, d\reg2
		\op q\reg0, q\reg1, q\reg2
	.endr
	.endm

	.macro f16_dq_fcmp_imm0 reg0 reg1
	.irp op, vceq.f16, vcge.f16, vcgt.f16, vcle.f16, vclt.f16
		\op d\reg0, d\reg1, #0
		\op q\reg0, q\reg1, #0
	.endr
	.endm

	.macro f16_q_fcmp reg0 reg1 reg2
	.irp op, vacgeq.f16, vacgtq.f16, vacltq.f16, vacleq.f16, vceqq.f16, vcgeq.f16, vcgtq.f16, vcleq.f16, vcltq.f16
		\op q\reg0, q\reg1, q\reg2
	.endr
	.endm

	.macro f16_dq_addsub reg0 reg1 reg2
	.irp op, vadd.f16, vsub.f16
		\op d\reg0, d\reg1, d\reg2
		\op q\reg0, q\reg1, q\reg2
	.endr
	.endm

	.macro f16_q_addsub reg0 reg1 reg2
	.irp op, vaddq.f16, vsubq.f16
		\op q\reg0, q\reg1, q\reg2
	.endr
	.endm

	.macro f16_dq_vmaxnm reg0 reg1 reg2
	.irp op, vmaxnm.f16, vminnm.f16
		\op d\reg0, d\reg1, d\reg2
		\op q\reg0, q\reg1, q\reg2
	.endr
	.endm

	.macro f16_dq_fmac reg0 reg1 reg2
	.irp op, vfma.f16, vfms.f16
		\op d\reg0, d\reg1, d\reg2
		\op q\reg0, q\reg1, q\reg2
	.endr
	.endm

	.macro f16_dq_fmacmaybe reg0 reg1 reg2
	.irp op, vmla.f16, vmls.f16
		\op d\reg0, d\reg1, d\reg2
		\op q\reg0, q\reg1, q\reg2
	.endr
	.endm

	.macro f16_dq_vrint reg0 reg1
	.irp op, vrintz.f16, vrintx.f16, vrinta.f16, vrintn.f16, vrintp.f16, vrintm.f16
		\op d\reg0, d\reg1
		\op q\reg0, q\reg1
	.endr
	.endm

	.macro f16_dq_recip reg0 reg1
	.irp op, vrecpe.f16, vrsqrte.f16
		\op d\reg0, d\reg1
		\op q\reg0, q\reg1
	.endr
	.endm

	.macro f16_q_recip reg0 reg1
	.irp op, vrecpeq.f16, vrsqrteq.f16
		\op q\reg0, q\reg1
	.endr
	.endm

	.macro f16_dq_step reg0 reg1 reg2
	.irp op, vrecps.f16, vrsqrts.f16
		\op d\reg0, d\reg1, d\reg2
		\op q\reg0, q\reg1, q\reg2
	.endr
	.endm

	.macro f16_q_step reg0 reg1 reg2
	.irp op, vrecpsq.f16, vrsqrtsq.f16
		\op q\reg0, q\reg1, q\reg2
	.endr
	.endm

	.macro f16_dq_cvt reg0 reg1
	.irp op, vcvta.s16.f16, vcvtm.s16.f16, vcvtn.s16.f16, vcvtp.s16.f16, vcvta.u16.f16, vcvtm.u16.f16, vcvtn.u16.f16, vcvtp.u16.f16,
		\op d\reg0, d\reg1
		\op q\reg0, q\reg1
	.endr
	.endm

	.macro f16_dq_cvtz reg0 reg1
	.irp op, vcvt.s16.f16, vcvt.u16.f16, vcvt.f16.s16, vcvt.f16.u16,
		\op d\reg0, d\reg1
		\op q\reg0, q\reg1
	.endr
	.endm

	.macro f16_dq_cvtz_fixed reg0 reg1 imm
	.irp op, vcvt.s16.f16, vcvt.u16.f16, vcvt.f16.s16, vcvt.f16.u16,
		\op d\reg0, d\reg1, #\imm
		\op q\reg0, q\reg1, #\imm
	.endr
	.endm

	.macro f16_dq op reg0 reg1 reg2
		\op d\reg0, d\reg1, d\reg2
		\op q\reg0, q\reg1, q\reg2
	.endm

	.macro f16_d op reg0 reg1 reg2
		\op d\reg0, d\reg1, d\reg2
	.endm

	.macro f16_q op reg0 reg1 reg2
		\op q\reg0, q\reg1, q\reg2
	.endm

	.macro f16_dq_2 op reg0 reg1
		\op d\reg0, d\reg1
		\op q\reg0, q\reg1
	.endm

	.macro f16_d_2 op reg0 reg1
		\op d\reg0, d\reg1
	.endm

	.macro f16_q_2 op reg0 reg1
		\op q\reg0, q\reg1
	.endm

func:
	# neon_dyadic_if_su
	f16_dq_ifsu 2 4 14
	f16_q_ifsu 0 8 14
	f16_d  vabd.f16 1 3 15
	f16_d  vabd.f16 0 1 8

	# neon_abs_neg
	f16_dq_abs_neg 0 8
	f16_q_abs_neg 2 6
	f16_d_2  vabs.f16 7 3
	f16_d_2  vneg.f16 9 1

	# neon_fcmp
	f16_dq_fcmp 2 4 14
	f16_q_fcmp 0 8 14

	# neon_addsub_if_i
	f16_dq_addsub 2 4 14
	f16_q_addsub 0 8 14

	# neon_vmaxnm
	f16_dq_vmaxnm 2 4 14

	# neon_fmac
	f16_dq_fmac 2 4 14

	# neon_mac_maybe_scalar
	f16_dq_fmacmaybe 2 4 14

	# vrint
	f16_dq_vrint 4 14

	# neon_dyadic_if_i_d
	f16_d vpadd.f16 4 8 14

	# neon_recip_est
	f16_dq_recip 4 8
	f16_q_recip 0 10

	# neon_step
	f16_dq_step 8 10 12
	f16_q_step 2 0 4

	# neon_dyadic_if_su_d
	f16_d vpmax.f16 4 8 14
	f16_d vpmin.f16 10 8 2

	# neon_mul
	f16_d vmul.f16 4 8 14
	f16_d vmul.f16 7 0 1
	f16_q vmul.f16 2 8 0

	# neon_cvt
	f16_dq_cvt 6 12

	# neon_cvtz
	f16_dq_cvtz 14, 0

	# neon_cvtz_fixed
	f16_dq_cvtz_fixed 14, 0, 3

	# neon_fcmp_imm0
	f16_dq_fcmp_imm0 14, 2

	.macro f16_d_by_scalar op reg0 reg1 reg2 idx
		\op d\reg0, d\reg1, d\reg2[\idx]
	.endm

	.macro f16_q_by_scalar op reg0 reg1 reg2 idx
		\op q\reg0, q\reg1, d\reg2[\idx]
	.endm

	.macro f16_dq_fmacmaybe_by_scalar reg0 reg1 reg2 idx
	.irp op, vmla.f16, vmls.f16
		\op d\reg0, d\reg1, d\reg2[\idx]
		\op q\reg0, q\reg1, d\reg2[\idx]
	.endr
	.endm

	# neon_mul (by scalar)
	f16_d_by_scalar vmul.f16 7 0 1 0
	f16_d_by_scalar vmul.f16 4 8 6 2
	f16_q_by_scalar vmul.f16 2 8 0 1
	f16_q_by_scalar vmul.f16 2 8 7 3

	# neon_mac_maybe_scalar (by scalar)
	f16_dq_fmacmaybe_by_scalar 2 4 1 0
	f16_dq_fmacmaybe_by_scalar 1 8 7 3
