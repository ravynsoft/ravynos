# name: VFPv3 additional constant and conversion ops
# as: -mfpu=vfp3
# objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section \.text:
0[0-9a-f]+ <[^>]+> eef08a04 	(vmov\.f32|fconsts)	s17, #4.*
0[0-9a-f]+ <[^>]+> eeba9a05 	(vmov\.f32|fconsts)	s18, #165.*
0[0-9a-f]+ <[^>]+> eef49a00 	(vmov\.f32|fconsts)	s19, #64.*
0[0-9a-f]+ <[^>]+> eef01b04 	(vmov\.f64|fconstd)	d17, #4.*
0[0-9a-f]+ <[^>]+> eefa2b05 	(vmov\.f64|fconstd)	d18, #165.*
0[0-9a-f]+ <[^>]+> eef43b00 	(vmov\.f64|fconstd)	d19, #64.*
0[0-9a-f]+ <[^>]+> eefa8a63 	(vcvt\.f32\.s16	s17, s17, #9|fshtos	s17, #9)
0[0-9a-f]+ <[^>]+> eefa1b63 	(vcvt\.f64\.s16	d17, d17, #9|fshtod	d17, #9)
0[0-9a-f]+ <[^>]+> eefa8aeb 	(vcvt\.f32\.s32	s17, s17, #9|fsltos	s17, #9)
0[0-9a-f]+ <[^>]+> eefa1beb 	(vcvt\.f64\.s32	d17, d17, #9|fsltod	d17, #9)
0[0-9a-f]+ <[^>]+> eefb8a63 	(vcvt\.f32\.u16	s17, s17, #9|fuhtos	s17, #9)
0[0-9a-f]+ <[^>]+> eefb1b63 	(vcvt\.f64\.u16	d17, d17, #9|fuhtod	d17, #9)
0[0-9a-f]+ <[^>]+> eefb8aeb 	(vcvt\.f32\.u32	s17, s17, #9|fultos	s17, #9)
0[0-9a-f]+ <[^>]+> eefb1beb 	(vcvt\.f64\.u32	d17, d17, #9|fultod	d17, #9)
0[0-9a-f]+ <[^>]+> eefe9a64 	(vcvt\.s16\.f32	s19, s19, #7|ftoshs	s19, #7)
0[0-9a-f]+ <[^>]+> eefe3b64 	(vcvt\.s16\.f64	d19, d19, #7|ftoshd	d19, #7)
0[0-9a-f]+ <[^>]+> eefe9aec 	(vcvt\.s32\.f32	s19, s19, #7|ftosls	s19, #7)
0[0-9a-f]+ <[^>]+> eefe3bec 	(vcvt\.s32\.f64	d19, d19, #7|ftosld	d19, #7)
0[0-9a-f]+ <[^>]+> eeff9a64 	(vcvt\.u16\.f32	s19, s19, #7|ftouhs	s19, #7)
0[0-9a-f]+ <[^>]+> eeff3b64 	(vcvt\.u16\.f64	d19, d19, #7|ftouhd	d19, #7)
0[0-9a-f]+ <[^>]+> eeff9aec 	(vcvt\.u32\.f32	s19, s19, #7|ftouls	s19, #7)
0[0-9a-f]+ <[^>]+> eeff3bec 	(vcvt\.u32\.f64	d19, d19, #7|ftould	d19, #7)
