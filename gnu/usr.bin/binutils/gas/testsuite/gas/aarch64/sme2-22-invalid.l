[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `fclamp 0,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `fclamp {z0\.h-z1\.h},0,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `fclamp {z0\.h-z1\.h},z0\.h,0'
[^ :]+:[0-9]+: Error: operand mismatch -- `fclamp {z0\.b-z1\.b},z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fclamp {z0\.h-z1\.h}, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fclamp {z0\.s-z1\.s}, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	fclamp {z0\.d-z1\.d}, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `fclamp {z0\.b-z3\.b},z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fclamp {z0\.h-z3\.h}, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fclamp {z0\.s-z3\.s}, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	fclamp {z0\.d-z3\.d}, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `fclamp {z0\.q-z1\.q},z0\.q,z0\.q'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fclamp {z0\.h-z1\.h}, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fclamp {z0\.s-z1\.s}, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	fclamp {z0\.d-z1\.d}, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected a list of 2 or 4 registers at operand 1 -- `fclamp {z0\.h-z2\.h},z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `fclamp {z1\.h-z2\.h},z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `fclamp {z1\.h-z4\.h},z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `fclamp {z2\.h-z5\.h},z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `fclamp {z3\.h-z6\.h},z0\.h,z0\.h'
