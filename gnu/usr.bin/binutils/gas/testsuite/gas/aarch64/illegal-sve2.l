[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: operand mismatch -- `movprfx z0\.d,z1\.d'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	movprfx z0, z1
[^ :]+:[0-9]+: Warning: predicated instruction expected after `movprfx' -- `adclb z0\.d,z1\.d,z2\.d'
[^ :]+:[0-9]+: Error: operand mismatch -- `adclb z0\.d,z0\.s,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	adclb z0\.s, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	adclb z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `adclb z32\.d,z0\.d,z0\.d'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `adclb z0\.d,z32\.d,z0\.d'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `adclb z0\.d,z0\.d,z32\.d'
[^ :]+:[0-9]+: Error: operand mismatch -- `adclt z0\.d,z0\.s,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	adclt z0\.s, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	adclt z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `adclt z32\.s,z0\.s,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `adclt z0\.s,z32\.s,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `adclt z0\.s,z0\.s,z32\.s'
[^ :]+:[0-9]+: Error: operand mismatch -- `addhnb z0\.b,z0\.h,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	addhnb z0\.b, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	addhnb z0\.h, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	addhnb z0\.s, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `addhnb z32\.b,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `addhnb z0\.b,z32\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `addhnb z0\.b,z0\.h,z32\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `addhnt z0\.b,z0\.h,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	addhnt z0\.b, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	addhnt z0\.h, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	addhnt z0\.s, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `addhnt z32\.b,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `addhnt z0\.b,z32\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `addhnt z0\.b,z0\.h,z32\.h'
[^ :]+:[0-9]+: Warning: register size not compatible with previous `movprfx' at operand 1 -- `addp z0\.b,p0/m,z0\.b,z1\.b'
[^ :]+:[0-9]+: Warning: predicate register differs from that in preceding `movprfx' at operand 2 -- `addp z0\.d,p1/m,z0\.d,z1\.d'
[^ :]+:[0-9]+: Error: operand mismatch -- `addp z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	addp z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	addp z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	addp z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	addp z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `addp z0\.h,p0/m,z1\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `addp z32\.s,p0/m,z32\.s,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `addp z0\.s,p0/m,z0\.s,z32\.s'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `addp z0\.s,p8/m,z0\.s,z0\.s'
[^ :]+:[0-9]+: Warning: SVE `movprfx' compatible instruction expected -- `aesd z0\.b,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `aesd z0\.b,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `aesd z0\.b,z0\.s,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	aesd z0\.b, z0\.b, z0\.b
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `aesd z32\.b,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `aesd z0\.b,z0\.b,z32\.b'
[^ :]+:[0-9]+: Warning: SVE `movprfx' compatible instruction expected -- `aese z0\.b,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `aese z0\.b,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `aese z0\.b,z0\.s,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	aese z0\.b, z0\.b, z0\.b
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `aese z32\.b,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `aese z0\.b,z0\.b,z32\.b'
[^ :]+:[0-9]+: Warning: SVE `movprfx' compatible instruction expected -- `aesimc z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `aesimc z0\.b,z1\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `aesimc z0\.b,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	aesimc z0\.b, z0\.b
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `aesimc z32\.b,z0\.b'
[^ :]+:[0-9]+: Warning: SVE `movprfx' compatible instruction expected -- `aesmc z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `aesmc z0\.b,z1\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `aesmc z0\.b,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	aesmc z0\.b, z0\.b
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `aesmc z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `bcax z0\.d,z1\.d,z0\.d,z0\.d'
[^ :]+:[0-9]+: Error: operand mismatch -- `bcax z0\.d,z0\.d,z0\.h,z0\.d'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bcax z0\.d, z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `bcax z0\.d,z0\.h,z0\.d,z0\.d'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bcax z0\.d, z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `bcax z32\.d,z32\.d,z0\.d,z0\.d'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `bcax z0\.d,z0\.d,z32\.d,z0\.d'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `bcax z0\.d,z0\.d,z0\.d,z32\.d'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `bsl z0\.d,z1\.d,z0\.d,z0\.d'
[^ :]+:[0-9]+: Error: operand mismatch -- `bsl z0\.d,z0\.d,z0\.h,z0\.d'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bsl z0\.d, z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `bsl z0\.d,z0\.h,z0\.d,z0\.d'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bsl z0\.d, z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `bsl z32\.d,z32\.d,z0\.d,z0\.d'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `bsl z0\.d,z0\.d,z32\.d,z0\.d'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `bsl z0\.d,z0\.d,z0\.d,z32\.d'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `bsl1n z0\.d,z1\.d,z0\.d,z0\.d'
[^ :]+:[0-9]+: Error: operand mismatch -- `bsl1n z0\.d,z0\.d,z0\.h,z0\.d'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bsl1n z0\.d, z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `bsl1n z0\.d,z0\.h,z0\.d,z0\.d'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bsl1n z0\.d, z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `bsl1n z32\.d,z32\.d,z0\.d,z0\.d'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `bsl1n z0\.d,z0\.d,z32\.d,z0\.d'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `bsl1n z0\.d,z0\.d,z0\.d,z32\.d'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `bsl2n z0\.d,z1\.d,z0\.d,z0\.d'
[^ :]+:[0-9]+: Error: operand mismatch -- `bsl2n z0\.d,z0\.d,z0\.h,z0\.d'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bsl2n z0\.d, z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `bsl2n z0\.d,z0\.h,z0\.d,z0\.d'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bsl2n z0\.d, z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `bsl2n z32\.d,z32\.d,z0\.d,z0\.d'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `bsl2n z0\.d,z0\.d,z32\.d,z0\.d'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `bsl2n z0\.d,z0\.d,z0\.d,z32\.d'
[^ :]+:[0-9]+: Error: operand mismatch -- `bdep z0\.b,z0\.h,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bdep z0\.b, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	bdep z0\.h, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	bdep z0\.s, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	bdep z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `bdep z32\.h,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `bdep z0\.s,z32\.s,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `bdep z0\.d,z0\.d,z32\.d'
[^ :]+:[0-9]+: Error: operand mismatch -- `bext z0\.b,z0\.h,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bext z0\.b, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	bext z0\.h, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	bext z0\.s, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	bext z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `bext z32\.h,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `bext z0\.s,z32\.s,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `bext z0\.d,z0\.d,z32\.d'
[^ :]+:[0-9]+: Error: operand mismatch -- `bgrp z0\.b,z0\.h,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bgrp z0\.b, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	bgrp z0\.h, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	bgrp z0\.s, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	bgrp z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `bgrp z32\.h,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `bgrp z0\.s,z32\.s,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `bgrp z0\.d,z0\.d,z32\.d'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `cadd z18\.b,z17\.b,z21\.b,#90'
[^ :]+:[0-9]+: Error: rotate expected to be 90 or 270 at operand 4 -- `cadd z0\.b,z0\.b,z0\.b,#91'
[^ :]+:[0-9]+: Error: operand mismatch -- `cadd z0\.b,z0\.h,z0\.h,#90'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	cadd z0\.h, z0\.h, z0\.h, #90
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	cadd z0\.b, z0\.b, z0\.b, #90
[^ :]+:[0-9]+: Info:    	cadd z0\.s, z0\.s, z0\.s, #90
[^ :]+:[0-9]+: Info:    	cadd z0\.d, z0\.d, z0\.d, #90
[^ :]+:[0-9]+: Error: rotate expected to be 0, 90, 180 or 270 at operand 4 -- `cdot z0\.s,z0\.b,z0\.b\[0\],#1'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `cdot z0\.s,z0\.b,z0\.b\[4\],#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `cdot z0\.s,z0\.d,z0\.b\[0\],#0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	cdot z0\.s, z0\.b, z0\.b\[0\], #0
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `cdot z32\.s,z0\.b,z0\.b\[0\],#0'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `cdot z0\.s,z32\.b,z0\.b\[0\],#0'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `cdot z0\.s,z0\.b,z8\.b\[0\],#0'
[^ :]+:[0-9]+: Error: rotate expected to be 0, 90, 180 or 270 at operand 4 -- `cdot z0\.d,z0\.h,z0\.h\[0\],#1'
[^ :]+:[0-9]+: Error: operand mismatch -- `cdot z0\.d,z0\.d,z0\.h\[0\],#0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	cdot z0\.d, z0\.h, z0\.h\[0\], #0
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `cdot z32\.d,z0\.h,z0\.h\[0\],#0'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `cdot z0\.d,z32\.h,z0\.h\[0\],#0'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `cdot z0\.d,z0\.h,z16\.h\[0\],#0'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `cdot z32\.s,z0\.b,z0\.b,#0'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `cdot z0\.s,z32\.b,z0\.b,#0'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `cdot z0\.s,z0\.b,z32\.b,#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `cdot z0\.s,z0\.b,z0\.s,#0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	cdot z0\.s, z0\.b, z0\.b, #0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	cdot z0\.d, z0\.h, z0\.h, #0
[^ :]+:[0-9]+: Error: rotate expected to be 0, 90, 180 or 270 at operand 4 -- `cdot z0\.s,z0\.b,z0\.b,#1'
[^ :]+:[0-9]+: Error: operand mismatch -- `cdot z0\.d,z0\.h,z0\.b,#0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	cdot z0\.d, z0\.h, z0\.h, #0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	cdot z0\.s, z0\.b, z0\.b, #0
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `cmla z32\.h,z0\.h,z0\.h\[0\],#0'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `cmla z0\.h,z32\.h,z0\.h\[0\],#0'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `cmla z0\.h,z0\.h,z8\.h\[0\],#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `cmla z0\.h,z0\.h,z0\.d\[0\],#0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	cmla z0\.h, z0\.h, z0\.h\[0\], #0
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `cmla z0\.h,z0\.h,z0\.h\[4\],#0'
[^ :]+:[0-9]+: Error: rotate expected to be 0, 90, 180 or 270 at operand 4 -- `cmla z0\.h,z0\.h,z0\.h\[0\],#1'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `cmla z32\.s,z0\.s,z0\.s\[0\],#0'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `cmla z0\.s,z32\.s,z0\.s\[0\],#0'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `cmla z0\.s,z0\.s,z16\.s\[0\],#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `cmla z0\.s,z0\.s,z0\.d\[0\],#0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	cmla z0\.s, z0\.s, z0\.s\[0\], #0
[^ :]+:[0-9]+: Error: register element index out of range 0 to 1 at operand 3 -- `cmla z0\.s,z0\.s,z0\.s\[2\],#0'
[^ :]+:[0-9]+: Error: rotate expected to be 0, 90, 180 or 270 at operand 4 -- `cmla z0\.s,z0\.s,z0\.s\[0\],#1'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `cmla z32\.b,z0\.b,z0\.b,#0'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `cmla z0\.b,z32\.b,z0\.b,#0'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `cmla z0\.b,z0\.b,z32\.b,#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `cmla z0\.b,z0\.b,z0\.h,#0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	cmla z0\.b, z0\.b, z0\.b, #0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	cmla z0\.h, z0\.h, z0\.h, #0
[^ :]+:[0-9]+: Info:    	cmla z0\.s, z0\.s, z0\.s, #0
[^ :]+:[0-9]+: Info:    	cmla z0\.d, z0\.d, z0\.d, #0
[^ :]+:[0-9]+: Error: rotate expected to be 0, 90, 180 or 270 at operand 4 -- `cmla z0\.b,z0\.b,z0\.b,#1'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `eor3 z0\.d,z1\.d,z0\.d,z0\.d'
[^ :]+:[0-9]+: Error: operand mismatch -- `eor3 z0\.d,z0\.d,z0\.h,z0\.d'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	eor3 z0\.d, z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `eor3 z0\.d,z0\.h,z0\.d,z0\.d'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	eor3 z0\.d, z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `eorbt z0\.b,z0\.h,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	eorbt z0\.b, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	eorbt z0\.h, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	eorbt z0\.s, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	eorbt z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `eorbt z32\.h,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `eorbt z0\.s,z32\.s,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `eorbt z0\.s,z0\.s,z32\.s'
[^ :]+:[0-9]+: Error: operand mismatch -- `eortb z0\.b,z0\.h,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	eortb z0\.b, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	eortb z0\.h, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	eortb z0\.s, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	eortb z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `eortb z32\.h,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `eortb z0\.s,z32\.s,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `eortb z0\.s,z0\.s,z32\.s'
[^ :]+:[0-9]+: Error: syntax error in register list at operand 2 -- `ext z0\.b,{,},#0'
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 at operand 2 -- `ext z0\.b,{z0\.b,z2\.b},#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `ext z0\.h,{z0\.b,z1\.b},#0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ext z0\.b, {z0\.b-z1\.b}, #0
[^ :]+:[0-9]+: Error: type mismatch in vector register list at operand 2 -- `ext z0\.b,{z0\.h,z1\.b},#0'
[^ :]+:[0-9]+: Error: type mismatch in vector register list at operand 2 -- `ext z0\.b,{z0\.b,z1\.h},#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `ext z0\.b,{z0\.h,z1\.h},#0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ext z0\.b, {z0\.b-z1\.b}, #0
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 2 -- `ext z0\.b,{z0\.b,z1\.b,z2\.b},#0'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 2 -- `ext z0\.b,{z0\.b},#0'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `ext z0\.b,z0\.b,#0'
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 at operand 2 -- `ext z0\.b,{z31\.b,z1\.b},#0'
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 at operand 2 -- `ext z0\.b,{z0\.b,z31\.b},#0'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 255 at operand 3 -- `ext z0\.b,{z0\.b,z1\.b},#256'
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `ext z32\.b,{z0\.b,z1\.b},#0'
[^ :]+:[0-9]+: Error: operand 2 must be a list of SVE vector registers -- `ext z0\.b,{z31\.b,z32\.b},#0'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `ext z0\.b,{z32\.b,z33\.b},#0'
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `faddp z32\.h,p0/m,z32\.h,z0\.h'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `faddp z0\.h,p8/m,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `faddp z0\.h,p0/m,z0\.h,z32\.h'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `faddp z0\.h,p0/m,z1\.h,z0\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `faddp z0\.h,p0/z,z0\.h,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	faddp z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	faddp z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	faddp z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `faddp z0\.h,p0/m,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	faddp z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	faddp z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	faddp z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Warning: SVE `movprfx' compatible instruction expected -- `fcvtlt z0\.s,p0/m,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `fcvtlt z32\.s,p0/m,z0\.h'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `fcvtlt z0\.s,p8/m,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `fcvtlt z0\.s,p0/m,z32\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `fcvtlt z0\.s,p0/m,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fcvtlt z0\.d, p0/m, z0\.s
[^ :]+:[0-9]+: Error: operand mismatch -- `fcvtlt z0\.s,p0/z,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fcvtlt z0\.s, p0/m, z0\.h
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `fcvtlt z32\.d,p0/m,z0\.s'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `fcvtlt z0\.d,p8/m,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `fcvtlt z0\.d,p0/m,z32\.s'
[^ :]+:[0-9]+: Error: operand mismatch -- `fcvtlt z0\.d,p0/m,z0\.d'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fcvtlt z0\.d, p0/m, z0\.s
[^ :]+:[0-9]+: Error: operand mismatch -- `fcvtlt z0\.d,p0/z,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fcvtlt z0\.d, p0/m, z0\.s
[^ :]+:[0-9]+: Warning: SVE `movprfx' compatible instruction expected -- `fcvtnt z0\.h,p0/m,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `fcvtnt z32\.h,p0/m,z0\.s'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `fcvtnt z0\.h,p8/m,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `fcvtnt z0\.h,p0/m,z32\.s'
[^ :]+:[0-9]+: Error: operand mismatch -- `fcvtnt z0\.h,p0/m,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fcvtnt z0\.h, p0/m, z0\.s
[^ :]+:[0-9]+: Error: operand mismatch -- `fcvtnt z0\.h,p0/z,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fcvtnt z0\.h, p0/m, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `fcvtnt z32\.s,p0/m,z0\.d'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `fcvtnt z0\.s,p8/m,z0\.d'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `fcvtnt z0\.s,p0/m,z32\.d'
[^ :]+:[0-9]+: Error: operand mismatch -- `fcvtnt z0\.s,p0/m,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fcvtnt z0\.s, p0/m, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `fcvtnt z0\.s,p0/z,z0\.d'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fcvtnt z0\.s, p0/m, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `fcvtx z32\.s,p0/m,z0\.d'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `fcvtx z0\.s,p8/m,z0\.d'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `fcvtx z0\.s,p0/m,z32\.d'
[^ :]+:[0-9]+: Error: operand mismatch -- `fcvtx z0\.s,p0/m,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fcvtx z0\.s, p0/m, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `fcvtx z0\.s,p0/z,z0\.d'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fcvtx z0\.s, p0/m, z0\.d
[^ :]+:[0-9]+: Warning: register size not compatible with previous `movprfx' at operand 1 -- `fcvtx z0\.s,p0/m,z2\.d'
[^ :]+:[0-9]+: Warning: SVE `movprfx' compatible instruction expected -- `fcvtxnt z0\.s,p0/m,z0\.d'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `fcvtxnt z32\.s,p0/m,z0\.d'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `fcvtxnt z0\.s,p8/m,z0\.d'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `fcvtxnt z0\.s,p0/m,z32\.d'
[^ :]+:[0-9]+: Error: operand mismatch -- `fcvtxnt z0\.s,p0/m,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fcvtxnt z0\.s, p0/m, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `fcvtxnt z0\.s,p0/z,z0\.d'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fcvtxnt z0\.s, p0/m, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `flogb z0\.b,p0/m,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	flogb z0\.h, p0/m, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	flogb z0\.s, p0/m, z0\.s
[^ :]+:[0-9]+: Info:    	flogb z0\.d, p0/m, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `flogb z0\.b,p0/m,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	flogb z0\.h, p0/m, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	flogb z0\.s, p0/m, z0\.s
[^ :]+:[0-9]+: Info:    	flogb z0\.d, p0/m, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `flogb z0\.h,p0/z,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	flogb z0\.h, p0/m, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	flogb z0\.s, p0/m, z0\.s
[^ :]+:[0-9]+: Info:    	flogb z0\.d, p0/m, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `flogb z32\.h,p0/m,z0\.h'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `flogb z0\.h,p8/m,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `flogb z0\.h,p0/m,z32\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `fmaxnmp z0\.b,p0/m,z0\.h,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fmaxnmp z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fmaxnmp z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	fmaxnmp z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `fmaxnmp z0\.h,p0/z,z0\.h,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fmaxnmp z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fmaxnmp z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	fmaxnmp z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `fmaxnmp z1\.h,p0/m,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `fmaxnmp z32\.h,p0/m,z32\.h,z0\.h'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `fmaxnmp z0\.h,p8/m,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `fmaxnmp z0\.h,p0/m,z0\.h,z32\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `fmaxp z0\.b,p0/m,z0\.h,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fmaxp z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fmaxp z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	fmaxp z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `fmaxp z0\.h,p0/z,z0\.h,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fmaxp z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fmaxp z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	fmaxp z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `fmaxp z1\.h,p0/m,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `fmaxp z32\.h,p0/m,z32\.h,z0\.h'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `fmaxp z0\.h,p8/m,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `fmaxp z0\.h,p0/m,z0\.h,z32\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `fminnmp z0\.b,p0/m,z0\.h,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fminnmp z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fminnmp z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	fminnmp z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `fminnmp z0\.h,p0/z,z0\.h,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fminnmp z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fminnmp z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	fminnmp z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `fminnmp z1\.h,p0/m,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `fminnmp z32\.h,p0/m,z32\.h,z0\.h'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `fminnmp z0\.h,p8/m,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `fminnmp z0\.h,p0/m,z0\.h,z32\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `fminp z0\.b,p0/m,z0\.h,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fminp z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fminp z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	fminp z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `fminp z0\.h,p0/z,z0\.h,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fminp z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fminp z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	fminp z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `fminp z1\.h,p0/m,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `fminp z32\.h,p0/m,z32\.h,z0\.h'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `fminp z0\.h,p8/m,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `fminp z0\.h,p0/m,z0\.h,z32\.h'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `fmlalb z0\.s,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `fmlalb z0\.s,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `fmlalb z0\.s,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `fmlalb z32\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `fmlalb z0\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fmlalb z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `fmlalb z32\.s,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `fmlalb z0\.s,z32\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `fmlalb z0\.s,z0\.h,z32\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `fmlalb z0\.s,z0\.h,z0\.d'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fmlalb z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `fmlalt z0\.s,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `fmlalt z0\.s,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `fmlalt z0\.s,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `fmlalt z32\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `fmlalt z0\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fmlalt z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `fmlalt z32\.s,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `fmlalt z0\.s,z32\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `fmlalt z0\.s,z0\.h,z32\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `fmlalt z0\.s,z0\.h,z0\.d'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fmlalt z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `fmlslb z0\.s,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `fmlslb z0\.s,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `fmlslb z0\.s,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `fmlslb z32\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `fmlslb z0\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fmlslb z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `fmlslb z32\.s,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `fmlslb z0\.s,z32\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `fmlslb z0\.s,z0\.h,z32\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `fmlslb z0\.s,z0\.h,z0\.d'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fmlslb z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `fmlslt z0\.s,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `fmlslt z0\.s,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `fmlslt z0\.s,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `fmlslt z32\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `fmlslt z0\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fmlslt z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `fmlslt z32\.s,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `fmlslt z0\.s,z32\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `fmlslt z0\.s,z0\.h,z32\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `fmlslt z0\.s,z0\.h,z0\.d'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fmlslt z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `histcnt z32\.s,p0/z,z0\.s,z0\.s'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `histcnt z0\.s,p8/z,z0\.s,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `histcnt z0\.s,p0/z,z32\.s,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `histcnt z0\.s,p0/z,z0\.s,z32\.s'
[^ :]+:[0-9]+: Error: operand mismatch -- `histcnt z0\.s,p0/m,z0\.s,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	histcnt z0\.s, p0/z, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	histcnt z0\.d, p0/z, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `histcnt z0\.d,p0/z,z0\.s,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	histcnt z0\.s, p0/z, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	histcnt z0\.d, p0/z, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `histseg z32\.b,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `histseg z0\.b,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `histseg z0\.b,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `histseg z0\.b,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	histseg z0\.b, z0\.b, z0\.b
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ldnt1b {z0\.d,z1\.d},p0/z,\[z0\.d,x0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ldnt1b {z0\.d},p0/m,\[z0\.d\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ldnt1b {z0\.d}, p0/z, \[z0\.d, xzr\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `ldnt1b {z32\.d},p0/z,\[z0\.d\]'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `ldnt1b {z0\.d},p8/z,\[z0\.d\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ldnt1b {z0\.d},p0/z,\[z32\.d\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ldnt1b {z0\.d},p0/z,\[z0\.d,sp\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ldnt1b {z0\.d},p0/z,\[z0\.d,x32\]'
[^ :]+:[0-9]+: Error: invalid use of 32-bit register offset at operand 3 -- `ldnt1b {z0\.d},p0/z,\[z0\.d,w16\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ldnt1b {z0\.d},p0/z,\[z0\.d,z0\.d\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ldnt1b {z0\.s},p0/z,\[z0\.d\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ldnt1b {z0\.s}, p0/z, \[z0\.s, xzr\]
[^ :]+:[0-9]+: Error: operand mismatch -- `ldnt1b {z0\.d},p0/z,\[z0\.s\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ldnt1b {z0\.s}, p0/z, \[z0\.s, xzr\]
[^ :]+:[0-9]+: Error: type mismatch in vector register list at operand 1 -- `ldnt1b {z0\.s,z1\.d},p0/z,\[z0\.s,x0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ldnt1b {z0\.s},p0/m,\[z0\.s\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ldnt1b {z0\.s}, p0/z, \[z0\.s, xzr\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `ldnt1b {z32\.s},p0/z,\[z0\.s\]'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `ldnt1b {z0\.s},p8/z,\[z0\.s\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ldnt1b {z0\.s},p0/z,\[z32\.s\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ldnt1b {z0\.s},p0/z,\[z0\.s,sp\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ldnt1b {z0\.s},p0/z,\[z0\.s,x32\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ldnt1b {z0\.s},p0/z,\[z0\.s,z0\.s\]'
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ldnt1d {z0\.d,z1\.d},p0/z,\[z0\.d,x0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ldnt1d {z0\.d},p0/m,\[z0\.d\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ldnt1d {z0\.d}, p0/z, \[z0\.d, xzr\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `ldnt1d {z32\.d},p0/z,\[z0\.d\]'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `ldnt1d {z0\.d},p8/z,\[z0\.d\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ldnt1d {z0\.d},p0/z,\[z32\.d\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ldnt1d {z0\.d},p0/z,\[z0\.d,sp\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ldnt1d {z0\.d},p0/z,\[z0\.d,x32\]'
[^ :]+:[0-9]+: Error: invalid use of 32-bit register offset at operand 3 -- `ldnt1d {z0\.d},p0/z,\[z0\.d,w16\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ldnt1d {z0\.d},p0/z,\[z0\.d,z0\.d\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ldnt1d {z0\.s},p0/z,\[z0\.d\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ldnt1d {z0\.d}, p0/z, \[z0\.d, xzr\]
[^ :]+:[0-9]+: Error: operand mismatch -- `ldnt1d {z0\.d},p0/z,\[z0\.s\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ldnt1d {z0\.d}, p0/z, \[z0\.d, xzr\]
[^ :]+:[0-9]+: Error: operand mismatch -- `ldnt1d {z0\.d},p0/m,\[z0\.d\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ldnt1d {z0\.d}, p0/z, \[z0\.d, xzr\]
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ldnt1h {z0\.d,z1\.d},p0/z,\[z0\.d,x0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ldnt1h {z0\.d},p0/m,\[z0\.d\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ldnt1h {z0\.d}, p0/z, \[z0\.d, xzr\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `ldnt1h {z32\.d},p0/z,\[z0\.d\]'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `ldnt1h {z0\.d},p8/z,\[z0\.d\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ldnt1h {z0\.d},p0/z,\[z32\.d\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ldnt1h {z0\.d},p0/z,\[z0\.d,sp\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ldnt1h {z0\.d},p0/z,\[z0\.d,x32\]'
[^ :]+:[0-9]+: Error: invalid use of 32-bit register offset at operand 3 -- `ldnt1h {z0\.d},p0/z,\[z0\.d,w16\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ldnt1h {z0\.d},p0/z,\[z0\.d,z0\.d\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ldnt1h {z0\.s},p0/z,\[z0\.d\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ldnt1h {z0\.s}, p0/z, \[z0\.s, xzr\]
[^ :]+:[0-9]+: Error: type mismatch in vector register list at operand 1 -- `ldnt1h {z0\.s,z1\.d},p0/z,\[z0\.s,x0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `ldnt1h {z32\.s},p0/z,\[z0\.s\]'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `ldnt1h {z0\.s},p8/z,\[z0\.s\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ldnt1h {z0\.s},p0/z,\[z32\.s\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ldnt1h {z0\.s},p0/z,\[z0\.s,sp\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ldnt1h {z0\.s},p0/z,\[z0\.s,x32\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ldnt1h {z0\.s},p0/z,\[z0\.s,z0\.s\]'
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ldnt1sb {z0\.d,z1\.d},p0/z,\[z0\.d,x0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ldnt1sb {z0\.d},p0/m,\[z0\.d\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ldnt1sb {z0\.d}, p0/z, \[z0\.d, xzr\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	ldnt1sb {z0\.s}, p0/z, \[z0\.s, xzr\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `ldnt1sb {z32\.d},p0/z,\[z0\.d\]'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `ldnt1sb {z0\.d},p8/z,\[z0\.d\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ldnt1sb {z0\.d},p0/z,\[z32\.d\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ldnt1sb {z0\.d},p0/z,\[z0\.d,sp\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ldnt1sb {z0\.d},p0/z,\[z0\.d,x32\]'
[^ :]+:[0-9]+: Error: invalid use of 32-bit register offset at operand 3 -- `ldnt1sb {z0\.d},p0/z,\[z0\.d,w16\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ldnt1sb {z0\.d},p0/z,\[z0\.d,z0\.d\]'
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ldnt1sh {z0\.d,z1\.d},p0/z,\[z0\.d,x0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ldnt1sh {z0\.d},p0/m,\[z0\.d\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ldnt1sh {z0\.d}, p0/z, \[z0\.d, xzr\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	ldnt1sh {z0\.s}, p0/z, \[z0\.s, xzr\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `ldnt1sh {z32\.d},p0/z,\[z0\.d\]'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `ldnt1sh {z0\.d},p8/z,\[z0\.d\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ldnt1sh {z0\.d},p0/z,\[z32\.d\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ldnt1sh {z0\.d},p0/z,\[z0\.d,sp\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ldnt1sh {z0\.d},p0/z,\[z0\.d,x32\]'
[^ :]+:[0-9]+: Error: invalid use of 32-bit register offset at operand 3 -- `ldnt1sh {z0\.d},p0/z,\[z0\.d,w16\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ldnt1sh {z0\.d},p0/z,\[z0\.d,z0\.d\]'
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ldnt1sh {z0\.d,z1\.d},p0/z,\[z0\.d,x0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ldnt1sh {z0\.d},p0/m,\[z0\.d\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ldnt1sh {z0\.d}, p0/z, \[z0\.d, xzr\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	ldnt1sh {z0\.s}, p0/z, \[z0\.s, xzr\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `ldnt1sh {z32\.d},p0/z,\[z0\.d\]'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `ldnt1sh {z0\.d},p8/z,\[z0\.d\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ldnt1sh {z0\.d},p0/z,\[z32\.d\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ldnt1sh {z0\.d},p0/z,\[z0\.d,sp\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ldnt1sh {z0\.d},p0/z,\[z0\.d,x32\]'
[^ :]+:[0-9]+: Error: invalid use of 32-bit register offset at operand 3 -- `ldnt1sh {z0\.d},p0/z,\[z0\.d,w16\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ldnt1sh {z0\.d},p0/z,\[z0\.d,z0\.d\]'
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `ldnt1w {z0\.d,z1\.d},p0/z,\[z0\.d,x0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ldnt1w {z0\.d},p0/m,\[z0\.d\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ldnt1w {z0\.d}, p0/z, \[z0\.d, xzr\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `ldnt1w {z32\.d},p0/z,\[z0\.d\]'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `ldnt1w {z0\.d},p8/z,\[z0\.d\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ldnt1w {z0\.d},p0/z,\[z32\.d\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ldnt1w {z0\.d},p0/z,\[z0\.d,sp\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ldnt1w {z0\.d},p0/z,\[z0\.d,x32\]'
[^ :]+:[0-9]+: Error: invalid use of 32-bit register offset at operand 3 -- `ldnt1w {z0\.d},p0/z,\[z0\.d,w16\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ldnt1w {z0\.d},p0/z,\[z0\.d,z0\.d\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `ldnt1w {z0\.s},p0/z,\[z0\.d\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ldnt1w {z0\.s}, p0/z, \[z0\.s, xzr\]
[^ :]+:[0-9]+: Error: type mismatch in vector register list at operand 1 -- `ldnt1w {z0\.s,z1\.d},p0/z,\[z0\.s,x0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `ldnt1w {z32\.s},p0/z,\[z0\.s\]'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `ldnt1w {z0\.s},p8/z,\[z0\.s\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `ldnt1w {z0\.s},p0/z,\[z32\.s\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `ldnt1w {z0\.s},p0/z,\[z0\.s,sp\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ldnt1w {z0\.s},p0/z,\[z0\.s,x32\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `ldnt1w {z0\.s},p0/z,\[z0\.s,z0\.s\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `match p0\.h,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	match p0\.b, p0/z, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	match p0\.h, p0/z, z0\.h, z0\.h
[^ :]+:[0-9]+: Error: expected an SVE predicate register at operand 1 -- `match p16\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `match p0\.b,p8/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `match p0\.b,p0/z,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `match p0\.b,p0/z,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `mla z0\.h,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `mla z0\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	mla z0\.h, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `mla z0\.h,z0\.h,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	mla z0\.h, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `mla z32\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector or predicate register at operand 2 -- `mla z0\.h,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `mla z0\.h,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `mla z0\.s,z0\.s,z0\.s\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `mla z0\.h,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	mla z0\.s, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `mla z0\.s,z0\.s,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	mla z0\.s, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `mla z32\.s,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector or predicate register at operand 2 -- `mla z0\.s,z32\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `mla z0\.s,z0\.s,z8\.s\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 1 at operand 3 -- `mla z0\.d,z0\.d,z0\.d\[2\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `mla z0\.h,z0\.d,z0\.d\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	mla z0\.d, z0\.d, z0\.d\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `mla z0\.d,z0\.d,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	mla z0\.d, z0\.d, z0\.d\[0\]
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `mla z32\.d,z0\.d,z0\.d\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector or predicate register at operand 2 -- `mla z0\.d,z32\.d,z0\.d\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `mla z0\.d,z0\.d,z16\.d\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `mls z0\.h,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `mls z0\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	mls z0\.h, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `mls z0\.h,z0\.h,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	mls z0\.h, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `mls z32\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector or predicate register at operand 2 -- `mls z0\.h,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `mls z0\.h,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `mls z0\.s,z0\.s,z0\.s\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `mls z0\.h,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	mls z0\.s, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `mls z0\.s,z0\.s,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	mls z0\.s, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `mls z32\.s,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector or predicate register at operand 2 -- `mls z0\.s,z32\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `mls z0\.s,z0\.s,z8\.s\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 1 at operand 3 -- `mls z0\.d,z0\.d,z0\.d\[2\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `mls z0\.h,z0\.d,z0\.d\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	mls z0\.d, z0\.d, z0\.d\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `mls z0\.d,z0\.d,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	mls z0\.d, z0\.d, z0\.d\[0\]
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `mls z32\.d,z0\.d,z0\.d\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector or predicate register at operand 2 -- `mls z0\.d,z32\.d,z0\.d\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `mls z0\.d,z0\.d,z16\.d\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `mul z0\.h,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `mul z0\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	mul z0\.h, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `mul z0\.h,z0\.h,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	mul z0\.h, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected an integer or vector register at operand 1 -- `mul z32\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector or predicate register at operand 2 -- `mul z0\.h,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `mul z0\.h,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `mul z0\.s,z0\.s,z0\.s\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `mul z0\.h,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	mul z0\.s, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `mul z0\.s,z0\.s,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	mul z0\.s, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: expected an integer or vector register at operand 1 -- `mul z32\.s,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector or predicate register at operand 2 -- `mul z0\.s,z32\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `mul z0\.s,z0\.s,z8\.s\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 1 at operand 3 -- `mul z0\.d,z0\.d,z0\.d\[2\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `mul z0\.h,z0\.d,z0\.d\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	mul z0\.d, z0\.d, z0\.d\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `mul z0\.d,z0\.d,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	mul z0\.d, z0\.d, z0\.d\[0\]
[^ :]+:[0-9]+: Error: expected an integer or vector register at operand 1 -- `mul z32\.d,z0\.d,z0\.d\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector or predicate register at operand 2 -- `mul z0\.d,z32\.d,z0\.d\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `mul z0\.d,z0\.d,z16\.d\[0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `mul z0\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	mul z0\.b, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	mul z0\.h, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	mul z0\.s, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	mul z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an integer or vector register at operand 1 -- `mul z32\.b,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector or predicate register at operand 2 -- `mul z0\.b,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `nmatch p0\.h,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	nmatch p0\.b, p0/z, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	nmatch p0\.h, p0/z, z0\.h, z0\.h
[^ :]+:[0-9]+: Error: operand mismatch -- `nmatch p0\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	nmatch p0\.b, p0/z, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	nmatch p0\.h, p0/z, z0\.h, z0\.h
[^ :]+:[0-9]+: Error: expected an SVE predicate register at operand 1 -- `nmatch p16\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `nmatch p0\.b,p8/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `nmatch p0\.b,p0/z,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `nmatch p0\.b,p0/z,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `nbsl z0\.d,z1\.d,z0\.d,z0\.d'
[^ :]+:[0-9]+: Error: operand mismatch -- `nbsl z0\.d,z0\.d,z0\.h,z0\.d'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	nbsl z0\.d, z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `nbsl z0\.d,z0\.h,z0\.d,z0\.d'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	nbsl z0\.d, z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `pmul z0\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	pmul z0\.b, z0\.b, z0\.b
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `pmul z32\.b,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `pmul z0\.b,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `pmul z0\.b,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `pmullb z32\.q,z0\.d,z0\.d'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `pmullb z0\.q,z32\.d,z0\.d'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `pmullb z0\.q,z0\.d,z32\.d'
[^ :]+:[0-9]+: Error: operand mismatch -- `pmullb z0\.d,z0\.d,z0\.d'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	pmullb z0\.q, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `pmullb z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `pmullb z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `pmullb z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `pmullb z0\.b,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	pmullb z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	pmullb z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `pmullt z32\.q,z0\.d,z0\.d'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `pmullt z0\.q,z32\.d,z0\.d'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `pmullt z0\.q,z0\.d,z32\.d'
[^ :]+:[0-9]+: Error: operand mismatch -- `pmullt z0\.d,z0\.d,z0\.d'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	pmullt z0\.q, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `pmullt z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `pmullt z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `pmullt z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `pmullt z0\.b,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	pmullt z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	pmullt z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: operand mismatch -- `raddhnb z0\.h,z0\.h,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	raddhnb z0\.b, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	raddhnb z0\.h, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	raddhnb z0\.s, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `raddhnb z32\.b,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `raddhnb z0\.b,z32\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `raddhnb z0\.b,z0\.h,z32\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `raddhnt z0\.h,z0\.h,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	raddhnt z0\.b, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	raddhnt z0\.h, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	raddhnt z0\.s, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `raddhnt z32\.b,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `raddhnt z0\.b,z32\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `raddhnt z0\.b,z0\.h,z32\.h'
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `rax1 z32\.d,z0\.d,z0\.d'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `rax1 z0\.d,z32\.d,z0\.d'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `rax1 z0\.d,z0\.d,z32\.d'
[^ :]+:[0-9]+: Error: operand mismatch -- `rax1 z0\.d,z0\.d,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	rax1 z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `rshrnb z32\.b,z0\.h,#8'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `rshrnb z0\.b,z32\.h,#8'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `rshrnb z0\.b,z0\.h,#9'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `rshrnb z0\.b,z0\.h,#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `rshrnb z0\.h,z0\.h,#8'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	rshrnb z0\.b, z0\.h, #8
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	rshrnb z0\.h, z0\.s, #8
[^ :]+:[0-9]+: Info:    	rshrnb z0\.s, z0\.d, #8
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `rshrnb z0\.h,z0\.s,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `rshrnb z0\.h,z0\.s,#17'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `rshrnb z0\.s,z0\.d,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `rshrnb z0\.s,z0\.d,#33'
[^ :]+:[0-9]+: Warning: SVE `movprfx' compatible instruction expected -- `rshrnt z0\.b,z1\.h,#8'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `rshrnt z32\.b,z0\.h,#8'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `rshrnt z0\.b,z32\.h,#8'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `rshrnt z0\.b,z0\.h,#9'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `rshrnt z0\.b,z0\.h,#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `rshrnt z0\.h,z0\.h,#8'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	rshrnt z0\.b, z0\.h, #8
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	rshrnt z0\.h, z0\.s, #8
[^ :]+:[0-9]+: Info:    	rshrnt z0\.s, z0\.d, #8
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `rshrnt z0\.h,z0\.s,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `rshrnt z0\.h,z0\.s,#17'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `rshrnt z0\.s,z0\.d,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `rshrnt z0\.s,z0\.d,#33'
[^ :]+:[0-9]+: Error: operand mismatch -- `rsubhnb z0\.h,z0\.h,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	rsubhnb z0\.b, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	rsubhnb z0\.h, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	rsubhnb z0\.s, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `rsubhnb z32\.b,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `rsubhnb z0\.b,z32\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `rsubhnb z0\.b,z0\.h,z32\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `rsubhnt z0\.h,z0\.h,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	rsubhnt z0\.b, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	rsubhnt z0\.h, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	rsubhnt z0\.s, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `rsubhnt z32\.b,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `rsubhnt z0\.b,z32\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `rsubhnt z0\.b,z0\.h,z32\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `saba z0\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	saba z0\.b, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	saba z0\.h, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	saba z0\.s, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	saba z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `saba z32\.b,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `saba z0\.b,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `saba z0\.b,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `sabalb z0\.b,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sabalb z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sabalb z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sabalb z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sabalb z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sabalb z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sabalb z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `sabalt z0\.b,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sabalt z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sabalt z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sabalt z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sabalt z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sabalt z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sabalt z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `sabdlb z0\.b,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sabdlb z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sabdlb z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sabdlb z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sabdlb z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sabdlb z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sabdlb z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `sabdlt z0\.b,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sabdlt z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sabdlt z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sabdlt z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sabdlt z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sabdlt z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sabdlt z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `sadalp z0\.b,p0/m,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sadalp z0\.h, p0/m, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sadalp z0\.s, p0/m, z0\.h
[^ :]+:[0-9]+: Info:    	sadalp z0\.d, p0/m, z0\.s
[^ :]+:[0-9]+: Error: operand mismatch -- `sadalp z0\.h,p0/z,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sadalp z0\.h, p0/m, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sadalp z0\.s, p0/m, z0\.h
[^ :]+:[0-9]+: Info:    	sadalp z0\.d, p0/m, z0\.s
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `sadalp z0\.h,p8/m,z0\.b'
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `sadalp z32\.h,p0/m,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sadalp z0\.h,p0/m,z32\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `saddlb z0\.b,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	saddlb z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	saddlb z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	saddlb z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `saddlb z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `saddlb z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `saddlb z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `saddlbt z0\.b,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	saddlbt z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	saddlbt z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	saddlbt z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `saddlbt z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `saddlbt z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `saddlbt z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `saddlt z0\.b,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	saddlt z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	saddlt z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	saddlt z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `saddlt z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `saddlt z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `saddlt z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `saddwb z0\.b,z0\.h,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	saddwb z0\.h, z0\.h, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	saddwb z0\.s, z0\.s, z0\.h
[^ :]+:[0-9]+: Info:    	saddwb z0\.d, z0\.d, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `saddwb z32\.h,z0\.h,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `saddwb z0\.h,z32\.h,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `saddwb z0\.h,z0\.h,z32\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `saddwt z0\.b,z0\.h,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	saddwt z0\.h, z0\.h, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	saddwt z0\.s, z0\.s, z0\.h
[^ :]+:[0-9]+: Info:    	saddwt z0\.d, z0\.d, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `saddwt z32\.h,z0\.h,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `saddwt z0\.h,z32\.h,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `saddwt z0\.h,z0\.h,z32\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `sbclb z0\.d,z0\.s,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sbclb z0\.s, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sbclb z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sbclb z32\.s,z0\.s,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sbclb z0\.s,z32\.s,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sbclb z0\.s,z0\.s,z32\.s'
[^ :]+:[0-9]+: Error: operand mismatch -- `sbclt z0\.d,z0\.s,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sbclt z0\.s, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sbclt z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sbclt z32\.s,z0\.s,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sbclt z0\.s,z32\.s,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sbclt z0\.s,z0\.s,z32\.s'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `shadd z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `shadd z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `shadd z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `shadd z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `shadd z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `shadd z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	shadd z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	shadd z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	shadd z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	shadd z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `shadd z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	shadd z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	shadd z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	shadd z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	shadd z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `shrnb z32\.b,z0\.h,#8'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `shrnb z0\.b,z32\.h,#8'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `shrnb z0\.b,z0\.h,#9'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `shrnb z0\.b,z0\.h,#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `shrnb z0\.h,z0\.h,#8'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	shrnb z0\.b, z0\.h, #8
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	shrnb z0\.h, z0\.s, #8
[^ :]+:[0-9]+: Info:    	shrnb z0\.s, z0\.d, #8
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `shrnb z0\.h,z0\.s,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `shrnb z0\.h,z0\.s,#17'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `shrnb z0\.s,z0\.d,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `shrnb z0\.s,z0\.d,#33'
[^ :]+:[0-9]+: Warning: SVE `movprfx' compatible instruction expected -- `shrnt z0\.b,z1\.h,#8'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `shrnt z32\.b,z0\.h,#8'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `shrnt z0\.b,z32\.h,#8'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `shrnt z0\.b,z0\.h,#9'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `shrnt z0\.b,z0\.h,#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `shrnt z0\.h,z0\.h,#8'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	shrnt z0\.b, z0\.h, #8
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	shrnt z0\.h, z0\.s, #8
[^ :]+:[0-9]+: Info:    	shrnt z0\.s, z0\.d, #8
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `shrnt z0\.h,z0\.s,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `shrnt z0\.h,z0\.s,#17'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `shrnt z0\.s,z0\.d,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `shrnt z0\.s,z0\.d,#33'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `shsub z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `shsub z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `shsub z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `shsub z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `shsub z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `shsub z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	shsub z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	shsub z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	shsub z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	shsub z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `shsub z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	shsub z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	shsub z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	shsub z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	shsub z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `shsubr z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `shsubr z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `shsubr z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `shsubr z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `shsubr z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `shsubr z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	shsubr z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	shsubr z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	shsubr z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	shsubr z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `shsubr z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	shsubr z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	shsubr z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	shsubr z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	shsubr z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `sli z0\.h,z0\.b,#0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sli z0\.b, z0\.b, #0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sli z0\.h, z0\.h, #0
[^ :]+:[0-9]+: Info:    	sli z0\.s, z0\.s, #0
[^ :]+:[0-9]+: Info:    	sli z0\.d, z0\.d, #0
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sli z32\.b,z0\.b,#0'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sli z0\.b,z32\.b,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 7 at operand 3 -- `sli z0\.b,z0\.b,#8'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 15 at operand 3 -- `sli z0\.h,z0\.h,#16'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 31 at operand 3 -- `sli z0\.s,z0\.s,#32'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 63 at operand 3 -- `sli z0\.d,z0\.d,#64'
[^ :]+:[0-9]+: Warning: SVE `movprfx' compatible instruction expected -- `sm4e z0\.s,z0\.s,z1\.s'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `sm4e z1\.s,z0\.s,z0\.s'
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `sm4e z32\.s,z0\.s,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sm4e z0\.s,z32\.s,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sm4e z0\.s,z0\.s,z32\.s'
[^ :]+:[0-9]+: Error: operand mismatch -- `sm4e z0\.s,z0\.s,z0\.d'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sm4e z0\.s, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `sm4ekey z32\.s,z0\.s,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sm4ekey z0\.s,z32\.s,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sm4ekey z0\.s,z0\.s,z32\.s'
[^ :]+:[0-9]+: Error: operand mismatch -- `sm4ekey z0\.s,z0\.s,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sm4ekey z0\.s, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: operand mismatch -- `smaxp z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	smaxp z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	smaxp z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	smaxp z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	smaxp z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `smaxp z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	smaxp z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	smaxp z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	smaxp z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	smaxp z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `smaxp z1\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `smaxp z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `smaxp z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `smaxp z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `smaxp z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `sminp z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sminp z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sminp z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sminp z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	sminp z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `sminp z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sminp z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sminp z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sminp z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	sminp z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `sminp z1\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `sminp z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sminp z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `sminp z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `sminp z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `smlalb z32\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `smlalb z0\.s,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `smlalb z0\.s,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `smlalb z0\.s,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `smlalb z0\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	smlalb z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `smlalb z32\.d,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `smlalb z0\.d,z32\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `smlalb z0\.d,z0\.s,z16\.s\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `smlalb z0\.d,z0\.s,z0\.s\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `smlalb z0\.s,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	smlalb z0\.d, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `smlalb z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `smlalb z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `smlalb z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `smlalb z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `smlalb z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	smlalb z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	smlalb z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	smlalb z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `smlalt z32\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `smlalt z0\.s,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `smlalt z0\.s,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `smlalt z0\.s,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `smlalt z0\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	smlalt z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `smlalt z32\.d,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `smlalt z0\.d,z32\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `smlalt z0\.d,z0\.s,z16\.s\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `smlalt z0\.d,z0\.s,z0\.s\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `smlalt z0\.s,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	smlalt z0\.d, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `smlalt z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `smlalt z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `smlalt z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `smlalt z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `smlalt z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	smlalt z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	smlalt z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	smlalt z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `smlslb z32\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `smlslb z0\.s,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `smlslb z0\.s,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `smlslb z0\.s,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `smlslb z0\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	smlslb z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `smlslb z32\.d,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `smlslb z0\.d,z32\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `smlslb z0\.d,z0\.s,z16\.s\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `smlslb z0\.d,z0\.s,z0\.s\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `smlslb z0\.s,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	smlslb z0\.d, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `smlslb z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `smlslb z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `smlslb z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `smlslb z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `smlslb z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	smlslb z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	smlslb z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	smlslb z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `smlslt z32\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `smlslt z0\.s,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `smlslt z0\.s,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `smlslt z0\.s,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `smlslt z0\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	smlslt z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `smlslt z32\.d,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `smlslt z0\.d,z32\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `smlslt z0\.d,z0\.s,z16\.s\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `smlslt z0\.d,z0\.s,z0\.s\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `smlslt z0\.s,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	smlslt z0\.d, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `smlslt z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `smlslt z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `smlslt z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `smlslt z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `smlslt z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	smlslt z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	smlslt z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	smlslt z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: operand mismatch -- `smulh z0\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	smulh z0\.b, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	smulh z0\.h, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	smulh z0\.s, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	smulh z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an integer register or SVE vector register at operand 1 -- `smulh z32\.b,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector or predicate register at operand 2 -- `smulh z0\.b,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `smulh z0\.b,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `smullb z32\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `smullb z0\.s,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `smullb z0\.s,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `smullb z0\.s,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `smullb z0\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	smullb z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `smullb z32\.d,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `smullb z0\.d,z32\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `smullb z0\.d,z0\.s,z16\.s\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `smullb z0\.d,z0\.s,z0\.s\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `smullb z0\.s,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	smullb z0\.d, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `smullb z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `smullb z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `smullb z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `smullb z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `smullb z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	smullb z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	smullb z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	smullb z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `smullt z32\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `smullt z0\.s,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `smullt z0\.s,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `smullt z0\.s,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `smullt z0\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	smullt z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `smullt z32\.d,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `smullt z0\.d,z32\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `smullt z0\.d,z0\.s,z16\.s\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `smullt z0\.d,z0\.s,z0\.s\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `smullt z0\.s,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	smullt z0\.d, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `smullt z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `smullt z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `smullt z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `smullt z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `smullt z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	smullt z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	smullt z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	smullt z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 at operand 3 -- `splice z0\.b,p0,{z0\.b,z2\.b}'
[^ :]+:[0-9]+: Error: operand mismatch -- `splice z0\.h,p0,{z0\.b,z1\.b}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	splice z0\.b, p0, {z0\.b-z1\.b}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	splice z0\.h, p0, {z0\.h-z1\.h}
[^ :]+:[0-9]+: Info:    	splice z0\.s, p0, {z0\.s-z1\.s}
[^ :]+:[0-9]+: Info:    	splice z0\.d, p0, {z0\.d-z1\.d}
[^ :]+:[0-9]+: Error: type mismatch in vector register list at operand 3 -- `splice z0\.b,p0,{z0\.h,z1\.b}'
[^ :]+:[0-9]+: Error: type mismatch in vector register list at operand 3 -- `splice z0\.b,p0,{z0\.b,z1\.h}'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `splice z32\.b,p0,{z0\.b,z1\.b}'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `splice z0\.b,p8,{z0\.b,z1\.b}'
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 at operand 3 -- `splice z0\.b,p0,{z31\.b,z1\.b}'
[^ :]+:[0-9]+: Error: operand 3 must be a list of SVE vector registers -- `splice z0\.b,p0,{z31\.b,z32\.b}'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `splice z0\.b,p0,{z32\.b,z1\.b}'
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sqabs z32\.b,p0/m,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `sqabs z0\.b,p8/m,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sqabs z0\.b,p0/m,z32\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqabs z0\.b,p0/m,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqabs z0\.b, p0/m, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqabs z0\.h, p0/m, z0\.h
[^ :]+:[0-9]+: Info:    	sqabs z0\.s, p0/m, z0\.s
[^ :]+:[0-9]+: Info:    	sqabs z0\.d, p0/m, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `sqabs z0\.b,p0/z,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqabs z0\.b, p0/m, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqabs z0\.h, p0/m, z0\.h
[^ :]+:[0-9]+: Info:    	sqabs z0\.s, p0/m, z0\.s
[^ :]+:[0-9]+: Info:    	sqabs z0\.d, p0/m, z0\.d
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sqadd z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sqadd z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `sqadd z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `sqadd z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `sqadd z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqadd z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqadd z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqadd z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sqadd z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	sqadd z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `sqadd z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqadd z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqadd z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sqadd z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	sqadd z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: rotate expected to be 90 or 270 at operand 4 -- `sqcadd z0\.b,z0\.b,z0\.b,#180'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `sqcadd z0\.b,z1\.b,z0\.b,#90'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqcadd z32\.b,z0\.b,z0\.b,#90'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqcadd z0\.b,z32\.b,z0\.b,#90'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sqcadd z0\.b,z0\.b,z32\.b,#90'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqcadd z0\.b,z0\.b,z0\.h,#90'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqcadd z0\.b, z0\.b, z0\.b, #90
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqcadd z0\.h, z0\.h, z0\.h, #90
[^ :]+:[0-9]+: Info:    	sqcadd z0\.s, z0\.s, z0\.s, #90
[^ :]+:[0-9]+: Info:    	sqcadd z0\.d, z0\.d, z0\.d, #90
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqdmlalb z32\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqdmlalb z0\.s,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `sqdmlalb z0\.s,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `sqdmlalb z0\.s,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmlalb z0\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmlalb z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqdmlalb z32\.d,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqdmlalb z0\.d,z32\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `sqdmlalb z0\.d,z0\.s,z16\.s\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `sqdmlalb z0\.d,z0\.s,z0\.s\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmlalb z0\.s,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmlalb z0\.d, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqdmlalb z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqdmlalb z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sqdmlalb z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `sqdmlalb z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmlalb z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmlalb z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqdmlalb z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sqdmlalb z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqdmlalbt z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqdmlalbt z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sqdmlalbt z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `sqdmlalbt z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmlalbt z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmlalbt z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqdmlalbt z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sqdmlalbt z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqdmlalt z32\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqdmlalt z0\.s,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `sqdmlalt z0\.s,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `sqdmlalt z0\.s,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmlalt z0\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmlalt z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqdmlalt z32\.d,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqdmlalt z0\.d,z32\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `sqdmlalt z0\.d,z0\.s,z16\.s\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `sqdmlalt z0\.d,z0\.s,z0\.s\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmlalt z0\.s,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmlalt z0\.d, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqdmlalt z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqdmlalt z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sqdmlalt z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `sqdmlalt z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmlalt z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmlalt z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqdmlalt z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sqdmlalt z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqdmlslb z32\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqdmlslb z0\.s,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `sqdmlslb z0\.s,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `sqdmlslb z0\.s,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmlslb z0\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmlslb z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqdmlslb z32\.d,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqdmlslb z0\.d,z32\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `sqdmlslb z0\.d,z0\.s,z16\.s\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `sqdmlslb z0\.d,z0\.s,z0\.s\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmlslb z0\.s,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmlslb z0\.d, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqdmlslb z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqdmlslb z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sqdmlslb z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `sqdmlslb z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmlslb z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmlslb z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqdmlslb z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sqdmlslb z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqdmlslbt z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqdmlslbt z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sqdmlslbt z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `sqdmlslbt z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmlslbt z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmlslbt z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqdmlslbt z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sqdmlslbt z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqdmlslt z32\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqdmlslt z0\.s,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `sqdmlslt z0\.s,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `sqdmlslt z0\.s,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmlslt z0\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmlslt z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqdmlslt z32\.d,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqdmlslt z0\.d,z32\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `sqdmlslt z0\.d,z0\.s,z16\.s\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `sqdmlslt z0\.d,z0\.s,z0\.s\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmlslt z0\.s,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmlslt z0\.d, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqdmlslt z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqdmlslt z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sqdmlslt z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `sqdmlslt z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmlslt z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmlslt z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqdmlslt z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sqdmlslt z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `sqdmulh z32\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqdmulh z0\.h,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `sqdmulh z0\.h,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `sqdmulh z0\.h,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmulh z0\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmulh z0\.h, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmulh z0\.h,z0\.h,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmulh z0\.h, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `sqdmulh z32\.s,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqdmulh z0\.s,z32\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `sqdmulh z0\.s,z0\.s,z8\.s\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `sqdmulh z0\.s,z0\.s,z0\.s\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmulh z0\.s,z0\.h,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmulh z0\.s, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmulh z0\.s,z0\.s,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmulh z0\.s, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `sqdmulh z32\.d,z0\.d,z0\.d\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqdmulh z0\.d,z32\.d,z0\.d\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `sqdmulh z0\.d,z0\.d,z16\.d\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 1 at operand 3 -- `sqdmulh z0\.d,z0\.d,z0\.d\[2\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmulh z0\.d,z0\.h,z0\.d\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmulh z0\.d, z0\.d, z0\.d\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmulh z0\.d,z0\.d,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmulh z0\.d, z0\.d, z0\.d\[0\]
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `sqdmulh z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqdmulh z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sqdmulh z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `sqdmulh z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmulh z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmulh z0\.h, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqdmulh z0\.b, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    	sqdmulh z0\.s, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	sqdmulh z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqdmullb z32\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqdmullb z0\.s,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `sqdmullb z0\.s,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `sqdmullb z0\.s,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmullb z0\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmullb z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqdmullb z32\.d,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqdmullb z0\.d,z32\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `sqdmullb z0\.d,z0\.s,z16\.s\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `sqdmullb z0\.d,z0\.s,z0\.s\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmullb z0\.s,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmullb z0\.d, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqdmullb z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqdmullb z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sqdmullb z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `sqdmullb z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmullb z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmullb z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqdmullb z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sqdmullb z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqdmullt z32\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqdmullt z0\.s,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `sqdmullt z0\.s,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `sqdmullt z0\.s,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmullt z0\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmullt z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqdmullt z32\.d,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqdmullt z0\.d,z32\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `sqdmullt z0\.d,z0\.s,z16\.s\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `sqdmullt z0\.d,z0\.s,z0\.s\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmullt z0\.s,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmullt z0\.d, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqdmullt z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqdmullt z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sqdmullt z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `sqdmullt z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqdmullt z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqdmullt z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqdmullt z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sqdmullt z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sqneg z32\.b,p0/m,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `sqneg z0\.b,p8/m,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sqneg z0\.b,p0/m,z32\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqneg z0\.b,p0/m,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqneg z0\.b, p0/m, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqneg z0\.h, p0/m, z0\.h
[^ :]+:[0-9]+: Info:    	sqneg z0\.s, p0/m, z0\.s
[^ :]+:[0-9]+: Info:    	sqneg z0\.d, p0/m, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `sqneg z0\.b,p0/z,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqneg z0\.b, p0/m, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqneg z0\.h, p0/m, z0\.h
[^ :]+:[0-9]+: Info:    	sqneg z0\.s, p0/m, z0\.s
[^ :]+:[0-9]+: Info:    	sqneg z0\.d, p0/m, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqrdcmlah z32\.h,z0\.h,z0\.h\[0\],#0'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqrdcmlah z0\.h,z32\.h,z0\.h\[0\],#0'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `sqrdcmlah z0\.h,z0\.h,z8\.h\[0\],#0'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `sqrdcmlah z0\.h,z0\.h,z0\.h\[4\],#0'
[^ :]+:[0-9]+: Error: rotate expected to be 0, 90, 180 or 270 at operand 4 -- `sqrdcmlah z0\.h,z0\.h,z0\.h\[0\],#1'
[^ :]+:[0-9]+: Error: rotate expected to be 0, 90, 180 or 270 at operand 4 -- `sqrdcmlah z0\.h,z0\.h,z0\.h\[0\],#360'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdcmlah z0\.h,z0\.h,z0\.s\[0\],#0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdcmlah z0\.h, z0\.h, z0\.h\[0\], #0
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdcmlah z0\.h,z0\.s,z0\.h\[0\],#0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdcmlah z0\.h, z0\.h, z0\.h\[0\], #0
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqrdcmlah z32\.s,z0\.s,z0\.s\[0\],#0'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqrdcmlah z0\.s,z32\.s,z0\.s\[0\],#0'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `sqrdcmlah z0\.s,z0\.s,z16\.s\[0\],#0'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 1 at operand 3 -- `sqrdcmlah z0\.s,z0\.s,z0\.s\[2\],#0'
[^ :]+:[0-9]+: Error: rotate expected to be 0, 90, 180 or 270 at operand 4 -- `sqrdcmlah z0\.s,z0\.s,z0\.s\[0\],#1'
[^ :]+:[0-9]+: Error: rotate expected to be 0, 90, 180 or 270 at operand 4 -- `sqrdcmlah z0\.s,z0\.s,z0\.s\[0\],#360'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdcmlah z0\.s,z0\.s,z0\.h\[0\],#0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdcmlah z0\.s, z0\.s, z0\.s\[0\], #0
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdcmlah z0\.s,z0\.h,z0\.s\[0\],#0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdcmlah z0\.s, z0\.s, z0\.s\[0\], #0
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqrdcmlah z32\.b,z0\.b,z0\.b,#0'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqrdcmlah z0\.b,z32\.b,z0\.b,#0'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sqrdcmlah z0\.b,z0\.b,z32\.b,#0'
[^ :]+:[0-9]+: Error: rotate expected to be 0, 90, 180 or 270 at operand 4 -- `sqrdcmlah z0\.b,z0\.b,z0\.b,#1'
[^ :]+:[0-9]+: Error: rotate expected to be 0, 90, 180 or 270 at operand 4 -- `sqrdcmlah z0\.b,z0\.b,z0\.b,#360'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdcmlah z0\.b,z0\.b,z0\.h,#0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdcmlah z0\.b, z0\.b, z0\.b, #0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqrdcmlah z0\.h, z0\.h, z0\.h, #0
[^ :]+:[0-9]+: Info:    	sqrdcmlah z0\.s, z0\.s, z0\.s, #0
[^ :]+:[0-9]+: Info:    	sqrdcmlah z0\.d, z0\.d, z0\.d, #0
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sqrdmlah z32\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqrdmlah z0\.h,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `sqrdmlah z0\.h,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `sqrdmlah z0\.h,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdmlah z0\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdmlah z0\.h, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdmlah z0\.h,z0\.h,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdmlah z0\.h, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sqrdmlah z32\.s,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqrdmlah z0\.s,z32\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `sqrdmlah z0\.s,z0\.s,z8\.s\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `sqrdmlah z0\.s,z0\.s,z0\.s\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdmlah z0\.s,z0\.h,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdmlah z0\.s, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdmlah z0\.s,z0\.s,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdmlah z0\.s, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sqrdmlah z32\.d,z0\.d,z0\.d\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqrdmlah z0\.d,z32\.d,z0\.d\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `sqrdmlah z0\.d,z0\.d,z16\.d\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 1 at operand 3 -- `sqrdmlah z0\.d,z0\.d,z0\.d\[2\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdmlah z0\.d,z0\.h,z0\.d\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdmlah z0\.d, z0\.d, z0\.d\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdmlah z0\.d,z0\.d,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdmlah z0\.d, z0\.d, z0\.d\[0\]
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sqrdmlah z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqrdmlah z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sqrdmlah z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `sqrdmlah z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdmlah z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdmlah z0\.h, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqrdmlah z0\.b, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    	sqrdmlah z0\.s, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	sqrdmlah z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sqrdmlsh z32\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqrdmlsh z0\.h,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `sqrdmlsh z0\.h,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `sqrdmlsh z0\.h,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdmlsh z0\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdmlsh z0\.h, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdmlsh z0\.h,z0\.h,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdmlsh z0\.h, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sqrdmlsh z32\.s,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqrdmlsh z0\.s,z32\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `sqrdmlsh z0\.s,z0\.s,z8\.s\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `sqrdmlsh z0\.s,z0\.s,z0\.s\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdmlsh z0\.s,z0\.h,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdmlsh z0\.s, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdmlsh z0\.s,z0\.s,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdmlsh z0\.s, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sqrdmlsh z32\.d,z0\.d,z0\.d\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqrdmlsh z0\.d,z32\.d,z0\.d\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `sqrdmlsh z0\.d,z0\.d,z16\.d\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 1 at operand 3 -- `sqrdmlsh z0\.d,z0\.d,z0\.d\[2\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdmlsh z0\.d,z0\.h,z0\.d\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdmlsh z0\.d, z0\.d, z0\.d\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdmlsh z0\.d,z0\.d,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdmlsh z0\.d, z0\.d, z0\.d\[0\]
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sqrdmlsh z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqrdmlsh z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sqrdmlsh z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `sqrdmlsh z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdmlsh z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdmlsh z0\.h, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqrdmlsh z0\.b, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    	sqrdmlsh z0\.s, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	sqrdmlsh z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sqrdmulh z32\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqrdmulh z0\.h,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `sqrdmulh z0\.h,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `sqrdmulh z0\.h,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdmulh z0\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdmulh z0\.h, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdmulh z0\.h,z0\.h,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdmulh z0\.h, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sqrdmulh z32\.s,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqrdmulh z0\.s,z32\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `sqrdmulh z0\.s,z0\.s,z8\.s\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `sqrdmulh z0\.s,z0\.s,z0\.s\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdmulh z0\.s,z0\.h,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdmulh z0\.s, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdmulh z0\.s,z0\.s,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdmulh z0\.s, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sqrdmulh z32\.d,z0\.d,z0\.d\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqrdmulh z0\.d,z32\.d,z0\.d\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `sqrdmulh z0\.d,z0\.d,z16\.d\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 1 at operand 3 -- `sqrdmulh z0\.d,z0\.d,z0\.d\[2\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdmulh z0\.d,z0\.h,z0\.d\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdmulh z0\.d, z0\.d, z0\.d\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdmulh z0\.d,z0\.d,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdmulh z0\.d, z0\.d, z0\.d\[0\]
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sqrdmulh z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqrdmulh z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sqrdmulh z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `sqrdmulh z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrdmulh z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrdmulh z0\.h, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqrdmulh z0\.b, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    	sqrdmulh z0\.s, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	sqrdmulh z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sqrshl z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sqrshl z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `sqrshl z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `sqrshl z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `sqrshl z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrshl z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrshl z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqrshl z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sqrshl z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	sqrshl z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrshl z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrshl z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqrshl z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sqrshl z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	sqrshl z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqrshlr z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sqrshlr z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `sqrshlr z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `sqrshlr z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `sqrshlr z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrshlr z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrshlr z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqrshlr z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sqrshlr z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	sqrshlr z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrshlr z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrshlr z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqrshlr z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sqrshlr z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	sqrshlr z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqrshrnb z32\.b,z0\.h,#8'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqrshrnb z0\.b,z32\.h,#8'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `sqrshrnb z0\.b,z0\.h,#9'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `sqrshrnb z0\.b,z0\.h,#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrshrnb z0\.h,z0\.h,#8'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrshrnb z0\.b, z0\.h, #8
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqrshrnb z0\.h, z0\.s, #8
[^ :]+:[0-9]+: Info:    	sqrshrnb z0\.s, z0\.d, #8
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `sqrshrnb z0\.h,z0\.s,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `sqrshrnb z0\.h,z0\.s,#17'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `sqrshrnb z0\.s,z0\.d,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `sqrshrnb z0\.s,z0\.d,#33'
[^ :]+:[0-9]+: Warning: SVE `movprfx' compatible instruction expected -- `sqrshrnt z0\.b,z0\.h,#1'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqrshrnt z32\.b,z0\.h,#8'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqrshrnt z0\.b,z32\.h,#8'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `sqrshrnt z0\.b,z0\.h,#9'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `sqrshrnt z0\.b,z0\.h,#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrshrnt z0\.h,z0\.h,#8'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrshrnt z0\.b, z0\.h, #8
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqrshrnt z0\.h, z0\.s, #8
[^ :]+:[0-9]+: Info:    	sqrshrnt z0\.s, z0\.d, #8
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `sqrshrnt z0\.h,z0\.s,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `sqrshrnt z0\.h,z0\.s,#17'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `sqrshrnt z0\.s,z0\.d,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `sqrshrnt z0\.s,z0\.d,#33'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqrshrunb z32\.b,z0\.h,#8'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqrshrunb z0\.b,z32\.h,#8'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `sqrshrunb z0\.b,z0\.h,#9'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `sqrshrunb z0\.b,z0\.h,#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrshrunb z0\.h,z0\.h,#8'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrshrunb z0\.b, z0\.h, #8
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqrshrunb z0\.h, z0\.s, #8
[^ :]+:[0-9]+: Info:    	sqrshrunb z0\.s, z0\.d, #8
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `sqrshrunb z0\.h,z0\.s,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `sqrshrunb z0\.h,z0\.s,#17'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `sqrshrunb z0\.s,z0\.d,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `sqrshrunb z0\.s,z0\.d,#33'
[^ :]+:[0-9]+: Warning: SVE `movprfx' compatible instruction expected -- `sqrshrunt z0\.b,z0\.h,#1'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqrshrunt z32\.b,z0\.h,#8'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqrshrunt z0\.b,z32\.h,#8'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `sqrshrunt z0\.b,z0\.h,#9'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `sqrshrunt z0\.b,z0\.h,#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrshrunt z0\.h,z0\.h,#8'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrshrunt z0\.b, z0\.h, #8
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqrshrunt z0\.h, z0\.s, #8
[^ :]+:[0-9]+: Info:    	sqrshrunt z0\.s, z0\.d, #8
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `sqrshrunt z0\.h,z0\.s,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `sqrshrunt z0\.h,z0\.s,#17'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `sqrshrunt z0\.s,z0\.d,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `sqrshrunt z0\.s,z0\.d,#33'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqshl z0\.h,p0/m,z0\.b,#0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqshl z0\.b, p0/m, z0\.b, #0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqshl z0\.h, p0/m, z0\.h, #0
[^ :]+:[0-9]+: Info:    	sqshl z0\.s, p0/m, z0\.s, #0
[^ :]+:[0-9]+: Info:    	sqshl z0\.d, p0/m, z0\.d, #0
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sqshl z32\.b,p0/m,z32\.b,#0'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `sqshl z0\.b,p0/m,z1\.b,#0'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `sqshl z0\.b,p8/m,z0\.b,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 7 at operand 4 -- `sqshl z0\.b,p0/m,z0\.b,#8'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 15 at operand 4 -- `sqshl z0\.h,p0/m,z0\.h,#16'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 31 at operand 4 -- `sqshl z0\.s,p0/m,z0\.s,#32'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 63 at operand 4 -- `sqshl z0\.d,p0/m,z0\.d,#64'
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sqshl z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sqshl z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: constant expression required at operand 4 -- `sqshl z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `sqshl z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `sqshl z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqshl z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqshl z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqshl z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sqshl z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	sqshl z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `sqshl z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqshl z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqshl z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sqshl z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	sqshl z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqshlr z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sqshlr z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `sqshlr z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `sqshlr z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `sqshlr z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqshlr z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqshlr z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqshlr z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sqshlr z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	sqshlr z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `sqshlr z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqshlr z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqshlr z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sqshlr z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	sqshlr z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `sqshlu z0\.h,p0/m,z0\.b,#0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqshlu z0\.b, p0/m, z0\.b, #0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqshlu z0\.h, p0/m, z0\.h, #0
[^ :]+:[0-9]+: Info:    	sqshlu z0\.s, p0/m, z0\.s, #0
[^ :]+:[0-9]+: Info:    	sqshlu z0\.d, p0/m, z0\.d, #0
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sqshlu z32\.b,p0/m,z32\.b,#0'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `sqshlu z0\.b,p0/m,z1\.b,#0'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `sqshlu z0\.b,p8/m,z0\.b,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 7 at operand 4 -- `sqshlu z0\.b,p0/m,z0\.b,#8'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 15 at operand 4 -- `sqshlu z0\.h,p0/m,z0\.h,#16'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 31 at operand 4 -- `sqshlu z0\.s,p0/m,z0\.s,#32'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 63 at operand 4 -- `sqshlu z0\.d,p0/m,z0\.d,#64'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqshrnb z32\.b,z0\.h,#8'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqshrnb z0\.b,z32\.h,#8'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `sqshrnb z0\.b,z0\.h,#9'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `sqshrnb z0\.b,z0\.h,#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqshrnb z0\.h,z0\.h,#8'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqshrnb z0\.b, z0\.h, #8
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqshrnb z0\.h, z0\.s, #8
[^ :]+:[0-9]+: Info:    	sqshrnb z0\.s, z0\.d, #8
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `sqshrnb z0\.h,z0\.s,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `sqshrnb z0\.h,z0\.s,#17'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `sqshrnb z0\.s,z0\.d,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `sqshrnb z0\.s,z0\.d,#33'
[^ :]+:[0-9]+: Warning: SVE `movprfx' compatible instruction expected -- `sqshrnt z0\.b,z0\.h,#1'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqshrnt z32\.b,z0\.h,#8'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqshrnt z0\.b,z32\.h,#8'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `sqshrnt z0\.b,z0\.h,#9'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `sqshrnt z0\.b,z0\.h,#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqshrnt z0\.h,z0\.h,#8'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqshrnt z0\.b, z0\.h, #8
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqshrnt z0\.h, z0\.s, #8
[^ :]+:[0-9]+: Info:    	sqshrnt z0\.s, z0\.d, #8
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `sqshrnt z0\.h,z0\.s,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `sqshrnt z0\.h,z0\.s,#17'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `sqshrnt z0\.s,z0\.d,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `sqshrnt z0\.s,z0\.d,#33'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqshrunb z32\.b,z0\.h,#8'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqshrunb z0\.b,z32\.h,#8'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `sqshrunb z0\.b,z0\.h,#9'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `sqshrunb z0\.b,z0\.h,#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqshrunb z0\.h,z0\.h,#8'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqshrunb z0\.b, z0\.h, #8
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqshrunb z0\.h, z0\.s, #8
[^ :]+:[0-9]+: Info:    	sqshrunb z0\.s, z0\.d, #8
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `sqshrunb z0\.h,z0\.s,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `sqshrunb z0\.h,z0\.s,#17'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `sqshrunb z0\.s,z0\.d,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `sqshrunb z0\.s,z0\.d,#33'
[^ :]+:[0-9]+: Warning: SVE `movprfx' compatible instruction expected -- `sqshrunt z0\.b,z0\.h,#1'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqshrunt z32\.b,z0\.h,#8'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqshrunt z0\.b,z32\.h,#8'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `sqshrunt z0\.b,z0\.h,#9'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `sqshrunt z0\.b,z0\.h,#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqshrunt z0\.h,z0\.h,#8'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqshrunt z0\.b, z0\.h, #8
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqshrunt z0\.h, z0\.s, #8
[^ :]+:[0-9]+: Info:    	sqshrunt z0\.s, z0\.d, #8
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `sqshrunt z0\.h,z0\.s,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `sqshrunt z0\.h,z0\.s,#17'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `sqshrunt z0\.s,z0\.d,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `sqshrunt z0\.s,z0\.d,#33'
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sqsub z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sqsub z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `sqsub z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `sqsub z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `sqsub z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqsub z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqsub z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqsub z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sqsub z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	sqsub z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `sqsub z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqsub z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqsub z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sqsub z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	sqsub z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqsubr z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sqsubr z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `sqsubr z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `sqsubr z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `sqsubr z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqsubr z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqsubr z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqsubr z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sqsubr z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	sqsubr z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `sqsubr z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqsubr z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqsubr z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sqsubr z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	sqsubr z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqxtnb z32\.b,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqxtnb z0\.b,z32\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqxtnb z0\.b,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqxtnb z0\.b, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqxtnb z0\.h, z0\.s
[^ :]+:[0-9]+: Info:    	sqxtnb z0\.s, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqxtnt z32\.b,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqxtnt z0\.b,z32\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqxtnt z0\.b,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqxtnt z0\.b, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqxtnt z0\.h, z0\.s
[^ :]+:[0-9]+: Info:    	sqxtnt z0\.s, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqxtunb z32\.b,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqxtunb z0\.b,z32\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqxtunb z0\.b,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqxtunb z0\.b, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqxtunb z0\.h, z0\.s
[^ :]+:[0-9]+: Info:    	sqxtunb z0\.s, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqxtunt z32\.b,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sqxtunt z0\.b,z32\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqxtunt z0\.b,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqxtunt z0\.b, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqxtunt z0\.h, z0\.s
[^ :]+:[0-9]+: Info:    	sqxtunt z0\.s, z0\.d
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `srhadd z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `srhadd z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `srhadd z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `srhadd z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `srhadd z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `srhadd z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	srhadd z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	srhadd z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	srhadd z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	srhadd z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `srhadd z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	srhadd z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	srhadd z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	srhadd z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	srhadd z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `sri z0\.h,z0\.b,#1'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sri z0\.b, z0\.b, #1
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sri z0\.h, z0\.h, #1
[^ :]+:[0-9]+: Info:    	sri z0\.s, z0\.s, #1
[^ :]+:[0-9]+: Info:    	sri z0\.d, z0\.d, #1
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sri z32\.b,z0\.b,#1'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sri z0\.b,z32\.b,#1'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `sri z0\.b,z0\.b,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `sri z0\.b,z0\.b,#9'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `sri z0\.h,z0\.h,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `sri z0\.h,z0\.h,#17'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `sri z0\.s,z0\.s,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `sri z0\.s,z0\.s,#33'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 64 at operand 3 -- `sri z0\.d,z0\.d,#0'
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `srshl z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `srshl z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `srshl z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `srshl z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `srshl z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `srshl z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	srshl z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	srshl z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	srshl z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	srshl z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `srshl z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	srshl z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	srshl z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	srshl z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	srshl z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `srshlr z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `srshlr z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `srshlr z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `srshlr z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `srshlr z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `srshlr z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	srshlr z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	srshlr z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	srshlr z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	srshlr z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `srshlr z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	srshlr z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	srshlr z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	srshlr z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	srshlr z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `srshr z0\.h,p0/m,z0\.b,#1'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	srshr z0\.b, p0/m, z0\.b, #1
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	srshr z0\.h, p0/m, z0\.h, #1
[^ :]+:[0-9]+: Info:    	srshr z0\.s, p0/m, z0\.s, #1
[^ :]+:[0-9]+: Info:    	srshr z0\.d, p0/m, z0\.d, #1
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `srshr z32\.b,p0/m,z32\.b,#1'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `srshr z0\.b,p0/m,z1\.b,#1'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `srshr z0\.b,p8/m,z0\.b,#1'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 4 -- `srshr z0\.b,p0/m,z0\.b,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 4 -- `srshr z0\.b,p0/m,z0\.b,#9'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 4 -- `srshr z0\.h,p0/m,z0\.h,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 4 -- `srshr z0\.h,p0/m,z0\.h,#17'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 4 -- `srshr z0\.s,p0/m,z0\.s,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 4 -- `srshr z0\.s,p0/m,z0\.s,#33'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 64 at operand 4 -- `srshr z0\.d,p0/m,z0\.d,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 64 at operand 4 -- `srshr z0\.d,p0/m,z0\.d,#65'
[^ :]+:[0-9]+: Error: operand mismatch -- `srsra z0\.h,z0\.b,#1'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	srsra z0\.b, z0\.b, #1
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	srsra z0\.h, z0\.h, #1
[^ :]+:[0-9]+: Info:    	srsra z0\.s, z0\.s, #1
[^ :]+:[0-9]+: Info:    	srsra z0\.d, z0\.d, #1
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `srsra z32\.b,z0\.b,#1'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `srsra z0\.b,z32\.b,#1'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `srsra z0\.b,z0\.b,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `srsra z0\.b,z0\.b,#9'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `srsra z0\.h,z0\.h,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `srsra z0\.h,z0\.h,#17'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `srsra z0\.s,z0\.s,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `srsra z0\.s,z0\.s,#33'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 64 at operand 3 -- `srsra z0\.d,z0\.d,#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `sshllb z0\.b,z0\.b,#0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sshllb z0\.h, z0\.b, #0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sshllb z0\.s, z0\.h, #0
[^ :]+:[0-9]+: Info:    	sshllb z0\.d, z0\.s, #0
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sshllb z32\.h,z0\.b,#0'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sshllb z0\.h,z32\.b,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 7 at operand 3 -- `sshllb z0\.h,z0\.b,#8'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 15 at operand 3 -- `sshllb z0\.s,z0\.h,#16'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 31 at operand 3 -- `sshllb z0\.d,z0\.s,#32'
[^ :]+:[0-9]+: Error: operand mismatch -- `sshllt z0\.b,z0\.b,#0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sshllt z0\.h, z0\.b, #0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sshllt z0\.s, z0\.h, #0
[^ :]+:[0-9]+: Info:    	sshllt z0\.d, z0\.s, #0
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sshllt z32\.h,z0\.b,#0'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sshllt z0\.h,z32\.b,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 7 at operand 3 -- `sshllt z0\.h,z0\.b,#8'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 15 at operand 3 -- `sshllt z0\.s,z0\.h,#16'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 31 at operand 3 -- `sshllt z0\.d,z0\.s,#32'
[^ :]+:[0-9]+: Error: operand mismatch -- `ssra z0\.h,z0\.b,#1'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ssra z0\.b, z0\.b, #1
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	ssra z0\.h, z0\.h, #1
[^ :]+:[0-9]+: Info:    	ssra z0\.s, z0\.s, #1
[^ :]+:[0-9]+: Info:    	ssra z0\.d, z0\.d, #1
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `ssra z32\.b,z0\.b,#1'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `ssra z0\.b,z32\.b,#1'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `ssra z0\.b,z0\.b,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `ssra z0\.b,z0\.b,#9'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `ssra z0\.h,z0\.h,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `ssra z0\.h,z0\.h,#17'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `ssra z0\.s,z0\.s,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `ssra z0\.s,z0\.s,#33'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 64 at operand 3 -- `ssra z0\.d,z0\.d,#0'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `ssublb z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `ssublb z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `ssublb z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `ssublb z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `ssublb z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ssublb z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	ssublb z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	ssublb z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `ssublbt z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `ssublbt z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `ssublbt z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `ssublbt z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `ssublbt z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ssublbt z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	ssublbt z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	ssublbt z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `ssublt z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `ssublt z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `ssublt z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `ssublt z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `ssublt z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ssublt z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	ssublt z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	ssublt z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `ssubltb z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `ssubltb z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `ssubltb z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `ssubltb z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `ssubltb z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ssubltb z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	ssubltb z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	ssubltb z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `ssubwb z32\.h,z0\.h,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `ssubwb z0\.h,z32\.h,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `ssubwb z0\.h,z0\.h,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `ssubwb z0\.s,z0\.s,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `ssubwb z0\.h,z0\.h,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ssubwb z0\.h, z0\.h, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	ssubwb z0\.s, z0\.s, z0\.h
[^ :]+:[0-9]+: Info:    	ssubwb z0\.d, z0\.d, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `ssubwt z32\.h,z0\.h,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `ssubwt z0\.h,z32\.h,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `ssubwt z0\.h,z0\.h,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `ssubwt z0\.s,z0\.s,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `ssubwt z0\.h,z0\.h,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ssubwt z0\.h, z0\.h, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	ssubwt z0\.s, z0\.s, z0\.h
[^ :]+:[0-9]+: Info:    	ssubwt z0\.d, z0\.d, z0\.s
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `stnt1b {z0\.d,z1\.d},p0,\[z0\.d,x0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `stnt1b {z0\.d},p0/m,\[z0\.d\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	stnt1b {z0\.d}, p0, \[z0\.d, xzr\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `stnt1b {z32\.d},p0,\[z0\.d\]'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `stnt1b {z0\.d},p8,\[z0\.d\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `stnt1b {z0\.d},p0,\[z32\.d\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `stnt1b {z0\.d},p0,\[z0\.d,sp\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `stnt1b {z0\.d},p0,\[z0\.d,x32\]'
[^ :]+:[0-9]+: Error: invalid use of 32-bit register offset at operand 3 -- `stnt1b {z0\.d},p0,\[z0\.d,w16\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `stnt1b {z0\.d},p0,\[z0\.d,z0\.d\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `stnt1b {z0\.s},p0,\[z0\.d\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	stnt1b {z0\.s}, p0, \[z0\.s, xzr\]
[^ :]+:[0-9]+: Error: type mismatch in vector register list at operand 1 -- `stnt1b {z0\.s,z1\.d},p0,\[z0\.s,x0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `stnt1b {z32\.s},p0,\[z0\.s\]'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `stnt1b {z0\.s},p8,\[z0\.s\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `stnt1b {z0\.s},p0,\[z32\.s\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `stnt1b {z0\.s},p0,\[z0\.s,sp\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `stnt1b {z0\.s},p0,\[z0\.s,x32\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `stnt1b {z0\.s},p0,\[z0\.s,z0\.s\]'
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `stnt1d {z0\.d,z1\.d},p0,\[z0\.d,x0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `stnt1d {z0\.d},p0/m,\[z0\.d\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	stnt1d {z0\.d}, p0, \[z0\.d, xzr\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `stnt1d {z32\.d},p0,\[z0\.d\]'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `stnt1d {z0\.d},p8,\[z0\.d\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `stnt1d {z0\.d},p0,\[z32\.d\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `stnt1d {z0\.d},p0,\[z0\.d,sp\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `stnt1d {z0\.d},p0,\[z0\.d,x32\]'
[^ :]+:[0-9]+: Error: invalid use of 32-bit register offset at operand 3 -- `stnt1d {z0\.d},p0,\[z0\.d,w16\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `stnt1d {z0\.d},p0,\[z0\.d,z0\.d\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `stnt1d {z0\.s},p0,\[z0\.d\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	stnt1d {z0\.d}, p0, \[z0\.d, xzr\]
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `stnt1h {z0\.d,z1\.d},p0,\[z0\.d,x0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `stnt1h {z0\.d},p0/m,\[z0\.d\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	stnt1h {z0\.d}, p0, \[z0\.d, xzr\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `stnt1h {z32\.d},p0,\[z0\.d\]'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `stnt1h {z0\.d},p8,\[z0\.d\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `stnt1h {z0\.d},p0,\[z32\.d\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `stnt1h {z0\.d},p0,\[z0\.d,sp\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `stnt1h {z0\.d},p0,\[z0\.d,x32\]'
[^ :]+:[0-9]+: Error: invalid use of 32-bit register offset at operand 3 -- `stnt1h {z0\.d},p0,\[z0\.d,w16\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `stnt1h {z0\.d},p0,\[z0\.d,z0\.d\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `stnt1h {z0\.s},p0,\[z0\.d\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	stnt1h {z0\.s}, p0, \[z0\.s, xzr\]
[^ :]+:[0-9]+: Error: type mismatch in vector register list at operand 1 -- `stnt1h {z0\.s,z1\.d},p0,\[z0\.s,x0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `stnt1h {z32\.s},p0,\[z0\.s\]'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `stnt1h {z0\.s},p8,\[z0\.s\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `stnt1h {z0\.s},p0,\[z32\.s\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `stnt1h {z0\.s},p0,\[z0\.s,sp\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `stnt1h {z0\.s},p0,\[z0\.s,x32\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `stnt1h {z0\.s},p0,\[z0\.s,z0\.s\]'
[^ :]+:[0-9]+: Error: expected a single-register list at operand 1 -- `stnt1w {z0\.d,z1\.d},p0,\[z0\.d,x0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `stnt1w {z0\.d},p0/m,\[z0\.d\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	stnt1w {z0\.d}, p0, \[z0\.d, xzr\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `stnt1w {z32\.d},p0,\[z0\.d\]'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `stnt1w {z0\.d},p8,\[z0\.d\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `stnt1w {z0\.d},p0,\[z32\.d\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `stnt1w {z0\.d},p0,\[z0\.d,sp\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `stnt1w {z0\.d},p0,\[z0\.d,x32\]'
[^ :]+:[0-9]+: Error: invalid use of 32-bit register offset at operand 3 -- `stnt1w {z0\.d},p0,\[z0\.d,w16\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `stnt1w {z0\.d},p0,\[z0\.d,z0\.d\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `stnt1w {z0\.s},p0,\[z0\.d\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	stnt1w {z0\.s}, p0, \[z0\.s, xzr\]
[^ :]+:[0-9]+: Error: type mismatch in vector register list at operand 1 -- `stnt1w {z0\.s,z1\.d},p0,\[z0\.s,x0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `stnt1w {z32\.s},p0,\[z0\.s\]'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `stnt1w {z0\.s},p8,\[z0\.s\]'
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `stnt1w {z0\.s},p0,\[z32\.s\]'
[^ :]+:[0-9]+: Error: invalid offset register at operand 3 -- `stnt1w {z0\.s},p0,\[z0\.s,sp\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `stnt1w {z0\.s},p0,\[z0\.s,x32\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `stnt1w {z0\.s},p0,\[z0\.s,z0\.s\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `subhnb z0\.h,z0\.h,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	subhnb z0\.b, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	subhnb z0\.h, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	subhnb z0\.s, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `subhnb z32\.b,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `subhnb z0\.b,z32\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `subhnb z0\.b,z0\.h,z32\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `subhnt z0\.h,z0\.h,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	subhnt z0\.b, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	subhnt z0\.h, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	subhnt z0\.s, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `subhnt z32\.b,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `subhnt z0\.b,z32\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `subhnt z0\.b,z0\.h,z32\.h'
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `suqadd z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `suqadd z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `suqadd z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `suqadd z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `suqadd z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `suqadd z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	suqadd z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	suqadd z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	suqadd z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	suqadd z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `suqadd z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	suqadd z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	suqadd z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	suqadd z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	suqadd z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `tbl z32\.b,{z0\.b,z1\.b},z0\.b'
[^ :]+:[0-9]+: Error: operand 2 must be a list of SVE vector registers -- `tbl z0\.b,{z31\.b,z32\.b},z0\.b'
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 at operand 2 -- `tbl z0\.b,{z31\.b,z1\.b},z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `tbl z0\.b,{z0\.b,z1\.b},z32\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `tbl z0\.b,{z0\.b,z1\.b},z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	tbl z0\.b, {z0\.b-z1\.b}, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	tbl z0\.h, {z0\.h-z1\.h}, z0\.h
[^ :]+:[0-9]+: Info:    	tbl z0\.s, {z0\.s-z1\.s}, z0\.s
[^ :]+:[0-9]+: Info:    	tbl z0\.d, {z0\.d-z1\.d}, z0\.d
[^ :]+:[0-9]+: Error: type mismatch in vector register list at operand 2 -- `tbl z0\.b,{z0\.b,z1\.h},z0\.b'
[^ :]+:[0-9]+: Error: type mismatch in vector register list at operand 2 -- `tbl z0\.b,{z0\.h,z0\.b},z0\.b'
[^ :]+:[0-9]+: Error: invalid register list at operand 2 -- `tbl z0\.h,{z0\.b,z0\.b},z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `tbl z0\.h,{z0\.b,z1\.b},z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	tbl z0\.b, {z0\.b-z1\.b}, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	tbl z0\.h, {z0\.h-z1\.h}, z0\.h
[^ :]+:[0-9]+: Info:    	tbl z0\.s, {z0\.s-z1\.s}, z0\.s
[^ :]+:[0-9]+: Info:    	tbl z0\.d, {z0\.d-z1\.d}, z0\.d
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `tbx z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `tbx z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `tbx z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `tbx z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `tbx z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	tbx z0\.h, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	tbx z0\.b, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    	tbx z0\.s, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	tbx z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `uaba z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `uaba z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `uaba z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `uaba z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `uaba z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uaba z0\.h, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uaba z0\.b, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    	uaba z0\.s, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	uaba z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `uabalb z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `uabalb z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `uabalb z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `uabalb z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `uabalb z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uabalb z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uabalb z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uabalb z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `uabalt z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `uabalt z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `uabalt z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `uabalt z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `uabalt z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uabalt z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uabalt z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uabalt z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `uabdlb z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `uabdlb z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `uabdlb z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `uabdlb z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `uabdlb z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uabdlb z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uabdlb z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uabdlb z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `uabdlt z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `uabdlt z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `uabdlt z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `uabdlt z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `uabdlt z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uabdlt z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uabdlt z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uabdlt z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: operand mismatch -- `uadalp z0\.b,p0/m,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uadalp z0\.h, p0/m, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uadalp z0\.s, p0/m, z0\.h
[^ :]+:[0-9]+: Info:    	uadalp z0\.d, p0/m, z0\.s
[^ :]+:[0-9]+: Error: operand mismatch -- `uadalp z0\.h,p0/z,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uadalp z0\.h, p0/m, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uadalp z0\.s, p0/m, z0\.h
[^ :]+:[0-9]+: Info:    	uadalp z0\.d, p0/m, z0\.s
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `uadalp z0\.h,p8/m,z0\.b'
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `uadalp z32\.h,p0/m,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `uadalp z0\.h,p0/m,z32\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `uaddlb z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `uaddlb z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `uaddlb z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `uaddlb z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `uaddlb z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uaddlb z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uaddlb z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uaddlb z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `uaddlt z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `uaddlt z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `uaddlt z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `uaddlt z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `uaddlt z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uaddlt z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uaddlt z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uaddlt z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `uaddwb z32\.h,z0\.h,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `uaddwb z0\.h,z32\.h,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `uaddwb z0\.h,z0\.h,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `uaddwb z0\.s,z0\.s,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `uaddwb z0\.h,z0\.h,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uaddwb z0\.h, z0\.h, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uaddwb z0\.s, z0\.s, z0\.h
[^ :]+:[0-9]+: Info:    	uaddwb z0\.d, z0\.d, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `uaddwt z32\.h,z0\.h,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `uaddwt z0\.h,z32\.h,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `uaddwt z0\.h,z0\.h,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `uaddwt z0\.s,z0\.s,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `uaddwt z0\.h,z0\.h,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uaddwt z0\.h, z0\.h, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uaddwt z0\.s, z0\.s, z0\.h
[^ :]+:[0-9]+: Info:    	uaddwt z0\.d, z0\.d, z0\.s
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `uhadd z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `uhadd z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `uhadd z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `uhadd z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `uhadd z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `uhadd z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uhadd z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uhadd z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uhadd z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	uhadd z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `uhadd z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uhadd z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uhadd z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uhadd z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	uhadd z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `uhsub z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `uhsub z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `uhsub z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `uhsub z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `uhsub z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `uhsub z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uhsub z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uhsub z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uhsub z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	uhsub z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `uhsub z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uhsub z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uhsub z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uhsub z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	uhsub z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `uhsubr z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `uhsubr z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `uhsubr z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `uhsubr z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `uhsubr z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `uhsubr z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uhsubr z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uhsubr z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uhsubr z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	uhsubr z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `uhsubr z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uhsubr z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uhsubr z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uhsubr z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	uhsubr z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `umaxp z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `umaxp z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `umaxp z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `umaxp z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `umaxp z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `umaxp z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	umaxp z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	umaxp z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	umaxp z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	umaxp z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `umaxp z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	umaxp z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	umaxp z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	umaxp z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	umaxp z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `uminp z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `uminp z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `uminp z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `uminp z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `uminp z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `uminp z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uminp z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uminp z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uminp z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	uminp z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `uminp z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uminp z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uminp z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uminp z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	uminp z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `umlalb z32\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `umlalb z0\.s,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `umlalb z0\.s,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `umlalb z0\.s,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `umlalb z0\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	umlalb z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `umlalb z32\.d,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `umlalb z0\.d,z32\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `umlalb z0\.d,z0\.s,z16\.s\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `umlalb z0\.d,z0\.s,z0\.s\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `umlalb z0\.s,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	umlalb z0\.d, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `umlalb z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `umlalb z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `umlalb z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `umlalb z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `umlalb z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	umlalb z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	umlalb z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	umlalb z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `umlalt z32\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `umlalt z0\.s,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `umlalt z0\.s,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `umlalt z0\.s,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `umlalt z0\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	umlalt z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `umlalt z32\.d,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `umlalt z0\.d,z32\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `umlalt z0\.d,z0\.s,z16\.s\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `umlalt z0\.d,z0\.s,z0\.s\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `umlalt z0\.s,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	umlalt z0\.d, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `umlalt z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `umlalt z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `umlalt z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `umlalt z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `umlalt z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	umlalt z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	umlalt z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	umlalt z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `umlslb z32\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `umlslb z0\.s,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `umlslb z0\.s,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `umlslb z0\.s,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `umlslb z0\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	umlslb z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `umlslb z32\.d,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `umlslb z0\.d,z32\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `umlslb z0\.d,z0\.s,z16\.s\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `umlslb z0\.d,z0\.s,z0\.s\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `umlslb z0\.s,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	umlslb z0\.d, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `umlslb z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `umlslb z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `umlslb z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `umlslb z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `umlslb z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	umlslb z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	umlslb z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	umlslb z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `umlslt z32\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `umlslt z0\.s,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `umlslt z0\.s,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `umlslt z0\.s,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `umlslt z0\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	umlslt z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `umlslt z32\.d,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `umlslt z0\.d,z32\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `umlslt z0\.d,z0\.s,z16\.s\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `umlslt z0\.d,z0\.s,z0\.s\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `umlslt z0\.s,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	umlslt z0\.d, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `umlslt z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `umlslt z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `umlslt z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `umlslt z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `umlslt z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	umlslt z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	umlslt z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	umlslt z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an integer register or SVE vector register at operand 1 -- `umulh z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector or predicate register at operand 2 -- `umulh z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `umulh z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `umulh z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `umulh z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	umulh z0\.h, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	umulh z0\.b, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    	umulh z0\.s, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	umulh z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `umullb z32\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `umullb z0\.s,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `umullb z0\.s,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `umullb z0\.s,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `umullb z0\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	umullb z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `umullb z32\.d,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `umullb z0\.d,z32\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `umullb z0\.d,z0\.s,z16\.s\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `umullb z0\.d,z0\.s,z0\.s\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `umullb z0\.s,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	umullb z0\.d, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `umullb z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `umullb z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `umullb z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `umullb z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `umullb z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	umullb z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	umullb z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	umullb z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `umullt z32\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `umullt z0\.s,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `umullt z0\.s,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `umullt z0\.s,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `umullt z0\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	umullt z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `umullt z32\.d,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `umullt z0\.d,z32\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `umullt z0\.d,z0\.s,z16\.s\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `umullt z0\.d,z0\.s,z0\.s\[4\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `umullt z0\.s,z0\.s,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	umullt z0\.d, z0\.s, z0\.s\[0\]
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `umullt z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `umullt z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `umullt z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `umullt z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `umullt z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	umullt z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	umullt z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	umullt z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `uqadd z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `uqadd z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `uqadd z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `uqadd z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `uqadd z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `uqadd z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uqadd z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uqadd z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uqadd z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	uqadd z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `uqadd z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uqadd z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uqadd z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uqadd z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	uqadd z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `uqrshl z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `uqrshl z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `uqrshl z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `uqrshl z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `uqrshl z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `uqrshl z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uqrshl z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uqrshl z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uqrshl z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	uqrshl z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `uqrshl z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uqrshl z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uqrshl z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uqrshl z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	uqrshl z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `uqrshlr z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `uqrshlr z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `uqrshlr z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `uqrshlr z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `uqrshlr z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `uqrshlr z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uqrshlr z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uqrshlr z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uqrshlr z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	uqrshlr z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `uqrshlr z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uqrshlr z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uqrshlr z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uqrshlr z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	uqrshlr z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `uqrshrnb z32\.b,z0\.h,#8'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `uqrshrnb z0\.b,z32\.h,#8'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `uqrshrnb z0\.b,z0\.h,#9'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `uqrshrnb z0\.b,z0\.h,#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `uqrshrnb z0\.h,z0\.h,#8'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uqrshrnb z0\.b, z0\.h, #8
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uqrshrnb z0\.h, z0\.s, #8
[^ :]+:[0-9]+: Info:    	uqrshrnb z0\.s, z0\.d, #8
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `uqrshrnb z0\.h,z0\.s,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `uqrshrnb z0\.h,z0\.s,#17'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `uqrshrnb z0\.s,z0\.d,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `uqrshrnb z0\.s,z0\.d,#33'
[^ :]+:[0-9]+: Warning: SVE `movprfx' compatible instruction expected -- `uqrshrnt z0\.b,z0\.h,#1'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `uqrshrnt z32\.b,z0\.h,#8'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `uqrshrnt z0\.b,z32\.h,#8'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `uqrshrnt z0\.b,z0\.h,#9'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `uqrshrnt z0\.b,z0\.h,#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `uqrshrnt z0\.h,z0\.h,#8'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uqrshrnt z0\.b, z0\.h, #8
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uqrshrnt z0\.h, z0\.s, #8
[^ :]+:[0-9]+: Info:    	uqrshrnt z0\.s, z0\.d, #8
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `uqrshrnt z0\.h,z0\.s,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `uqrshrnt z0\.h,z0\.s,#17'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `uqrshrnt z0\.s,z0\.d,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `uqrshrnt z0\.s,z0\.d,#33'
[^ :]+:[0-9]+: Error: operand mismatch -- `uqshl z0\.h,p0/m,z0\.b,#0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uqshl z0\.b, p0/m, z0\.b, #0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uqshl z0\.h, p0/m, z0\.h, #0
[^ :]+:[0-9]+: Info:    	uqshl z0\.s, p0/m, z0\.s, #0
[^ :]+:[0-9]+: Info:    	uqshl z0\.d, p0/m, z0\.d, #0
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `uqshl z32\.b,p0/m,z32\.b,#0'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `uqshl z0\.b,p0/m,z1\.b,#0'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `uqshl z0\.b,p8/m,z0\.b,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 7 at operand 4 -- `uqshl z0\.b,p0/m,z0\.b,#8'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 15 at operand 4 -- `uqshl z0\.h,p0/m,z0\.h,#16'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 31 at operand 4 -- `uqshl z0\.s,p0/m,z0\.s,#32'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 63 at operand 4 -- `uqshl z0\.d,p0/m,z0\.d,#64'
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `uqshl z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `uqshl z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: constant expression required at operand 4 -- `uqshl z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `uqshl z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `uqshl z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `uqshl z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uqshl z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uqshl z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uqshl z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	uqshl z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `uqshl z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uqshl z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uqshl z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uqshl z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	uqshl z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `uqshlr z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `uqshlr z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `uqshlr z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `uqshlr z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `uqshlr z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `uqshlr z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uqshlr z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uqshlr z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uqshlr z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	uqshlr z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `uqshlr z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uqshlr z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uqshlr z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uqshlr z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	uqshlr z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `uqshrnb z32\.b,z0\.h,#8'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `uqshrnb z0\.b,z32\.h,#8'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `uqshrnb z0\.b,z0\.h,#9'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `uqshrnb z0\.b,z0\.h,#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `uqshrnb z0\.h,z0\.h,#8'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uqshrnb z0\.b, z0\.h, #8
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uqshrnb z0\.h, z0\.s, #8
[^ :]+:[0-9]+: Info:    	uqshrnb z0\.s, z0\.d, #8
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `uqshrnb z0\.h,z0\.s,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `uqshrnb z0\.h,z0\.s,#17'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `uqshrnb z0\.s,z0\.d,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `uqshrnb z0\.s,z0\.d,#33'
[^ :]+:[0-9]+: Warning: SVE `movprfx' compatible instruction expected -- `uqshrnt z0\.b,z0\.h,#1'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `uqshrnt z32\.b,z0\.h,#8'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `uqshrnt z0\.b,z32\.h,#8'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `uqshrnt z0\.b,z0\.h,#9'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `uqshrnt z0\.b,z0\.h,#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `uqshrnt z0\.h,z0\.h,#8'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uqshrnt z0\.b, z0\.h, #8
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uqshrnt z0\.h, z0\.s, #8
[^ :]+:[0-9]+: Info:    	uqshrnt z0\.s, z0\.d, #8
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `uqshrnt z0\.h,z0\.s,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `uqshrnt z0\.h,z0\.s,#17'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `uqshrnt z0\.s,z0\.d,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `uqshrnt z0\.s,z0\.d,#33'
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `uqsub z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `uqsub z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `uqsub z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `uqsub z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `uqsub z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `uqsub z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uqsub z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uqsub z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uqsub z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	uqsub z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `uqsub z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uqsub z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uqsub z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uqsub z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	uqsub z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `uqsubr z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `uqsubr z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `uqsubr z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `uqsubr z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `uqsubr z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `uqsubr z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uqsubr z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uqsubr z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uqsubr z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	uqsubr z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `uqsubr z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uqsubr z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uqsubr z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uqsubr z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	uqsubr z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `uqxtnb z32\.b,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `uqxtnb z0\.b,z32\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `uqxtnb z0\.b,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uqxtnb z0\.b, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uqxtnb z0\.h, z0\.s
[^ :]+:[0-9]+: Info:    	uqxtnb z0\.s, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `uqxtnt z32\.b,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `uqxtnt z0\.b,z32\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `uqxtnt z0\.b,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uqxtnt z0\.b, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uqxtnt z0\.h, z0\.s
[^ :]+:[0-9]+: Info:    	uqxtnt z0\.s, z0\.d
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `urecpe z32\.s,p0/m,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `urecpe z0\.s,p0/m,z32\.s'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `urecpe z0\.s,p8/m,z0\.s'
[^ :]+:[0-9]+: Error: operand mismatch -- `urecpe z0\.d,p0/m,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	urecpe z0\.s, p0/m, z0\.s
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `urhadd z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `urhadd z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `urhadd z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `urhadd z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `urhadd z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `urhadd z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	urhadd z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	urhadd z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	urhadd z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	urhadd z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `urhadd z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	urhadd z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	urhadd z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	urhadd z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	urhadd z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `urshl z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `urshl z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `urshl z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `urshl z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `urshl z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `urshl z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	urshl z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	urshl z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	urshl z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	urshl z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `urshl z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	urshl z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	urshl z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	urshl z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	urshl z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `urshlr z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `urshlr z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `urshlr z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `urshlr z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `urshlr z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `urshlr z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	urshlr z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	urshlr z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	urshlr z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	urshlr z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `urshlr z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	urshlr z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	urshlr z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	urshlr z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	urshlr z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `urshr z0\.h,p0/m,z0\.b,#1'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	urshr z0\.b, p0/m, z0\.b, #1
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	urshr z0\.h, p0/m, z0\.h, #1
[^ :]+:[0-9]+: Info:    	urshr z0\.s, p0/m, z0\.s, #1
[^ :]+:[0-9]+: Info:    	urshr z0\.d, p0/m, z0\.d, #1
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `urshr z32\.b,p0/m,z32\.b,#1'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `urshr z0\.b,p0/m,z1\.b,#1'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `urshr z0\.b,p8/m,z0\.b,#1'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 4 -- `urshr z0\.b,p0/m,z0\.b,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 4 -- `urshr z0\.b,p0/m,z0\.b,#9'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 4 -- `urshr z0\.h,p0/m,z0\.h,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 4 -- `urshr z0\.h,p0/m,z0\.h,#17'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 4 -- `urshr z0\.s,p0/m,z0\.s,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 4 -- `urshr z0\.s,p0/m,z0\.s,#33'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 64 at operand 4 -- `urshr z0\.d,p0/m,z0\.d,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 64 at operand 4 -- `urshr z0\.d,p0/m,z0\.d,#65'
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `ursqrte z32\.s,p0/m,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `ursqrte z0\.s,p0/m,z32\.s'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `ursqrte z0\.s,p8/m,z0\.s'
[^ :]+:[0-9]+: Error: operand mismatch -- `ursqrte z0\.d,p0/m,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ursqrte z0\.s, p0/m, z0\.s
[^ :]+:[0-9]+: Error: operand mismatch -- `ursra z0\.h,z0\.b,#1'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ursra z0\.b, z0\.b, #1
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	ursra z0\.h, z0\.h, #1
[^ :]+:[0-9]+: Info:    	ursra z0\.s, z0\.s, #1
[^ :]+:[0-9]+: Info:    	ursra z0\.d, z0\.d, #1
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `ursra z32\.b,z0\.b,#1'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `ursra z0\.b,z32\.b,#1'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `ursra z0\.b,z0\.b,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `ursra z0\.b,z0\.b,#9'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `ursra z0\.h,z0\.h,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `ursra z0\.h,z0\.h,#17'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `ursra z0\.s,z0\.s,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `ursra z0\.s,z0\.s,#33'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 64 at operand 3 -- `ursra z0\.d,z0\.d,#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `ushllb z0\.b,z0\.b,#0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ushllb z0\.h, z0\.b, #0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	ushllb z0\.s, z0\.h, #0
[^ :]+:[0-9]+: Info:    	ushllb z0\.d, z0\.s, #0
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `ushllb z32\.h,z0\.b,#0'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `ushllb z0\.h,z32\.b,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 7 at operand 3 -- `ushllb z0\.h,z0\.b,#8'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 15 at operand 3 -- `ushllb z0\.s,z0\.h,#16'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 31 at operand 3 -- `ushllb z0\.d,z0\.s,#32'
[^ :]+:[0-9]+: Error: operand mismatch -- `ushllt z0\.b,z0\.b,#0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ushllt z0\.h, z0\.b, #0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	ushllt z0\.s, z0\.h, #0
[^ :]+:[0-9]+: Info:    	ushllt z0\.d, z0\.s, #0
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `ushllt z32\.h,z0\.b,#0'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `ushllt z0\.h,z32\.b,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 7 at operand 3 -- `ushllt z0\.h,z0\.b,#8'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 15 at operand 3 -- `ushllt z0\.s,z0\.h,#16'
[^ :]+:[0-9]+: Error: immediate value out of range 0 to 31 at operand 3 -- `ushllt z0\.d,z0\.s,#32'
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `usqadd z32\.b,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `usqadd z0\.b,p0/m,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `usqadd z0\.b,p0/m,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: operand 3 must be the same register as operand 1 -- `usqadd z0\.b,p0/m,z1\.b,z0\.b'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `usqadd z0\.b,p8/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `usqadd z0\.h,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	usqadd z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	usqadd z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	usqadd z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	usqadd z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `usqadd z0\.b,p0/z,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	usqadd z0\.b, p0/m, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	usqadd z0\.h, p0/m, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	usqadd z0\.s, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	usqadd z0\.d, p0/m, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `usra z0\.h,z0\.b,#1'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	usra z0\.b, z0\.b, #1
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	usra z0\.h, z0\.h, #1
[^ :]+:[0-9]+: Info:    	usra z0\.s, z0\.s, #1
[^ :]+:[0-9]+: Info:    	usra z0\.d, z0\.d, #1
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `usra z32\.b,z0\.b,#1'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `usra z0\.b,z32\.b,#1'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `usra z0\.b,z0\.b,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 3 -- `usra z0\.b,z0\.b,#9'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `usra z0\.h,z0\.h,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `usra z0\.h,z0\.h,#17'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `usra z0\.s,z0\.s,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `usra z0\.s,z0\.s,#33'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 64 at operand 3 -- `usra z0\.d,z0\.d,#0'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `usublb z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `usublb z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `usublb z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `usublb z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `usublb z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	usublb z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	usublb z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	usublb z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `usublt z32\.h,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `usublt z0\.h,z32\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `usublt z0\.h,z0\.b,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `usublt z0\.s,z0\.h,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `usublt z0\.h,z0\.b,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	usublt z0\.h, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	usublt z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	usublt z0\.d, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `usubwb z32\.h,z0\.h,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `usubwb z0\.h,z32\.h,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `usubwb z0\.h,z0\.h,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `usubwb z0\.s,z0\.s,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `usubwb z0\.h,z0\.h,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	usubwb z0\.h, z0\.h, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	usubwb z0\.s, z0\.s, z0\.h
[^ :]+:[0-9]+: Info:    	usubwb z0\.d, z0\.d, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `usubwt z32\.h,z0\.h,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `usubwt z0\.h,z32\.h,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `usubwt z0\.h,z0\.h,z32\.b'
[^ :]+:[0-9]+: Error: unexpected character `x' in element size at operand 3 -- `usubwt z0\.s,z0\.s,z0\.x'
[^ :]+:[0-9]+: Error: operand mismatch -- `usubwt z0\.h,z0\.h,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	usubwt z0\.h, z0\.h, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	usubwt z0\.s, z0\.s, z0\.h
[^ :]+:[0-9]+: Info:    	usubwt z0\.d, z0\.d, z0\.s
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `whilege p16\.b,x0,x0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `whilege p0\.b,x32,x0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 3 -- `whilege p0\.b,x0,x32'
[^ :]+:[0-9]+: Error: operand mismatch -- `whilege p0/m,x0,x0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	whilege p0\.b, x0, x0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	whilege p0\.h, x0, x0
[^ :]+:[0-9]+: Info:    	whilege p0\.s, x0, x0
[^ :]+:[0-9]+: Info:    	whilege p0\.d, x0, x0
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `whilege p0\.b,x31,x0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 3 -- `whilege p0\.b,x0,x31'
[^ :]+:[0-9]+: Error: operand mismatch -- `whilege p0\.b,x0,w0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	whilege p0\.b, x0, x0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	whilege p0\.h, x0, x0
[^ :]+:[0-9]+: Info:    	whilege p0\.s, x0, x0
[^ :]+:[0-9]+: Info:    	whilege p0\.d, x0, x0
[^ :]+:[0-9]+: Error: operand mismatch -- `whilege p0\.b,w0,x0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	whilege p0\.b, x0, x0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	whilege p0\.h, x0, x0
[^ :]+:[0-9]+: Info:    	whilege p0\.s, x0, x0
[^ :]+:[0-9]+: Info:    	whilege p0\.d, x0, x0
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `whilege p16\.b,w0,w0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `whilege p0\.b,w32,w0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 3 -- `whilege p0\.b,w0,w32'
[^ :]+:[0-9]+: Error: operand mismatch -- `whilege p0/m,w0,w0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	whilege p0\.b, w0, w0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	whilege p0\.h, w0, w0
[^ :]+:[0-9]+: Info:    	whilege p0\.s, w0, w0
[^ :]+:[0-9]+: Info:    	whilege p0\.d, w0, w0
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `whilege p0\.b,w31,w0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 3 -- `whilege p0\.b,w0,w31'
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `whilegt p16\.b,x0,x0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `whilegt p0\.b,x32,x0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 3 -- `whilegt p0\.b,x0,x32'
[^ :]+:[0-9]+: Error: operand mismatch -- `whilegt p0/m,x0,x0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	whilegt p0\.b, x0, x0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	whilegt p0\.h, x0, x0
[^ :]+:[0-9]+: Info:    	whilegt p0\.s, x0, x0
[^ :]+:[0-9]+: Info:    	whilegt p0\.d, x0, x0
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `whilegt p0\.b,x31,x0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 3 -- `whilegt p0\.b,x0,x31'
[^ :]+:[0-9]+: Error: operand mismatch -- `whilegt p0\.b,x0,w0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	whilegt p0\.b, x0, x0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	whilegt p0\.h, x0, x0
[^ :]+:[0-9]+: Info:    	whilegt p0\.s, x0, x0
[^ :]+:[0-9]+: Info:    	whilegt p0\.d, x0, x0
[^ :]+:[0-9]+: Error: operand mismatch -- `whilegt p0\.b,w0,x0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	whilegt p0\.b, x0, x0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	whilegt p0\.h, x0, x0
[^ :]+:[0-9]+: Info:    	whilegt p0\.s, x0, x0
[^ :]+:[0-9]+: Info:    	whilegt p0\.d, x0, x0
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `whilegt p16\.b,w0,w0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `whilegt p0\.b,w32,w0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 3 -- `whilegt p0\.b,w0,w32'
[^ :]+:[0-9]+: Error: operand mismatch -- `whilegt p0/m,w0,w0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	whilegt p0\.b, w0, w0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	whilegt p0\.h, w0, w0
[^ :]+:[0-9]+: Info:    	whilegt p0\.s, w0, w0
[^ :]+:[0-9]+: Info:    	whilegt p0\.d, w0, w0
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `whilegt p0\.b,w31,w0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 3 -- `whilegt p0\.b,w0,w31'
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `whilehi p16\.b,x0,x0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `whilehi p0\.b,x32,x0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 3 -- `whilehi p0\.b,x0,x32'
[^ :]+:[0-9]+: Error: operand mismatch -- `whilehi p0/m,x0,x0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	whilehi p0\.b, x0, x0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	whilehi p0\.h, x0, x0
[^ :]+:[0-9]+: Info:    	whilehi p0\.s, x0, x0
[^ :]+:[0-9]+: Info:    	whilehi p0\.d, x0, x0
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `whilehi p0\.b,x31,x0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 3 -- `whilehi p0\.b,x0,x31'
[^ :]+:[0-9]+: Error: operand mismatch -- `whilehi p0\.b,x0,w0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	whilehi p0\.b, x0, x0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	whilehi p0\.h, x0, x0
[^ :]+:[0-9]+: Info:    	whilehi p0\.s, x0, x0
[^ :]+:[0-9]+: Info:    	whilehi p0\.d, x0, x0
[^ :]+:[0-9]+: Error: operand mismatch -- `whilehi p0\.b,w0,x0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	whilehi p0\.b, x0, x0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	whilehi p0\.h, x0, x0
[^ :]+:[0-9]+: Info:    	whilehi p0\.s, x0, x0
[^ :]+:[0-9]+: Info:    	whilehi p0\.d, x0, x0
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `whilehi p16\.b,w0,w0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `whilehi p0\.b,w32,w0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 3 -- `whilehi p0\.b,w0,w32'
[^ :]+:[0-9]+: Error: operand mismatch -- `whilehi p0/m,w0,w0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	whilehi p0\.b, w0, w0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	whilehi p0\.h, w0, w0
[^ :]+:[0-9]+: Info:    	whilehi p0\.s, w0, w0
[^ :]+:[0-9]+: Info:    	whilehi p0\.d, w0, w0
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `whilehi p0\.b,w31,w0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 3 -- `whilehi p0\.b,w0,w31'
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `whilehs p16\.b,x0,x0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `whilehs p0\.b,x32,x0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 3 -- `whilehs p0\.b,x0,x32'
[^ :]+:[0-9]+: Error: operand mismatch -- `whilehs p0/m,x0,x0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	whilehs p0\.b, x0, x0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	whilehs p0\.h, x0, x0
[^ :]+:[0-9]+: Info:    	whilehs p0\.s, x0, x0
[^ :]+:[0-9]+: Info:    	whilehs p0\.d, x0, x0
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `whilehs p0\.b,x31,x0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 3 -- `whilehs p0\.b,x0,x31'
[^ :]+:[0-9]+: Error: operand mismatch -- `whilehs p0\.b,x0,w0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	whilehs p0\.b, x0, x0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	whilehs p0\.h, x0, x0
[^ :]+:[0-9]+: Info:    	whilehs p0\.s, x0, x0
[^ :]+:[0-9]+: Info:    	whilehs p0\.d, x0, x0
[^ :]+:[0-9]+: Error: operand mismatch -- `whilehs p0\.b,w0,x0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	whilehs p0\.b, x0, x0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	whilehs p0\.h, x0, x0
[^ :]+:[0-9]+: Info:    	whilehs p0\.s, x0, x0
[^ :]+:[0-9]+: Info:    	whilehs p0\.d, x0, x0
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `whilehs p16\.b,w0,w0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `whilehs p0\.b,w32,w0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 3 -- `whilehs p0\.b,w0,w32'
[^ :]+:[0-9]+: Error: operand mismatch -- `whilehs p0/m,w0,w0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	whilehs p0\.b, w0, w0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	whilehs p0\.h, w0, w0
[^ :]+:[0-9]+: Info:    	whilehs p0\.s, w0, w0
[^ :]+:[0-9]+: Info:    	whilehs p0\.d, w0, w0
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `whilehs p0\.b,w31,w0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 3 -- `whilehs p0\.b,w0,w31'
[^ :]+:[0-9]+: Error: operand mismatch -- `whilerw p0\.b,w0,x0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	whilerw p0\.b, x0, x0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	whilerw p0\.h, x0, x0
[^ :]+:[0-9]+: Info:    	whilerw p0\.s, x0, x0
[^ :]+:[0-9]+: Info:    	whilerw p0\.d, x0, x0
[^ :]+:[0-9]+: Error: operand mismatch -- `whilerw p0/m,x0,x0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	whilerw p0\.b, x0, x0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	whilerw p0\.h, x0, x0
[^ :]+:[0-9]+: Info:    	whilerw p0\.s, x0, x0
[^ :]+:[0-9]+: Info:    	whilerw p0\.d, x0, x0
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `whilerw p0\.b,x32,x0'
[^ :]+:[0-9]+: Error: expected an SVE predicate register at operand 1 -- `whilerw p16\.b,x0,x0'
[^ :]+:[0-9]+: Error: operand mismatch -- `whilewr p0\.b,w0,x0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	whilewr p0\.b, x0, x0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	whilewr p0\.h, x0, x0
[^ :]+:[0-9]+: Info:    	whilewr p0\.s, x0, x0
[^ :]+:[0-9]+: Info:    	whilewr p0\.d, x0, x0
[^ :]+:[0-9]+: Error: operand mismatch -- `whilewr p0/m,x0,x0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	whilewr p0\.b, x0, x0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	whilewr p0\.h, x0, x0
[^ :]+:[0-9]+: Info:    	whilewr p0\.s, x0, x0
[^ :]+:[0-9]+: Info:    	whilewr p0\.d, x0, x0
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `whilewr p0\.b,x32,x0'
[^ :]+:[0-9]+: Error: expected an SVE predicate register at operand 1 -- `whilewr p16\.b,x0,x0'
[^ :]+:[0-9]+: Error: operand mismatch -- `xar z0\.h,z0\.b,z0\.b,#1'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	xar z0\.b, z0\.b, z0\.b, #1
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	xar z0\.h, z0\.h, z0\.h, #1
[^ :]+:[0-9]+: Info:    	xar z0\.s, z0\.s, z0\.s, #1
[^ :]+:[0-9]+: Info:    	xar z0\.d, z0\.d, z0\.d, #1
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `xar z0\.b,z1\.b,z0\.b,#1'
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `xar z32\.b,z32\.b,z0\.b,#1'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `xar z0\.b,z0\.b,z32\.b,#1'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 4 -- `xar z0\.b,z0\.b,z0\.b,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 8 at operand 4 -- `xar z0\.b,z0\.b,z0\.b,#9'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 4 -- `xar z0\.h,z0\.h,z0\.h,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 4 -- `xar z0\.h,z0\.h,z0\.h,#17'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 4 -- `xar z0\.s,z0\.s,z0\.s,#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 4 -- `xar z0\.s,z0\.s,z0\.s,#33'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 64 at operand 4 -- `xar z0\.d,z0\.d,z0\.d,#0'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqshl z1\.s,p0/m,z1\.s,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqshl z1\.s, p0/m, z1\.s, z0\.s
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqshl z1\.b, p0/m, z1\.b, z0\.b
[^ :]+:[0-9]+: Info:    	sqshl z1\.h, p0/m, z1\.h, z0\.h
[^ :]+:[0-9]+: Info:    	sqshl z1\.d, p0/m, z1\.d, z0\.d
