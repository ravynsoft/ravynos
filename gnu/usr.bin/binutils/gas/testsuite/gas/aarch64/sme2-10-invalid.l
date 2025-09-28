[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `fmax 0,{z0\.h-z1\.h},z0\.h'
[^ :]+:[0-9]+: Error: expected '{' at operand 2 -- `fmax {z0\.h-z1\.h},0,z0\.h'
[^ :]+:[0-9]+: Error: expected a register or register list at operand 3 -- `fmax {z0\.h-z1\.h},{z0\.h-z1\.h},0'
[^ :]+:[0-9]+: Error: operand mismatch -- `fmax {z0\.b-z1\.b},{z0\.b-z1\.b},z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fmax {z0\.h-z1\.h}, {z0\.h-z1\.h}, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fmax {z0\.s-z1\.s}, {z0\.s-z1\.s}, z0\.s
[^ :]+:[0-9]+: Info:    	fmax {z0\.d-z1\.d}, {z0\.d-z1\.d}, z0\.d
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `fmax {z0\.h-z1\.h},{z0\.h-z2\.h},z0\.h'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `fmax {z0\.h-z1\.h},{z0\.h-z3\.h},z0\.h'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `fmax {z0\.h-z1\.h},{z0\.h,z8\.h},z0\.h'
[^ :]+:[0-9]+: Error: expected a list of 2 or 4 registers at operand 1 -- `fmax {z0\.h-z2\.h},{z0\.h-z2\.h},z0\.h'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `fmax {z0\.h-z1\.h},{z2\.h-z3\.h},z0\.h'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `fmax {z1\.h-z2\.h},{z1\.h-z2\.h},z0\.h'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `fmax {z31\.h,z0\.h},{z31\.h,z0\.h},z0\.h'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `fmax {z0\.h-z1\.h},{z0\.h-z1\.h},z16\.h'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `fmax {z0\.h-z1\.h},{z0\.h-z1\.h},z31\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `fmax {z0\.b-z3\.b},{z0\.b-z3\.b},z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fmax {z0\.h-z3\.h}, {z0\.h-z3\.h}, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fmax {z0\.s-z3\.s}, {z0\.s-z3\.s}, z0\.s
[^ :]+:[0-9]+: Info:    	fmax {z0\.d-z3\.d}, {z0\.d-z3\.d}, z0\.d
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `fmax {z0\.h-z3\.h},{z0\.h-z2\.h},z0\.h'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `fmax {z0\.h-z3\.h},{z0\.h-z1\.h},z0\.h'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `fmax {z0\.h-z3\.h},{z2\.h-z5\.h},z0\.h'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `fmax {z1\.h-z4\.h},{z1\.h-z4\.h},z0\.h'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `fmax {z2\.h-z5\.h},{z2\.h-z5\.h},z0\.h'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `fmax {z3\.h-z6\.h},{z3\.h-z6\.h},z0\.h'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `fmax {z31\.h,z0\.h,z1\.h,z2\.h},{z31\.h,z0\.h,z1\.h,z2\.h},z0\.h'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `fmax {z0\.h-z3\.h},{z0\.h-z3\.h},z16\.h'
[^ :]+:[0-9]+: Error: z0-z15 expected at operand 3 -- `fmax {z0\.h-z3\.h},{z0\.h-z3\.h},z31\.h'
[^ :]+:[0-9]+: Error: operand mismatch -- `fmax {z0\.b-z1\.b},{z0\.b-z1\.b},{z0\.b-z1\.b}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fmax {z0\.h-z1\.h}, {z0\.h-z1\.h}, {z0\.h-z1\.h}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fmax {z0\.s-z1\.s}, {z0\.s-z1\.s}, {z0\.s-z1\.s}
[^ :]+:[0-9]+: Info:    	fmax {z0\.d-z1\.d}, {z0\.d-z1\.d}, {z0\.d-z1\.d}
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `fmax {z0\.h-z1\.h},{z0\.h-z2\.h},{z0\.h-z1\.h}'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `fmax {z0\.h-z1\.h},{z0\.h-z3\.h},{z0\.h-z1\.h}'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 3 -- `fmax {z0\.h-z1\.h},{z0\.h-z1\.h},{z0\.h-z2\.h}'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 3 -- `fmax {z0\.h-z1\.h},{z0\.h-z1\.h},{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Error: expected a list of 2 or 4 registers at operand 1 -- `fmax {z0\.h-z2\.h},{z0\.h-z2\.h},{z0\.h-z1\.h}'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `fmax {z0\.h-z1\.h},{z2\.h-z3\.h},{z0\.h-z1\.h}'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `fmax {z1\.h-z2\.h},{z1\.h-z2\.h},{z0\.h-z1\.h}'
[^ :]+:[0-9]+: Error: start register out of range at operand 3 -- `fmax {z0\.h-z1\.h},{z0\.h-z1\.h},{z1\.h-z2\.h}'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `fmax {z31\.h,z0\.h},{z31\.h,z0\.h},{z0\.h-z1\.h}'
[^ :]+:[0-9]+: Error: start register out of range at operand 3 -- `fmax {z0\.h-z1\.h},{z0\.h-z1\.h},{z31\.h,z0\.h}'
[^ :]+:[0-9]+: Error: operand mismatch -- `fmax {z0\.b-z3\.b},{z0\.b-z3\.b},{z0\.b-z3\.b}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fmax {z0\.h-z3\.h}, {z0\.h-z3\.h}, {z0\.h-z3\.h}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fmax {z0\.s-z3\.s}, {z0\.s-z3\.s}, {z0\.s-z3\.s}
[^ :]+:[0-9]+: Info:    	fmax {z0\.d-z3\.d}, {z0\.d-z3\.d}, {z0\.d-z3\.d}
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `fmax {z0\.h-z3\.h},{z0\.h-z1\.h},{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `fmax {z0\.h-z3\.h},{z0\.h-z2\.h},{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 3 -- `fmax {z0\.h-z3\.h},{z0\.h-z3\.h},{z0\.h-z1\.h}'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 3 -- `fmax {z0\.h-z3\.h},{z0\.h-z3\.h},{z0\.h-z2\.h}'
[^ :]+:[0-9]+: Error: operand 2 must be the same register as operand 1 -- `fmax {z0\.h-z3\.h},{z4\.h-z7\.h},{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `fmax {z1\.h-z4\.h},{z1\.h-z4\.h},{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `fmax {z2\.h-z5\.h},{z2\.h-z5\.h},{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `fmax {z3\.h-z6\.h},{z3\.h-z6\.h},{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Error: start register out of range at operand 3 -- `fmax {z0\.h-z3\.h},{z0\.h-z3\.h},{z1\.h-z4\.h}'
[^ :]+:[0-9]+: Error: start register out of range at operand 3 -- `fmax {z0\.h-z3\.h},{z0\.h-z3\.h},{z2\.h-z5\.h}'
[^ :]+:[0-9]+: Error: start register out of range at operand 3 -- `fmax {z0\.h-z3\.h},{z0\.h-z3\.h},{z3\.h-z6\.h}'
