#objdump: -dr --prefix-addresses --show-raw-insn
#skip: *-*-pe *-*-wince *-*-vxworks
#name: Group relocation tests (ldrs)

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> e1c00fdf 	ldrd	r0, \[r0, #255\]	@ 0xff
			0: R_ARM_LDRS_PC_G1	f
0[0-9a-f]+ <[^>]+> e1c00fdf 	ldrd	r0, \[r0, #255\]	@ 0xff
			4: R_ARM_LDRS_PC_G2	f
0[0-9a-f]+ <[^>]+> e1c00fdf 	ldrd	r0, \[r0, #255\]	@ 0xff
			8: R_ARM_LDRS_SB_G0	f
0[0-9a-f]+ <[^>]+> e1c00fdf 	ldrd	r0, \[r0, #255\]	@ 0xff
			c: R_ARM_LDRS_SB_G1	f
0[0-9a-f]+ <[^>]+> e1c00fdf 	ldrd	r0, \[r0, #255\]	@ 0xff
			10: R_ARM_LDRS_SB_G2	f
0[0-9a-f]+ <[^>]+> e1c00fff 	strd	r0, \[r0, #255\]	@ 0xff
			14: R_ARM_LDRS_PC_G1	f
0[0-9a-f]+ <[^>]+> e1c00fff 	strd	r0, \[r0, #255\]	@ 0xff
			18: R_ARM_LDRS_PC_G2	f
0[0-9a-f]+ <[^>]+> e1c00fff 	strd	r0, \[r0, #255\]	@ 0xff
			1c: R_ARM_LDRS_SB_G0	f
0[0-9a-f]+ <[^>]+> e1c00fff 	strd	r0, \[r0, #255\]	@ 0xff
			20: R_ARM_LDRS_SB_G1	f
0[0-9a-f]+ <[^>]+> e1c00fff 	strd	r0, \[r0, #255\]	@ 0xff
			24: R_ARM_LDRS_SB_G2	f
0[0-9a-f]+ <[^>]+> e1d00fbf 	ldrh	r0, \[r0, #255\]	@ 0xff
			28: R_ARM_LDRS_PC_G1	f
0[0-9a-f]+ <[^>]+> e1d00fbf 	ldrh	r0, \[r0, #255\]	@ 0xff
			2c: R_ARM_LDRS_PC_G2	f
0[0-9a-f]+ <[^>]+> e1d00fbf 	ldrh	r0, \[r0, #255\]	@ 0xff
			30: R_ARM_LDRS_SB_G0	f
0[0-9a-f]+ <[^>]+> e1d00fbf 	ldrh	r0, \[r0, #255\]	@ 0xff
			34: R_ARM_LDRS_SB_G1	f
0[0-9a-f]+ <[^>]+> e1d00fbf 	ldrh	r0, \[r0, #255\]	@ 0xff
			38: R_ARM_LDRS_SB_G2	f
0[0-9a-f]+ <[^>]+> e1c00fbf 	strh	r0, \[r0, #255\]	@ 0xff
			3c: R_ARM_LDRS_PC_G1	f
0[0-9a-f]+ <[^>]+> e1c00fbf 	strh	r0, \[r0, #255\]	@ 0xff
			40: R_ARM_LDRS_PC_G2	f
0[0-9a-f]+ <[^>]+> e1c00fbf 	strh	r0, \[r0, #255\]	@ 0xff
			44: R_ARM_LDRS_SB_G0	f
0[0-9a-f]+ <[^>]+> e1c00fbf 	strh	r0, \[r0, #255\]	@ 0xff
			48: R_ARM_LDRS_SB_G1	f
0[0-9a-f]+ <[^>]+> e1c00fbf 	strh	r0, \[r0, #255\]	@ 0xff
			4c: R_ARM_LDRS_SB_G2	f
0[0-9a-f]+ <[^>]+> e1d00fff 	ldrsh	r0, \[r0, #255\]	@ 0xff
			50: R_ARM_LDRS_PC_G1	f
0[0-9a-f]+ <[^>]+> e1d00fff 	ldrsh	r0, \[r0, #255\]	@ 0xff
			54: R_ARM_LDRS_PC_G2	f
0[0-9a-f]+ <[^>]+> e1d00fff 	ldrsh	r0, \[r0, #255\]	@ 0xff
			58: R_ARM_LDRS_SB_G0	f
0[0-9a-f]+ <[^>]+> e1d00fff 	ldrsh	r0, \[r0, #255\]	@ 0xff
			5c: R_ARM_LDRS_SB_G1	f
0[0-9a-f]+ <[^>]+> e1d00fff 	ldrsh	r0, \[r0, #255\]	@ 0xff
			60: R_ARM_LDRS_SB_G2	f
0[0-9a-f]+ <[^>]+> e1d00fdf 	ldrsb	r0, \[r0, #255\]	@ 0xff
			64: R_ARM_LDRS_PC_G1	f
0[0-9a-f]+ <[^>]+> e1d00fdf 	ldrsb	r0, \[r0, #255\]	@ 0xff
			68: R_ARM_LDRS_PC_G2	f
0[0-9a-f]+ <[^>]+> e1d00fdf 	ldrsb	r0, \[r0, #255\]	@ 0xff
			6c: R_ARM_LDRS_SB_G0	f
0[0-9a-f]+ <[^>]+> e1d00fdf 	ldrsb	r0, \[r0, #255\]	@ 0xff
			70: R_ARM_LDRS_SB_G1	f
0[0-9a-f]+ <[^>]+> e1d00fdf 	ldrsb	r0, \[r0, #255\]	@ 0xff
			74: R_ARM_LDRS_SB_G2	f
0[0-9a-f]+ <[^>]+> e1400fdf 	ldrd	r0, \[r0, #-255\]	@ 0xffffff01
			78: R_ARM_LDRS_PC_G1	f
0[0-9a-f]+ <[^>]+> e1400fdf 	ldrd	r0, \[r0, #-255\]	@ 0xffffff01
			7c: R_ARM_LDRS_PC_G2	f
0[0-9a-f]+ <[^>]+> e1400fdf 	ldrd	r0, \[r0, #-255\]	@ 0xffffff01
			80: R_ARM_LDRS_SB_G0	f
0[0-9a-f]+ <[^>]+> e1400fdf 	ldrd	r0, \[r0, #-255\]	@ 0xffffff01
			84: R_ARM_LDRS_SB_G1	f
0[0-9a-f]+ <[^>]+> e1400fdf 	ldrd	r0, \[r0, #-255\]	@ 0xffffff01
			88: R_ARM_LDRS_SB_G2	f
0[0-9a-f]+ <[^>]+> e1400fff 	strd	r0, \[r0, #-255\]	@ 0xffffff01
			8c: R_ARM_LDRS_PC_G1	f
0[0-9a-f]+ <[^>]+> e1400fff 	strd	r0, \[r0, #-255\]	@ 0xffffff01
			90: R_ARM_LDRS_PC_G2	f
0[0-9a-f]+ <[^>]+> e1400fff 	strd	r0, \[r0, #-255\]	@ 0xffffff01
			94: R_ARM_LDRS_SB_G0	f
0[0-9a-f]+ <[^>]+> e1400fff 	strd	r0, \[r0, #-255\]	@ 0xffffff01
			98: R_ARM_LDRS_SB_G1	f
0[0-9a-f]+ <[^>]+> e1400fff 	strd	r0, \[r0, #-255\]	@ 0xffffff01
			9c: R_ARM_LDRS_SB_G2	f
0[0-9a-f]+ <[^>]+> e1500fbf 	ldrh	r0, \[r0, #-255\]	@ 0xffffff01
			a0: R_ARM_LDRS_PC_G1	f
0[0-9a-f]+ <[^>]+> e1500fbf 	ldrh	r0, \[r0, #-255\]	@ 0xffffff01
			a4: R_ARM_LDRS_PC_G2	f
0[0-9a-f]+ <[^>]+> e1500fbf 	ldrh	r0, \[r0, #-255\]	@ 0xffffff01
			a8: R_ARM_LDRS_SB_G0	f
0[0-9a-f]+ <[^>]+> e1500fbf 	ldrh	r0, \[r0, #-255\]	@ 0xffffff01
			ac: R_ARM_LDRS_SB_G1	f
0[0-9a-f]+ <[^>]+> e1500fbf 	ldrh	r0, \[r0, #-255\]	@ 0xffffff01
			b0: R_ARM_LDRS_SB_G2	f
0[0-9a-f]+ <[^>]+> e1400fbf 	strh	r0, \[r0, #-255\]	@ 0xffffff01
			b4: R_ARM_LDRS_PC_G1	f
0[0-9a-f]+ <[^>]+> e1400fbf 	strh	r0, \[r0, #-255\]	@ 0xffffff01
			b8: R_ARM_LDRS_PC_G2	f
0[0-9a-f]+ <[^>]+> e1400fbf 	strh	r0, \[r0, #-255\]	@ 0xffffff01
			bc: R_ARM_LDRS_SB_G0	f
0[0-9a-f]+ <[^>]+> e1400fbf 	strh	r0, \[r0, #-255\]	@ 0xffffff01
			c0: R_ARM_LDRS_SB_G1	f
0[0-9a-f]+ <[^>]+> e1400fbf 	strh	r0, \[r0, #-255\]	@ 0xffffff01
			c4: R_ARM_LDRS_SB_G2	f
0[0-9a-f]+ <[^>]+> e1500fff 	ldrsh	r0, \[r0, #-255\]	@ 0xffffff01
			c8: R_ARM_LDRS_PC_G1	f
0[0-9a-f]+ <[^>]+> e1500fff 	ldrsh	r0, \[r0, #-255\]	@ 0xffffff01
			cc: R_ARM_LDRS_PC_G2	f
0[0-9a-f]+ <[^>]+> e1500fff 	ldrsh	r0, \[r0, #-255\]	@ 0xffffff01
			d0: R_ARM_LDRS_SB_G0	f
0[0-9a-f]+ <[^>]+> e1500fff 	ldrsh	r0, \[r0, #-255\]	@ 0xffffff01
			d4: R_ARM_LDRS_SB_G1	f
0[0-9a-f]+ <[^>]+> e1500fff 	ldrsh	r0, \[r0, #-255\]	@ 0xffffff01
			d8: R_ARM_LDRS_SB_G2	f
0[0-9a-f]+ <[^>]+> e1500fdf 	ldrsb	r0, \[r0, #-255\]	@ 0xffffff01
			dc: R_ARM_LDRS_PC_G1	f
0[0-9a-f]+ <[^>]+> e1500fdf 	ldrsb	r0, \[r0, #-255\]	@ 0xffffff01
			e0: R_ARM_LDRS_PC_G2	f
0[0-9a-f]+ <[^>]+> e1500fdf 	ldrsb	r0, \[r0, #-255\]	@ 0xffffff01
			e4: R_ARM_LDRS_SB_G0	f
0[0-9a-f]+ <[^>]+> e1500fdf 	ldrsb	r0, \[r0, #-255\]	@ 0xffffff01
			e8: R_ARM_LDRS_SB_G1	f
0[0-9a-f]+ <[^>]+> e1500fdf 	ldrsb	r0, \[r0, #-255\]	@ 0xffffff01
			ec: R_ARM_LDRS_SB_G2	f
0[0-9a-f]+ <[^>]+> e1c00fdf 	ldrd	r0, \[r0, #255\]	@ 0xff
			f0: R_ARM_LDRS_PC_G1	localsym
0[0-9a-f]+ <[^>]+> e1c00fdf 	ldrd	r0, \[r0, #255\]	@ 0xff
			f4: R_ARM_LDRS_PC_G2	localsym
0[0-9a-f]+ <[^>]+> e1c00fdf 	ldrd	r0, \[r0, #255\]	@ 0xff
			f8: R_ARM_LDRS_SB_G0	localsym
0[0-9a-f]+ <[^>]+> e1c00fdf 	ldrd	r0, \[r0, #255\]	@ 0xff
			fc: R_ARM_LDRS_SB_G1	localsym
0[0-9a-f]+ <[^>]+> e1c00fdf 	ldrd	r0, \[r0, #255\]	@ 0xff
			100: R_ARM_LDRS_SB_G2	localsym
0[0-9a-f]+ <[^>]+> e1c00fff 	strd	r0, \[r0, #255\]	@ 0xff
			104: R_ARM_LDRS_PC_G1	localsym
0[0-9a-f]+ <[^>]+> e1c00fff 	strd	r0, \[r0, #255\]	@ 0xff
			108: R_ARM_LDRS_PC_G2	localsym
0[0-9a-f]+ <[^>]+> e1c00fff 	strd	r0, \[r0, #255\]	@ 0xff
			10c: R_ARM_LDRS_SB_G0	localsym
0[0-9a-f]+ <[^>]+> e1c00fff 	strd	r0, \[r0, #255\]	@ 0xff
			110: R_ARM_LDRS_SB_G1	localsym
0[0-9a-f]+ <[^>]+> e1c00fff 	strd	r0, \[r0, #255\]	@ 0xff
			114: R_ARM_LDRS_SB_G2	localsym
0[0-9a-f]+ <[^>]+> e1d00fbf 	ldrh	r0, \[r0, #255\]	@ 0xff
			118: R_ARM_LDRS_PC_G1	localsym
0[0-9a-f]+ <[^>]+> e1d00fbf 	ldrh	r0, \[r0, #255\]	@ 0xff
			11c: R_ARM_LDRS_PC_G2	localsym
0[0-9a-f]+ <[^>]+> e1d00fbf 	ldrh	r0, \[r0, #255\]	@ 0xff
			120: R_ARM_LDRS_SB_G0	localsym
0[0-9a-f]+ <[^>]+> e1d00fbf 	ldrh	r0, \[r0, #255\]	@ 0xff
			124: R_ARM_LDRS_SB_G1	localsym
0[0-9a-f]+ <[^>]+> e1d00fbf 	ldrh	r0, \[r0, #255\]	@ 0xff
			128: R_ARM_LDRS_SB_G2	localsym
0[0-9a-f]+ <[^>]+> e1c00fbf 	strh	r0, \[r0, #255\]	@ 0xff
			12c: R_ARM_LDRS_PC_G1	localsym
0[0-9a-f]+ <[^>]+> e1c00fbf 	strh	r0, \[r0, #255\]	@ 0xff
			130: R_ARM_LDRS_PC_G2	localsym
0[0-9a-f]+ <[^>]+> e1c00fbf 	strh	r0, \[r0, #255\]	@ 0xff
			134: R_ARM_LDRS_SB_G0	localsym
0[0-9a-f]+ <[^>]+> e1c00fbf 	strh	r0, \[r0, #255\]	@ 0xff
			138: R_ARM_LDRS_SB_G1	localsym
0[0-9a-f]+ <[^>]+> e1c00fbf 	strh	r0, \[r0, #255\]	@ 0xff
			13c: R_ARM_LDRS_SB_G2	localsym
0[0-9a-f]+ <[^>]+> e1d00fff 	ldrsh	r0, \[r0, #255\]	@ 0xff
			140: R_ARM_LDRS_PC_G1	localsym
0[0-9a-f]+ <[^>]+> e1d00fff 	ldrsh	r0, \[r0, #255\]	@ 0xff
			144: R_ARM_LDRS_PC_G2	localsym
0[0-9a-f]+ <[^>]+> e1d00fff 	ldrsh	r0, \[r0, #255\]	@ 0xff
			148: R_ARM_LDRS_SB_G0	localsym
0[0-9a-f]+ <[^>]+> e1d00fff 	ldrsh	r0, \[r0, #255\]	@ 0xff
			14c: R_ARM_LDRS_SB_G1	localsym
0[0-9a-f]+ <[^>]+> e1d00fff 	ldrsh	r0, \[r0, #255\]	@ 0xff
			150: R_ARM_LDRS_SB_G2	localsym
0[0-9a-f]+ <[^>]+> e1d00fdf 	ldrsb	r0, \[r0, #255\]	@ 0xff
			154: R_ARM_LDRS_PC_G1	localsym
0[0-9a-f]+ <[^>]+> e1d00fdf 	ldrsb	r0, \[r0, #255\]	@ 0xff
			158: R_ARM_LDRS_PC_G2	localsym
0[0-9a-f]+ <[^>]+> e1d00fdf 	ldrsb	r0, \[r0, #255\]	@ 0xff
			15c: R_ARM_LDRS_SB_G0	localsym
0[0-9a-f]+ <[^>]+> e1d00fdf 	ldrsb	r0, \[r0, #255\]	@ 0xff
			160: R_ARM_LDRS_SB_G1	localsym
0[0-9a-f]+ <[^>]+> e1d00fdf 	ldrsb	r0, \[r0, #255\]	@ 0xff
			164: R_ARM_LDRS_SB_G2	localsym
0[0-9a-f]+ <[^>]+> e1400fdf 	ldrd	r0, \[r0, #-255\]	@ 0xffffff01
			168: R_ARM_LDRS_PC_G1	localsym
0[0-9a-f]+ <[^>]+> e1400fdf 	ldrd	r0, \[r0, #-255\]	@ 0xffffff01
			16c: R_ARM_LDRS_PC_G2	localsym
0[0-9a-f]+ <[^>]+> e1400fdf 	ldrd	r0, \[r0, #-255\]	@ 0xffffff01
			170: R_ARM_LDRS_SB_G0	localsym
0[0-9a-f]+ <[^>]+> e1400fdf 	ldrd	r0, \[r0, #-255\]	@ 0xffffff01
			174: R_ARM_LDRS_SB_G1	localsym
0[0-9a-f]+ <[^>]+> e1400fdf 	ldrd	r0, \[r0, #-255\]	@ 0xffffff01
			178: R_ARM_LDRS_SB_G2	localsym
0[0-9a-f]+ <[^>]+> e1400fff 	strd	r0, \[r0, #-255\]	@ 0xffffff01
			17c: R_ARM_LDRS_PC_G1	localsym
0[0-9a-f]+ <[^>]+> e1400fff 	strd	r0, \[r0, #-255\]	@ 0xffffff01
			180: R_ARM_LDRS_PC_G2	localsym
0[0-9a-f]+ <[^>]+> e1400fff 	strd	r0, \[r0, #-255\]	@ 0xffffff01
			184: R_ARM_LDRS_SB_G0	localsym
0[0-9a-f]+ <[^>]+> e1400fff 	strd	r0, \[r0, #-255\]	@ 0xffffff01
			188: R_ARM_LDRS_SB_G1	localsym
0[0-9a-f]+ <[^>]+> e1400fff 	strd	r0, \[r0, #-255\]	@ 0xffffff01
			18c: R_ARM_LDRS_SB_G2	localsym
0[0-9a-f]+ <[^>]+> e1500fbf 	ldrh	r0, \[r0, #-255\]	@ 0xffffff01
			190: R_ARM_LDRS_PC_G1	localsym
0[0-9a-f]+ <[^>]+> e1500fbf 	ldrh	r0, \[r0, #-255\]	@ 0xffffff01
			194: R_ARM_LDRS_PC_G2	localsym
0[0-9a-f]+ <[^>]+> e1500fbf 	ldrh	r0, \[r0, #-255\]	@ 0xffffff01
			198: R_ARM_LDRS_SB_G0	localsym
0[0-9a-f]+ <[^>]+> e1500fbf 	ldrh	r0, \[r0, #-255\]	@ 0xffffff01
			19c: R_ARM_LDRS_SB_G1	localsym
0[0-9a-f]+ <[^>]+> e1500fbf 	ldrh	r0, \[r0, #-255\]	@ 0xffffff01
			1a0: R_ARM_LDRS_SB_G2	localsym
0[0-9a-f]+ <[^>]+> e1400fbf 	strh	r0, \[r0, #-255\]	@ 0xffffff01
			1a4: R_ARM_LDRS_PC_G1	localsym
0[0-9a-f]+ <[^>]+> e1400fbf 	strh	r0, \[r0, #-255\]	@ 0xffffff01
			1a8: R_ARM_LDRS_PC_G2	localsym
0[0-9a-f]+ <[^>]+> e1400fbf 	strh	r0, \[r0, #-255\]	@ 0xffffff01
			1ac: R_ARM_LDRS_SB_G0	localsym
0[0-9a-f]+ <[^>]+> e1400fbf 	strh	r0, \[r0, #-255\]	@ 0xffffff01
			1b0: R_ARM_LDRS_SB_G1	localsym
0[0-9a-f]+ <[^>]+> e1400fbf 	strh	r0, \[r0, #-255\]	@ 0xffffff01
			1b4: R_ARM_LDRS_SB_G2	localsym
0[0-9a-f]+ <[^>]+> e1500fff 	ldrsh	r0, \[r0, #-255\]	@ 0xffffff01
			1b8: R_ARM_LDRS_PC_G1	localsym
0[0-9a-f]+ <[^>]+> e1500fff 	ldrsh	r0, \[r0, #-255\]	@ 0xffffff01
			1bc: R_ARM_LDRS_PC_G2	localsym
0[0-9a-f]+ <[^>]+> e1500fff 	ldrsh	r0, \[r0, #-255\]	@ 0xffffff01
			1c0: R_ARM_LDRS_SB_G0	localsym
0[0-9a-f]+ <[^>]+> e1500fff 	ldrsh	r0, \[r0, #-255\]	@ 0xffffff01
			1c4: R_ARM_LDRS_SB_G1	localsym
0[0-9a-f]+ <[^>]+> e1500fff 	ldrsh	r0, \[r0, #-255\]	@ 0xffffff01
			1c8: R_ARM_LDRS_SB_G2	localsym
0[0-9a-f]+ <[^>]+> e1500fdf 	ldrsb	r0, \[r0, #-255\]	@ 0xffffff01
			1cc: R_ARM_LDRS_PC_G1	localsym
0[0-9a-f]+ <[^>]+> e1500fdf 	ldrsb	r0, \[r0, #-255\]	@ 0xffffff01
			1d0: R_ARM_LDRS_PC_G2	localsym
0[0-9a-f]+ <[^>]+> e1500fdf 	ldrsb	r0, \[r0, #-255\]	@ 0xffffff01
			1d4: R_ARM_LDRS_SB_G0	localsym
0[0-9a-f]+ <[^>]+> e1500fdf 	ldrsb	r0, \[r0, #-255\]	@ 0xffffff01
			1d8: R_ARM_LDRS_SB_G1	localsym
0[0-9a-f]+ <[^>]+> e1500fdf 	ldrsb	r0, \[r0, #-255\]	@ 0xffffff01
			1dc: R_ARM_LDRS_SB_G2	localsym
0[0-9a-f]+ <[^>]+> e3a00000 	mov	r0, #0
