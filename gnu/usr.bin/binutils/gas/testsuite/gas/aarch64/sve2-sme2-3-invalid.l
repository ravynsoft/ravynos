[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `bfmlslb 0,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `bfmlslb z0\.s,0,z0\.h\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `bfmlslb z0\.s,z0\.h,0'
[^ :]+:[0-9]+: Error: z0-z7 expected at operand 3 -- `bfmlslb z0\.s,z0\.h,z8\.h\[0\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `bfmlslb z0\.s,z0\.h,z0\.h\[-1\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 7 at operand 3 -- `bfmlslb z0\.s,z0\.h,z0\.h\[8\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `bfmlslb z0\.h,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bfmlslb z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `bfmlslb z0\.d,z0\.h,z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bfmlslb z0\.s, z0\.h, z0\.h\[0\]
[^ :]+:[0-9]+: Warning: output register of preceding `movprfx' used as input at operand 2 -- `bfmlslb z0\.s,z0\.h,z1\.h\[0\]'
[^ :]+:[0-9]+: Warning: output register of preceding `movprfx' not used in current instruction at operand 1 -- `bfmlslb z0\.s,z1\.h,z2\.h\[0\]'
[^ :]+:[0-9]+: Warning: predicated instruction expected after `movprfx' -- `bfmlslb z0\.s,z1\.h,z2\.h\[0\]'
[^ :]+:[0-9]+: Warning: predicated instruction expected after `movprfx' -- `bfmlslb z0\.s,z1\.h,z2\.h\[0\]'
