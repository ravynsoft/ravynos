movprfx z0.d, z1.d
adclb z0.d, z1.d, z2.d

movprfx z0.d, p0/m, z1.d
adclb z0.d, z1.d, z2.d

adclb z0.d, z0.s, z0.s
adclb z32.d, z0.d, z0.d
adclb z0.d, z32.d, z0.d
adclb z0.d, z0.d, z32.d
adclt z0.d, z0.s, z0.s
adclt z32.s, z0.s, z0.s
adclt z0.s, z32.s, z0.s
adclt z0.s, z0.s, z32.s

addhnb z0.b, z0.h, z0.b
addhnb z32.b, z0.h, z0.h
addhnb z0.b, z32.h, z0.h
addhnb z0.b, z0.h, z32.h
addhnt z0.b, z0.h, z0.b
addhnt z32.b, z0.h, z0.h
addhnt z0.b, z32.h, z0.h
addhnt z0.b, z0.h, z32.h

movprfx z0.d, p0/m, z1.d
addp z0.b, p0/m, z0.b, z1.b

movprfx z0.d, p0/m, z1.d
addp z0.d, p1/m, z0.d, z1.d

addp z0.b, p0/z, z0.b, z0.b
addp z0.h, p0/m, z1.h, z0.h
addp z32.s, p0/m, z32.s, z0.s
addp z0.s, p0/m, z0.s, z32.s
addp z0.s, p8/m, z0.s, z0.s

movprfx z0, z1
aesd z0.b, z0.b, z0.b

aesd z0.b, z1.b, z0.b
aesd z0.b, z0.s, z0.b
aesd z32.b, z0.b, z0.b
aesd z0.b, z0.b, z32.b

movprfx z0, z1
aese z0.b, z0.b, z0.b

aese z0.b, z1.b, z0.b
aese z0.b, z0.s, z0.b
aese z32.b, z0.b, z0.b
aese z0.b, z0.b, z32.b

movprfx z0, z1
aesimc z0.b, z0.b

aesimc z0.b, z1.b
aesimc z0.b, z0.s
aesimc z32.b, z0.b

movprfx z0, z1
aesmc z0.b, z0.b

aesmc z0.b, z1.b
aesmc z0.b, z0.s
aesmc z32.b, z0.b

bcax z0.d, z1.d, z0.d, z0.d
bcax z0.d, z0.d, z0.h, z0.d
bcax z0.d, z0.h, z0.d, z0.d
bcax z32.d, z32.d, z0.d, z0.d
bcax z0.d, z0.d, z32.d, z0.d
bcax z0.d, z0.d, z0.d, z32.d

bsl z0.d, z1.d, z0.d, z0.d
bsl z0.d, z0.d, z0.h, z0.d
bsl z0.d, z0.h, z0.d, z0.d
bsl z32.d, z32.d, z0.d, z0.d
bsl z0.d, z0.d, z32.d, z0.d
bsl z0.d, z0.d, z0.d, z32.d

bsl1n z0.d, z1.d, z0.d, z0.d
bsl1n z0.d, z0.d, z0.h, z0.d
bsl1n z0.d, z0.h, z0.d, z0.d
bsl1n z32.d, z32.d, z0.d, z0.d
bsl1n z0.d, z0.d, z32.d, z0.d
bsl1n z0.d, z0.d, z0.d, z32.d

bsl2n z0.d, z1.d, z0.d, z0.d
bsl2n z0.d, z0.d, z0.h, z0.d
bsl2n z0.d, z0.h, z0.d, z0.d
bsl2n z32.d, z32.d, z0.d, z0.d
bsl2n z0.d, z0.d, z32.d, z0.d
bsl2n z0.d, z0.d, z0.d, z32.d

bdep z0.b, z0.h, z0.b
bdep z32.h, z0.h, z0.h
bdep z0.s, z32.s, z0.s
bdep z0.d, z0.d, z32.d

bext z0.b, z0.h, z0.b
bext z32.h, z0.h, z0.h
bext z0.s, z32.s, z0.s
bext z0.d, z0.d, z32.d

bgrp z0.b, z0.h, z0.b
bgrp z32.h, z0.h, z0.h
bgrp z0.s, z32.s, z0.s
bgrp z0.d, z0.d, z32.d

cadd z18.b, z17.b, z21.b, #90
cadd z0.b, z0.b, z0.b, #91
cadd z0.b, z0.h, z0.h, #90

cdot z0.s, z0.b, z0.b[0], #1
cdot z0.s, z0.b, z0.b[4], #0
cdot z0.s, z0.d, z0.b[0], #0
cdot z32.s, z0.b, z0.b[0], #0
cdot z0.s, z32.b, z0.b[0], #0
cdot z0.s, z0.b, z8.b[0], #0

cdot z0.d, z0.h, z0.h[0], #1
cdot z0.d, z0.h, z0.h[1], #0
cdot z0.d, z0.d, z0.h[0], #0
cdot z32.d, z0.h, z0.h[0], #0
cdot z0.d, z32.h, z0.h[0], #0
cdot z0.d, z0.h, z16.h[0], #0

cdot z32.s, z0.b, z0.b, #0
cdot z0.s, z32.b, z0.b, #0
cdot z0.s, z0.b, z32.b, #0
cdot z0.s, z0.b, z0.s, #0
cdot z0.s, z0.b, z0.b, #1
cdot z0.d, z0.h, z0.b, #0

cmla z32.h, z0.h, z0.h[0], #0
cmla z0.h, z32.h, z0.h[0], #0
cmla z0.h, z0.h, z8.h[0], #0
cmla z0.h, z0.h, z0.d[0], #0
cmla z0.h, z0.h, z0.h[4], #0
cmla z0.h, z0.h, z0.h[0], #1

cmla z32.s, z0.s, z0.s[0], #0
cmla z0.s, z32.s, z0.s[0], #0
cmla z0.s, z0.s, z16.s[0], #0
cmla z0.s, z0.s, z0.d[0], #0
cmla z0.s, z0.s, z0.s[2], #0
cmla z0.s, z0.s, z0.s[0], #1

cmla z32.b, z0.b, z0.b, #0
cmla z0.b, z32.b, z0.b, #0
cmla z0.b, z0.b, z32.b, #0
cmla z0.b, z0.b, z0.h, #0
cmla z0.b, z0.b, z0.b, #1

eor3 z0.d, z1.d, z0.d, z0.d
eor3 z0.d, z0.d, z0.h, z0.d
eor3 z0.d, z0.h, z0.d, z0.d

eorbt z0.b, z0.h, z0.b
eorbt z32.h, z0.h, z0.h
eorbt z0.s, z32.s, z0.s
eorbt z0.s, z0.s, z32.s

eortb z0.b, z0.h, z0.b
eortb z32.h, z0.h, z0.h
eortb z0.s, z32.s, z0.s
eortb z0.s, z0.s, z32.s

ext z0.b, {,}, #0
ext z0.b, { z0.b, z2.b }, #0
ext z0.h, { z0.b, z1.b }, #0
ext z0.b, { z0.h, z1.b }, #0
ext z0.b, { z0.b, z1.h }, #0
ext z0.b, { z0.h, z1.h }, #0
ext z0.b, { z0.b, z1.b, z2.b }, #0
ext z0.b, { z0.b }, #0
ext z0.b,  z0.b, #0
ext z0.b, { z31.b, z1.b }, #0
ext z0.b, { z0.b, z31.b }, #0
ext z0.b, { z0.b, z1.b }, #256
ext z32.b, { z0.b, z1.b }, #0
ext z0.b, { z31.b, z32.b }, #0
ext z0.b, { z32.b, z33.b }, #0

faddp z32.h, p0/m, z32.h, z0.h
faddp z0.h, p8/m, z0.h, z0.h
faddp z0.h, p0/m, z0.h, z32.h
faddp z0.h, p0/m, z1.h, z0.h
faddp z0.h, p0/z, z0.h, z0.h
faddp z0.h, p0/m, z0.b, z0.h

movprfx z0.s, p0/m, z1.s
fcvtlt z0.s, p0/m, z0.h

fcvtlt z32.s, p0/m, z0.h
fcvtlt z0.s, p8/m, z0.h
fcvtlt z0.s, p0/m, z32.h
fcvtlt z0.s, p0/m, z0.s
fcvtlt z0.s, p0/z, z0.h
fcvtlt z32.d, p0/m, z0.s
fcvtlt z0.d, p8/m, z0.s
fcvtlt z0.d, p0/m, z32.s
fcvtlt z0.d, p0/m, z0.d
fcvtlt z0.d, p0/z, z0.s

movprfx z0.s, p0/m, z1.s
fcvtnt z0.h, p0/m, z0.s

fcvtnt z32.h, p0/m, z0.s
fcvtnt z0.h, p8/m, z0.s
fcvtnt z0.h, p0/m, z32.s
fcvtnt z0.h, p0/m, z0.h
fcvtnt z0.h, p0/z, z0.s
fcvtnt z32.s, p0/m, z0.d
fcvtnt z0.s, p8/m, z0.d
fcvtnt z0.s, p0/m, z32.d
fcvtnt z0.s, p0/m, z0.s
fcvtnt z0.s, p0/z, z0.d

fcvtx z32.s, p0/m, z0.d
fcvtx z0.s, p8/m, z0.d
fcvtx z0.s, p0/m, z32.d
fcvtx z0.s, p0/m, z0.s
fcvtx z0.s, p0/z, z0.d

movprfx z0.s, p0/z, z1.s
fcvtx z0.s, p0/m, z2.d

movprfx z0.s, p0/m, z1.s
fcvtxnt z0.s, p0/m, z0.d

fcvtxnt z32.s, p0/m, z0.d
fcvtxnt z0.s, p8/m, z0.d
fcvtxnt z0.s, p0/m, z32.d
fcvtxnt z0.s, p0/m, z0.s
fcvtxnt z0.s, p0/z, z0.d

flogb z0.b, p0/m, z0.b
flogb z0.b, p0/m, z0.h
flogb z0.h, p0/z, z0.h
flogb z32.h, p0/m, z0.h
flogb z0.h, p8/m, z0.h
flogb z0.h, p0/m, z32.h

fmaxnmp z0.b, p0/m, z0.h, z0.h
fmaxnmp z0.h, p0/z, z0.h, z0.h
fmaxnmp z1.h, p0/m, z0.h, z0.h
fmaxnmp z32.h, p0/m, z32.h, z0.h
fmaxnmp z0.h, p8/m, z0.h, z0.h
fmaxnmp z0.h, p0/m, z0.h, z32.h

fmaxp z0.b, p0/m, z0.h, z0.h
fmaxp z0.h, p0/z, z0.h, z0.h
fmaxp z1.h, p0/m, z0.h, z0.h
fmaxp z32.h, p0/m, z32.h, z0.h
fmaxp z0.h, p8/m, z0.h, z0.h
fmaxp z0.h, p0/m, z0.h, z32.h

fminnmp z0.b, p0/m, z0.h, z0.h
fminnmp z0.h, p0/z, z0.h, z0.h
fminnmp z1.h, p0/m, z0.h, z0.h
fminnmp z32.h, p0/m, z32.h, z0.h
fminnmp z0.h, p8/m, z0.h, z0.h
fminnmp z0.h, p0/m, z0.h, z32.h

fminp z0.b, p0/m, z0.h, z0.h
fminp z0.h, p0/z, z0.h, z0.h
fminp z1.h, p0/m, z0.h, z0.h
fminp z32.h, p0/m, z32.h, z0.h
fminp z0.h, p8/m, z0.h, z0.h
fminp z0.h, p0/m, z0.h, z32.h

fmlalb z0.s, z0.h, z0.h[8]
fmlalb z0.s, z0.h, z8.h[0]
fmlalb z0.s, z32.h, z0.h[0]
fmlalb z32.s, z0.h, z0.h[0]
fmlalb z0.h, z0.h, z0.h[0]

fmlalb z32.s, z0.h, z0.h
fmlalb z0.s, z32.h, z0.h
fmlalb z0.s, z0.h, z32.h
fmlalb z0.s, z0.h, z0.d

fmlalt z0.s, z0.h, z0.h[8]
fmlalt z0.s, z0.h, z8.h[0]
fmlalt z0.s, z32.h, z0.h[0]
fmlalt z32.s, z0.h, z0.h[0]
fmlalt z0.h, z0.h, z0.h[0]

fmlalt z32.s, z0.h, z0.h
fmlalt z0.s, z32.h, z0.h
fmlalt z0.s, z0.h, z32.h
fmlalt z0.s, z0.h, z0.d

fmlslb z0.s, z0.h, z0.h[8]
fmlslb z0.s, z0.h, z8.h[0]
fmlslb z0.s, z32.h, z0.h[0]
fmlslb z32.s, z0.h, z0.h[0]
fmlslb z0.h, z0.h, z0.h[0]

fmlslb z32.s, z0.h, z0.h
fmlslb z0.s, z32.h, z0.h
fmlslb z0.s, z0.h, z32.h
fmlslb z0.s, z0.h, z0.d

fmlslt z0.s, z0.h, z0.h[8]
fmlslt z0.s, z0.h, z8.h[0]
fmlslt z0.s, z32.h, z0.h[0]
fmlslt z32.s, z0.h, z0.h[0]
fmlslt z0.h, z0.h, z0.h[0]

fmlslt z32.s, z0.h, z0.h
fmlslt z0.s, z32.h, z0.h
fmlslt z0.s, z0.h, z32.h
fmlslt z0.s, z0.h, z0.d

histcnt z32.s, p0/z, z0.s, z0.s
histcnt z0.s, p8/z, z0.s, z0.s
histcnt z0.s, p0/z, z32.s, z0.s
histcnt z0.s, p0/z, z0.s, z32.s
histcnt z0.s, p0/m, z0.s, z0.s
histcnt z0.d, p0/z, z0.s, z0.s

histseg z32.b, z0.b, z0.b
histseg z0.b, z32.b, z0.b
histseg z0.b, z0.b, z32.b
histseg z0.b, z0.b, z0.h

ldnt1b { z0.d, z1.d }, p0/z, [z0.d, x0]
ldnt1b { z0.d }, p0/m, [z0.d]
ldnt1b { z32.d }, p0/z, [z0.d]
ldnt1b { z0.d }, p8/z, [z0.d]
ldnt1b { z0.d }, p0/z, [z32.d]
ldnt1b { z0.d }, p0/z, [z0.d, sp]
ldnt1b { z0.d }, p0/z, [z0.d, x32]
ldnt1b { z0.d }, p0/z, [z0.d, w16]
ldnt1b { z0.d }, p0/z, [z0.d, z0.d]
ldnt1b { z0.s }, p0/z, [z0.d]
ldnt1b { z0.d }, p0/z, [z0.s]
ldnt1b { z0.s, z1.d }, p0/z, [z0.s, x0]
ldnt1b { z0.s }, p0/m, [z0.s]
ldnt1b { z32.s }, p0/z, [z0.s]
ldnt1b { z0.s }, p8/z, [z0.s]
ldnt1b { z0.s }, p0/z, [z32.s]
ldnt1b { z0.s }, p0/z, [z0.s, sp]
ldnt1b { z0.s }, p0/z, [z0.s, x32]
ldnt1b { z0.s }, p0/z, [z0.s, z0.s]

ldnt1d { z0.d, z1.d }, p0/z, [z0.d, x0]
ldnt1d { z0.d }, p0/m, [z0.d]
ldnt1d { z32.d }, p0/z, [z0.d]
ldnt1d { z0.d }, p8/z, [z0.d]
ldnt1d { z0.d }, p0/z, [z32.d]
ldnt1d { z0.d }, p0/z, [z0.d, sp]
ldnt1d { z0.d }, p0/z, [z0.d, x32]
ldnt1d { z0.d }, p0/z, [z0.d, w16]
ldnt1d { z0.d }, p0/z, [z0.d, z0.d]
ldnt1d { z0.s }, p0/z, [z0.d]
ldnt1d { z0.d }, p0/z, [z0.s]
ldnt1d { z0.d }, p0/m, [z0.d]

ldnt1h { z0.d, z1.d }, p0/z, [z0.d, x0]
ldnt1h { z0.d }, p0/m, [z0.d]
ldnt1h { z32.d }, p0/z, [z0.d]
ldnt1h { z0.d }, p8/z, [z0.d]
ldnt1h { z0.d }, p0/z, [z32.d]
ldnt1h { z0.d }, p0/z, [z0.d, sp]
ldnt1h { z0.d }, p0/z, [z0.d, x32]
ldnt1h { z0.d }, p0/z, [z0.d, w16]
ldnt1h { z0.d }, p0/z, [z0.d, z0.d]
ldnt1h { z0.s }, p0/z, [z0.d]
ldnt1h { z0.s, z1.d }, p0/z, [z0.s, x0]
ldnt1h { z32.s }, p0/z, [z0.s]
ldnt1h { z0.s }, p8/z, [z0.s]
ldnt1h { z0.s }, p0/z, [z32.s]
ldnt1h { z0.s }, p0/z, [z0.s, sp]
ldnt1h { z0.s }, p0/z, [z0.s, x32]
ldnt1h { z0.s }, p0/z, [z0.s, z0.s]

ldnt1sb { z0.d, z1.d }, p0/z, [z0.d, x0]
ldnt1sb { z0.d }, p0/m, [z0.d]
ldnt1sb { z32.d }, p0/z, [z0.d]
ldnt1sb { z0.d }, p8/z, [z0.d]
ldnt1sb { z0.d }, p0/z, [z32.d]
ldnt1sb { z0.d }, p0/z, [z0.d, sp]
ldnt1sb { z0.d }, p0/z, [z0.d, x32]
ldnt1sb { z0.d }, p0/z, [z0.d, w16]
ldnt1sb { z0.d }, p0/z, [z0.d, z0.d]

ldnt1sh { z0.d, z1.d }, p0/z, [z0.d, x0]
ldnt1sh { z0.d }, p0/m, [z0.d]
ldnt1sh { z32.d }, p0/z, [z0.d]
ldnt1sh { z0.d }, p8/z, [z0.d]
ldnt1sh { z0.d }, p0/z, [z32.d]
ldnt1sh { z0.d }, p0/z, [z0.d, sp]
ldnt1sh { z0.d }, p0/z, [z0.d, x32]
ldnt1sh { z0.d }, p0/z, [z0.d, w16]
ldnt1sh { z0.d }, p0/z, [z0.d, z0.d]

ldnt1sh { z0.d, z1.d }, p0/z, [z0.d, x0]
ldnt1sh { z0.d }, p0/m, [z0.d]
ldnt1sh { z32.d }, p0/z, [z0.d]
ldnt1sh { z0.d }, p8/z, [z0.d]
ldnt1sh { z0.d }, p0/z, [z32.d]
ldnt1sh { z0.d }, p0/z, [z0.d, sp]
ldnt1sh { z0.d }, p0/z, [z0.d, x32]
ldnt1sh { z0.d }, p0/z, [z0.d, w16]
ldnt1sh { z0.d }, p0/z, [z0.d, z0.d]

ldnt1w { z0.d, z1.d }, p0/z, [z0.d, x0]
ldnt1w { z0.d }, p0/m, [z0.d]
ldnt1w { z32.d }, p0/z, [z0.d]
ldnt1w { z0.d }, p8/z, [z0.d]
ldnt1w { z0.d }, p0/z, [z32.d]
ldnt1w { z0.d }, p0/z, [z0.d, sp]
ldnt1w { z0.d }, p0/z, [z0.d, x32]
ldnt1w { z0.d }, p0/z, [z0.d, w16]
ldnt1w { z0.d }, p0/z, [z0.d, z0.d]
ldnt1w { z0.s }, p0/z, [z0.d]
ldnt1w { z0.s, z1.d }, p0/z, [z0.s, x0]
ldnt1w { z32.s }, p0/z, [z0.s]
ldnt1w { z0.s }, p8/z, [z0.s]
ldnt1w { z0.s }, p0/z, [z32.s]
ldnt1w { z0.s }, p0/z, [z0.s, sp]
ldnt1w { z0.s }, p0/z, [z0.s, x32]
ldnt1w { z0.s }, p0/z, [z0.s, z0.s]

match p0.h, p0/z, z0.b, z0.b
match p16.b, p0/z, z0.b, z0.b
match p0.b, p8/z, z0.b, z0.b
match p0.b, p0/z, z32.b, z0.b
match p0.b, p0/z, z0.b, z32.b

mla z0.h, z0.h, z0.h[8]
mla z0.s, z0.h, z0.h[0]
mla z0.h, z0.h, z0.s[0]
mla z32.h, z0.h, z0.h[0]
mla z0.h, z32.h, z0.h[0]
mla z0.h, z0.h, z8.h[0]

mla z0.s, z0.s, z0.s[4]
mla z0.h, z0.s, z0.s[0]
mla z0.s, z0.s, z0.h[0]
mla z32.s, z0.s, z0.s[0]
mla z0.s, z32.s, z0.s[0]
mla z0.s, z0.s, z8.s[0]

mla z0.d, z0.d, z0.d[2]
mla z0.h, z0.d, z0.d[0]
mla z0.d, z0.d, z0.h[0]
mla z32.d, z0.d, z0.d[0]
mla z0.d, z32.d, z0.d[0]
mla z0.d, z0.d, z16.d[0]

mls z0.h, z0.h, z0.h[8]
mls z0.s, z0.h, z0.h[0]
mls z0.h, z0.h, z0.s[0]
mls z32.h, z0.h, z0.h[0]
mls z0.h, z32.h, z0.h[0]
mls z0.h, z0.h, z8.h[0]

mls z0.s, z0.s, z0.s[4]
mls z0.h, z0.s, z0.s[0]
mls z0.s, z0.s, z0.h[0]
mls z32.s, z0.s, z0.s[0]
mls z0.s, z32.s, z0.s[0]
mls z0.s, z0.s, z8.s[0]

mls z0.d, z0.d, z0.d[2]
mls z0.h, z0.d, z0.d[0]
mls z0.d, z0.d, z0.h[0]
mls z32.d, z0.d, z0.d[0]
mls z0.d, z32.d, z0.d[0]
mls z0.d, z0.d, z16.d[0]

mul z0.h, z0.h, z0.h[8]
mul z0.s, z0.h, z0.h[0]
mul z0.h, z0.h, z0.s[0]
mul z32.h, z0.h, z0.h[0]
mul z0.h, z32.h, z0.h[0]
mul z0.h, z0.h, z8.h[0]

mul z0.s, z0.s, z0.s[4]
mul z0.h, z0.s, z0.s[0]
mul z0.s, z0.s, z0.h[0]
mul z32.s, z0.s, z0.s[0]
mul z0.s, z32.s, z0.s[0]
mul z0.s, z0.s, z8.s[0]

mul z0.d, z0.d, z0.d[2]
mul z0.h, z0.d, z0.d[0]
mul z0.d, z0.d, z0.h[0]
mul z32.d, z0.d, z0.d[0]
mul z0.d, z32.d, z0.d[0]
mul z0.d, z0.d, z16.d[0]

mul z0.h, z0.b, z0.b
mul z32.b, z0.b, z0.b
mul z0.b, z32.b, z0.b
mul z0.b, z0.b, z0.b

nmatch p0.h, p0/z, z0.b, z0.b
nmatch p0.b, p0/m, z0.b, z0.b
nmatch p16.b, p0/z, z0.b, z0.b
nmatch p0.b, p8/z, z0.b, z0.b
nmatch p0.b, p0/z, z32.b, z0.b
nmatch p0.b, p0/z, z0.b, z32.b

nbsl z0.d, z1.d, z0.d, z0.d
nbsl z0.d, z0.d, z0.h, z0.d
nbsl z0.d, z0.h, z0.d, z0.d

pmul z0.h, z0.b, z0.b
pmul z32.b, z0.b, z0.b
pmul z0.b, z32.b, z0.b
pmul z0.b, z0.b, z32.b

pmullb z32.q, z0.d, z0.d
pmullb z0.q, z32.d, z0.d
pmullb z0.q, z0.d, z32.d
pmullb z0.d, z0.d, z0.d

pmullb z32.h, z0.b, z0.b
pmullb z0.h, z32.b, z0.b
pmullb z0.h, z0.b, z32.b
pmullb z0.b, z0.b, z0.b

pmullt z32.q, z0.d, z0.d
pmullt z0.q, z32.d, z0.d
pmullt z0.q, z0.d, z32.d
pmullt z0.d, z0.d, z0.d

pmullt z32.h, z0.b, z0.b
pmullt z0.h, z32.b, z0.b
pmullt z0.h, z0.b, z32.b
pmullt z0.b, z0.b, z0.b

raddhnb z0.h, z0.h, z0.h
raddhnb z32.b, z0.h, z0.h
raddhnb z0.b, z32.h, z0.h
raddhnb z0.b, z0.h, z32.h

raddhnt z0.h, z0.h, z0.h
raddhnt z32.b, z0.h, z0.h
raddhnt z0.b, z32.h, z0.h
raddhnt z0.b, z0.h, z32.h

rax1 z32.d, z0.d, z0.d
rax1 z0.d, z32.d, z0.d
rax1 z0.d, z0.d, z32.d
rax1 z0.d, z0.d, z0.h

# Too high a shift, too low a shift, invalid arguments.
rshrnb z32.b, z0.h, #8
rshrnb z0.b, z32.h, #8
rshrnb z0.b, z0.h, #9
rshrnb z0.b, z0.h, #0
rshrnb z0.h, z0.h, #8
rshrnb z0.h, z0.s, #0
rshrnb z0.h, z0.s, #17
rshrnb z0.s, z0.d, #0
rshrnb z0.s, z0.d, #33

movprfx z0, z1
rshrnt z0.b, z1.h, #8

rshrnt z32.b, z0.h, #8
rshrnt z0.b, z32.h, #8
rshrnt z0.b, z0.h, #9
rshrnt z0.b, z0.h, #0
rshrnt z0.h, z0.h, #8
rshrnt z0.h, z0.s, #0
rshrnt z0.h, z0.s, #17
rshrnt z0.s, z0.d, #0
rshrnt z0.s, z0.d, #33

rsubhnb z0.h, z0.h, z0.h
rsubhnb z32.b, z0.h, z0.h
rsubhnb z0.b, z32.h, z0.h
rsubhnb z0.b, z0.h, z32.h

rsubhnt z0.h, z0.h, z0.h
rsubhnt z32.b, z0.h, z0.h
rsubhnt z0.b, z32.h, z0.h
rsubhnt z0.b, z0.h, z32.h

saba z0.h, z0.b, z0.b
saba z32.b, z0.b, z0.b
saba z0.b, z32.b, z0.b
saba z0.b, z0.b, z32.b

sabalb z0.b, z0.b, z0.b
sabalb z32.h, z0.b, z0.b
sabalb z0.h, z32.b, z0.b
sabalb z0.h, z0.b, z32.b

sabalt z0.b, z0.b, z0.b
sabalt z32.h, z0.b, z0.b
sabalt z0.h, z32.b, z0.b
sabalt z0.h, z0.b, z32.b

sabdlb z0.b, z0.b, z0.b
sabdlb z32.h, z0.b, z0.b
sabdlb z0.h, z32.b, z0.b
sabdlb z0.h, z0.b, z32.b

sabdlt z0.b, z0.b, z0.b
sabdlt z32.h, z0.b, z0.b
sabdlt z0.h, z32.b, z0.b
sabdlt z0.h, z0.b, z32.b

sadalp z0.b, p0/m, z0.b
sadalp z0.h, p0/z, z0.b
sadalp z0.h, p8/m, z0.b
sadalp z32.h, p0/m, z0.b
sadalp z0.h, p0/m, z32.b

saddlb z0.b, z0.b, z0.b
saddlb z32.h, z0.b, z0.b
saddlb z0.h, z32.b, z0.b
saddlb z0.h, z0.b, z32.b

saddlbt z0.b, z0.b, z0.b
saddlbt z32.h, z0.b, z0.b
saddlbt z0.h, z32.b, z0.b
saddlbt z0.h, z0.b, z32.b

saddlt z0.b, z0.b, z0.b
saddlt z32.h, z0.b, z0.b
saddlt z0.h, z32.b, z0.b
saddlt z0.h, z0.b, z32.b

saddwb z0.b, z0.h, z0.b
saddwb z32.h, z0.h, z0.b
saddwb z0.h, z32.h, z0.b
saddwb z0.h, z0.h, z32.b

saddwt z0.b, z0.h, z0.b
saddwt z32.h, z0.h, z0.b
saddwt z0.h, z32.h, z0.b
saddwt z0.h, z0.h, z32.b

sbclb z0.d, z0.s, z0.s
sbclb z32.s, z0.s, z0.s
sbclb z0.s, z32.s, z0.s
sbclb z0.s, z0.s, z32.s

sbclt z0.d, z0.s, z0.s
sbclt z32.s, z0.s, z0.s
sbclt z0.s, z32.s, z0.s
sbclt z0.s, z0.s, z32.s

shadd z0.b, p0/m, z1.b, z0.b
shadd z32.b, p0/m, z0.b, z0.b
shadd z0.b, p8/m, z0.b, z0.b
shadd z0.b, p0/m, z32.b, z0.b
shadd z0.b, p0/m, z0.b, z32.b
shadd z0.h, p0/m, z0.b, z0.b
shadd z0.b, p0/z, z0.b, z0.b

shrnb z32.b, z0.h, #8
shrnb z0.b, z32.h, #8
shrnb z0.b, z0.h, #9
shrnb z0.b, z0.h, #0
shrnb z0.h, z0.h, #8
shrnb z0.h, z0.s, #0
shrnb z0.h, z0.s, #17
shrnb z0.s, z0.d, #0
shrnb z0.s, z0.d, #33

movprfx z0, z1
shrnt z0.b, z1.h, #8

shrnt z32.b, z0.h, #8
shrnt z0.b, z32.h, #8
shrnt z0.b, z0.h, #9
shrnt z0.b, z0.h, #0
shrnt z0.h, z0.h, #8
shrnt z0.h, z0.s, #0
shrnt z0.h, z0.s, #17
shrnt z0.s, z0.d, #0
shrnt z0.s, z0.d, #33

shsub z0.b, p0/m, z1.b, z0.b
shsub z32.b, p0/m, z0.b, z0.b
shsub z0.b, p8/m, z0.b, z0.b
shsub z0.b, p0/m, z32.b, z0.b
shsub z0.b, p0/m, z0.b, z32.b
shsub z0.h, p0/m, z0.b, z0.b
shsub z0.b, p0/z, z0.b, z0.b

shsubr z0.b, p0/m, z1.b, z0.b
shsubr z32.b, p0/m, z0.b, z0.b
shsubr z0.b, p8/m, z0.b, z0.b
shsubr z0.b, p0/m, z32.b, z0.b
shsubr z0.b, p0/m, z0.b, z32.b
shsubr z0.h, p0/m, z0.b, z0.b
shsubr z0.b, p0/z, z0.b, z0.b

sli z0.h, z0.b, #0
sli z32.b, z0.b, #0
sli z0.b, z32.b, #0
sli z0.b, z0.b, #8
sli z0.h, z0.h, #16
sli z0.s, z0.s, #32
sli z0.d, z0.d, #64

movprfx z0, z1
sm4e z0.s, z0.s, z1.s

sm4e z1.s, z0.s, z0.s
sm4e z32.s, z0.s, z0.s
sm4e z0.s, z32.s, z0.s
sm4e z0.s, z0.s, z32.s
sm4e z0.s, z0.s, z0.d

sm4ekey z32.s, z0.s, z0.s
sm4ekey z0.s, z32.s, z0.s
sm4ekey z0.s, z0.s, z32.s
sm4ekey z0.s, z0.s, z0.h

smaxp z0.h, p0/m, z0.b, z0.b
smaxp z0.b, p0/z, z0.b, z0.b
smaxp z1.b, p0/m, z0.b, z0.b
smaxp z32.b, p0/m, z0.b, z0.b
smaxp z0.b, p0/m, z32.b, z0.b
smaxp z0.b, p0/m, z0.b, z32.b
smaxp z0.b, p8/m, z0.b, z0.b

sminp z0.h, p0/m, z0.b, z0.b
sminp z0.b, p0/z, z0.b, z0.b
sminp z1.b, p0/m, z0.b, z0.b
sminp z32.b, p0/m, z0.b, z0.b
sminp z0.b, p0/m, z32.b, z0.b
sminp z0.b, p0/m, z0.b, z32.b
sminp z0.b, p8/m, z0.b, z0.b

smlalb z32.s, z0.h, z0.h[0]
smlalb z0.s, z32.h, z0.h[0]
smlalb z0.s, z0.h, z8.h[0]
smlalb z0.s, z0.h, z0.h[8]
smlalb z0.h, z0.h, z0.h[0]

smlalb z32.d, z0.s, z0.s[0]
smlalb z0.d, z32.s, z0.s[0]
smlalb z0.d, z0.s, z16.s[0]
smlalb z0.d, z0.s, z0.s[4]
smlalb z0.s, z0.s, z0.s[0]

smlalb z32.h, z0.b, z0.b
smlalb z0.h, z32.b, z0.b
smlalb z0.h, z0.b, z32.b
smlalb z0.s, z0.h, z0.x
smlalb z0.h, z0.b, z0.h

smlalt z32.s, z0.h, z0.h[0]
smlalt z0.s, z32.h, z0.h[0]
smlalt z0.s, z0.h, z8.h[0]
smlalt z0.s, z0.h, z0.h[8]
smlalt z0.h, z0.h, z0.h[0]

smlalt z32.d, z0.s, z0.s[0]
smlalt z0.d, z32.s, z0.s[0]
smlalt z0.d, z0.s, z16.s[0]
smlalt z0.d, z0.s, z0.s[4]
smlalt z0.s, z0.s, z0.s[0]

smlalt z32.h, z0.b, z0.b
smlalt z0.h, z32.b, z0.b
smlalt z0.h, z0.b, z32.b
smlalt z0.s, z0.h, z0.x
smlalt z0.h, z0.b, z0.h

smlslb z32.s, z0.h, z0.h[0]
smlslb z0.s, z32.h, z0.h[0]
smlslb z0.s, z0.h, z8.h[0]
smlslb z0.s, z0.h, z0.h[8]
smlslb z0.h, z0.h, z0.h[0]

smlslb z32.d, z0.s, z0.s[0]
smlslb z0.d, z32.s, z0.s[0]
smlslb z0.d, z0.s, z16.s[0]
smlslb z0.d, z0.s, z0.s[4]
smlslb z0.s, z0.s, z0.s[0]

smlslb z32.h, z0.b, z0.b
smlslb z0.h, z32.b, z0.b
smlslb z0.h, z0.b, z32.b
smlslb z0.s, z0.h, z0.x
smlslb z0.h, z0.b, z0.h

smlslt z32.s, z0.h, z0.h[0]
smlslt z0.s, z32.h, z0.h[0]
smlslt z0.s, z0.h, z8.h[0]
smlslt z0.s, z0.h, z0.h[8]
smlslt z0.h, z0.h, z0.h[0]

smlslt z32.d, z0.s, z0.s[0]
smlslt z0.d, z32.s, z0.s[0]
smlslt z0.d, z0.s, z16.s[0]
smlslt z0.d, z0.s, z0.s[4]
smlslt z0.s, z0.s, z0.s[0]

smlslt z32.h, z0.b, z0.b
smlslt z0.h, z32.b, z0.b
smlslt z0.h, z0.b, z32.b
smlslt z0.s, z0.h, z0.x
smlslt z0.h, z0.b, z0.h

smulh z0.h, z0.b, z0.b
smulh z32.b, z0.b, z0.b
smulh z0.b, z32.b, z0.b
smulh z0.b, z0.b, z32.b

smullb z32.s, z0.h, z0.h[0]
smullb z0.s, z32.h, z0.h[0]
smullb z0.s, z0.h, z8.h[0]
smullb z0.s, z0.h, z0.h[8]
smullb z0.h, z0.h, z0.h[0]

smullb z32.d, z0.s, z0.s[0]
smullb z0.d, z32.s, z0.s[0]
smullb z0.d, z0.s, z16.s[0]
smullb z0.d, z0.s, z0.s[4]
smullb z0.s, z0.s, z0.s[0]

smullb z32.h, z0.b, z0.b
smullb z0.h, z32.b, z0.b
smullb z0.h, z0.b, z32.b
smullb z0.s, z0.h, z0.x
smullb z0.h, z0.b, z0.h

smullt z32.s, z0.h, z0.h[0]
smullt z0.s, z32.h, z0.h[0]
smullt z0.s, z0.h, z8.h[0]
smullt z0.s, z0.h, z0.h[8]
smullt z0.h, z0.h, z0.h[0]

smullt z32.d, z0.s, z0.s[0]
smullt z0.d, z32.s, z0.s[0]
smullt z0.d, z0.s, z16.s[0]
smullt z0.d, z0.s, z0.s[4]
smullt z0.s, z0.s, z0.s[0]

smullt z32.h, z0.b, z0.b
smullt z0.h, z32.b, z0.b
smullt z0.h, z0.b, z32.b
smullt z0.s, z0.h, z0.x
smullt z0.h, z0.b, z0.h

splice z0.b, p0, { z0.b, z2.b }
splice z0.h, p0, { z0.b, z1.b }
splice z0.b, p0, { z0.h, z1.b }
splice z0.b, p0, { z0.b, z1.h }
splice z32.b, p0, { z0.b, z1.b }
splice z0.b, p8, { z0.b, z1.b }
splice z0.b, p0, { z31.b, z1.b }
splice z0.b, p0, { z31.b, z32.b }
splice z0.b, p0, { z32.b, z1.b }

sqabs z32.b, p0/m, z0.b
sqabs z0.b, p8/m, z0.b
sqabs z0.b, p0/m, z32.b
sqabs z0.b, p0/m, z0.h
sqabs z0.b, p0/z, z0.b

sqadd z32.b, p0/m, z0.b, z0.b
sqadd z0.b, p0/m, z32.b, z0.b
sqadd z0.b, p0/m, z0.b, z32.b
sqadd z0.b, p0/m, z1.b, z0.b
sqadd z0.b, p8/m, z0.b, z0.b
sqadd z0.h, p0/m, z0.b, z0.b
sqadd z0.b, p0/z, z0.b, z0.b

sqcadd z0.b, z0.b, z0.b, #180
sqcadd z0.b, z1.b, z0.b, #90
sqcadd z32.b, z0.b, z0.b, #90
sqcadd z0.b, z32.b, z0.b, #90
sqcadd z0.b, z0.b, z32.b, #90
sqcadd z0.b, z0.b, z0.h, #90

sqdmlalb z32.s, z0.h, z0.h[0]
sqdmlalb z0.s, z32.h, z0.h[0]
sqdmlalb z0.s, z0.h, z8.h[0]
sqdmlalb z0.s, z0.h, z0.h[8]
sqdmlalb z0.h, z0.h, z0.h[0]

sqdmlalb z32.d, z0.s, z0.s[0]
sqdmlalb z0.d, z32.s, z0.s[0]
sqdmlalb z0.d, z0.s, z16.s[0]
sqdmlalb z0.d, z0.s, z0.s[4]
sqdmlalb z0.s, z0.s, z0.s[0]

sqdmlalb z32.h, z0.b, z0.b
sqdmlalb z0.h, z32.b, z0.b
sqdmlalb z0.h, z0.b, z32.b
sqdmlalb z0.s, z0.h, z0.x
sqdmlalb z0.h, z0.b, z0.h

sqdmlalbt z32.h, z0.b, z0.b
sqdmlalbt z0.h, z32.b, z0.b
sqdmlalbt z0.h, z0.b, z32.b
sqdmlalbt z0.s, z0.h, z0.x
sqdmlalbt z0.h, z0.b, z0.h

sqdmlalt z32.s, z0.h, z0.h[0]
sqdmlalt z0.s, z32.h, z0.h[0]
sqdmlalt z0.s, z0.h, z8.h[0]
sqdmlalt z0.s, z0.h, z0.h[8]
sqdmlalt z0.h, z0.h, z0.h[0]

sqdmlalt z32.d, z0.s, z0.s[0]
sqdmlalt z0.d, z32.s, z0.s[0]
sqdmlalt z0.d, z0.s, z16.s[0]
sqdmlalt z0.d, z0.s, z0.s[4]
sqdmlalt z0.s, z0.s, z0.s[0]

sqdmlalt z32.h, z0.b, z0.b
sqdmlalt z0.h, z32.b, z0.b
sqdmlalt z0.h, z0.b, z32.b
sqdmlalt z0.s, z0.h, z0.x
sqdmlalt z0.h, z0.b, z0.h

sqdmlslb z32.s, z0.h, z0.h[0]
sqdmlslb z0.s, z32.h, z0.h[0]
sqdmlslb z0.s, z0.h, z8.h[0]
sqdmlslb z0.s, z0.h, z0.h[8]
sqdmlslb z0.h, z0.h, z0.h[0]

sqdmlslb z32.d, z0.s, z0.s[0]
sqdmlslb z0.d, z32.s, z0.s[0]
sqdmlslb z0.d, z0.s, z16.s[0]
sqdmlslb z0.d, z0.s, z0.s[4]
sqdmlslb z0.s, z0.s, z0.s[0]

sqdmlslb z32.h, z0.b, z0.b
sqdmlslb z0.h, z32.b, z0.b
sqdmlslb z0.h, z0.b, z32.b
sqdmlslb z0.s, z0.h, z0.x
sqdmlslb z0.h, z0.b, z0.h

sqdmlslbt z32.h, z0.b, z0.b
sqdmlslbt z0.h, z32.b, z0.b
sqdmlslbt z0.h, z0.b, z32.b
sqdmlslbt z0.s, z0.h, z0.x
sqdmlslbt z0.h, z0.b, z0.h

sqdmlslt z32.s, z0.h, z0.h[0]
sqdmlslt z0.s, z32.h, z0.h[0]
sqdmlslt z0.s, z0.h, z8.h[0]
sqdmlslt z0.s, z0.h, z0.h[8]
sqdmlslt z0.h, z0.h, z0.h[0]

sqdmlslt z32.d, z0.s, z0.s[0]
sqdmlslt z0.d, z32.s, z0.s[0]
sqdmlslt z0.d, z0.s, z16.s[0]
sqdmlslt z0.d, z0.s, z0.s[4]
sqdmlslt z0.s, z0.s, z0.s[0]

sqdmlslt z32.h, z0.b, z0.b
sqdmlslt z0.h, z32.b, z0.b
sqdmlslt z0.h, z0.b, z32.b
sqdmlslt z0.s, z0.h, z0.x
sqdmlslt z0.h, z0.b, z0.h

sqdmulh z32.h, z0.h, z0.h[0]
sqdmulh z0.h, z32.h, z0.h[0]
sqdmulh z0.h, z0.h, z8.h[0]
sqdmulh z0.h, z0.h, z0.h[8]
sqdmulh z0.s, z0.h, z0.h[0]
sqdmulh z0.h, z0.h, z0.s[0]

sqdmulh z32.s, z0.s, z0.s[0]
sqdmulh z0.s, z32.s, z0.s[0]
sqdmulh z0.s, z0.s, z8.s[0]
sqdmulh z0.s, z0.s, z0.s[4]
sqdmulh z0.s, z0.h, z0.s[0]
sqdmulh z0.s, z0.s, z0.h[0]

sqdmulh z32.d, z0.d, z0.d[0]
sqdmulh z0.d, z32.d, z0.d[0]
sqdmulh z0.d, z0.d, z16.d[0]
sqdmulh z0.d, z0.d, z0.d[2]
sqdmulh z0.d, z0.h, z0.d[0]
sqdmulh z0.d, z0.d, z0.h[0]

sqdmulh z32.h, z0.b, z0.b
sqdmulh z0.h, z32.b, z0.b
sqdmulh z0.h, z0.b, z32.b
sqdmulh z0.s, z0.h, z0.x
sqdmulh z0.h, z0.b, z0.h

sqdmullb z32.s, z0.h, z0.h[0]
sqdmullb z0.s, z32.h, z0.h[0]
sqdmullb z0.s, z0.h, z8.h[0]
sqdmullb z0.s, z0.h, z0.h[8]
sqdmullb z0.h, z0.h, z0.h[0]

sqdmullb z32.d, z0.s, z0.s[0]
sqdmullb z0.d, z32.s, z0.s[0]
sqdmullb z0.d, z0.s, z16.s[0]
sqdmullb z0.d, z0.s, z0.s[4]
sqdmullb z0.s, z0.s, z0.s[0]

sqdmullb z32.h, z0.b, z0.b
sqdmullb z0.h, z32.b, z0.b
sqdmullb z0.h, z0.b, z32.b
sqdmullb z0.s, z0.h, z0.x
sqdmullb z0.h, z0.b, z0.h

sqdmullt z32.s, z0.h, z0.h[0]
sqdmullt z0.s, z32.h, z0.h[0]
sqdmullt z0.s, z0.h, z8.h[0]
sqdmullt z0.s, z0.h, z0.h[8]
sqdmullt z0.h, z0.h, z0.h[0]

sqdmullt z32.d, z0.s, z0.s[0]
sqdmullt z0.d, z32.s, z0.s[0]
sqdmullt z0.d, z0.s, z16.s[0]
sqdmullt z0.d, z0.s, z0.s[4]
sqdmullt z0.s, z0.s, z0.s[0]

sqdmullt z32.h, z0.b, z0.b
sqdmullt z0.h, z32.b, z0.b
sqdmullt z0.h, z0.b, z32.b
sqdmullt z0.s, z0.h, z0.x
sqdmullt z0.h, z0.b, z0.h

sqneg z32.b, p0/m, z0.b
sqneg z0.b, p8/m, z0.b
sqneg z0.b, p0/m, z32.b
sqneg z0.b, p0/m, z0.h
sqneg z0.b, p0/z, z0.b

sqrdcmlah z32.h, z0.h, z0.h[0], #0
sqrdcmlah z0.h, z32.h, z0.h[0], #0
sqrdcmlah z0.h, z0.h, z8.h[0], #0
sqrdcmlah z0.h, z0.h, z0.h[4], #0
sqrdcmlah z0.h, z0.h, z0.h[0], #1
sqrdcmlah z0.h, z0.h, z0.h[0], #360
sqrdcmlah z0.h, z0.h, z0.s[0], #0
sqrdcmlah z0.h, z0.s, z0.h[0], #0

sqrdcmlah z32.s, z0.s, z0.s[0], #0
sqrdcmlah z0.s, z32.s, z0.s[0], #0
sqrdcmlah z0.s, z0.s, z16.s[0], #0
sqrdcmlah z0.s, z0.s, z0.s[2], #0
sqrdcmlah z0.s, z0.s, z0.s[0], #1
sqrdcmlah z0.s, z0.s, z0.s[0], #360
sqrdcmlah z0.s, z0.s, z0.h[0], #0
sqrdcmlah z0.s, z0.h, z0.s[0], #0

sqrdcmlah z32.b, z0.b, z0.b, #0
sqrdcmlah z0.b, z32.b, z0.b, #0
sqrdcmlah z0.b, z0.b, z32.b, #0
sqrdcmlah z0.b, z0.b, z0.b, #1
sqrdcmlah z0.b, z0.b, z0.b, #360
sqrdcmlah z0.b, z0.b, z0.h, #0

sqrdmlah z32.h, z0.h, z0.h[0]
sqrdmlah z0.h, z32.h, z0.h[0]
sqrdmlah z0.h, z0.h, z8.h[0]
sqrdmlah z0.h, z0.h, z0.h[8]
sqrdmlah z0.s, z0.h, z0.h[0]
sqrdmlah z0.h, z0.h, z0.s[0]

sqrdmlah z32.s, z0.s, z0.s[0]
sqrdmlah z0.s, z32.s, z0.s[0]
sqrdmlah z0.s, z0.s, z8.s[0]
sqrdmlah z0.s, z0.s, z0.s[4]
sqrdmlah z0.s, z0.h, z0.s[0]
sqrdmlah z0.s, z0.s, z0.h[0]

sqrdmlah z32.d, z0.d, z0.d[0]
sqrdmlah z0.d, z32.d, z0.d[0]
sqrdmlah z0.d, z0.d, z16.d[0]
sqrdmlah z0.d, z0.d, z0.d[2]
sqrdmlah z0.d, z0.h, z0.d[0]
sqrdmlah z0.d, z0.d, z0.h[0]

sqrdmlah z32.h, z0.b, z0.b
sqrdmlah z0.h, z32.b, z0.b
sqrdmlah z0.h, z0.b, z32.b
sqrdmlah z0.s, z0.h, z0.x
sqrdmlah z0.h, z0.b, z0.h

sqrdmlsh z32.h, z0.h, z0.h[0]
sqrdmlsh z0.h, z32.h, z0.h[0]
sqrdmlsh z0.h, z0.h, z8.h[0]
sqrdmlsh z0.h, z0.h, z0.h[8]
sqrdmlsh z0.s, z0.h, z0.h[0]
sqrdmlsh z0.h, z0.h, z0.s[0]

sqrdmlsh z32.s, z0.s, z0.s[0]
sqrdmlsh z0.s, z32.s, z0.s[0]
sqrdmlsh z0.s, z0.s, z8.s[0]
sqrdmlsh z0.s, z0.s, z0.s[4]
sqrdmlsh z0.s, z0.h, z0.s[0]
sqrdmlsh z0.s, z0.s, z0.h[0]

sqrdmlsh z32.d, z0.d, z0.d[0]
sqrdmlsh z0.d, z32.d, z0.d[0]
sqrdmlsh z0.d, z0.d, z16.d[0]
sqrdmlsh z0.d, z0.d, z0.d[2]
sqrdmlsh z0.d, z0.h, z0.d[0]
sqrdmlsh z0.d, z0.d, z0.h[0]

sqrdmlsh z32.h, z0.b, z0.b
sqrdmlsh z0.h, z32.b, z0.b
sqrdmlsh z0.h, z0.b, z32.b
sqrdmlsh z0.s, z0.h, z0.x
sqrdmlsh z0.h, z0.b, z0.h

sqrdmulh z32.h, z0.h, z0.h[0]
sqrdmulh z0.h, z32.h, z0.h[0]
sqrdmulh z0.h, z0.h, z8.h[0]
sqrdmulh z0.h, z0.h, z0.h[8]
sqrdmulh z0.s, z0.h, z0.h[0]
sqrdmulh z0.h, z0.h, z0.s[0]

sqrdmulh z32.s, z0.s, z0.s[0]
sqrdmulh z0.s, z32.s, z0.s[0]
sqrdmulh z0.s, z0.s, z8.s[0]
sqrdmulh z0.s, z0.s, z0.s[4]
sqrdmulh z0.s, z0.h, z0.s[0]
sqrdmulh z0.s, z0.s, z0.h[0]

sqrdmulh z32.d, z0.d, z0.d[0]
sqrdmulh z0.d, z32.d, z0.d[0]
sqrdmulh z0.d, z0.d, z16.d[0]
sqrdmulh z0.d, z0.d, z0.d[2]
sqrdmulh z0.d, z0.h, z0.d[0]
sqrdmulh z0.d, z0.d, z0.h[0]

sqrdmulh z32.h, z0.b, z0.b
sqrdmulh z0.h, z32.b, z0.b
sqrdmulh z0.h, z0.b, z32.b
sqrdmulh z0.s, z0.h, z0.x
sqrdmulh z0.h, z0.b, z0.h

sqrshl z32.b, p0/m, z0.b, z0.b
sqrshl z0.b, p0/m, z32.b, z0.b
sqrshl z0.b, p0/m, z0.b, z32.b
sqrshl z0.b, p0/m, z1.b, z0.b
sqrshl z0.b, p8/m, z0.b, z0.b
sqrshl z0.h, p0/m, z0.b, z0.b
sqrshl z0.b, p0/z, z0.b, z0.b

sqrshlr z32.b, p0/m, z0.b, z0.b
sqrshlr z0.b, p0/m, z32.b, z0.b
sqrshlr z0.b, p0/m, z0.b, z32.b
sqrshlr z0.b, p0/m, z1.b, z0.b
sqrshlr z0.b, p8/m, z0.b, z0.b
sqrshlr z0.h, p0/m, z0.b, z0.b
sqrshlr z0.b, p0/z, z0.b, z0.b

sqrshrnb z32.b, z0.h, #8
sqrshrnb z0.b, z32.h, #8
sqrshrnb z0.b, z0.h, #9
sqrshrnb z0.b, z0.h, #0
sqrshrnb z0.h, z0.h, #8
sqrshrnb z0.h, z0.s, #0
sqrshrnb z0.h, z0.s, #17
sqrshrnb z0.s, z0.d, #0
sqrshrnb z0.s, z0.d, #33

movprfx z0, z1
sqrshrnt z0.b, z0.h, #1

sqrshrnt z32.b, z0.h, #8
sqrshrnt z0.b, z32.h, #8
sqrshrnt z0.b, z0.h, #9
sqrshrnt z0.b, z0.h, #0
sqrshrnt z0.h, z0.h, #8
sqrshrnt z0.h, z0.s, #0
sqrshrnt z0.h, z0.s, #17
sqrshrnt z0.s, z0.d, #0
sqrshrnt z0.s, z0.d, #33

sqrshrunb z32.b, z0.h, #8
sqrshrunb z0.b, z32.h, #8
sqrshrunb z0.b, z0.h, #9
sqrshrunb z0.b, z0.h, #0
sqrshrunb z0.h, z0.h, #8
sqrshrunb z0.h, z0.s, #0
sqrshrunb z0.h, z0.s, #17
sqrshrunb z0.s, z0.d, #0
sqrshrunb z0.s, z0.d, #33

movprfx z0, z1
sqrshrunt z0.b, z0.h, #1

sqrshrunt z32.b, z0.h, #8
sqrshrunt z0.b, z32.h, #8
sqrshrunt z0.b, z0.h, #9
sqrshrunt z0.b, z0.h, #0
sqrshrunt z0.h, z0.h, #8
sqrshrunt z0.h, z0.s, #0
sqrshrunt z0.h, z0.s, #17
sqrshrunt z0.s, z0.d, #0
sqrshrunt z0.s, z0.d, #33

sqshl z0.h, p0/m, z0.b, #0
sqshl z32.b, p0/m, z32.b, #0
sqshl z0.b, p0/m, z1.b, #0
sqshl z0.b, p8/m, z0.b, #0
sqshl z0.b, p0/m, z0.b, #8
sqshl z0.h, p0/m, z0.h, #16
sqshl z0.s, p0/m, z0.s, #32
sqshl z0.d, p0/m, z0.d, #64

sqshl z32.b, p0/m, z0.b, z0.b
sqshl z0.b, p0/m, z32.b, z0.b
sqshl z0.b, p0/m, z0.b, z32.b
sqshl z0.b, p0/m, z1.b, z0.b
sqshl z0.b, p8/m, z0.b, z0.b
sqshl z0.h, p0/m, z0.b, z0.b
sqshl z0.b, p0/z, z0.b, z0.b

sqshlr z32.b, p0/m, z0.b, z0.b
sqshlr z0.b, p0/m, z32.b, z0.b
sqshlr z0.b, p0/m, z0.b, z32.b
sqshlr z0.b, p0/m, z1.b, z0.b
sqshlr z0.b, p8/m, z0.b, z0.b
sqshlr z0.h, p0/m, z0.b, z0.b
sqshlr z0.b, p0/z, z0.b, z0.b

sqshlu z0.h, p0/m, z0.b, #0
sqshlu z32.b, p0/m, z32.b, #0
sqshlu z0.b, p0/m, z1.b, #0
sqshlu z0.b, p8/m, z0.b, #0
sqshlu z0.b, p0/m, z0.b, #8
sqshlu z0.h, p0/m, z0.h, #16
sqshlu z0.s, p0/m, z0.s, #32
sqshlu z0.d, p0/m, z0.d, #64

sqshrnb z32.b, z0.h, #8
sqshrnb z0.b, z32.h, #8
sqshrnb z0.b, z0.h, #9
sqshrnb z0.b, z0.h, #0
sqshrnb z0.h, z0.h, #8
sqshrnb z0.h, z0.s, #0
sqshrnb z0.h, z0.s, #17
sqshrnb z0.s, z0.d, #0
sqshrnb z0.s, z0.d, #33

movprfx z0, z1
sqshrnt z0.b, z0.h, #1

sqshrnt z32.b, z0.h, #8
sqshrnt z0.b, z32.h, #8
sqshrnt z0.b, z0.h, #9
sqshrnt z0.b, z0.h, #0
sqshrnt z0.h, z0.h, #8
sqshrnt z0.h, z0.s, #0
sqshrnt z0.h, z0.s, #17
sqshrnt z0.s, z0.d, #0
sqshrnt z0.s, z0.d, #33

sqshrunb z32.b, z0.h, #8
sqshrunb z0.b, z32.h, #8
sqshrunb z0.b, z0.h, #9
sqshrunb z0.b, z0.h, #0
sqshrunb z0.h, z0.h, #8
sqshrunb z0.h, z0.s, #0
sqshrunb z0.h, z0.s, #17
sqshrunb z0.s, z0.d, #0
sqshrunb z0.s, z0.d, #33

movprfx z0, z1
sqshrunt z0.b, z0.h, #1

sqshrunt z32.b, z0.h, #8
sqshrunt z0.b, z32.h, #8
sqshrunt z0.b, z0.h, #9
sqshrunt z0.b, z0.h, #0
sqshrunt z0.h, z0.h, #8
sqshrunt z0.h, z0.s, #0
sqshrunt z0.h, z0.s, #17
sqshrunt z0.s, z0.d, #0
sqshrunt z0.s, z0.d, #33

sqsub z32.b, p0/m, z0.b, z0.b
sqsub z0.b, p0/m, z32.b, z0.b
sqsub z0.b, p0/m, z0.b, z32.b
sqsub z0.b, p0/m, z1.b, z0.b
sqsub z0.b, p8/m, z0.b, z0.b
sqsub z0.h, p0/m, z0.b, z0.b
sqsub z0.b, p0/z, z0.b, z0.b

sqsubr z32.b, p0/m, z0.b, z0.b
sqsubr z0.b, p0/m, z32.b, z0.b
sqsubr z0.b, p0/m, z0.b, z32.b
sqsubr z0.b, p0/m, z1.b, z0.b
sqsubr z0.b, p8/m, z0.b, z0.b
sqsubr z0.h, p0/m, z0.b, z0.b
sqsubr z0.b, p0/z, z0.b, z0.b

sqxtnb z32.b, z0.h
sqxtnb z0.b, z32.h
sqxtnb z0.b, z0.s

sqxtnt z32.b, z0.h
sqxtnt z0.b, z32.h
sqxtnt z0.b, z0.s

sqxtunb z32.b, z0.h
sqxtunb z0.b, z32.h
sqxtunb z0.b, z0.s

sqxtunt z32.b, z0.h
sqxtunt z0.b, z32.h
sqxtunt z0.b, z0.s

srhadd z32.b, p0/m, z0.b, z0.b
srhadd z0.b, p0/m, z32.b, z0.b
srhadd z0.b, p0/m, z0.b, z32.b
srhadd z0.b, p0/m, z1.b, z0.b
srhadd z0.b, p8/m, z0.b, z0.b
srhadd z0.h, p0/m, z0.b, z0.b
srhadd z0.b, p0/z, z0.b, z0.b

sri z0.h, z0.b, #1
sri z32.b, z0.b, #1
sri z0.b, z32.b, #1
sri z0.b, z0.b, #0
sri z0.b, z0.b, #9
sri z0.h, z0.h, #0
sri z0.h, z0.h, #17
sri z0.s, z0.s, #0
sri z0.s, z0.s, #33
sri z0.d, z0.d, #0
sri z0.d, z0.d, #64

srshl z32.b, p0/m, z0.b, z0.b
srshl z0.b, p0/m, z32.b, z0.b
srshl z0.b, p0/m, z0.b, z32.b
srshl z0.b, p0/m, z1.b, z0.b
srshl z0.b, p8/m, z0.b, z0.b
srshl z0.h, p0/m, z0.b, z0.b
srshl z0.b, p0/z, z0.b, z0.b

srshlr z32.b, p0/m, z0.b, z0.b
srshlr z0.b, p0/m, z32.b, z0.b
srshlr z0.b, p0/m, z0.b, z32.b
srshlr z0.b, p0/m, z1.b, z0.b
srshlr z0.b, p8/m, z0.b, z0.b
srshlr z0.h, p0/m, z0.b, z0.b
srshlr z0.b, p0/z, z0.b, z0.b

srshr z0.h, p0/m, z0.b, #1
srshr z32.b, p0/m, z32.b, #1
srshr z0.b, p0/m, z1.b, #1
srshr z0.b, p8/m, z0.b, #1
srshr z0.b, p0/m, z0.b, #0
srshr z0.b, p0/m, z0.b, #9
srshr z0.h, p0/m, z0.h, #0
srshr z0.h, p0/m, z0.h, #17
srshr z0.s, p0/m, z0.s, #0
srshr z0.s, p0/m, z0.s, #33
srshr z0.d, p0/m, z0.d, #0
srshr z0.d, p0/m, z0.d, #65

srsra z0.h, z0.b, #1
srsra z32.b, z0.b, #1
srsra z0.b, z32.b, #1
srsra z0.b, z0.b, #0
srsra z0.b, z0.b, #9
srsra z0.h, z0.h, #0
srsra z0.h, z0.h, #17
srsra z0.s, z0.s, #0
srsra z0.s, z0.s, #33
srsra z0.d, z0.d, #0
srsra z0.d, z0.d, #64

sshllb z0.b, z0.b, #0
sshllb z32.h, z0.b, #0
sshllb z0.h, z32.b, #0
sshllb z0.h, z0.b, #8
sshllb z0.s, z0.h, #16
sshllb z0.d, z0.s, #32

sshllt z0.b, z0.b, #0
sshllt z32.h, z0.b, #0
sshllt z0.h, z32.b, #0
sshllt z0.h, z0.b, #8
sshllt z0.s, z0.h, #16
sshllt z0.d, z0.s, #32

ssra z0.h, z0.b, #1
ssra z32.b, z0.b, #1
ssra z0.b, z32.b, #1
ssra z0.b, z0.b, #0
ssra z0.b, z0.b, #9
ssra z0.h, z0.h, #0
ssra z0.h, z0.h, #17
ssra z0.s, z0.s, #0
ssra z0.s, z0.s, #33
ssra z0.d, z0.d, #0
ssra z0.d, z0.d, #64

ssublb z32.h, z0.b, z0.b
ssublb z0.h, z32.b, z0.b
ssublb z0.h, z0.b, z32.b
ssublb z0.s, z0.h, z0.x
ssublb z0.h, z0.b, z0.h

ssublbt z32.h, z0.b, z0.b
ssublbt z0.h, z32.b, z0.b
ssublbt z0.h, z0.b, z32.b
ssublbt z0.s, z0.h, z0.x
ssublbt z0.h, z0.b, z0.h

ssublt z32.h, z0.b, z0.b
ssublt z0.h, z32.b, z0.b
ssublt z0.h, z0.b, z32.b
ssublt z0.s, z0.h, z0.x
ssublt z0.h, z0.b, z0.h

ssubltb z32.h, z0.b, z0.b
ssubltb z0.h, z32.b, z0.b
ssubltb z0.h, z0.b, z32.b
ssubltb z0.s, z0.h, z0.x
ssubltb z0.h, z0.b, z0.h

ssubwb z32.h, z0.h, z0.b
ssubwb z0.h, z32.h, z0.b
ssubwb z0.h, z0.h, z32.b
ssubwb z0.s, z0.s, z0.x
ssubwb z0.h, z0.h, z0.h

ssubwt z32.h, z0.h, z0.b
ssubwt z0.h, z32.h, z0.b
ssubwt z0.h, z0.h, z32.b
ssubwt z0.s, z0.s, z0.x
ssubwt z0.h, z0.h, z0.h

stnt1b { z0.d, z1.d }, p0, [z0.d, x0]
stnt1b { z0.d }, p0/m, [z0.d]
stnt1b { z32.d }, p0, [z0.d]
stnt1b { z0.d }, p8, [z0.d]
stnt1b { z0.d }, p0, [z32.d]
stnt1b { z0.d }, p0, [z0.d, sp]
stnt1b { z0.d }, p0, [z0.d, x32]
stnt1b { z0.d }, p0, [z0.d, w16]
stnt1b { z0.d }, p0, [z0.d, z0.d]
stnt1b { z0.s }, p0, [z0.d]
stnt1b { z0.s, z1.d }, p0, [z0.s, x0]
stnt1b { z32.s }, p0, [z0.s]
stnt1b { z0.s }, p8, [z0.s]
stnt1b { z0.s }, p0, [z32.s]
stnt1b { z0.s }, p0, [z0.s, sp]
stnt1b { z0.s }, p0, [z0.s, x32]
stnt1b { z0.s }, p0, [z0.s, z0.s]

stnt1d { z0.d, z1.d }, p0, [z0.d, x0]
stnt1d { z0.d }, p0/m, [z0.d]
stnt1d { z32.d }, p0, [z0.d]
stnt1d { z0.d }, p8, [z0.d]
stnt1d { z0.d }, p0, [z32.d]
stnt1d { z0.d }, p0, [z0.d, sp]
stnt1d { z0.d }, p0, [z0.d, x32]
stnt1d { z0.d }, p0, [z0.d, w16]
stnt1d { z0.d }, p0, [z0.d, z0.d]
stnt1d { z0.s }, p0, [z0.d]

stnt1h { z0.d, z1.d }, p0, [z0.d, x0]
stnt1h { z0.d }, p0/m, [z0.d]
stnt1h { z32.d }, p0, [z0.d]
stnt1h { z0.d }, p8, [z0.d]
stnt1h { z0.d }, p0, [z32.d]
stnt1h { z0.d }, p0, [z0.d, sp]
stnt1h { z0.d }, p0, [z0.d, x32]
stnt1h { z0.d }, p0, [z0.d, w16]
stnt1h { z0.d }, p0, [z0.d, z0.d]
stnt1h { z0.s }, p0, [z0.d]
stnt1h { z0.s, z1.d }, p0, [z0.s, x0]
stnt1h { z32.s }, p0, [z0.s]
stnt1h { z0.s }, p8, [z0.s]
stnt1h { z0.s }, p0, [z32.s]
stnt1h { z0.s }, p0, [z0.s, sp]
stnt1h { z0.s }, p0, [z0.s, x32]
stnt1h { z0.s }, p0, [z0.s, z0.s]

stnt1w { z0.d, z1.d }, p0, [z0.d, x0]
stnt1w { z0.d }, p0/m, [z0.d]
stnt1w { z32.d }, p0, [z0.d]
stnt1w { z0.d }, p8, [z0.d]
stnt1w { z0.d }, p0, [z32.d]
stnt1w { z0.d }, p0, [z0.d, sp]
stnt1w { z0.d }, p0, [z0.d, x32]
stnt1w { z0.d }, p0, [z0.d, w16]
stnt1w { z0.d }, p0, [z0.d, z0.d]
stnt1w { z0.s }, p0, [z0.d]
stnt1w { z0.s, z1.d }, p0, [z0.s, x0]
stnt1w { z32.s }, p0, [z0.s]
stnt1w { z0.s }, p8, [z0.s]
stnt1w { z0.s }, p0, [z32.s]
stnt1w { z0.s }, p0, [z0.s, sp]
stnt1w { z0.s }, p0, [z0.s, x32]
stnt1w { z0.s }, p0, [z0.s, z0.s]

subhnb z0.h, z0.h, z0.h
subhnb z32.b, z0.h, z0.h
subhnb z0.b, z32.h, z0.h
subhnb z0.b, z0.h, z32.h

subhnt z0.h, z0.h, z0.h
subhnt z32.b, z0.h, z0.h
subhnt z0.b, z32.h, z0.h
subhnt z0.b, z0.h, z32.h

suqadd z32.b, p0/m, z0.b, z0.b
suqadd z0.b, p0/m, z32.b, z0.b
suqadd z0.b, p0/m, z0.b, z32.b
suqadd z0.b, p0/m, z1.b, z0.b
suqadd z0.b, p8/m, z0.b, z0.b
suqadd z0.h, p0/m, z0.b, z0.b
suqadd z0.b, p0/z, z0.b, z0.b

tbl z32.b, { z0.b, z1.b }, z0.b
tbl z0.b, { z31.b, z32.b }, z0.b
tbl z0.b, { z31.b, z1.b }, z0.b
tbl z0.b, { z0.b, z1.b }, z32.b
tbl z0.b, { z0.b, z1.b }, z0.h
tbl z0.b, { z0.b, z1.h }, z0.b
tbl z0.b, { z0.h, z0.b }, z0.b
tbl z0.h, { z0.b, z0.b }, z0.b
tbl z0.h, { z0.b, z1.b }, z0.b

tbx z32.h, z0.b, z0.b
tbx z0.h, z32.b, z0.b
tbx z0.h, z0.b, z32.b
tbx z0.s, z0.h, z0.x
tbx z0.h, z0.b, z0.h

uaba z32.h, z0.b, z0.b
uaba z0.h, z32.b, z0.b
uaba z0.h, z0.b, z32.b
uaba z0.s, z0.h, z0.x
uaba z0.h, z0.b, z0.h

uabalb z32.h, z0.b, z0.b
uabalb z0.h, z32.b, z0.b
uabalb z0.h, z0.b, z32.b
uabalb z0.s, z0.h, z0.x
uabalb z0.h, z0.b, z0.h

uabalt z32.h, z0.b, z0.b
uabalt z0.h, z32.b, z0.b
uabalt z0.h, z0.b, z32.b
uabalt z0.s, z0.h, z0.x
uabalt z0.h, z0.b, z0.h

uabdlb z32.h, z0.b, z0.b
uabdlb z0.h, z32.b, z0.b
uabdlb z0.h, z0.b, z32.b
uabdlb z0.s, z0.h, z0.x
uabdlb z0.h, z0.b, z0.h

uabdlt z32.h, z0.b, z0.b
uabdlt z0.h, z32.b, z0.b
uabdlt z0.h, z0.b, z32.b
uabdlt z0.s, z0.h, z0.x
uabdlt z0.h, z0.b, z0.h

uadalp z0.b, p0/m, z0.b
uadalp z0.h, p0/z, z0.b
uadalp z0.h, p8/m, z0.b
uadalp z32.h, p0/m, z0.b
uadalp z0.h, p0/m, z32.b

uaddlb z32.h, z0.b, z0.b
uaddlb z0.h, z32.b, z0.b
uaddlb z0.h, z0.b, z32.b
uaddlb z0.s, z0.h, z0.x
uaddlb z0.h, z0.b, z0.h

uaddlt z32.h, z0.b, z0.b
uaddlt z0.h, z32.b, z0.b
uaddlt z0.h, z0.b, z32.b
uaddlt z0.s, z0.h, z0.x
uaddlt z0.h, z0.b, z0.h

uaddwb z32.h, z0.h, z0.b
uaddwb z0.h, z32.h, z0.b
uaddwb z0.h, z0.h, z32.b
uaddwb z0.s, z0.s, z0.x
uaddwb z0.h, z0.h, z0.h

uaddwt z32.h, z0.h, z0.b
uaddwt z0.h, z32.h, z0.b
uaddwt z0.h, z0.h, z32.b
uaddwt z0.s, z0.s, z0.x
uaddwt z0.h, z0.h, z0.h

uhadd z32.b, p0/m, z0.b, z0.b
uhadd z0.b, p0/m, z32.b, z0.b
uhadd z0.b, p0/m, z0.b, z32.b
uhadd z0.b, p0/m, z1.b, z0.b
uhadd z0.b, p8/m, z0.b, z0.b
uhadd z0.h, p0/m, z0.b, z0.b
uhadd z0.b, p0/z, z0.b, z0.b

uhsub z32.b, p0/m, z0.b, z0.b
uhsub z0.b, p0/m, z32.b, z0.b
uhsub z0.b, p0/m, z0.b, z32.b
uhsub z0.b, p0/m, z1.b, z0.b
uhsub z0.b, p8/m, z0.b, z0.b
uhsub z0.h, p0/m, z0.b, z0.b
uhsub z0.b, p0/z, z0.b, z0.b

uhsubr z32.b, p0/m, z0.b, z0.b
uhsubr z0.b, p0/m, z32.b, z0.b
uhsubr z0.b, p0/m, z0.b, z32.b
uhsubr z0.b, p0/m, z1.b, z0.b
uhsubr z0.b, p8/m, z0.b, z0.b
uhsubr z0.h, p0/m, z0.b, z0.b
uhsubr z0.b, p0/z, z0.b, z0.b

umaxp z32.b, p0/m, z0.b, z0.b
umaxp z0.b, p0/m, z32.b, z0.b
umaxp z0.b, p0/m, z0.b, z32.b
umaxp z0.b, p0/m, z1.b, z0.b
umaxp z0.b, p8/m, z0.b, z0.b
umaxp z0.h, p0/m, z0.b, z0.b
umaxp z0.b, p0/z, z0.b, z0.b

uminp z32.b, p0/m, z0.b, z0.b
uminp z0.b, p0/m, z32.b, z0.b
uminp z0.b, p0/m, z0.b, z32.b
uminp z0.b, p0/m, z1.b, z0.b
uminp z0.b, p8/m, z0.b, z0.b
uminp z0.h, p0/m, z0.b, z0.b
uminp z0.b, p0/z, z0.b, z0.b

umlalb z32.s, z0.h, z0.h[0]
umlalb z0.s, z32.h, z0.h[0]
umlalb z0.s, z0.h, z8.h[0]
umlalb z0.s, z0.h, z0.h[8]
umlalb z0.h, z0.h, z0.h[0]

umlalb z32.d, z0.s, z0.s[0]
umlalb z0.d, z32.s, z0.s[0]
umlalb z0.d, z0.s, z16.s[0]
umlalb z0.d, z0.s, z0.s[4]
umlalb z0.s, z0.s, z0.s[0]

umlalb z32.h, z0.b, z0.b
umlalb z0.h, z32.b, z0.b
umlalb z0.h, z0.b, z32.b
umlalb z0.s, z0.h, z0.x
umlalb z0.h, z0.b, z0.h

umlalt z32.s, z0.h, z0.h[0]
umlalt z0.s, z32.h, z0.h[0]
umlalt z0.s, z0.h, z8.h[0]
umlalt z0.s, z0.h, z0.h[8]
umlalt z0.h, z0.h, z0.h[0]

umlalt z32.d, z0.s, z0.s[0]
umlalt z0.d, z32.s, z0.s[0]
umlalt z0.d, z0.s, z16.s[0]
umlalt z0.d, z0.s, z0.s[4]
umlalt z0.s, z0.s, z0.s[0]

umlalt z32.h, z0.b, z0.b
umlalt z0.h, z32.b, z0.b
umlalt z0.h, z0.b, z32.b
umlalt z0.s, z0.h, z0.x
umlalt z0.h, z0.b, z0.h

umlslb z32.s, z0.h, z0.h[0]
umlslb z0.s, z32.h, z0.h[0]
umlslb z0.s, z0.h, z8.h[0]
umlslb z0.s, z0.h, z0.h[8]
umlslb z0.h, z0.h, z0.h[0]

umlslb z32.d, z0.s, z0.s[0]
umlslb z0.d, z32.s, z0.s[0]
umlslb z0.d, z0.s, z16.s[0]
umlslb z0.d, z0.s, z0.s[4]
umlslb z0.s, z0.s, z0.s[0]

umlslb z32.h, z0.b, z0.b
umlslb z0.h, z32.b, z0.b
umlslb z0.h, z0.b, z32.b
umlslb z0.s, z0.h, z0.x
umlslb z0.h, z0.b, z0.h

umlslt z32.s, z0.h, z0.h[0]
umlslt z0.s, z32.h, z0.h[0]
umlslt z0.s, z0.h, z8.h[0]
umlslt z0.s, z0.h, z0.h[8]
umlslt z0.h, z0.h, z0.h[0]

umlslt z32.d, z0.s, z0.s[0]
umlslt z0.d, z32.s, z0.s[0]
umlslt z0.d, z0.s, z16.s[0]
umlslt z0.d, z0.s, z0.s[4]
umlslt z0.s, z0.s, z0.s[0]

umlslt z32.h, z0.b, z0.b
umlslt z0.h, z32.b, z0.b
umlslt z0.h, z0.b, z32.b
umlslt z0.s, z0.h, z0.x
umlslt z0.h, z0.b, z0.h

umulh z32.h, z0.b, z0.b
umulh z0.h, z32.b, z0.b
umulh z0.h, z0.b, z32.b
umulh z0.s, z0.h, z0.x
umulh z0.h, z0.b, z0.h

umullb z32.s, z0.h, z0.h[0]
umullb z0.s, z32.h, z0.h[0]
umullb z0.s, z0.h, z8.h[0]
umullb z0.s, z0.h, z0.h[8]
umullb z0.h, z0.h, z0.h[0]

umullb z32.d, z0.s, z0.s[0]
umullb z0.d, z32.s, z0.s[0]
umullb z0.d, z0.s, z16.s[0]
umullb z0.d, z0.s, z0.s[4]
umullb z0.s, z0.s, z0.s[0]

umullb z32.h, z0.b, z0.b
umullb z0.h, z32.b, z0.b
umullb z0.h, z0.b, z32.b
umullb z0.s, z0.h, z0.x
umullb z0.h, z0.b, z0.h

umullt z32.s, z0.h, z0.h[0]
umullt z0.s, z32.h, z0.h[0]
umullt z0.s, z0.h, z8.h[0]
umullt z0.s, z0.h, z0.h[8]
umullt z0.h, z0.h, z0.h[0]

umullt z32.d, z0.s, z0.s[0]
umullt z0.d, z32.s, z0.s[0]
umullt z0.d, z0.s, z16.s[0]
umullt z0.d, z0.s, z0.s[4]
umullt z0.s, z0.s, z0.s[0]

umullt z32.h, z0.b, z0.b
umullt z0.h, z32.b, z0.b
umullt z0.h, z0.b, z32.b
umullt z0.s, z0.h, z0.x
umullt z0.h, z0.b, z0.h

uqadd z32.b, p0/m, z0.b, z0.b
uqadd z0.b, p0/m, z32.b, z0.b
uqadd z0.b, p0/m, z0.b, z32.b
uqadd z0.b, p0/m, z1.b, z0.b
uqadd z0.b, p8/m, z0.b, z0.b
uqadd z0.h, p0/m, z0.b, z0.b
uqadd z0.b, p0/z, z0.b, z0.b

uqrshl z32.b, p0/m, z0.b, z0.b
uqrshl z0.b, p0/m, z32.b, z0.b
uqrshl z0.b, p0/m, z0.b, z32.b
uqrshl z0.b, p0/m, z1.b, z0.b
uqrshl z0.b, p8/m, z0.b, z0.b
uqrshl z0.h, p0/m, z0.b, z0.b
uqrshl z0.b, p0/z, z0.b, z0.b

uqrshlr z32.b, p0/m, z0.b, z0.b
uqrshlr z0.b, p0/m, z32.b, z0.b
uqrshlr z0.b, p0/m, z0.b, z32.b
uqrshlr z0.b, p0/m, z1.b, z0.b
uqrshlr z0.b, p8/m, z0.b, z0.b
uqrshlr z0.h, p0/m, z0.b, z0.b
uqrshlr z0.b, p0/z, z0.b, z0.b

uqrshrnb z32.b, z0.h, #8
uqrshrnb z0.b, z32.h, #8
uqrshrnb z0.b, z0.h, #9
uqrshrnb z0.b, z0.h, #0
uqrshrnb z0.h, z0.h, #8
uqrshrnb z0.h, z0.s, #0
uqrshrnb z0.h, z0.s, #17
uqrshrnb z0.s, z0.d, #0
uqrshrnb z0.s, z0.d, #33

movprfx z0, z1
uqrshrnt z0.b, z0.h, #1

uqrshrnt z32.b, z0.h, #8
uqrshrnt z0.b, z32.h, #8
uqrshrnt z0.b, z0.h, #9
uqrshrnt z0.b, z0.h, #0
uqrshrnt z0.h, z0.h, #8
uqrshrnt z0.h, z0.s, #0
uqrshrnt z0.h, z0.s, #17
uqrshrnt z0.s, z0.d, #0
uqrshrnt z0.s, z0.d, #33

uqshl z0.h, p0/m, z0.b, #0
uqshl z32.b, p0/m, z32.b, #0
uqshl z0.b, p0/m, z1.b, #0
uqshl z0.b, p8/m, z0.b, #0
uqshl z0.b, p0/m, z0.b, #8
uqshl z0.h, p0/m, z0.h, #16
uqshl z0.s, p0/m, z0.s, #32
uqshl z0.d, p0/m, z0.d, #64

uqshl z32.b, p0/m, z0.b, z0.b
uqshl z0.b, p0/m, z32.b, z0.b
uqshl z0.b, p0/m, z0.b, z32.b
uqshl z0.b, p0/m, z1.b, z0.b
uqshl z0.b, p8/m, z0.b, z0.b
uqshl z0.h, p0/m, z0.b, z0.b
uqshl z0.b, p0/z, z0.b, z0.b

uqshlr z32.b, p0/m, z0.b, z0.b
uqshlr z0.b, p0/m, z32.b, z0.b
uqshlr z0.b, p0/m, z0.b, z32.b
uqshlr z0.b, p0/m, z1.b, z0.b
uqshlr z0.b, p8/m, z0.b, z0.b
uqshlr z0.h, p0/m, z0.b, z0.b
uqshlr z0.b, p0/z, z0.b, z0.b

uqshrnb z32.b, z0.h, #8
uqshrnb z0.b, z32.h, #8
uqshrnb z0.b, z0.h, #9
uqshrnb z0.b, z0.h, #0
uqshrnb z0.h, z0.h, #8
uqshrnb z0.h, z0.s, #0
uqshrnb z0.h, z0.s, #17
uqshrnb z0.s, z0.d, #0
uqshrnb z0.s, z0.d, #33

movprfx z0, z1
uqshrnt z0.b, z0.h, #1

uqshrnt z32.b, z0.h, #8
uqshrnt z0.b, z32.h, #8
uqshrnt z0.b, z0.h, #9
uqshrnt z0.b, z0.h, #0
uqshrnt z0.h, z0.h, #8
uqshrnt z0.h, z0.s, #0
uqshrnt z0.h, z0.s, #17
uqshrnt z0.s, z0.d, #0
uqshrnt z0.s, z0.d, #33

uqsub z32.b, p0/m, z0.b, z0.b
uqsub z0.b, p0/m, z32.b, z0.b
uqsub z0.b, p0/m, z0.b, z32.b
uqsub z0.b, p0/m, z1.b, z0.b
uqsub z0.b, p8/m, z0.b, z0.b
uqsub z0.h, p0/m, z0.b, z0.b
uqsub z0.b, p0/z, z0.b, z0.b

uqsubr z32.b, p0/m, z0.b, z0.b
uqsubr z0.b, p0/m, z32.b, z0.b
uqsubr z0.b, p0/m, z0.b, z32.b
uqsubr z0.b, p0/m, z1.b, z0.b
uqsubr z0.b, p8/m, z0.b, z0.b
uqsubr z0.h, p0/m, z0.b, z0.b
uqsubr z0.b, p0/z, z0.b, z0.b

uqxtnb z32.b, z0.h
uqxtnb z0.b, z32.h
uqxtnb z0.b, z0.s

uqxtnt z32.b, z0.h
uqxtnt z0.b, z32.h
uqxtnt z0.b, z0.s

urecpe z32.s, p0/m, z0.s
urecpe z0.s, p0/m, z32.s
urecpe z0.s, p8/m, z0.s
urecpe z0.d, p0/m, z0.s

urhadd z32.b, p0/m, z0.b, z0.b
urhadd z0.b, p0/m, z32.b, z0.b
urhadd z0.b, p0/m, z0.b, z32.b
urhadd z0.b, p0/m, z1.b, z0.b
urhadd z0.b, p8/m, z0.b, z0.b
urhadd z0.h, p0/m, z0.b, z0.b
urhadd z0.b, p0/z, z0.b, z0.b

urshl z32.b, p0/m, z0.b, z0.b
urshl z0.b, p0/m, z32.b, z0.b
urshl z0.b, p0/m, z0.b, z32.b
urshl z0.b, p0/m, z1.b, z0.b
urshl z0.b, p8/m, z0.b, z0.b
urshl z0.h, p0/m, z0.b, z0.b
urshl z0.b, p0/z, z0.b, z0.b

urshlr z32.b, p0/m, z0.b, z0.b
urshlr z0.b, p0/m, z32.b, z0.b
urshlr z0.b, p0/m, z0.b, z32.b
urshlr z0.b, p0/m, z1.b, z0.b
urshlr z0.b, p8/m, z0.b, z0.b
urshlr z0.h, p0/m, z0.b, z0.b
urshlr z0.b, p0/z, z0.b, z0.b

urshr z0.h, p0/m, z0.b, #1
urshr z32.b, p0/m, z32.b, #1
urshr z0.b, p0/m, z1.b, #1
urshr z0.b, p8/m, z0.b, #1
urshr z0.b, p0/m, z0.b, #0
urshr z0.b, p0/m, z0.b, #9
urshr z0.h, p0/m, z0.h, #0
urshr z0.h, p0/m, z0.h, #17
urshr z0.s, p0/m, z0.s, #0
urshr z0.s, p0/m, z0.s, #33
urshr z0.d, p0/m, z0.d, #0
urshr z0.d, p0/m, z0.d, #65

ursqrte z32.s, p0/m, z0.s
ursqrte z0.s, p0/m, z32.s
ursqrte z0.s, p8/m, z0.s
ursqrte z0.d, p0/m, z0.s

ursra z0.h, z0.b, #1
ursra z32.b, z0.b, #1
ursra z0.b, z32.b, #1
ursra z0.b, z0.b, #0
ursra z0.b, z0.b, #9
ursra z0.h, z0.h, #0
ursra z0.h, z0.h, #17
ursra z0.s, z0.s, #0
ursra z0.s, z0.s, #33
ursra z0.d, z0.d, #0
ursra z0.d, z0.d, #64

ushllb z0.b, z0.b, #0
ushllb z32.h, z0.b, #0
ushllb z0.h, z32.b, #0
ushllb z0.h, z0.b, #8
ushllb z0.s, z0.h, #16
ushllb z0.d, z0.s, #32

ushllt z0.b, z0.b, #0
ushllt z32.h, z0.b, #0
ushllt z0.h, z32.b, #0
ushllt z0.h, z0.b, #8
ushllt z0.s, z0.h, #16
ushllt z0.d, z0.s, #32

usqadd z32.b, p0/m, z0.b, z0.b
usqadd z0.b, p0/m, z32.b, z0.b
usqadd z0.b, p0/m, z0.b, z32.b
usqadd z0.b, p0/m, z1.b, z0.b
usqadd z0.b, p8/m, z0.b, z0.b
usqadd z0.h, p0/m, z0.b, z0.b
usqadd z0.b, p0/z, z0.b, z0.b

usra z0.h, z0.b, #1
usra z32.b, z0.b, #1
usra z0.b, z32.b, #1
usra z0.b, z0.b, #0
usra z0.b, z0.b, #9
usra z0.h, z0.h, #0
usra z0.h, z0.h, #17
usra z0.s, z0.s, #0
usra z0.s, z0.s, #33
usra z0.d, z0.d, #0
usra z0.d, z0.d, #64

usublb z32.h, z0.b, z0.b
usublb z0.h, z32.b, z0.b
usublb z0.h, z0.b, z32.b
usublb z0.s, z0.h, z0.x
usublb z0.h, z0.b, z0.h

usublt z32.h, z0.b, z0.b
usublt z0.h, z32.b, z0.b
usublt z0.h, z0.b, z32.b
usublt z0.s, z0.h, z0.x
usublt z0.h, z0.b, z0.h

usubwb z32.h, z0.h, z0.b
usubwb z0.h, z32.h, z0.b
usubwb z0.h, z0.h, z32.b
usubwb z0.s, z0.s, z0.x
usubwb z0.h, z0.h, z0.h

usubwt z32.h, z0.h, z0.b
usubwt z0.h, z32.h, z0.b
usubwt z0.h, z0.h, z32.b
usubwt z0.s, z0.s, z0.x
usubwt z0.h, z0.h, z0.h

whilege p16.b, x0, x0
whilege p0.b, x32, x0
whilege p0.b, x0, x32
whilege p0/m, x0, x0
whilege p0.b, x31, x0
whilege p0.b, x0, x31

whilege p0.b, x0, w0
whilege p0.b, w0, x0

whilege p16.b, w0, w0
whilege p0.b, w32, w0
whilege p0.b, w0, w32
whilege p0/m, w0, w0
whilege p0.b, w31, w0
whilege p0.b, w0, w31

whilegt p16.b, x0, x0
whilegt p0.b, x32, x0
whilegt p0.b, x0, x32
whilegt p0/m, x0, x0
whilegt p0.b, x31, x0
whilegt p0.b, x0, x31

whilegt p0.b, x0, w0
whilegt p0.b, w0, x0

whilegt p16.b, w0, w0
whilegt p0.b, w32, w0
whilegt p0.b, w0, w32
whilegt p0/m, w0, w0
whilegt p0.b, w31, w0
whilegt p0.b, w0, w31

whilehi p16.b, x0, x0
whilehi p0.b, x32, x0
whilehi p0.b, x0, x32
whilehi p0/m, x0, x0
whilehi p0.b, x31, x0
whilehi p0.b, x0, x31

whilehi p0.b, x0, w0
whilehi p0.b, w0, x0

whilehi p16.b, w0, w0
whilehi p0.b, w32, w0
whilehi p0.b, w0, w32
whilehi p0/m, w0, w0
whilehi p0.b, w31, w0
whilehi p0.b, w0, w31

whilehs p16.b, x0, x0
whilehs p0.b, x32, x0
whilehs p0.b, x0, x32
whilehs p0/m, x0, x0
whilehs p0.b, x31, x0
whilehs p0.b, x0, x31

whilehs p0.b, x0, w0
whilehs p0.b, w0, x0

whilehs p16.b, w0, w0
whilehs p0.b, w32, w0
whilehs p0.b, w0, w32
whilehs p0/m, w0, w0
whilehs p0.b, w31, w0
whilehs p0.b, w0, w31

whilerw p0.b, w0, x0
whilerw p0/m, x0, x0
whilerw p0.b, x32, x0
whilerw p16.b, x0, x0

whilewr p0.b, w0, x0
whilewr p0/m, x0, x0
whilewr p0.b, x32, x0
whilewr p16.b, x0, x0

xar z0.h, z0.b, z0.b, #1
xar z0.b, z1.b, z0.b, #1
xar z32.b, z32.b, z0.b, #1
xar z0.b, z0.b, z32.b, #1
xar z0.b, z0.b, z0.b, #0
xar z0.b, z0.b, z0.b, #9
xar z0.h, z0.h, z0.h, #0
xar z0.h, z0.h, z0.h, #17
xar z0.s, z0.s, z0.s, #0
xar z0.s, z0.s, z0.s, #33
xar z0.d, z0.d, z0.d, #0
xar z0.d, z0.d, z0.d, #64

.equ z0.h, 1
sqshl z1.s, p0/m, z1.s, z0.h
