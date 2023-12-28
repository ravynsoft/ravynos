/* The instructions with non-zero register numbers are there to ensure we have
   the correct argument positioning (i.e. check that the first argument is at
   the end of the word etc).
   The instructions with all-zero register numbers are to ensure the previous
   encoding didn't just "happen" to fit -- so that if we change the registers
   that changes the correct part of the word.
   Each of the numbered patterns begin and end with a 1, so we can replace
   them with all-zeros and see the entire range has changed.
   17 -> 10001
   21 -> 10101
   27 -> 11011
   */

movprfx z0, z1
adclb z0.d, z1.d, z2.d

adclb z17.s, z21.s, z27.s
adclb z0.s, z0.s, z0.s
adclb z0.d, z0.d, z0.d

adclt z17.s, z21.s, z27.s
adclt z0.s, z0.s, z0.s
adclt z0.d, z0.d, z0.d

addhnb z17.b, z21.h, z27.h
addhnb z0.b, z0.h, z0.h
addhnb z0.h, z0.s, z0.s
addhnb z0.s, z0.d, z0.d

addhnt z17.b, z21.h, z27.h
addhnt z0.b, z0.h, z0.h
addhnt z0.h, z0.s, z0.s
addhnt z0.s, z0.d, z0.d

movprfx z0.d, p0/m, z1.d
addp z0.d, p0/m, z0.d, z1.d

addp z17.b, p5/m, z17.b, z21.b
addp z0.b, p0/m, z0.b, z0.b
addp z0.h, p0/m, z0.h, z0.h
addp z0.s, p0/m, z0.s, z0.s
addp z0.d, p0/m, z0.d, z0.d

aesd z17.b, z17.b, z21.b
aesd z0.b, z0.b, z0.b
aese z17.b, z17.b, z21.b
aese z0.b, z0.b, z0.b

aesimc z17.b, z17.b
aesimc z0.b, z0.b

aesmc z17.b, z17.b
aesmc z0.b, z0.b

bcax z17.d, z17.d, z21.d, z27.d
bcax z0.d, z0.d, z0.d, z0.d

bsl z17.d, z17.d, z21.d, z27.d
bsl z0.d, z0.d, z0.d, z0.d

bsl1n z17.d, z17.d, z21.d, z27.d
bsl1n z0.d, z0.d, z0.d, z0.d

bsl2n z17.d, z17.d, z21.d, z27.d
bsl2n z0.d, z0.d, z0.d, z0.d

bdep z17.b, z21.b, z27.b
bdep z0.b, z0.b, z0.b
bdep z0.h, z0.h, z0.h
bdep z0.s, z0.s, z0.s
bdep z0.d, z0.d, z0.d

bext z17.b, z21.b, z27.b
bext z0.b, z0.b, z0.b
bext z0.h, z0.h, z0.h
bext z0.s, z0.s, z0.s
bext z0.d, z0.d, z0.d

bgrp z17.b, z21.b, z27.b
bgrp z0.b, z0.b, z0.b
bgrp z0.h, z0.h, z0.h
bgrp z0.s, z0.s, z0.s
bgrp z0.d, z0.d, z0.d

cadd z17.b, z17.b, z21.b, #90
cadd z0.b, z0.b, z0.b, #90
cadd z0.h, z0.h, z0.h, #90
cadd z0.s, z0.s, z0.s, #90
cadd z0.d, z0.d, z0.d, #90
cadd z0.b, z0.b, z0.b, #270

cdot z17.s, z21.b, z3.b[3], #0
cdot z0.s, z0.b, z0.b[0], #0
cdot z0.s, z0.b, z0.b[0], #90
cdot z0.s, z0.b, z0.b[0], #180
cdot z0.s, z0.b, z0.b[0], #270

cdot z17.d, z21.h, z11.h[1], #0
cdot z0.d, z0.h, z0.h[0], #0
cdot z0.d, z0.h, z0.h[0], #90
cdot z0.d, z0.h, z0.h[0], #180
cdot z0.d, z0.h, z0.h[0], #270

cdot z17.s, z21.b, z27.b, #0
cdot z0.s, z0.b, z0.b, #0
cdot z0.d, z0.h, z0.h, #0
cdot z0.s, z0.b, z0.b, #90
cdot z0.s, z0.b, z0.b, #180
cdot z0.s, z0.b, z0.b, #270

cmla z17.h, z21.h, z3.h[3], #0
cmla z0.h, z0.h, z0.h[0], #0
cmla z0.h, z0.h, z0.h[0], #90
cmla z0.h, z0.h, z0.h[0], #180
cmla z0.h, z0.h, z0.h[0], #270

cmla z17.s, z21.s, z11.s[1], #0
cmla z0.s, z0.s, z0.s[0], #0
cmla z0.s, z0.s, z0.s[0], #90
cmla z0.s, z0.s, z0.s[0], #180
cmla z0.s, z0.s, z0.s[0], #270

cmla z17.b, z21.b, z27.b, #0
cmla z0.b, z0.b, z0.b, #0
cmla z0.h, z0.h, z0.h, #0
cmla z0.s, z0.s, z0.s, #0
cmla z0.d, z0.d, z0.d, #0
cmla z0.b, z0.b, z0.b, #90
cmla z0.b, z0.b, z0.b, #180
cmla z0.b, z0.b, z0.b, #270

eor3 z17.d, z17.d, z21.d, z27.d
eor3 z0.d, z0.d, z0.d, z0.d

eorbt z17.b, z21.b, z27.b
eorbt z0.b, z0.b, z0.b
eorbt z0.h, z0.h, z0.h
eorbt z0.s, z0.s, z0.s
eorbt z0.d, z0.d, z0.d

eortb z17.b, z21.b, z27.b
eortb z0.b, z0.b, z0.b
eortb z0.h, z0.h, z0.h
eortb z0.s, z0.s, z0.s
eortb z0.d, z0.d, z0.d

ext z17.b, { z21.b, z22.b }, #221
ext z0.b, { z0.b, z1.b }, #0
ext z0.b, { z31.b, z0.b }, #0

faddp z17.h, p5/m, z17.h, z21.h
faddp z0.h, p0/m, z0.h, z0.h
faddp z0.s, p0/m, z0.s, z0.s
faddp z0.d, p0/m, z0.d, z0.d

fcvtlt z17.s, p5/m, z21.h
fcvtlt z0.s, p0/m, z0.h
fcvtlt z17.d, p5/m, z21.s
fcvtlt z0.d, p0/m, z0.s

fcvtnt z17.h, p5/m, z21.s
fcvtnt z0.h, p0/m, z0.s
fcvtnt z17.s, p5/m, z21.d
fcvtnt z0.s, p0/m, z0.d

fcvtx z17.s, p5/m, z21.d
fcvtx z0.s, p0/m, z0.d

movprfx z0.d, p0/z, z1.d
fcvtx z0.s, p0/m, z2.d

fcvtxnt z17.s, p5/m, z21.d
fcvtxnt z0.s, p0/m, z0.d

flogb z17.h, p5/m, z21.h
flogb z0.h, p0/m, z0.h
flogb z0.s, p0/m, z0.s
flogb z0.d, p0/m, z0.d

fmaxnmp z17.h, p5/m, z17.h, z21.h
fmaxnmp z0.h, p0/m, z0.h, z0.h
fmaxnmp z0.s, p0/m, z0.s, z0.s
fmaxnmp z0.d, p0/m, z0.d, z0.d

fmaxp z17.h, p5/m, z17.h, z21.h
fmaxp z0.h, p0/m, z0.h, z0.h
fmaxp z0.s, p0/m, z0.s, z0.s
fmaxp z0.d, p0/m, z0.d, z0.d

fminnmp z17.h, p5/m, z17.h, z21.h
fminnmp z0.h, p0/m, z0.h, z0.h
fminnmp z0.s, p0/m, z0.s, z0.s
fminnmp z0.d, p0/m, z0.d, z0.d

fminp z17.h, p5/m, z17.h, z21.h
fminp z0.h, p0/m, z0.h, z0.h
fminp z0.s, p0/m, z0.s, z0.s
fminp z0.d, p0/m, z0.d, z0.d

fmlalb z17.s, z21.h, z5.h[0]
fmlalb z0.s, z0.h, z0.h[5]
fmlalb z0.s, z0.h, z0.h[0]

fmlalb z17.s, z21.h, z27.h
fmlalb z0.s, z0.h, z0.h

fmlalt z17.s, z21.h, z5.h[0]
fmlalt z0.s, z0.h, z0.h[5]
fmlalt z0.s, z0.h, z0.h[0]

fmlalt z17.s, z21.h, z27.h
fmlalt z0.s, z0.h, z0.h

fmlslb z17.s, z21.h, z5.h[0]
fmlslb z0.s, z0.h, z0.h[5]
fmlslb z0.s, z0.h, z0.h[0]

fmlslb z17.s, z21.h, z27.h
fmlslb z0.s, z0.h, z0.h

fmlslt z17.s, z21.h, z5.h[0]
fmlslt z0.s, z0.h, z0.h[5]
fmlslt z0.s, z0.h, z0.h[0]

fmlslt z17.s, z21.h, z27.h
fmlslt z0.s, z0.h, z0.h

histcnt z17.s, p5/z, z21.s, z27.s
histcnt z0.s, p0/z, z0.s, z0.s
histcnt z0.d, p0/z, z0.d, z0.d

histseg z17.b, z21.b, z27.b
histseg z0.b, z0.b, z0.b

ldnt1b { z17.d }, p5/z, [z21.d, x27]
ldnt1b { z0.d }, p0/z, [z0.d, x0]
ldnt1b { z0.d }, p0/z, [z0.d]
ldnt1b { z0.d }, p0/z, [z0.d, xzr]
ldnt1b { z17.s }, p5/z, [z21.s, x27]
ldnt1b { z0.s }, p0/z, [z0.s, x0]
ldnt1b { z0.s }, p0/z, [z0.s]
ldnt1b { z0.s }, p0/z, [z0.s, xzr]

ldnt1d { z17.d }, p5/z, [z21.d, x27]
ldnt1d { z0.d }, p0/z, [z0.d, x0]
ldnt1d { z0.d }, p0/z, [z0.d]
ldnt1d { z0.d }, p0/z, [z0.d, xzr]

ldnt1h { z17.d }, p5/z, [z21.d, x27]
ldnt1h { z0.d }, p0/z, [z0.d, x0]
ldnt1h { z0.d }, p0/z, [z0.d]
ldnt1h { z0.d }, p0/z, [z0.d, xzr]
ldnt1h { z17.s }, p5/z, [z21.s, x27]
ldnt1h { z0.s }, p0/z, [z0.s, x0]
ldnt1h { z0.s }, p0/z, [z0.s]
ldnt1h { z0.s }, p0/z, [z0.s, xzr]

ldnt1sb { z17.s }, p5/z, [z21.s, x27]
ldnt1sb { z0.s }, p0/z, [z0.s, x0]
ldnt1sb { z0.s }, p0/z, [z0.s]
ldnt1sb { z0.s }, p0/z, [z0.s, xzr]
ldnt1sb { z0.d }, p0/z, [z0.d, x0]
ldnt1sb { z0.d }, p0/z, [z0.d]
ldnt1sb { z0.d }, p0/z, [z0.d, xzr]

ldnt1sh { z17.s }, p5/z, [z21.s, x27]
ldnt1sh { z0.s }, p0/z, [z0.s, x0]
ldnt1sh { z0.s }, p0/z, [z0.s]
ldnt1sh { z0.s }, p0/z, [z0.s, xzr]
ldnt1sh { z0.d }, p0/z, [z0.d, x0]
ldnt1sh { z0.d }, p0/z, [z0.d]
ldnt1sh { z0.d }, p0/z, [z0.d, xzr]

ldnt1sw { z17.d }, p5/z, [z21.d, x27]
ldnt1sw { z0.d }, p0/z, [z0.d, x0]
ldnt1sw { z0.d }, p0/z, [z0.d]
ldnt1sw { z0.d }, p0/z, [z0.d, xzr]

ldnt1w { z17.s }, p5/z, [z21.s, x27]
ldnt1w { z0.s }, p0/z, [z0.s, x0]
ldnt1w { z0.s }, p0/z, [z0.s]
ldnt1w { z0.s }, p0/z, [z0.s, xzr]
ldnt1w { z17.d }, p5/z, [z21.d, x27]
ldnt1w { z0.d }, p0/z, [z0.d, x0]
ldnt1w { z0.d }, p0/z, [z0.d]
ldnt1w { z0.d }, p0/z, [z0.d, xzr]

match p9.b, p5/z, z17.b, z21.b
match p0.b, p0/z, z17.b, z21.b
match p0.b, p0/z, z0.b, z0.b
match p0.h, p0/z, z0.h, z0.h

mla z17.h, z21.h, z3.h[3]
mla z0.h, z0.h, z0.h[4]
mla z0.h, z0.h, z0.h[0]

mla z17.s, z21.s, z3.s[3]
mla z0.s, z0.s, z0.s[0]

mla z17.d, z21.d, z11.d[1]
mla z0.d, z0.d, z0.d[0]

mls z17.h, z21.h, z3.h[3]
mls z0.h, z0.h, z0.h[4]
mls z0.h, z0.h, z0.h[0]

mls z17.s, z21.s, z3.s[3]
mls z0.s, z0.s, z0.s[0]

mls z17.d, z21.d, z11.d[1]
mls z0.d, z0.d, z0.d[0]

mul z17.h, z21.h, z3.h[3]
mul z0.h, z0.h, z0.h[4]
mul z0.h, z0.h, z0.h[0]

mul z17.s, z21.s, z3.s[3]
mul z0.s, z0.s, z0.s[0]

mul z17.d, z21.d, z11.d[1]
mul z0.d, z0.d, z0.d[0]

mul z17.b, z21.b, z27.b
mul z0.b, z0.b, z0.b
mul z0.h, z0.h, z0.h
mul z0.s, z0.s, z0.s
mul z0.d, z0.d, z0.d

nmatch p9.b, p5/z, z21.b, z27.b
nmatch p0.b, p0/z, z0.b, z0.b
nmatch p0.h, p0/z, z0.h, z0.h

nbsl z17.d, z17.d, z21.d, z27.d
nbsl z0.d, z0.d, z0.d, z0.d

pmul z17.b, z21.b, z27.b
pmul z0.b, z0.b, z0.b

pmullb z17.q, z21.d, z27.d
pmullb z0.q, z0.d, z0.d

pmullb z17.h, z21.b, z27.b
pmullb z0.h, z0.b, z0.b
pmullb z0.d, z0.s, z0.s

pmullt z17.q, z21.d, z27.d
pmullt z0.q, z0.d, z0.d

pmullt z17.h, z21.b, z27.b
pmullt z0.h, z0.b, z0.b
pmullt z0.d, z0.s, z0.s

raddhnb z17.b, z21.h, z27.h
raddhnb z0.b, z0.h, z0.h
raddhnb z0.h, z0.s, z0.s
raddhnb z0.s, z0.d, z0.d

raddhnt z17.b, z21.h, z27.h
raddhnt z0.b, z0.h, z0.h
raddhnt z0.h, z0.s, z0.s
raddhnt z0.s, z0.d, z0.d

rax1 z17.d, z21.d, z27.d
rax1 z0.d, z0.d, z0.d

# Shift is encoded as 2*esize - (tsz:imm3)
# For .b .h first two bits are 0, want 1001 to match pattern of ones on the
# outside, hence use 7.
# For all zeros except the minimum size bit, use maximum size.
rshrnb z17.b, z21.h, #7
rshrnb z0.b, z0.h, #1
rshrnb z0.b, z0.h, #8
# .h .s 0100001 = 15
rshrnb z0.h, z0.s, #1
rshrnb z0.h, z0.s, #15
rshrnb z0.h, z0.s, #16
# .s .d 1000001 = 31
rshrnb z0.s, z0.d, #1
rshrnb z0.s, z0.d, #31
rshrnb z0.s, z0.d, #32

rshrnt z17.b, z21.h, #7
rshrnt z0.b, z0.h, #1
rshrnt z0.b, z0.h, #8
rshrnt z0.h, z0.s, #1
rshrnt z0.h, z0.s, #15
rshrnt z0.h, z0.s, #16
rshrnt z0.s, z0.d, #1
rshrnt z0.s, z0.d, #31
rshrnt z0.s, z0.d, #32

rsubhnb z17.b, z21.h, z27.h
rsubhnb z0.b, z0.h, z0.h
rsubhnb z0.h, z0.s, z0.s
rsubhnb z0.s, z0.d, z0.d

rsubhnt z17.b, z21.h, z27.h
rsubhnt z0.b, z0.h, z0.h
rsubhnt z0.h, z0.s, z0.s
rsubhnt z0.s, z0.d, z0.d

saba z17.b, z21.b, z27.b
saba z0.b, z0.b, z0.b
saba z0.h, z0.h, z0.h
saba z0.s, z0.s, z0.s
saba z0.d, z0.d, z0.d

sabalb z17.h, z21.b, z27.b
sabalb z0.h, z0.b, z0.b
sabalb z0.s, z0.h, z0.h
sabalb z0.d, z0.s, z0.s

sabalt z17.h, z21.b, z27.b
sabalt z0.h, z0.b, z0.b
sabalt z0.s, z0.h, z0.h
sabalt z0.d, z0.s, z0.s

sabdlb z17.h, z21.b, z27.b
sabdlb z0.h, z0.b, z0.b
sabdlb z0.s, z0.h, z0.h
sabdlb z0.d, z0.s, z0.s

sabdlt z17.h, z21.b, z27.b
sabdlt z0.h, z0.b, z0.b
sabdlt z0.s, z0.h, z0.h
sabdlt z0.d, z0.s, z0.s

sadalp z17.h, p5/m, z21.b
sadalp z0.h, p0/m, z0.b
sadalp z0.s, p0/m, z0.h
sadalp z0.d, p0/m, z0.s

saddlb z17.h, z21.b, z27.b
saddlb z0.h, z0.b, z0.b
saddlb z0.s, z0.h, z0.h
saddlb z0.d, z0.s, z0.s

saddlbt z17.h, z21.b, z27.b
saddlbt z0.h, z0.b, z0.b
saddlbt z0.s, z0.h, z0.h
saddlbt z0.d, z0.s, z0.s

saddlt z17.h, z21.b, z27.b
saddlt z0.h, z0.b, z0.b
saddlt z0.s, z0.h, z0.h
saddlt z0.d, z0.s, z0.s

saddwb z17.h, z21.h, z27.b
saddwb z0.h, z0.h, z0.b
saddwb z0.s, z0.s, z0.h
saddwb z0.d, z0.d, z0.s

saddwt z17.h, z21.h, z27.b
saddwt z0.h, z0.h, z0.b
saddwt z0.s, z0.s, z0.h
saddwt z0.d, z0.d, z0.s

sbclb z17.s, z21.s, z27.s
sbclb z0.s, z0.s, z0.s
sbclb z0.d, z0.d, z0.d

sbclt z17.s, z21.s, z27.s
sbclt z0.s, z0.s, z0.s
sbclt z0.d, z0.d, z0.d

shadd z17.b, p5/m, z17.b, z21.b
shadd z0.b, p0/m, z0.b, z0.b
shadd z0.h, p0/m, z0.h, z0.h
shadd z0.s, p0/m, z0.s, z0.s
shadd z0.d, p0/m, z0.d, z0.d

shrnb z17.b, z21.h, #7
shrnb z0.b, z0.h, #1
shrnb z0.b, z0.h, #8
shrnb z0.h, z0.s, #1
shrnb z0.h, z0.s, #15
shrnb z0.h, z0.s, #16
shrnb z0.s, z0.d, #1
shrnb z0.s, z0.d, #31
shrnb z0.s, z0.d, #32

shrnt z17.b, z21.h, #7
shrnt z0.b, z0.h, #1
shrnt z0.b, z0.h, #8
shrnt z0.h, z0.s, #1
shrnt z0.h, z0.s, #15
shrnt z0.h, z0.s, #16
shrnt z0.s, z0.d, #1
shrnt z0.s, z0.d, #31
shrnt z0.s, z0.d, #32

shsub z17.b, p5/m, z17.b, z21.b
shsub z0.b, p0/m, z0.b, z0.b
shsub z0.h, p0/m, z0.h, z0.h
shsub z0.s, p0/m, z0.s, z0.s
shsub z0.d, p0/m, z0.d, z0.d

shsubr z17.b, p5/m, z17.b, z21.b
shsubr z0.b, p0/m, z0.b, z0.b
shsubr z0.h, p0/m, z0.h, z0.h
shsubr z0.s, p0/m, z0.s, z0.s
shsubr z0.d, p0/m, z0.d, z0.d

# shift - esize == 0b1001
# All other tests alternate between 1000... and 1111...
sli z17.b, z21.b, #1
sli z0.b, z0.b, #0
sli z0.b, z0.b, #7
sli z0.h, z0.h, #0
sli z0.h, z0.h, #15
sli z0.s, z0.s, #0
sli z0.s, z0.s, #31
sli z0.d, z0.d, #0
sli z0.d, z0.d, #63

sm4e z17.s, z17.s, z21.s
sm4e z0.s, z0.s, z0.s

sm4ekey z17.s, z21.s, z27.s
sm4ekey z0.s, z0.s, z0.s

smaxp z17.b, p5/m, z17.b, z21.b
smaxp z0.b, p0/m, z0.b, z0.b
smaxp z0.h, p0/m, z0.h, z0.h
smaxp z0.s, p0/m, z0.s, z0.s
smaxp z0.d, p0/m, z0.d, z0.d

sminp z17.b, p5/m, z17.b, z21.b
sminp z0.b, p0/m, z0.b, z0.b
sminp z0.h, p0/m, z0.h, z0.h
sminp z0.s, p0/m, z0.s, z0.s
sminp z0.d, p0/m, z0.d, z0.d

smlalb z17.s, z21.h, z5.h[0]
smlalb z0.s, z0.h, z0.h[5]
smlalb z0.s, z0.h, z0.h[0]

smlalb z17.d, z21.s, z9.s[0]
smlalb z0.d, z0.s, z0.s[3]
smlalb z0.d, z0.s, z0.s[0]

smlalb z17.h, z21.b, z27.b
smlalb z0.h, z0.b, z0.b
smlalb z0.s, z0.h, z0.h
smlalb z0.d, z0.s, z0.s

smlalt z17.s, z21.h, z5.h[0]
smlalt z0.s, z0.h, z0.h[5]
smlalt z0.s, z0.h, z0.h[0]

smlalt z17.d, z21.s, z9.s[0]
smlalt z0.d, z0.s, z0.s[3]
smlalt z0.d, z0.s, z0.s[0]

smlalt z17.h, z21.b, z27.b
smlalt z0.h, z0.b, z0.b
smlalt z0.s, z0.h, z0.h
smlalt z0.d, z0.s, z0.s

smlslb z17.s, z21.h, z5.h[0]
smlslb z0.s, z0.h, z0.h[5]
smlslb z0.s, z0.h, z0.h[0]

smlslb z17.d, z21.s, z9.s[0]
smlslb z0.d, z0.s, z0.s[3]
smlslb z0.d, z0.s, z0.s[0]

smlslb z17.h, z21.b, z27.b
smlslb z0.h, z0.b, z0.b
smlslb z0.s, z0.h, z0.h
smlslb z0.d, z0.s, z0.s

smlslt z17.s, z21.h, z5.h[0]
smlslt z0.s, z0.h, z0.h[5]
smlslt z0.s, z0.h, z0.h[0]

smlslt z17.d, z21.s, z9.s[0]
smlslt z0.d, z0.s, z0.s[3]
smlslt z0.d, z0.s, z0.s[0]

smlslt z17.h, z21.b, z27.b
smlslt z0.h, z0.b, z0.b
smlslt z0.s, z0.h, z0.h
smlslt z0.d, z0.s, z0.s

smulh z17.b, z21.b, z27.b
smulh z0.b, z0.b, z0.b
smulh z0.h, z0.h, z0.h
smulh z0.s, z0.s, z0.s
smulh z0.d, z0.d, z0.d

smullb z17.s, z21.h, z5.h[0]
smullb z0.s, z0.h, z0.h[5]
smullb z0.s, z0.h, z0.h[0]

smullb z17.d, z21.s, z9.s[0]
smullb z0.d, z0.s, z0.s[3]
smullb z0.d, z0.s, z0.s[0]

smullb z17.h, z21.b, z27.b
smullb z0.h, z0.b, z0.b
smullb z0.s, z0.h, z0.h
smullb z0.d, z0.s, z0.s

smullt z17.s, z21.h, z5.h[0]
smullt z0.s, z0.h, z0.h[5]
smullt z0.s, z0.h, z0.h[0]

smullt z17.d, z21.s, z9.s[0]
smullt z0.d, z0.s, z0.s[3]
smullt z0.d, z0.s, z0.s[0]

smullt z17.h, z21.b, z27.b
smullt z0.h, z0.b, z0.b
smullt z0.s, z0.h, z0.h
smullt z0.d, z0.s, z0.s

splice z17.b, p5, { z21.b, z22.b }
splice z0.b, p0, { z0.b, z1.b }
splice z0.h, p0, { z0.h, z1.h }
splice z0.s, p0, { z0.s, z1.s }
splice z0.d, p0, { z0.d, z1.d }
splice z0.b, p0, { z31.b, z0.b }

sqabs z17.b, p5/m, z21.b
sqabs z0.b, p0/m, z0.b
sqabs z0.h, p0/m, z0.h
sqabs z0.s, p0/m, z0.s
sqabs z0.d, p0/m, z0.d

sqadd z17.b, p5/m, z17.b, z21.b
sqadd z0.b, p0/m, z0.b, z0.b
sqadd z0.h, p0/m, z0.h, z0.h
sqadd z0.s, p0/m, z0.s, z0.s
sqadd z0.d, p0/m, z0.d, z0.d

sqcadd z17.b, z17.b, z21.b, #90
sqcadd z0.b, z0.b, z0.b, #270
sqcadd z0.b, z0.b, z0.b, #90
sqcadd z0.h, z0.h, z0.h, #90
sqcadd z0.s, z0.s, z0.s, #90
sqcadd z0.d, z0.d, z0.d, #90

sqdmlalb z17.s, z21.h, z5.h[0]
sqdmlalb z0.s, z0.h, z0.h[5]
sqdmlalb z0.s, z0.h, z0.h[0]

sqdmlalb z17.d, z21.s, z9.s[0]
sqdmlalb z0.d, z0.s, z0.s[3]
sqdmlalb z0.d, z0.s, z0.s[0]

sqdmlalb z17.h, z21.b, z27.b
sqdmlalb z0.h, z0.b, z0.b
sqdmlalb z0.s, z0.h, z0.h
sqdmlalb z0.d, z0.s, z0.s

sqdmlalbt z17.h, z21.b, z27.b
sqdmlalbt z0.h, z0.b, z0.b
sqdmlalbt z0.s, z0.h, z0.h
sqdmlalbt z0.d, z0.s, z0.s

sqdmlalt z17.s, z21.h, z5.h[0]
sqdmlalt z0.s, z0.h, z0.h[5]
sqdmlalt z0.s, z0.h, z0.h[0]

sqdmlalt z17.d, z21.s, z9.s[0]
sqdmlalt z0.d, z0.s, z0.s[3]
sqdmlalt z0.d, z0.s, z0.s[0]

sqdmlalt z17.h, z21.b, z27.b
sqdmlalt z0.h, z0.b, z0.b
sqdmlalt z0.s, z0.h, z0.h
sqdmlalt z0.d, z0.s, z0.s

sqdmlslb z17.s, z21.h, z5.h[0]
sqdmlslb z0.s, z0.h, z0.h[5]
sqdmlslb z0.s, z0.h, z0.h[0]

sqdmlslb z17.d, z21.s, z9.s[0]
sqdmlslb z0.d, z0.s, z0.s[3]
sqdmlslb z0.d, z0.s, z0.s[0]

sqdmlslb z17.h, z21.b, z27.b
sqdmlslb z0.h, z0.b, z0.b
sqdmlslb z0.s, z0.h, z0.h
sqdmlslb z0.d, z0.s, z0.s

sqdmlslbt z17.h, z21.b, z27.b
sqdmlslbt z0.h, z0.b, z0.b
sqdmlslbt z0.s, z0.h, z0.h
sqdmlslbt z0.d, z0.s, z0.s

sqdmlslt z17.s, z21.h, z5.h[0]
sqdmlslt z0.s, z0.h, z0.h[5]
sqdmlslt z0.s, z0.h, z0.h[0]

sqdmlslt z17.d, z21.s, z9.s[0]
sqdmlslt z0.d, z0.s, z0.s[3]
sqdmlslt z0.d, z0.s, z0.s[0]

sqdmlslt z17.h, z21.b, z27.b
sqdmlslt z0.h, z0.b, z0.b
sqdmlslt z0.s, z0.h, z0.h
sqdmlslt z0.d, z0.s, z0.s

sqdmulh z17.h, z21.h, z5.h[0]
sqdmulh z0.h, z0.h, z0.h[5]
sqdmulh z0.h, z0.h, z0.h[0]

sqdmulh z17.s, z21.s, z5.s[0]
sqdmulh z0.s, z0.s, z0.s[3]
sqdmulh z0.s, z0.s, z0.s[0]

sqdmulh z17.d, z21.d, z9.d[0]
sqdmulh z0.d, z0.d, z0.d[1]
sqdmulh z0.d, z0.d, z0.d[0]

sqdmulh z17.b, z21.b, z27.b
sqdmulh z0.b, z0.b, z0.b
sqdmulh z0.h, z0.h, z0.h
sqdmulh z0.s, z0.s, z0.s
sqdmulh z0.d, z0.d, z0.d

sqdmullb z17.s, z21.h, z5.h[0]
sqdmullb z0.s, z0.h, z0.h[5]
sqdmullb z0.s, z0.h, z0.h[0]

sqdmullb z17.d, z21.s, z9.s[0]
sqdmullb z0.d, z0.s, z0.s[3]
sqdmullb z0.d, z0.s, z0.s[0]

sqdmullb z17.h, z21.b, z27.b
sqdmullb z0.h, z0.b, z0.b
sqdmullb z0.s, z0.h, z0.h
sqdmullb z0.d, z0.s, z0.s

sqdmullt z17.s, z21.h, z5.h[0]
sqdmullt z0.s, z0.h, z0.h[5]
sqdmullt z0.s, z0.h, z0.h[0]

sqdmullt z17.d, z21.s, z9.s[0]
sqdmullt z0.d, z0.s, z0.s[3]
sqdmullt z0.d, z0.s, z0.s[0]

sqdmullt z17.h, z21.b, z27.b
sqdmullt z0.h, z0.b, z0.b
sqdmullt z0.s, z0.h, z0.h
sqdmullt z0.d, z0.s, z0.s

sqneg z17.b, p5/m, z21.b
sqneg z0.b, p0/m, z0.b
sqneg z0.h, p0/m, z0.h
sqneg z0.s, p0/m, z0.s
sqneg z0.d, p0/m, z0.d

sqrdcmlah z17.h, z21.h, z5.h[0], #0
sqrdcmlah z0.h, z0.h, z0.h[3], #0
sqrdcmlah z0.h, z0.h, z0.h[0], #90
sqrdcmlah z0.h, z0.h, z0.h[0], #180
sqrdcmlah z0.h, z0.h, z0.h[0], #270

sqrdcmlah z17.s, z21.s, z9.s[0], #0
sqrdcmlah z0.s, z0.s, z0.s[1], #0
sqrdcmlah z0.s, z0.s, z0.s[0], #90
sqrdcmlah z0.s, z0.s, z0.s[0], #180
sqrdcmlah z0.s, z0.s, z0.s[0], #270

sqrdcmlah z17.b, z21.b, z27.b, #0
sqrdcmlah z0.b, z0.b, z0.b, #0
sqrdcmlah z0.b, z0.b, z0.b, #90
sqrdcmlah z0.b, z0.b, z0.b, #180
sqrdcmlah z0.b, z0.b, z0.b, #270
sqrdcmlah z0.h, z0.h, z0.h, #0
sqrdcmlah z0.s, z0.s, z0.s, #0
sqrdcmlah z0.d, z0.d, z0.d, #0

sqrdmlah z17.h, z21.h, z5.h[0]
sqrdmlah z0.h, z0.h, z0.h[5]
sqrdmlah z0.h, z0.h, z0.h[0]

sqrdmlah z17.s, z21.s, z5.s[0]
sqrdmlah z0.s, z0.s, z0.s[3]
sqrdmlah z0.s, z0.s, z0.s[0]

sqrdmlah z17.d, z21.d, z9.d[0]
sqrdmlah z0.d, z0.d, z0.d[1]
sqrdmlah z0.d, z0.d, z0.d[0]

sqrdmlah z17.b, z21.b, z27.b
sqrdmlah z0.b, z0.b, z0.b
sqrdmlah z0.h, z0.h, z0.h
sqrdmlah z0.s, z0.s, z0.s
sqrdmlah z0.d, z0.d, z0.d

sqrdmlsh z17.h, z21.h, z5.h[0]
sqrdmlsh z0.h, z0.h, z0.h[5]
sqrdmlsh z0.h, z0.h, z0.h[0]

sqrdmlsh z17.s, z21.s, z5.s[0]
sqrdmlsh z0.s, z0.s, z0.s[3]
sqrdmlsh z0.s, z0.s, z0.s[0]

sqrdmlsh z17.d, z21.d, z9.d[0]
sqrdmlsh z0.d, z0.d, z0.d[1]
sqrdmlsh z0.d, z0.d, z0.d[0]

sqrdmlsh z17.b, z21.b, z27.b
sqrdmlsh z0.b, z0.b, z0.b
sqrdmlsh z0.h, z0.h, z0.h
sqrdmlsh z0.s, z0.s, z0.s
sqrdmlsh z0.d, z0.d, z0.d

sqrdmulh z17.h, z21.h, z5.h[0]
sqrdmulh z0.h, z0.h, z0.h[5]
sqrdmulh z0.h, z0.h, z0.h[0]

sqrdmulh z17.s, z21.s, z5.s[0]
sqrdmulh z0.s, z0.s, z0.s[3]
sqrdmulh z0.s, z0.s, z0.s[0]

sqrdmulh z17.d, z21.d, z9.d[0]
sqrdmulh z0.d, z0.d, z0.d[1]
sqrdmulh z0.d, z0.d, z0.d[0]

sqrdmulh z17.b, z21.b, z27.b
sqrdmulh z0.b, z0.b, z0.b
sqrdmulh z0.h, z0.h, z0.h
sqrdmulh z0.s, z0.s, z0.s
sqrdmulh z0.d, z0.d, z0.d

sqrshl z17.b, p5/m, z17.b, z21.b
sqrshl z0.b, p0/m, z0.b, z0.b
sqrshl z0.h, p0/m, z0.h, z0.h
sqrshl z0.s, p0/m, z0.s, z0.s
sqrshl z0.d, p0/m, z0.d, z0.d

sqrshlr z17.b, p5/m, z17.b, z21.b
sqrshlr z0.b, p0/m, z0.b, z0.b
sqrshlr z0.h, p0/m, z0.h, z0.h
sqrshlr z0.s, p0/m, z0.s, z0.s
sqrshlr z0.d, p0/m, z0.d, z0.d

sqrshrnb z17.b, z21.h, #7
sqrshrnb z0.b, z0.h, #1
sqrshrnb z0.b, z0.h, #8
sqrshrnb z0.h, z0.s, #1
sqrshrnb z0.h, z0.s, #15
sqrshrnb z0.h, z0.s, #16
sqrshrnb z0.s, z0.d, #1
sqrshrnb z0.s, z0.d, #31
sqrshrnb z0.s, z0.d, #32

sqrshrnt z17.b, z21.h, #7
sqrshrnt z0.b, z0.h, #1
sqrshrnt z0.b, z0.h, #8
sqrshrnt z0.h, z0.s, #1
sqrshrnt z0.h, z0.s, #15
sqrshrnt z0.h, z0.s, #16
sqrshrnt z0.s, z0.d, #1
sqrshrnt z0.s, z0.d, #31
sqrshrnt z0.s, z0.d, #32

sqrshrunb z17.b, z21.h, #7
sqrshrunb z0.b, z0.h, #1
sqrshrunb z0.b, z0.h, #8
sqrshrunb z0.h, z0.s, #1
sqrshrunb z0.h, z0.s, #15
sqrshrunb z0.h, z0.s, #16
sqrshrunb z0.s, z0.d, #1
sqrshrunb z0.s, z0.d, #31
sqrshrunb z0.s, z0.d, #32

sqrshrunt z17.b, z21.h, #7
sqrshrunt z0.b, z0.h, #1
sqrshrunt z0.b, z0.h, #8
sqrshrunt z0.h, z0.s, #1
sqrshrunt z0.h, z0.s, #15
sqrshrunt z0.h, z0.s, #16
sqrshrunt z0.s, z0.d, #1
sqrshrunt z0.s, z0.d, #31
sqrshrunt z0.s, z0.d, #32

sqshl z17.b, p5/m, z17.b, #1
sqshl z0.b, p0/m, z0.b, #0
sqshl z0.b, p0/m, z0.b, #7
sqshl z0.h, p0/m, z0.h, #0
sqshl z0.h, p0/m, z0.h, #15
sqshl z0.s, p0/m, z0.s, #0
sqshl z0.s, p0/m, z0.s, #31
sqshl z0.d, p0/m, z0.d, #0
sqshl z0.d, p0/m, z0.d, #63

sqshl z17.b, p5/m, z17.b, z21.b
sqshl z0.b, p0/m, z0.b, z0.b
sqshl z0.h, p0/m, z0.h, z0.h
sqshl z0.s, p0/m, z0.s, z0.s
sqshl z0.d, p0/m, z0.d, z0.d

sqshlr z17.b, p5/m, z17.b, z21.b
sqshlr z0.b, p0/m, z0.b, z0.b
sqshlr z0.h, p0/m, z0.h, z0.h
sqshlr z0.s, p0/m, z0.s, z0.s
sqshlr z0.d, p0/m, z0.d, z0.d

sqshlu z17.b, p5/m, z17.b, #1
sqshlu z0.b, p0/m, z0.b, #0
sqshlu z0.b, p0/m, z0.b, #7
sqshlu z0.h, p0/m, z0.h, #0
sqshlu z0.h, p0/m, z0.h, #15
sqshlu z0.s, p0/m, z0.s, #0
sqshlu z0.s, p0/m, z0.s, #31
sqshlu z0.d, p0/m, z0.d, #0
sqshlu z0.d, p0/m, z0.d, #63

sqshrnb z17.b, z21.h, #7
sqshrnb z0.b, z0.h, #1
sqshrnb z0.b, z0.h, #8
sqshrnb z0.h, z0.s, #1
sqshrnb z0.h, z0.s, #15
sqshrnb z0.h, z0.s, #16
sqshrnb z0.s, z0.d, #1
sqshrnb z0.s, z0.d, #31
sqshrnb z0.s, z0.d, #32

sqshrnt z17.b, z21.h, #7
sqshrnt z0.b, z0.h, #1
sqshrnt z0.b, z0.h, #8
sqshrnt z0.h, z0.s, #1
sqshrnt z0.h, z0.s, #15
sqshrnt z0.h, z0.s, #16
sqshrnt z0.s, z0.d, #1
sqshrnt z0.s, z0.d, #31
sqshrnt z0.s, z0.d, #32

sqshrunb z17.b, z21.h, #7
sqshrunb z0.b, z0.h, #1
sqshrunb z0.b, z0.h, #8
sqshrunb z0.h, z0.s, #1
sqshrunb z0.h, z0.s, #15
sqshrunb z0.h, z0.s, #16
sqshrunb z0.s, z0.d, #1
sqshrunb z0.s, z0.d, #31
sqshrunb z0.s, z0.d, #32

sqshrunt z17.b, z21.h, #7
sqshrunt z0.b, z0.h, #1
sqshrunt z0.b, z0.h, #8
sqshrunt z0.h, z0.s, #1
sqshrunt z0.h, z0.s, #15
sqshrunt z0.h, z0.s, #16
sqshrunt z0.s, z0.d, #1
sqshrunt z0.s, z0.d, #31
sqshrunt z0.s, z0.d, #32

sqsub z17.b, p5/m, z17.b, z21.b
sqsub z0.b, p0/m, z0.b, z0.b
sqsub z0.h, p0/m, z0.h, z0.h
sqsub z0.s, p0/m, z0.s, z0.s
sqsub z0.d, p0/m, z0.d, z0.d

sqsubr z17.b, p5/m, z17.b, z21.b
sqsubr z0.b, p0/m, z0.b, z0.b
sqsubr z0.h, p0/m, z0.h, z0.h
sqsubr z0.s, p0/m, z0.s, z0.s
sqsubr z0.d, p0/m, z0.d, z0.d

sqxtnb z17.b, z21.h
sqxtnb z0.b, z0.h
sqxtnb z0.h, z0.s
sqxtnb z0.s, z0.d

sqxtnt z17.b, z21.h
sqxtnt z0.b, z0.h
sqxtnt z0.h, z0.s
sqxtnt z0.s, z0.d

sqxtunb z17.b, z21.h
sqxtunb z0.b, z0.h
sqxtunb z0.h, z0.s
sqxtunb z0.s, z0.d

sqxtunt z17.b, z21.h
sqxtunt z0.b, z0.h
sqxtunt z0.h, z0.s
sqxtunt z0.s, z0.d

srhadd z17.b, p5/m, z17.b, z21.b
srhadd z0.b, p0/m, z0.b, z0.b
srhadd z0.h, p0/m, z0.h, z0.h
srhadd z0.s, p0/m, z0.s, z0.s
srhadd z0.d, p0/m, z0.d, z0.d

sri z17.b, z21.b, #7
sri z0.b, z0.b, #8
sri z0.b, z0.b, #1
sri z0.h, z0.h, #16
sri z0.h, z0.h, #1
sri z0.s, z0.s, #32
sri z0.s, z0.s, #1
sri z0.d, z0.d, #64
sri z0.d, z0.d, #1

srshl z17.b, p5/m, z17.b, z21.b
srshl z0.b, p0/m, z0.b, z0.b
srshl z0.h, p0/m, z0.h, z0.h
srshl z0.s, p0/m, z0.s, z0.s
srshl z0.d, p0/m, z0.d, z0.d

srshlr z17.b, p5/m, z17.b, z21.b
srshlr z0.b, p0/m, z0.b, z0.b
srshlr z0.h, p0/m, z0.h, z0.h
srshlr z0.s, p0/m, z0.s, z0.s
srshlr z0.d, p0/m, z0.d, z0.d

srshr z17.b, p5/m, z17.b, #7
srshr z0.b, p0/m, z0.b, #8
srshr z0.b, p0/m, z0.b, #1
srshr z0.h, p0/m, z0.h, #16
srshr z0.h, p0/m, z0.h, #1
srshr z0.s, p0/m, z0.s, #32
srshr z0.s, p0/m, z0.s, #1
srshr z0.d, p0/m, z0.d, #64
srshr z0.d, p0/m, z0.d, #1

srsra z17.b, z21.b, #7
srsra z0.b, z0.b, #8
srsra z0.b, z0.b, #1
srsra z0.h, z0.h, #16
srsra z0.h, z0.h, #1
srsra z0.s, z0.s, #32
srsra z0.s, z0.s, #1
srsra z0.d, z0.d, #64
srsra z0.d, z0.d, #1

sshllb z17.h, z21.b, #1
sshllb z0.h, z0.b, #0
sshllb z0.h, z0.b, #7
sshllb z0.s, z0.h, #0
sshllb z0.s, z0.h, #15
sshllb z0.d, z0.s, #0
sshllb z0.d, z0.s, #31

sshllt z17.h, z21.b, #1
sshllt z0.h, z0.b, #0
sshllt z0.h, z0.b, #7
sshllt z0.s, z0.h, #0
sshllt z0.s, z0.h, #15
sshllt z0.d, z0.s, #0
sshllt z0.d, z0.s, #31

ssra z17.b, z21.b, #7
ssra z0.b, z0.b, #8
ssra z0.b, z0.b, #1
ssra z0.h, z0.h, #16
ssra z0.h, z0.h, #1
ssra z0.s, z0.s, #32
ssra z0.s, z0.s, #1
ssra z0.d, z0.d, #64
ssra z0.d, z0.d, #1

ssublb z17.h, z21.b, z27.b
ssublb z0.h, z0.b, z0.b
ssublb z0.s, z0.h, z0.h
ssublb z0.d, z0.s, z0.s

ssublbt z17.h, z21.b, z27.b
ssublbt z0.h, z0.b, z0.b
ssublbt z0.s, z0.h, z0.h
ssublbt z0.d, z0.s, z0.s

ssublt z17.h, z21.b, z27.b
ssublt z0.h, z0.b, z0.b
ssublt z0.s, z0.h, z0.h
ssublt z0.d, z0.s, z0.s

ssubltb z17.h, z21.b, z27.b
ssubltb z0.h, z0.b, z0.b
ssubltb z0.s, z0.h, z0.h
ssubltb z0.d, z0.s, z0.s

ssubwb z17.h, z21.h, z27.b
ssubwb z0.h, z0.h, z0.b
ssubwb z0.s, z0.s, z0.h
ssubwb z0.d, z0.d, z0.s

ssubwt z17.h, z21.h, z27.b
ssubwt z0.h, z0.h, z0.b
ssubwt z0.s, z0.s, z0.h
ssubwt z0.d, z0.d, z0.s

stnt1b { z17.s }, p5, [z21.s, x27]
stnt1b { z0.s }, p0, [z0.s, x0]
stnt1b { z0.s }, p0, [z0.s]
stnt1b { z0.s }, p0, [z0.s, xzr]
stnt1b { z17.d }, p5, [z21.d, x27]
stnt1b { z0.d }, p0, [z0.d, x0]
stnt1b { z0.d }, p0, [z0.d]
stnt1b { z0.d }, p0, [z0.d, xzr]

stnt1d { z17.d }, p5, [z21.d, x27]
stnt1d { z0.d }, p0, [z0.d, x0]
stnt1d { z0.d }, p0, [z0.d]
stnt1d { z0.d }, p0, [z0.d, xzr]

stnt1h { z17.s }, p5, [z21.s, x27]
stnt1h { z0.s }, p0, [z0.s, x0]
stnt1h { z0.s }, p0, [z0.s]
stnt1h { z0.s }, p0, [z0.s, xzr]
stnt1h { z17.d }, p5, [z21.d, x27]
stnt1h { z0.d }, p0, [z0.d, x0]
stnt1h { z0.d }, p0, [z0.d]
stnt1h { z0.d }, p0, [z0.d, xzr]

stnt1w { z17.s }, p5, [z21.s, x27]
stnt1w { z0.s }, p0, [z0.s, x0]
stnt1w { z0.s }, p0, [z0.s]
stnt1w { z0.s }, p0, [z0.s, xzr]
stnt1w { z17.d }, p5, [z21.d, x27]
stnt1w { z0.d }, p0, [z0.d, x0]
stnt1w { z0.d }, p0, [z0.d]
stnt1w { z0.d }, p0, [z0.d, xzr]

subhnb z17.b, z21.h, z27.h
subhnb z0.b, z0.h, z0.h
subhnb z0.h, z0.s, z0.s
subhnb z0.s, z0.d, z0.d

subhnt z17.b, z21.h, z27.h
subhnt z0.b, z0.h, z0.h
subhnt z0.h, z0.s, z0.s
subhnt z0.s, z0.d, z0.d

suqadd z17.b, p5/m, z17.b, z21.b
suqadd z0.b, p0/m, z0.b, z0.b
suqadd z0.h, p0/m, z0.h, z0.h
suqadd z0.s, p0/m, z0.s, z0.s
suqadd z0.d, p0/m, z0.d, z0.d

tbl z17.b, { z21.b, z22.b }, z27.b
tbl z0.b, { z0.b, z1.b }, z0.b
tbl z0.h, { z0.h, z1.h }, z0.h
tbl z0.s, { z0.s, z1.s }, z0.s
tbl z0.d, { z0.d, z1.d }, z0.d
tbl z0.b, { z31.b, z0.b }, z0.b

tbx z17.b, z21.b, z27.b
tbx z0.b, z0.b, z0.b
tbx z0.h, z0.h, z0.h
tbx z0.s, z0.s, z0.s
tbx z0.d, z0.d, z0.d

uaba z17.b, z21.b, z27.b
uaba z0.b, z0.b, z0.b
uaba z0.h, z0.h, z0.h
uaba z0.s, z0.s, z0.s
uaba z0.d, z0.d, z0.d

uabalb z17.h, z21.b, z27.b
uabalb z0.h, z0.b, z0.b
uabalb z0.s, z0.h, z0.h
uabalb z0.d, z0.s, z0.s

uabalt z17.h, z21.b, z27.b
uabalt z0.h, z0.b, z0.b
uabalt z0.s, z0.h, z0.h
uabalt z0.d, z0.s, z0.s

uabdlb z17.h, z21.b, z27.b
uabdlb z0.h, z0.b, z0.b
uabdlb z0.s, z0.h, z0.h
uabdlb z0.d, z0.s, z0.s

uabdlt z17.h, z21.b, z27.b
uabdlt z0.h, z0.b, z0.b
uabdlt z0.s, z0.h, z0.h
uabdlt z0.d, z0.s, z0.s

uadalp z17.h, p5/m, z21.b
uadalp z0.h, p0/m, z0.b
uadalp z0.s, p0/m, z0.h
uadalp z0.d, p0/m, z0.s

uaddlb z17.h, z21.b, z27.b
uaddlb z0.h, z0.b, z0.b
uaddlb z0.s, z0.h, z0.h
uaddlb z0.d, z0.s, z0.s

uaddlt z17.h, z21.b, z27.b
uaddlt z0.h, z0.b, z0.b
uaddlt z0.s, z0.h, z0.h
uaddlt z0.d, z0.s, z0.s

uaddwb z17.h, z21.h, z27.b
uaddwb z0.h, z0.h, z0.b
uaddwb z0.s, z0.s, z0.h
uaddwb z0.d, z0.d, z0.s

uaddwt z17.h, z21.h, z27.b
uaddwt z0.h, z0.h, z0.b
uaddwt z0.s, z0.s, z0.h
uaddwt z0.d, z0.d, z0.s

uhadd z17.b, p5/m, z17.b, z21.b
uhadd z0.b, p0/m, z0.b, z0.b
uhadd z0.h, p0/m, z0.h, z0.h
uhadd z0.s, p0/m, z0.s, z0.s
uhadd z0.d, p0/m, z0.d, z0.d

uhsub z17.b, p5/m, z17.b, z21.b
uhsub z0.b, p0/m, z0.b, z0.b
uhsub z0.h, p0/m, z0.h, z0.h
uhsub z0.s, p0/m, z0.s, z0.s
uhsub z0.d, p0/m, z0.d, z0.d

uhsubr z17.b, p5/m, z17.b, z21.b
uhsubr z0.b, p0/m, z0.b, z0.b
uhsubr z0.h, p0/m, z0.h, z0.h
uhsubr z0.s, p0/m, z0.s, z0.s
uhsubr z0.d, p0/m, z0.d, z0.d

umaxp z17.b, p5/m, z17.b, z21.b
umaxp z0.b, p0/m, z0.b, z0.b
umaxp z0.h, p0/m, z0.h, z0.h
umaxp z0.s, p0/m, z0.s, z0.s
umaxp z0.d, p0/m, z0.d, z0.d

uminp z17.b, p5/m, z17.b, z21.b
uminp z0.b, p0/m, z0.b, z0.b
uminp z0.h, p0/m, z0.h, z0.h
uminp z0.s, p0/m, z0.s, z0.s
uminp z0.d, p0/m, z0.d, z0.d

umlalb z17.s, z21.h, z5.h[0]
umlalb z0.s, z0.h, z0.h[5]
umlalb z0.s, z0.h, z0.h[0]

umlalb z17.d, z21.s, z9.s[0]
umlalb z0.d, z0.s, z0.s[3]
umlalb z0.d, z0.s, z0.s[0]

umlalb z17.h, z21.b, z27.b
umlalb z0.h, z0.b, z0.b
umlalb z0.s, z0.h, z0.h
umlalb z0.d, z0.s, z0.s

umlalt z17.s, z21.h, z5.h[0]
umlalt z0.s, z0.h, z0.h[5]
umlalt z0.s, z0.h, z0.h[0]

umlalt z17.d, z21.s, z9.s[0]
umlalt z0.d, z0.s, z0.s[3]
umlalt z0.d, z0.s, z0.s[0]

umlalt z17.h, z21.b, z27.b
umlalt z0.h, z0.b, z0.b
umlalt z0.s, z0.h, z0.h
umlalt z0.d, z0.s, z0.s

umlslb z17.s, z21.h, z5.h[0]
umlslb z0.s, z0.h, z0.h[5]
umlslb z0.s, z0.h, z0.h[0]

umlslb z17.d, z21.s, z9.s[0]
umlslb z0.d, z0.s, z0.s[3]
umlslb z0.d, z0.s, z0.s[0]

umlslb z17.h, z21.b, z27.b
umlslb z0.h, z0.b, z0.b
umlslb z0.s, z0.h, z0.h
umlslb z0.d, z0.s, z0.s

umlslt z17.s, z21.h, z5.h[0]
umlslt z0.s, z0.h, z0.h[5]
umlslt z0.s, z0.h, z0.h[0]

umlslt z17.d, z21.s, z9.s[0]
umlslt z0.d, z0.s, z0.s[3]
umlslt z0.d, z0.s, z0.s[0]

umlslt z17.h, z21.b, z27.b
umlslt z0.h, z0.b, z0.b
umlslt z0.s, z0.h, z0.h
umlslt z0.d, z0.s, z0.s

umulh z17.b, z21.b, z27.b
umulh z0.b, z0.b, z0.b
umulh z0.h, z0.h, z0.h
umulh z0.s, z0.s, z0.s
umulh z0.d, z0.d, z0.d

umullb z17.s, z21.h, z5.h[0]
umullb z0.s, z0.h, z0.h[5]
umullb z0.s, z0.h, z0.h[0]

umullb z17.d, z21.s, z9.s[0]
umullb z0.d, z0.s, z0.s[3]
umullb z0.d, z0.s, z0.s[0]

umullb z17.h, z21.b, z27.b
umullb z0.h, z0.b, z0.b
umullb z0.s, z0.h, z0.h
umullb z0.d, z0.s, z0.s

umullt z17.s, z21.h, z5.h[0]
umullt z0.s, z0.h, z0.h[5]
umullt z0.s, z0.h, z0.h[0]

umullt z17.d, z21.s, z9.s[0]
umullt z0.d, z0.s, z0.s[3]
umullt z0.d, z0.s, z0.s[0]

umullt z17.h, z21.b, z27.b
umullt z0.h, z0.b, z0.b
umullt z0.s, z0.h, z0.h
umullt z0.d, z0.s, z0.s

uqadd z17.b, p5/m, z17.b, z21.b
uqadd z0.b, p0/m, z0.b, z0.b
uqadd z0.h, p0/m, z0.h, z0.h
uqadd z0.s, p0/m, z0.s, z0.s
uqadd z0.d, p0/m, z0.d, z0.d

uqrshl z17.b, p5/m, z17.b, z21.b
uqrshl z0.b, p0/m, z0.b, z0.b
uqrshl z0.h, p0/m, z0.h, z0.h
uqrshl z0.s, p0/m, z0.s, z0.s
uqrshl z0.d, p0/m, z0.d, z0.d

uqrshlr z17.b, p5/m, z17.b, z21.b
uqrshlr z0.b, p0/m, z0.b, z0.b
uqrshlr z0.h, p0/m, z0.h, z0.h
uqrshlr z0.s, p0/m, z0.s, z0.s
uqrshlr z0.d, p0/m, z0.d, z0.d

uqrshrnb z17.b, z21.h, #7
uqrshrnb z0.b, z0.h, #1
uqrshrnb z0.b, z0.h, #8
uqrshrnb z0.h, z0.s, #1
uqrshrnb z0.h, z0.s, #15
uqrshrnb z0.h, z0.s, #16
uqrshrnb z0.s, z0.d, #1
uqrshrnb z0.s, z0.d, #31
uqrshrnb z0.s, z0.d, #32

uqrshrnt z17.b, z21.h, #7
uqrshrnt z0.b, z0.h, #1
uqrshrnt z0.b, z0.h, #8
uqrshrnt z0.h, z0.s, #1
uqrshrnt z0.h, z0.s, #15
uqrshrnt z0.h, z0.s, #16
uqrshrnt z0.s, z0.d, #1
uqrshrnt z0.s, z0.d, #31
uqrshrnt z0.s, z0.d, #32

uqshl z17.b, p5/m, z17.b, #1
uqshl z0.b, p0/m, z0.b, #0
uqshl z0.b, p0/m, z0.b, #7
uqshl z0.h, p0/m, z0.h, #0
uqshl z0.h, p0/m, z0.h, #15
uqshl z0.s, p0/m, z0.s, #0
uqshl z0.s, p0/m, z0.s, #31
uqshl z0.d, p0/m, z0.d, #0
uqshl z0.d, p0/m, z0.d, #63

uqshl z17.b, p5/m, z17.b, z21.b
uqshl z0.b, p0/m, z0.b, z0.b
uqshl z0.h, p0/m, z0.h, z0.h
uqshl z0.s, p0/m, z0.s, z0.s
uqshl z0.d, p0/m, z0.d, z0.d

uqshlr z17.b, p5/m, z17.b, z21.b
uqshlr z0.b, p0/m, z0.b, z0.b
uqshlr z0.h, p0/m, z0.h, z0.h
uqshlr z0.s, p0/m, z0.s, z0.s
uqshlr z0.d, p0/m, z0.d, z0.d

uqshrnb z17.b, z21.h, #7
uqshrnb z0.b, z0.h, #1
uqshrnb z0.b, z0.h, #8
uqshrnb z0.h, z0.s, #1
uqshrnb z0.h, z0.s, #15
uqshrnb z0.h, z0.s, #16
uqshrnb z0.s, z0.d, #1
uqshrnb z0.s, z0.d, #31
uqshrnb z0.s, z0.d, #32

uqshrnt z17.b, z21.h, #7
uqshrnt z0.b, z0.h, #1
uqshrnt z0.b, z0.h, #8
uqshrnt z0.h, z0.s, #1
uqshrnt z0.h, z0.s, #15
uqshrnt z0.h, z0.s, #16
uqshrnt z0.s, z0.d, #1
uqshrnt z0.s, z0.d, #31
uqshrnt z0.s, z0.d, #32

uqsub z17.b, p5/m, z17.b, z21.b
uqsub z0.b, p0/m, z0.b, z0.b
uqsub z0.h, p0/m, z0.h, z0.h
uqsub z0.s, p0/m, z0.s, z0.s
uqsub z0.d, p0/m, z0.d, z0.d

uqsubr z17.b, p5/m, z17.b, z21.b
uqsubr z0.b, p0/m, z0.b, z0.b
uqsubr z0.h, p0/m, z0.h, z0.h
uqsubr z0.s, p0/m, z0.s, z0.s
uqsubr z0.d, p0/m, z0.d, z0.d

uqxtnb z17.b, z21.h
uqxtnb z0.b, z0.h
uqxtnb z0.h, z0.s
uqxtnb z0.s, z0.d

uqxtnt z17.b, z21.h
uqxtnt z0.b, z0.h
uqxtnt z0.h, z0.s
uqxtnt z0.s, z0.d

urecpe z17.s, p5/m, z21.s
urecpe z0.s, p0/m, z0.s

urhadd z17.b, p5/m, z17.b, z21.b
urhadd z0.b, p0/m, z0.b, z0.b
urhadd z0.h, p0/m, z0.h, z0.h
urhadd z0.s, p0/m, z0.s, z0.s
urhadd z0.d, p0/m, z0.d, z0.d

urshl z17.b, p5/m, z17.b, z21.b
urshl z0.b, p0/m, z0.b, z0.b
urshl z0.h, p0/m, z0.h, z0.h
urshl z0.s, p0/m, z0.s, z0.s
urshl z0.d, p0/m, z0.d, z0.d

urshlr z17.b, p5/m, z17.b, z21.b
urshlr z0.b, p0/m, z0.b, z0.b
urshlr z0.h, p0/m, z0.h, z0.h
urshlr z0.s, p0/m, z0.s, z0.s
urshlr z0.d, p0/m, z0.d, z0.d

urshr z17.b, p5/m, z17.b, #7
urshr z0.b, p0/m, z0.b, #8
urshr z0.b, p0/m, z0.b, #1
urshr z0.h, p0/m, z0.h, #16
urshr z0.h, p0/m, z0.h, #1
urshr z0.s, p0/m, z0.s, #32
urshr z0.s, p0/m, z0.s, #1
urshr z0.d, p0/m, z0.d, #64
urshr z0.d, p0/m, z0.d, #1

ursqrte z17.s, p5/m, z21.s
ursqrte z0.s, p0/m, z0.s

ursra z17.b, z21.b, #7
ursra z0.b, z0.b, #8
ursra z0.b, z0.b, #1
ursra z0.h, z0.h, #16
ursra z0.h, z0.h, #1
ursra z0.s, z0.s, #32
ursra z0.s, z0.s, #1
ursra z0.d, z0.d, #64
ursra z0.d, z0.d, #1

ushllb z17.h, z21.b, #1
ushllb z0.h, z0.b, #0
ushllb z0.h, z0.b, #7
ushllb z0.s, z0.h, #0
ushllb z0.s, z0.h, #15
ushllb z0.d, z0.s, #0
ushllb z0.d, z0.s, #31

ushllt z17.h, z21.b, #1
ushllt z0.h, z0.b, #0
ushllt z0.h, z0.b, #7
ushllt z0.s, z0.h, #0
ushllt z0.s, z0.h, #15
ushllt z0.d, z0.s, #0
ushllt z0.d, z0.s, #31

usqadd z17.b, p5/m, z17.b, z21.b
usqadd z0.b, p0/m, z0.b, z0.b
usqadd z0.h, p0/m, z0.h, z0.h
usqadd z0.s, p0/m, z0.s, z0.s
usqadd z0.d, p0/m, z0.d, z0.d

usra z17.b, z21.b, #7
usra z0.b, z0.b, #8
usra z0.b, z0.b, #1
usra z0.h, z0.h, #16
usra z0.h, z0.h, #1
usra z0.s, z0.s, #32
usra z0.s, z0.s, #1
usra z0.d, z0.d, #64
usra z0.d, z0.d, #1

usublb z17.h, z21.b, z27.b
usublb z0.h, z0.b, z0.b
usublb z0.s, z0.h, z0.h
usublb z0.d, z0.s, z0.s

usublt z17.h, z21.b, z27.b
usublt z0.h, z0.b, z0.b
usublt z0.s, z0.h, z0.h
usublt z0.d, z0.s, z0.s

usubwb z17.h, z21.h, z27.b
usubwb z0.h, z0.h, z0.b
usubwb z0.s, z0.s, z0.h
usubwb z0.d, z0.d, z0.s

usubwt z17.h, z21.h, z27.b
usubwt z0.h, z0.h, z0.b
usubwt z0.s, z0.s, z0.h
usubwt z0.d, z0.d, z0.s

whilege p9.b, x21, x27
whilege p0.b, x0, x0
whilege p0.b, xzr, x0
whilege p0.b, x0, xzr
whilege p0.h, x0, x0
whilege p0.s, x0, x0
whilege p0.d, x0, x0

whilege p9.b, w21, w27
whilege p0.b, w0, w0
whilege p0.b, wzr, w0
whilege p0.b, w0, wzr
whilege p0.h, w0, w0
whilege p0.s, w0, w0
whilege p0.d, w0, w0

whilegt p9.b, x21, x27
whilegt p0.b, x0, x0
whilegt p0.b, xzr, x0
whilegt p0.b, x0, xzr
whilegt p0.h, x0, x0
whilegt p0.s, x0, x0
whilegt p0.d, x0, x0

whilegt p9.b, w21, w27
whilegt p0.b, w0, w0
whilegt p0.b, wzr, w0
whilegt p0.b, w0, wzr
whilegt p0.h, w0, w0
whilegt p0.s, w0, w0
whilegt p0.d, w0, w0

whilehi p9.b, x21, x27
whilehi p0.b, x0, x0
whilehi p0.b, xzr, x0
whilehi p0.b, x0, xzr
whilehi p0.h, x0, x0
whilehi p0.s, x0, x0
whilehi p0.d, x0, x0

whilehi p9.b, w21, w27
whilehi p0.b, w0, w0
whilehi p0.b, wzr, w0
whilehi p0.b, w0, wzr
whilehi p0.h, w0, w0
whilehi p0.s, w0, w0
whilehi p0.d, w0, w0

whilehs p9.b, x21, x27
whilehs p0.b, x0, x0
whilehs p0.b, xzr, x0
whilehs p0.b, x0, xzr
whilehs p0.h, x0, x0
whilehs p0.s, x0, x0
whilehs p0.d, x0, x0

whilehs p9.b, w21, w27
whilehs p0.b, w0, w0
whilehs p0.b, wzr, w0
whilehs p0.b, w0, wzr
whilehs p0.h, w0, w0
whilehs p0.s, w0, w0
whilehs p0.d, w0, w0

whilerw p9.b, x21, x27
whilerw p0.b, x0, x0
whilerw p0.h, x0, x0
whilerw p0.s, x0, x0
whilerw p0.d, x0, x0

whilewr p9.b, x21, x27
whilewr p0.b, x0, x0
whilewr p0.h, x0, x0
whilewr p0.s, x0, x0
whilewr p0.d, x0, x0

xar z17.b, z17.b, z21.b, #7
xar z0.b, z0.b, z0.b, #8
xar z0.b, z0.b, z0.b, #1
xar z0.h, z0.h, z0.h, #16
xar z0.h, z0.h, z0.h, #1
xar z0.s, z0.s, z0.s, #32
xar z0.s, z0.s, z0.s, #1
xar z0.d, z0.d, z0.d, #64
xar z0.d, z0.d, z0.d, #1
