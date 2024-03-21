[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: operand mismatch -- `bfdot z0\.s,z1\.h,z2\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bfdot z0\.s, z1\.h, z2\.h
[^ :]+:[0-9]+: Error: operand mismatch -- `bfdot z0\.s,z1\.h,z3\.s\[3\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bfdot z0\.s, z1\.h, z3\.h\[3\]
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `bfdot z0\.s,z1\.h,z3\.h\[4\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `bfdot z0\.s,z1\.h,z8\.h\[3\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `bfmmla z0\.s,z1\.h,z2\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bfmmla z0\.s, z1\.h, z2\.h
[^ :]+:[0-9]+: Error: operand mismatch -- `bfcvt z0\.h,p1/z,z2\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bfcvt z0\.h, p1/m, z2\.s
[^ :]+:[0-9]+: Error: operand mismatch -- `bfcvt z0\.h,p1/m,z2\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bfcvt z0\.h, p1/m, z2\.s
[^ :]+:[0-9]+: Error: operand mismatch -- `bfcvtnt z0\.h,p1/z,z2\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bfcvtnt z0\.h, p1/m, z2\.s
[^ :]+:[0-9]+: Error: operand mismatch -- `bfcvtnt z0\.h,p1/m,z2\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bfcvtnt z0\.h, p1/m, z2\.s
[^ :]+:[0-9]+: Error: operand mismatch -- `bfmlalt z0\.s,z0\.h,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bfmlalt z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `bfmlalt z32\.s,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `bfmlalt z0\.s,z32\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `bfmlalt z0\.s,z0\.h,z32\.h'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `bfmlalt z0\.s,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `bfmlalt z0\.s,z0\.h,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bfmlalt z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `bfmlalt z32\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `bfmlalt z0\.s,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `bfmlalt z0\.s,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `bfmlalb z0\.s,z0\.h,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bfmlalb z0\.s, z0\.h, z0\.h
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `bfmlalb z32\.s,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `bfmlalb z0\.s,z32\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `bfmlalb z0\.s,z0\.h,z32\.h'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `bfmlalb z0\.s,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `bfmlalb z0\.s,z0\.h,z0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bfmlalb z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `bfmlalb z32\.s,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `bfmlalb z0\.s,z32\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `bfmlalb z0\.s,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `bfdot v0\.2s,v1\.4h,v2\.2s\[3\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bfdot v0\.2s, v1\.4h, v2\.2h\[3\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	bfdot v0\.4s, v1\.8h, v2\.2h\[3\]
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 3 -- `bfdot v0\.4s,v1\.8h,v2\.2h\[4\]'
[^ :]+:[0-9]+: Error: invalid element size 8 and vector size combination s at operand 3 -- `bfmmla v0\.4s,v1\.8h,v2\.8s'
[^ :]+:[0-9]+: Error: operand mismatch -- `bfmmla v0\.4s,v1\.4h,v2\.8h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bfmmla v0\.4s, v1\.8h, v2\.8h
[^ :]+:[0-9]+: Error: operand mismatch -- `bfmlalb v0\.4s,v0\.4h,v0\.8h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bfmlalb v0\.4s, v0\.8h, v0\.8h
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `bfmlalb v32\.4s,v0\.8h,v0\.8h'
[^ :]+:[0-9]+: Error: expected an Advanced SIMD vector register at operand 2 -- `bfmlalb v0\.4s,v32\.8h,v0\.8h'
[^ :]+:[0-9]+: Error: expected an Advanced SIMD vector register at operand 3 -- `bfmlalb v0\.4s,v0\.8h,v32\.8h'
[^ :]+:[0-9]+: Error: operand mismatch -- `bfmlalt v0\.4s,v0\.8h,v0\.4h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bfmlalt v0\.4s, v0\.8h, v0\.8h
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `bfmlalt v32\.4s,v0\.8h,v0\.8h'
[^ :]+:[0-9]+: Error: expected an Advanced SIMD vector register at operand 2 -- `bfmlalt v0\.4s,v32\.8h,v0\.8h'
[^ :]+:[0-9]+: Error: expected an Advanced SIMD vector register at operand 3 -- `bfmlalt v0\.4s,v0\.8h,v32\.8h'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `bfmlalb v0\.4s,v0\.8h,v0\.h\[8\]'
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `bfmlalb v32\.4s,v0\.8h,v0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an Advanced SIMD vector register at operand 2 -- `bfmlalb v0\.4s,v32\.8h,v0\.h\[0\]'
[^ :]+:[0-9]+: Error: register number out of range 0 to 15 at operand 3 -- `bfmlalb v0\.4s,v0\.8h,v16\.h\[0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `bfmlalb v0\.4s,v0\.4h,v0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bfmlalb v0\.4s, v0\.8h, v0\.h\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `bfmlalb v0\.4s,v0\.8h,v0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bfmlalb v0\.4s, v0\.8h, v0\.h\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `bfmlalt v0\.4s,v0\.8h,v0\.s\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bfmlalt v0\.4s, v0\.8h, v0\.h\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `bfmlalt v0\.4s,v0\.4h,v0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bfmlalt v0\.4s, v0\.8h, v0\.h\[0\]
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `bfmlalt v0\.4s,v0\.8h,v0\.h\[8\]'
[^ :]+:[0-9]+: Error: expected a vector register at operand 1 -- `bfmlalt v32\.4s,v0\.8h,v0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an Advanced SIMD vector register at operand 2 -- `bfmlalt v0\.4s,v32\.8h,v0\.h\[0\]'
[^ :]+:[0-9]+: Error: register number out of range 0 to 15 at operand 3 -- `bfmlalt v0\.4s,v0\.8h,v16\.h\[0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `bfcvt h0,h1'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bfcvt h0, s1
