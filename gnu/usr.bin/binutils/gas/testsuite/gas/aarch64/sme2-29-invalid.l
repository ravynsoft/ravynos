[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected '{' at operand 1 -- `sunpk 0,z0\.b'
[^ :]+:[0-9]+: Error: expected a register or register list at operand 2 -- `sunpk {z0\.h,z1\.h},0'
[^ :]+:[0-9]+: Error: missing braces at operand 1 -- `sunpk z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `sunpk {z0\.b,z1\.b},z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sunpk {z0\.h-z1\.h}, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sunpk {z0\.s-z1\.s}, z0\.h
[^ :]+:[0-9]+: Info:    	sunpk {z0\.d-z1\.d}, z0\.s
[^ :]+:[0-9]+: Error: operand mismatch -- `sunpk {z0\.h,z1\.h},z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sunpk {z0\.h-z1\.h}, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sunpk {z0\.s-z1\.s}, z0\.h
[^ :]+:[0-9]+: Info:    	sunpk {z0\.d-z1\.d}, z0\.s
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `sunpk {z1\.h,z2\.h},z0\.b'
[^ :]+:[0-9]+: Error: operand mismatch -- `sunpk {z0\.b,z2\.b},z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sunpk {z0\.h, z2\.h}, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sunpk {z0\.s, z2\.s}, z0\.h
[^ :]+:[0-9]+: Info:    	sunpk {z0\.d, z2\.d}, z0\.s
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 1 -- `sunpk {z1\.h-z3\.h},{z0\.b-z1\.b}'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 1 -- `sunpk {z2\.h-z4\.h},{z0\.b-z1\.b}'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 1 -- `sunpk {z3\.h-z5\.h},{z0\.b-z1\.b}'
[^ :]+:[0-9]+: Error: operand mismatch -- `sunpk {z0\.s-z3\.s},z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sunpk {z0\.h-z3\.h}, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sunpk {z0\.s-z3\.s}, z0\.h
[^ :]+:[0-9]+: Info:    	sunpk {z0\.d-z3\.d}, z0\.s
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `sunpk {z0\.s-z3\.s},{x0\.s-x1\.s}'
[^ :]+:[0-9]+: Error: operand mismatch -- `sunpk {z0\.s-z3\.s},{z0\.s-z3\.s}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sunpk {z0\.s-z3\.s}, {z0\.h-z3\.h}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sunpk {z0\.h-z3\.h}, {z0\.b-z3\.b}
[^ :]+:[0-9]+: Info:    	sunpk {z0\.d-z3\.d}, {z0\.s-z3\.s}
