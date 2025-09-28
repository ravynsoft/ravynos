
	.eabi_attribute Tag_Advanced_SIMD_arch, 2
	.eabi_attribute Tag_VFP_arch, 6
	
	@VMLA
	.inst 0xee000a00 @ VFP  vmla.f32 s0,s0,s0
	.inst 0xee000b00 @ VFP  vmla.f64 d0,d0,d0
	.inst 0xf2000d10 @ NEON vmla.f32 d0,d0,d0
	.inst 0xf2000d50 @ NEON vmla.f32 q0,q0,q0

	@VFMA new
	.inst 0xeea00a00 @ VFP  vfma.f32 s0,s0,s0
	.inst 0xeea00b00 @ VFP  vfma.f64 d0,d0,d0
	.inst 0xf2000c10 @ NEON vfma.f32 d0,d0,d0
	.inst 0xf2000c50 @ NEON vfma.f32 q0,q0,q0

	@VMLS
	.inst 0xee000a40 @ VFP  vmls.F32 s0,s0,s0
	.inst 0xee000b40 @ VFP  vmls.F64 d0,d0,d0
	.inst 0xf2200d10 @ NEON vmls.F32 d0,d0,d0
	.inst 0xf2200d50 @ NEON vmls.F32 q0,q0,q0

	@VFMS new
	.inst 0xeea00a40 @ VFP  vfms.F32 s0,s0,s0
	.inst 0xeea00b40 @ VFP  vfms.F64 d0,d0,d0
	.inst 0xf2200c10 @ NEON vfms.F32 d0,d0,d0
	.inst 0xf2200c50 @ NEON vfms.F32 q0,q0,q0

	@VNMLA
	.inst 0xee100a40 @ VFP  vnmla.F32 s0,s0,s0
	.inst 0xee100b40 @ VFP  vnmla.F64 d0,d0,d0

	@VFNMA new
	.inst 0xee900a40 @ VFP  vfnma.F32 s0,s0,s0
	.inst 0xee900b40 @ VFP  vfnma.F64 d0,d0,d0

	@VNMLS
	.inst 0xee100a00 @ VFP  vnmls.F32 s0,s0,s0
	.inst 0xee100b00 @ VFP  vnmls.F64 d0,d0,d0

	@VFNMS new
	.inst 0xee900a00 @ VFP  vfnms.F32 s0,s0,s0
	.inst 0xee900b00 @ VFP  vfnms.F64 d0,d0,d0
