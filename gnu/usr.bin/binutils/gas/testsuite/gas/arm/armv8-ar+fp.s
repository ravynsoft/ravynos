	.syntax unified
	.text
	.arch_extension fp

	.arm
	vseleq.f32	s0, s0, s0
	vselvs.f32	s1, s1, s1
	vselge.f32	s30, s30, s30
	vselgt.f32	s31, s31, s31
	vseleq.f64	d0, d0, d0
	vselvs.f64	d16, d16, d16
	vselge.f64	d15, d15, d15
	vselgt.f64	d31, d31, d31
	vmaxnm.f32	s0, s0, s0
	vmaxnm.f32	s1, s1, s1
	vmaxnm.f32	s30, s30, s30
	vmaxnm.f32	s31, s31, s31
	vmaxnm.f64	d0, d0, d0
	vmaxnm.f64	d16, d16, d16
	vmaxnm.f64	d15, d15, d15
	vmaxnm.f64	d31, d31, d31
	vminnm.f32	s0, s0, s0
	vminnm.f32	s1, s1, s1
	vminnm.f32	s30, s30, s30
	vminnm.f32	s31, s31, s31
	vminnm.f64	d0, d0, d0
	vminnm.f64	d16, d16, d16
	vminnm.f64	d15, d15, d15
	vminnm.f64	d31, d31, d31
	vcvta.s32.f32	s0, s0
	vcvtn.s32.f32	s1, s1
	vcvtp.u32.f32	s30, s30
	vcvtm.u32.f32	s31, s31
	vcvta.s32.f64	s0, d0
	vcvtn.s32.f64	s1, d16
	vcvtp.u32.f64	s30, d15
	vcvtm.u32.f64	s31, d31
	vrintz.f32	s0, s0
        vrintx.f32	s1, s1
        vrintreq.f32	s30, s30
	vrinta.f32	s0, s0
	vrintn.f32	s1, s1
	vrintp.f32	s30, s30
	vrintm.f32	s31, s31
	vrintz.f64	d0, d0
        vrintx.f64	d1, d1
        vrintreq.f64	d30, d30
	vrinta.f64	d0, d0
	vrintn.f64	d1, d1
	vrintp.f64	d30, d30
	vrintm.f64	d31, d31
	vcvtt.f16.f64	s0, d0
	vcvtb.f16.f64	s1, d16
	vcvtt.f16.f64	s30, d15
	vcvtb.f16.f64	s31, d31
	vcvtt.f64.f16	d0, s0
	vcvtb.f64.f16	d16, s1
	vcvtt.f64.f16	d15, s30
	vcvtb.f64.f16	d31, s31

	.thumb
	vseleq.f32	s0, s0, s0
	vselvs.f32	s1, s1, s1
	vselge.f32	s30, s30, s30
	vselgt.f32	s31, s31, s31
	vseleq.f64	d0, d0, d0
	vselvs.f64	d16, d16, d16
	vselge.f64	d15, d15, d15
	vselgt.f64	d31, d31, d31
	vmaxnm.f32	s0, s0, s0
	vmaxnm.f32	s1, s1, s1
	vmaxnm.f32	s30, s30, s30
	vmaxnm.f32	s31, s31, s31
	vmaxnm.f64	d0, d0, d0
	vmaxnm.f64	d16, d16, d16
	vmaxnm.f64	d15, d15, d15
	vmaxnm.f64	d31, d31, d31
	vminnm.f32	s0, s0, s0
	vminnm.f32	s1, s1, s1
	vminnm.f32	s30, s30, s30
	vminnm.f32	s31, s31, s31
	vminnm.f64	d0, d0, d0
	vminnm.f64	d16, d16, d16
	vminnm.f64	d15, d15, d15
	vminnm.f64	d31, d31, d31
	vcvta.s32.f32	s0, s0
	vcvtn.s32.f32	s1, s1
	vcvtp.u32.f32	s30, s30
	vcvtm.u32.f32	s31, s31
	vcvta.s32.f64	s0, d0
	vcvtn.s32.f64	s1, d16
	vcvtp.u32.f64	s30, d15
	vcvtm.u32.f64	s31, d31
	vrintz.f32	s0, s0
        vrintx.f32	s1, s1
        vrintr.f32	s30, s30
	vrinta.f32	s0, s0
	vrintn.f32	s1, s1
	vrintp.f32	s30, s30
	vrintm.f32	s31, s31
	vrintz.f64	d0, d0
        vrintx.f64	d1, d1
        vrintr.f64	d30, d30
	vrinta.f64	d0, d0
	vrintn.f64	d1, d1
	vrintp.f64	d30, d30
	vrintm.f64	d31, d31
	vcvtt.f16.f64	s0, d0
	vcvtb.f16.f64	s1, d16
	vcvtt.f16.f64	s30, d15
	vcvtb.f16.f64	s31, d31
	vcvtt.f64.f16	d0, s0
	vcvtb.f64.f16	d16, s1
	vcvtt.f64.f16	d15, s30
	vcvtb.f64.f16	d31, s31
	vmrs		r9, MVFR2
	vmsr		MVFR2, r7
	vmrs		r4, mvfr2
	vmsr		mvfr2, r5

