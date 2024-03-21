	.syntax unified
	.arch_extension simd

	.arm
	vmaxnm.f32	d0, d0, d0
	vmaxnm.f32	d16, d16, d16
	vmaxnm.f32	d15, d15, d15
	vmaxnm.f32	d31, d31, d31
	vmaxnm.f32	q0, q0, q0
	vmaxnm.f32	q8, q8, q8
	vmaxnm.f32	q7, q7, q7
	vmaxnm.f32	q15, q15, q15
	vminnm.f32	d0, d0, d0
	vminnm.f32	d16, d16, d16
	vminnm.f32	d15, d15, d15
	vminnm.f32	d31, d31, d31
	vminnm.f32	q0, q0, q0
	vminnm.f32	q8, q8, q8
	vminnm.f32	q7, q7, q7
	vminnm.f32	q15, q15, q15
	vcvta.s32.f32	d0, d0
	vcvtn.s32.f32	d16, d16
	vcvtp.u32.f32	d15, d15
	vcvtm.u32.f32	d31, d31
	vcvta.s32.f32	q0, q0
	vcvtn.s32.f32	q8, q8
	vcvtp.u32.f32	q7, q7
	vcvtm.u32.f32	q15, q15
	vrinta.f32	d0, d0
	vrintn.f32	d16, d16
	vrintm.f32	d15, d15
	vrintp.f32	d31, d31
	vrintx.f32	d0, d31
	vrintz.f32	d16, d15
	vrinta.f32	q0, q0
	vrintn.f32	q8, q8
	vrintm.f32	q7, q7
	vrintp.f32	q15, q15
	vrintx.f32	q0, q15
	vrintz.f32	q8, q7

	.thumb
	vmaxnm.f32	d0, d0, d0
	vmaxnm.f32	d16, d16, d16
	vmaxnm.f32	d15, d15, d15
	vmaxnm.f32	d31, d31, d31
	vmaxnm.f32	q0, q0, q0
	vmaxnm.f32	q8, q8, q8
	vmaxnm.f32	q7, q7, q7
	vmaxnm.f32	q15, q15, q15
	vminnm.f32	d0, d0, d0
	vminnm.f32	d16, d16, d16
	vminnm.f32	d15, d15, d15
	vminnm.f32	d31, d31, d31
	vminnm.f32	q0, q0, q0
	vminnm.f32	q8, q8, q8
	vminnm.f32	q7, q7, q7
	vminnm.f32	q15, q15, q15
	vcvta.s32.f32	d0, d0
	vcvtn.s32.f32	d16, d16
	vcvtp.u32.f32	d15, d15
	vcvtm.u32.f32	d31, d31
	vcvta.s32.f32	q0, q0
	vcvtn.s32.f32	q8, q8
	vcvtp.u32.f32	q7, q7
	vcvtm.u32.f32	q15, q15
	vrinta.f32	d0, d0
	vrintn.f32	d16, d16
	vrintm.f32	d15, d15
	vrintp.f32	d31, d31
	vrintx.f32	d0, d31
	vrintz.f32	d16, d15
	vrinta.f32	q0, q0
	vrintn.f32	q8, q8
	vrintm.f32	q7, q7
	vrintp.f32	q15, q15
	vrintx.f32	q0, q15
	vrintz.f32	q8, q7
