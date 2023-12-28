# Test relocation overflow and insufficiently divisible values for
# PC-relative operands.
.text
.nocmp
f7_0:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
f7_28:
	nop
f7_32:
	.space 256
f7_288:
	addkpc .S2 f7_32,b1,0
	addkpc .S2 f7_28,b1,0
	addkpc .S2 f7_32,b1,0
	addkpc .S2 f7_0,b1,0
	addkpc .S2 f7_544,b1,0
	addkpc .S2 f7_540,b1,0
	addkpc .S2 f7_288+1,b1,0
	nop
f7_320:
	.space 220
f7_540:
	nop
f7_544:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
f10_0:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
f10_28:
	nop
f10_32:
	.space 2048
f10_2080:
	bdec .S1 f10_32,a1
	bdec .S1 f10_28,a1
	bdec .S1 f10_32,a1
	bdec .S1 f10_0,a1
	bdec .S1 f10_4128,a1
	bdec .S1 f10_4124,a1
	bdec .S1 f10_2080+1,a1
	nop
f10_2112:
	.space 2012
f10_4124:
	nop
f10_4128:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
g10_0:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
g10_28:
	nop
g10_32:
	.space 2048
g10_2080:
	bpos .S1 g10_32,a1
	bpos .S1 g10_28,a1
	bpos .S1 g10_32,a1
	bpos .S1 g10_0,a1
	bpos .S1 g10_4128,a1
	bpos .S1 g10_4124,a1
	bpos .S1 g10_2080+1,a1
	nop
g10_2112:
	.space 2012
g10_4124:
	nop
g10_4128:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
f12_0:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
f12_28:
	nop
f12_32:
	.space 8192
f12_8224:
	bnop f12_32,2
	bnop f12_28,2
	bnop f12_32,2
	bnop f12_0,2
	bnop f12_16416,2
	bnop f12_16412,2
	bnop f12_8224+1,2
	nop
f12_8256:
	.space 8156
f12_16412:
	nop
f12_16416:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
g12_0:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
g12_28:
	nop
g12_32:
	.space 8192
g12_8224:
	callnop g12_32,2
	callnop g12_28,2
	callnop g12_32,2
	callnop g12_0,2
	callnop g12_16416,2
	callnop g12_16412,2
	callnop g12_8224+1,2
	nop
g12_8256:
	.space 8156
g12_16412:
	nop
g12_16416:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
f21_0:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
f21_28:
	nop
f21_32:
	.space 4194304
f21_4194336:
	b .S1 f21_32
	b .S1 f21_28
	b .S1 f21_32
	b .S1 f21_0
	b .S1 f21_8388640
	b .S1 f21_8388636
	b .S1 f21_4194336+1
	nop
f21_4194368:
	.space 4194268
f21_8388636:
	nop
f21_8388640:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
g21_0:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
g21_28:
	nop
g21_32:
	.space 4194304
g21_4194336:
	call .S1 g21_32
	call .S1 g21_28
	call .S1 g21_32
	call .S1 g21_0
	call .S1 g21_8388640
	call .S1 g21_8388636
	call .S1 g21_4194336+1
	nop
g21_4194368:
	.space 4194268
g21_8388636:
	nop
g21_8388640:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
h21_0:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
h21_28:
	nop
h21_32:
	.space 4194304
h21_4194336:
	callp .S2 h21_32,b3
	callp .S2 h21_28,b3
	callp .S2 h21_32,b3
	callp .S2 h21_0,b3
	callp .S2 h21_8388640,b3
	callp .S2 h21_8388636,b3
	callp .S2 h21_4194336+1,b3
	nop
h21_4194368:
	.space 4194268
h21_8388636:
	nop
h21_8388640:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
i21_0:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
i21_28:
	nop
i21_32:
	.space 4194304
i21_4194336:
	callret .S1 i21_32
	callret .S1 i21_28
	callret .S1 i21_32
	callret .S1 i21_0
	callret .S1 i21_8388640
	callret .S1 i21_8388636
	callret .S1 i21_4194336+1
	nop
i21_4194368:
	.space 4194268
i21_8388636:
	nop
i21_8388640:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
j21_0:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
j21_28:
	nop
j21_32:
	.space 4194304
j21_4194336:
	ret .S1 j21_32
	ret .S1 j21_28
	ret .S1 j21_32
	ret .S1 j21_0
	ret .S1 j21_8388640
	ret .S1 j21_8388636
	ret .S1 j21_4194336+1
	nop
j21_4194368:
	.space 4194268
j21_8388636:
	nop
j21_8388640:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
k21_0:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
k21_28:
	nop
k21_32:
	.space 4194304
k21_4194336:
	retp .S1 k21_32,a3
	retp .S1 k21_28,a3
	retp .S1 k21_32,a3
	retp .S1 k21_0,a3
	retp .S1 k21_8388640,a3
	retp .S1 k21_8388636,a3
	retp .S1 k21_4194336+1,a3
	nop
k21_4194368:
	.space 4194268
k21_8388636:
	nop
k21_8388640:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
