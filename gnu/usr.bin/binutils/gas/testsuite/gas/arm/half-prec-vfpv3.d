#objdump: -d --prefix-addresses --show-raw-insn 
#name: Half-precision vfpv3 instructions
#as: -mfpu=neon-fp16

.*: +file format .*arm.*

.*
0+000 <[^>]*> eeb20ae0 	vcvtt.f32.f16	s0, s1
0+004 <[^>]*> 0eb21ae1 	vcvtteq.f32.f16	s2, s3
0+008 <[^>]*> 1eb21ae1 	vcvttne.f32.f16	s2, s3
0+00c <[^>]*> 2eb21ae1 	vcvttcs.f32.f16	s2, s3
0+010 <[^>]*> 3eb21ae1 	vcvttcc.f32.f16	s2, s3
0+014 <[^>]*> 4eb21ae1 	vcvttmi.f32.f16	s2, s3
0+018 <[^>]*> 5eb21ae1 	vcvttpl.f32.f16	s2, s3
0+01c <[^>]*> 6eb21ae1 	vcvttvs.f32.f16	s2, s3
0+020 <[^>]*> 7eb21ae1 	vcvttvc.f32.f16	s2, s3
0+024 <[^>]*> 8eb21ae1 	vcvtthi.f32.f16	s2, s3
0+028 <[^>]*> 9eb21ae1 	vcvttls.f32.f16	s2, s3
0+02c <[^>]*> aeb21ae1 	vcvttge.f32.f16	s2, s3
0+030 <[^>]*> beb21ae1 	vcvttlt.f32.f16	s2, s3
0+034 <[^>]*> ceb21ae1 	vcvttgt.f32.f16	s2, s3
0+038 <[^>]*> deb21ae1 	vcvttle.f32.f16	s2, s3
0+03c <[^>]*> eeb21ae1 	vcvtt.f32.f16	s2, s3
0+040 <[^>]*> eeb30ae0 	vcvtt.f16.f32	s0, s1
0+044 <[^>]*> 0eb31ae1 	vcvtteq.f16.f32	s2, s3
0+048 <[^>]*> 1eb31ae1 	vcvttne.f16.f32	s2, s3
0+04c <[^>]*> 2eb31ae1 	vcvttcs.f16.f32	s2, s3
0+050 <[^>]*> 3eb31ae1 	vcvttcc.f16.f32	s2, s3
0+054 <[^>]*> 4eb31ae1 	vcvttmi.f16.f32	s2, s3
0+058 <[^>]*> 5eb31ae1 	vcvttpl.f16.f32	s2, s3
0+05c <[^>]*> 6eb31ae1 	vcvttvs.f16.f32	s2, s3
0+060 <[^>]*> 7eb31ae1 	vcvttvc.f16.f32	s2, s3
0+064 <[^>]*> 8eb31ae1 	vcvtthi.f16.f32	s2, s3
0+068 <[^>]*> 9eb31ae1 	vcvttls.f16.f32	s2, s3
0+06c <[^>]*> aeb31ae1 	vcvttge.f16.f32	s2, s3
0+070 <[^>]*> beb31ae1 	vcvttlt.f16.f32	s2, s3
0+074 <[^>]*> ceb31ae1 	vcvttgt.f16.f32	s2, s3
0+078 <[^>]*> deb31ae1 	vcvttle.f16.f32	s2, s3
0+07c <[^>]*> eeb31ae1 	vcvtt.f16.f32	s2, s3
0+080 <[^>]*> eeb20a60 	vcvtb.f32.f16	s0, s1
0+084 <[^>]*> 0eb21a61 	vcvtbeq.f32.f16	s2, s3
0+088 <[^>]*> 1eb21a61 	vcvtbne.f32.f16	s2, s3
0+08c <[^>]*> 2eb21a61 	vcvtbcs.f32.f16	s2, s3
0+090 <[^>]*> 3eb21a61 	vcvtbcc.f32.f16	s2, s3
0+094 <[^>]*> 4eb21a61 	vcvtbmi.f32.f16	s2, s3
0+098 <[^>]*> 5eb21a61 	vcvtbpl.f32.f16	s2, s3
0+09c <[^>]*> 6eb21a61 	vcvtbvs.f32.f16	s2, s3
0+0a0 <[^>]*> 7eb21a61 	vcvtbvc.f32.f16	s2, s3
0+0a4 <[^>]*> 8eb21a61 	vcvtbhi.f32.f16	s2, s3
0+0a8 <[^>]*> 9eb21a61 	vcvtbls.f32.f16	s2, s3
0+0ac <[^>]*> aeb21a61 	vcvtbge.f32.f16	s2, s3
0+0b0 <[^>]*> beb21a61 	vcvtblt.f32.f16	s2, s3
0+0b4 <[^>]*> ceb21a61 	vcvtbgt.f32.f16	s2, s3
0+0b8 <[^>]*> deb21a61 	vcvtble.f32.f16	s2, s3
0+0bc <[^>]*> eeb21a61 	vcvtb.f32.f16	s2, s3
0+0c0 <[^>]*> eeb30a60 	vcvtb.f16.f32	s0, s1
0+0c4 <[^>]*> 0eb31a61 	vcvtbeq.f16.f32	s2, s3
0+0c8 <[^>]*> 1eb31a61 	vcvtbne.f16.f32	s2, s3
0+0cc <[^>]*> 2eb31a61 	vcvtbcs.f16.f32	s2, s3
0+0d0 <[^>]*> 3eb31a61 	vcvtbcc.f16.f32	s2, s3
0+0d4 <[^>]*> 4eb31a61 	vcvtbmi.f16.f32	s2, s3
0+0d8 <[^>]*> 5eb31a61 	vcvtbpl.f16.f32	s2, s3
0+0dc <[^>]*> 6eb31a61 	vcvtbvs.f16.f32	s2, s3
0+0e0 <[^>]*> 7eb31a61 	vcvtbvc.f16.f32	s2, s3
0+0e4 <[^>]*> 8eb31a61 	vcvtbhi.f16.f32	s2, s3
0+0e8 <[^>]*> 9eb31a61 	vcvtbls.f16.f32	s2, s3
0+0ec <[^>]*> aeb31a61 	vcvtbge.f16.f32	s2, s3
0+0f0 <[^>]*> beb31a61 	vcvtblt.f16.f32	s2, s3
0+0f4 <[^>]*> ceb31a61 	vcvtbgt.f16.f32	s2, s3
0+0f8 <[^>]*> deb31a61 	vcvtble.f16.f32	s2, s3
0+0fc <[^>]*> eeb31a61 	vcvtb.f16.f32	s2, s3
