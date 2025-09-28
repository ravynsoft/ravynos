#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
.*:	d6bf03e0 	drps
.*:	d503201f 	nop
.*:	d503203f 	yield
.*:	d503205f 	wfe
.*:	d503207f 	wfi
.*:	d503209f 	sev
.*:	d50320bf 	sevl
.*:	d50322df 	clearbhb
.*:	d503201f 	nop
.*:	d503203f 	yield
.*:	d503205f 	wfe
.*:	d503207f 	wfi
.*:	d503209f 	sev
.*:	d50320bf 	sevl
.*:	d50320df 	hint	#0x6
.*:	d50320ff 	(hint	#0x7|xpaclri)
.*:	d503211f 	(hint	#0x8|pacia1716)
.*:	d503213f 	hint	#0x9
.*:	d503215f 	(hint	#0xa|pacib1716)
.*:	d503217f 	hint	#0xb
.*:	d503219f 	(hint	#0xc|autia1716)
.*:	d50321bf 	hint	#0xd
.*:	d50321df 	(hint	#0xe|autib1716)
.*:	d50321ff 	hint	#0xf
.*:	d503221f 	(hint	#0x10|esb)
.*:	d503223f 	(hint	#0x11|psb	csync)
.*:	d503225f 	(hint	#0x12|tsb	csync)
.*:	d503227f 	hint	#0x13
.*:	d503229f 	(hint	#0x14|csdb)
.*:	d50322bf 	hint	#0x15
.*:	d50322df 	(hint	#0x16|clearbhb)
.*:	d50322ff 	hint	#0x17
.*:	d503231f 	(hint	#0x18|paciaz)
.*:	d503233f 	(hint	#0x19|paciasp)
.*:	d503235f 	(hint	#0x1a|pacibz)
.*:	d503237f 	(hint	#0x1b|pacibsp)
.*:	d503239f 	(hint	#0x1c|autiaz)
.*:	d50323bf 	(hint	#0x1d|autiasp)
.*:	d50323df 	(hint	#0x1e|autibz)
.*:	d50323ff 	(hint	#0x1f|autibsp)
.*:	d503241f 	(hint	#0x20|bti)
.*:	d503243f 	hint	#0x21
.*:	d503245f 	(hint	#0x22|bti	c)
.*:	d503247f 	hint	#0x23
.*:	d503249f 	(hint	#0x24|bti	j)
.*:	d50324bf 	hint	#0x25
.*:	d50324df 	(hint	#0x26|bti	jc)
.*:	d50324ff 	hint	#0x27
.*:	d503251f 	hint	#0x28
.*:	d503253f 	hint	#0x29
.*:	d503255f 	hint	#0x2a
.*:	d503257f 	hint	#0x2b
.*:	d503259f 	hint	#0x2c
.*:	d50325bf 	hint	#0x2d
.*:	d50325df 	hint	#0x2e
.*:	d50325ff 	hint	#0x2f
.*:	d503261f 	hint	#0x30
.*:	d503263f 	hint	#0x31
.*:	d503265f 	hint	#0x32
.*:	d503267f 	hint	#0x33
.*:	d503269f 	hint	#0x34
.*:	d50326bf 	hint	#0x35
.*:	d50326df 	hint	#0x36
.*:	d50326ff 	hint	#0x37
.*:	d503271f 	hint	#0x38
.*:	d503273f 	hint	#0x39
.*:	d503275f 	hint	#0x3a
.*:	d503277f 	hint	#0x3b
.*:	d503279f 	hint	#0x3c
.*:	d50327bf 	hint	#0x3d
.*:	d50327df 	hint	#0x3e
.*:	d50327ff 	hint	#0x3f
.*:	d503281f 	hint	#0x40
.*:	d503283f 	hint	#0x41
.*:	d503285f 	hint	#0x42
.*:	d503287f 	hint	#0x43
.*:	d503289f 	hint	#0x44
.*:	d50328bf 	hint	#0x45
.*:	d50328df 	hint	#0x46
.*:	d50328ff 	hint	#0x47
.*:	d503291f 	hint	#0x48
.*:	d503293f 	hint	#0x49
.*:	d503295f 	hint	#0x4a
.*:	d503297f 	hint	#0x4b
.*:	d503299f 	hint	#0x4c
.*:	d50329bf 	hint	#0x4d
.*:	d50329df 	hint	#0x4e
.*:	d50329ff 	hint	#0x4f
.*:	d5032a1f 	hint	#0x50
.*:	d5032a3f 	hint	#0x51
.*:	d5032a5f 	hint	#0x52
.*:	d5032a7f 	hint	#0x53
.*:	d5032a9f 	hint	#0x54
.*:	d5032abf 	hint	#0x55
.*:	d5032adf 	hint	#0x56
.*:	d5032aff 	hint	#0x57
.*:	d5032b1f 	hint	#0x58
.*:	d5032b3f 	hint	#0x59
.*:	d5032b5f 	hint	#0x5a
.*:	d5032b7f 	hint	#0x5b
.*:	d5032b9f 	hint	#0x5c
.*:	d5032bbf 	hint	#0x5d
.*:	d5032bdf 	hint	#0x5e
.*:	d5032bff 	hint	#0x5f
.*:	d5032c1f 	hint	#0x60
.*:	d5032c3f 	hint	#0x61
.*:	d5032c5f 	hint	#0x62
.*:	d5032c7f 	hint	#0x63
.*:	d5032c9f 	hint	#0x64
.*:	d5032cbf 	hint	#0x65
.*:	d5032cdf 	hint	#0x66
.*:	d5032cff 	hint	#0x67
.*:	d5032d1f 	hint	#0x68
.*:	d5032d3f 	hint	#0x69
.*:	d5032d5f 	hint	#0x6a
.*:	d5032d7f 	hint	#0x6b
.*:	d5032d9f 	hint	#0x6c
.*:	d5032dbf 	hint	#0x6d
.*:	d5032ddf 	hint	#0x6e
.*:	d5032dff 	hint	#0x6f
.*:	d5032e1f 	hint	#0x70
.*:	d5032e3f 	hint	#0x71
.*:	d5032e5f 	hint	#0x72
.*:	d5032e7f 	hint	#0x73
.*:	d5032e9f 	hint	#0x74
.*:	d5032ebf 	hint	#0x75
.*:	d5032edf 	hint	#0x76
.*:	d5032eff 	hint	#0x77
.*:	d5032f1f 	hint	#0x78
.*:	d5032f3f 	hint	#0x79
.*:	d5032f5f 	hint	#0x7a
.*:	d5032f7f 	hint	#0x7b
.*:	d5032f9f 	hint	#0x7c
.*:	d5032fbf 	hint	#0x7d
.*:	d5032fdf 	hint	#0x7e
.*:	d5032fff 	hint	#0x7f
.*:	d52bf7e7 	sysl	x7, #3, C15, C7, #7
.*:	d503309f 	ssbb
.*:	d503319f 	dsb	oshld
.*:	d503329f 	dsb	oshst
.*:	d503339f 	dsb	osh
.*:	d503349f 	pssbb
.*:	d503359f 	dsb	nshld
.*:	d503369f 	dsb	nshst
.*:	d503379f 	dsb	nsh
.*:	d503389f 	dsb	#0x08
.*:	d503399f 	dsb	ishld
.*:	d5033a9f 	dsb	ishst
.*:	d5033b9f 	dsb	ish
.*:	d5033c9f 	dsb	#0x0c
.*:	d5033d9f 	dsb	ld
.*:	d5033e9f 	dsb	st
.*:	d5033f9f 	dsb	sy
.*:	d50330bf 	dmb	#0x00
.*:	d50331bf 	dmb	oshld
.*:	d50332bf 	dmb	oshst
.*:	d50333bf 	dmb	osh
.*:	d50334bf 	dmb	#0x04
.*:	d50335bf 	dmb	nshld
.*:	d50336bf 	dmb	nshst
.*:	d50337bf 	dmb	nsh
.*:	d50338bf 	dmb	#0x08
.*:	d50339bf 	dmb	ishld
.*:	d5033abf 	dmb	ishst
.*:	d5033bbf 	dmb	ish
.*:	d5033cbf 	dmb	#0x0c
.*:	d5033dbf 	dmb	ld
.*:	d5033ebf 	dmb	st
.*:	d5033fbf 	dmb	sy
.*:	d50330df 	isb	#0x0
.*:	d50331df 	isb	#0x1
.*:	d50332df 	isb	#0x2
.*:	d50333df 	isb	#0x3
.*:	d50334df 	isb	#0x4
.*:	d50335df 	isb	#0x5
.*:	d50336df 	isb	#0x6
.*:	d50337df 	isb	#0x7
.*:	d50338df 	isb	#0x8
.*:	d50339df 	isb	#0x9
.*:	d5033adf 	isb	#0xa
.*:	d5033bdf 	isb	#0xb
.*:	d5033cdf 	isb	#0xc
.*:	d5033ddf 	isb	#0xd
.*:	d5033edf 	isb	#0xe
.*:	d5033fdf 	isb
.*:	d5033fdf 	isb
.*:	d5033fdf 	isb
.*:	d503309f 	ssbb
.*:	d503349f 	pssbb
.*:	d503319f 	dsb	oshld
.*:	d503329f 	dsb	oshst
.*:	d503339f 	dsb	osh
.*:	d503359f 	dsb	nshld
.*:	d503369f 	dsb	nshst
.*:	d503379f 	dsb	nsh
.*:	d503389f 	dsb	#0x08
.*:	d503399f 	dsb	ishld
.*:	d5033a9f 	dsb	ishst
.*:	d5033b9f 	dsb	ish
.*:	d5033c9f 	dsb	#0x0c
.*:	d5033d9f 	dsb	ld
.*:	d5033e9f 	dsb	st
.*:	d5033f9f 	dsb	sy
.*:	d8000000 	prfm	pldl1keep, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f8af6be0 	prfm	pldl1keep, \[sp, x15\]
.*:	f8be58e0 	prfm	pldl1keep, \[x7, w30, uxtw #3\]
.*:	f9800c60 	prfm	pldl1keep, \[x3, #24\]
.*:	d8000001 	prfm	pldl1strm, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f8af6be1 	prfm	pldl1strm, \[sp, x15\]
.*:	f8be58e1 	prfm	pldl1strm, \[x7, w30, uxtw #3\]
.*:	f9800c61 	prfm	pldl1strm, \[x3, #24\]
.*:	d8000002 	prfm	pldl2keep, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f8af6be2 	prfm	pldl2keep, \[sp, x15\]
.*:	f8be58e2 	prfm	pldl2keep, \[x7, w30, uxtw #3\]
.*:	f9800c62 	prfm	pldl2keep, \[x3, #24\]
.*:	d8000003 	prfm	pldl2strm, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f8af6be3 	prfm	pldl2strm, \[sp, x15\]
.*:	f8be58e3 	prfm	pldl2strm, \[x7, w30, uxtw #3\]
.*:	f9800c63 	prfm	pldl2strm, \[x3, #24\]
.*:	d8000004 	prfm	pldl3keep, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f8af6be4 	prfm	pldl3keep, \[sp, x15\]
.*:	f8be58e4 	prfm	pldl3keep, \[x7, w30, uxtw #3\]
.*:	f9800c64 	prfm	pldl3keep, \[x3, #24\]
.*:	d8000005 	prfm	pldl3strm, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f8af6be5 	prfm	pldl3strm, \[sp, x15\]
.*:	f8be58e5 	prfm	pldl3strm, \[x7, w30, uxtw #3\]
.*:	f9800c65 	prfm	pldl3strm, \[x3, #24\]
.*:	d8000006 	prfm	#0x06, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f8af6be6 	prfm	#0x06, \[sp, x15\]
.*:	f8be58e6 	prfm	#0x06, \[x7, w30, uxtw #3\]
.*:	f9800c66 	prfm	#0x06, \[x3, #24\]
.*:	d8000007 	prfm	#0x07, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f8af6be7 	prfm	#0x07, \[sp, x15\]
.*:	f8be58e7 	prfm	#0x07, \[x7, w30, uxtw #3\]
.*:	f9800c67 	prfm	#0x07, \[x3, #24\]
.*:	d8000008 	prfm	plil1keep, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f8af6be8 	prfm	plil1keep, \[sp, x15\]
.*:	f8be58e8 	prfm	plil1keep, \[x7, w30, uxtw #3\]
.*:	f9800c68 	prfm	plil1keep, \[x3, #24\]
.*:	d8000009 	prfm	plil1strm, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f8af6be9 	prfm	plil1strm, \[sp, x15\]
.*:	f8be58e9 	prfm	plil1strm, \[x7, w30, uxtw #3\]
.*:	f9800c69 	prfm	plil1strm, \[x3, #24\]
.*:	d800000a 	prfm	plil2keep, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f8af6bea 	prfm	plil2keep, \[sp, x15\]
.*:	f8be58ea 	prfm	plil2keep, \[x7, w30, uxtw #3\]
.*:	f9800c6a 	prfm	plil2keep, \[x3, #24\]
.*:	d800000b 	prfm	plil2strm, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f8af6beb 	prfm	plil2strm, \[sp, x15\]
.*:	f8be58eb 	prfm	plil2strm, \[x7, w30, uxtw #3\]
.*:	f9800c6b 	prfm	plil2strm, \[x3, #24\]
.*:	d800000c 	prfm	plil3keep, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f8af6bec 	prfm	plil3keep, \[sp, x15\]
.*:	f8be58ec 	prfm	plil3keep, \[x7, w30, uxtw #3\]
.*:	f9800c6c 	prfm	plil3keep, \[x3, #24\]
.*:	d800000d 	prfm	plil3strm, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f8af6bed 	prfm	plil3strm, \[sp, x15\]
.*:	f8be58ed 	prfm	plil3strm, \[x7, w30, uxtw #3\]
.*:	f9800c6d 	prfm	plil3strm, \[x3, #24\]
.*:	d800000e 	prfm	#0x0e, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f8af6bee 	prfm	#0x0e, \[sp, x15\]
.*:	f8be58ee 	prfm	#0x0e, \[x7, w30, uxtw #3\]
.*:	f9800c6e 	prfm	#0x0e, \[x3, #24\]
.*:	d800000f 	prfm	#0x0f, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f8af6bef 	prfm	#0x0f, \[sp, x15\]
.*:	f8be58ef 	prfm	#0x0f, \[x7, w30, uxtw #3\]
.*:	f9800c6f 	prfm	#0x0f, \[x3, #24\]
.*:	d8000010 	prfm	pstl1keep, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f8af6bf0 	prfm	pstl1keep, \[sp, x15\]
.*:	f8be58f0 	prfm	pstl1keep, \[x7, w30, uxtw #3\]
.*:	f9800c70 	prfm	pstl1keep, \[x3, #24\]
.*:	d8000011 	prfm	pstl1strm, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f8af6bf1 	prfm	pstl1strm, \[sp, x15\]
.*:	f8be58f1 	prfm	pstl1strm, \[x7, w30, uxtw #3\]
.*:	f9800c71 	prfm	pstl1strm, \[x3, #24\]
.*:	d8000012 	prfm	pstl2keep, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f8af6bf2 	prfm	pstl2keep, \[sp, x15\]
.*:	f8be58f2 	prfm	pstl2keep, \[x7, w30, uxtw #3\]
.*:	f9800c72 	prfm	pstl2keep, \[x3, #24\]
.*:	d8000013 	prfm	pstl2strm, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f8af6bf3 	prfm	pstl2strm, \[sp, x15\]
.*:	f8be58f3 	prfm	pstl2strm, \[x7, w30, uxtw #3\]
.*:	f9800c73 	prfm	pstl2strm, \[x3, #24\]
.*:	d8000014 	prfm	pstl3keep, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f8af6bf4 	prfm	pstl3keep, \[sp, x15\]
.*:	f8be58f4 	prfm	pstl3keep, \[x7, w30, uxtw #3\]
.*:	f9800c74 	prfm	pstl3keep, \[x3, #24\]
.*:	d8000015 	prfm	pstl3strm, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f8af6bf5 	prfm	pstl3strm, \[sp, x15\]
.*:	f8be58f5 	prfm	pstl3strm, \[x7, w30, uxtw #3\]
.*:	f9800c75 	prfm	pstl3strm, \[x3, #24\]
.*:	d8000016 	prfm	#0x16, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f8af6bf6 	prfm	#0x16, \[sp, x15\]
.*:	f8be58f6 	prfm	#0x16, \[x7, w30, uxtw #3\]
.*:	f9800c76 	prfm	#0x16, \[x3, #24\]
.*:	d8000017 	prfm	#0x17, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f8af6bf7 	prfm	#0x17, \[sp, x15\]
.*:	f8be58f7 	prfm	#0x17, \[x7, w30, uxtw #3\]
.*:	f9800c77 	prfm	#0x17, \[x3, #24\]
.*:	d8000018 	prfm	#0x18, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f9800c78 	prfm	#0x18, \[x3, #24\]
.*:	d8000019 	prfm	#0x19, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f9800c79 	prfm	#0x19, \[x3, #24\]
.*:	d800001a 	prfm	#0x1a, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f9800c7a 	prfm	#0x1a, \[x3, #24\]
.*:	d800001b 	prfm	#0x1b, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f9800c7b 	prfm	#0x1b, \[x3, #24\]
.*:	d800001c 	prfm	#0x1c, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f9800c7c 	prfm	#0x1c, \[x3, #24\]
.*:	d800001d 	prfm	#0x1d, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f9800c7d 	prfm	#0x1d, \[x3, #24\]
.*:	d800001e 	prfm	#0x1e, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f9800c7e 	prfm	#0x1e, \[x3, #24\]
.*:	d800001f 	prfm	#0x1f, 0 <LABEL1>
.*: R_AARCH64_(P32_|)LD_PREL_LO19	LABEL1
.*:	f9800c7f 	prfm	#0x1f, \[x3, #24\]
.*:	f9800c60 	prfm	pldl1keep, \[x3, #24\]
.*:	f9800c61 	prfm	pldl1strm, \[x3, #24\]
.*:	f9800c62 	prfm	pldl2keep, \[x3, #24\]
.*:	f9800c63 	prfm	pldl2strm, \[x3, #24\]
.*:	f9800c64 	prfm	pldl3keep, \[x3, #24\]
.*:	f9800c65 	prfm	pldl3strm, \[x3, #24\]
.*:	f9800c68 	prfm	plil1keep, \[x3, #24\]
.*:	f9800c69 	prfm	plil1strm, \[x3, #24\]
.*:	f9800c6a 	prfm	plil2keep, \[x3, #24\]
.*:	f9800c6b 	prfm	plil2strm, \[x3, #24\]
.*:	f9800c6c 	prfm	plil3keep, \[x3, #24\]
.*:	f9800c6d 	prfm	plil3strm, \[x3, #24\]
.*:	f9800c70 	prfm	pstl1keep, \[x3, #24\]
.*:	f9800c71 	prfm	pstl1strm, \[x3, #24\]
.*:	f9800c72 	prfm	pstl2keep, \[x3, #24\]
.*:	f9800c73 	prfm	pstl2strm, \[x3, #24\]
.*:	f9800c74 	prfm	pstl3keep, \[x3, #24\]
.*:	f9800c75 	prfm	pstl3strm, \[x3, #24\]
.*:	f8a04817 	prfm	#0x17, \[x0, w0, uxtw\]
.*:	f8a04818 	rprfm	pldkeep, x0, \[x0\]
