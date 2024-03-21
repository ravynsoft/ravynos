	.syntax unified
	.text
	.arch armv7e-m
	.fpu fpv5-sp-d16

	.thumb
	vseleq.f32	s0, s0, s0
	vselvs.f32	s1, s1, s1
	vselge.f32	s30, s30, s30
	vselgt.f32	s31, s31, s31
	vmaxnm.f32	s0, s0, s0
	vmaxnm.f32	s1, s1, s1
	vmaxnm.f32	s30, s30, s30
	vmaxnm.f32	s31, s31, s31
	vminnm.f32	s0, s0, s0
	vminnm.f32	s1, s1, s1
	vminnm.f32	s30, s30, s30
	vminnm.f32	s31, s31, s31
	vcvta.s32.f32	s0, s0
	vcvtn.s32.f32	s1, s1
	vcvtp.u32.f32	s30, s30
	vcvtm.u32.f32	s31, s31
	vrintz.f32	s0, s0
        vrintx.f32	s1, s1
        vrintr.f32	s30, s30
	vrinta.f32	s0, s0
	vrintn.f32	s1, s1
	vrintp.f32	s30, s30
	vrintm.f32	s31, s31
