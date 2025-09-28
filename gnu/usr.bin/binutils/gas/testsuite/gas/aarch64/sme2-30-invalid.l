[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected '{' at operand 1 -- `uzp 0,z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected a register or register list at operand 2 -- `uzp {z0\.b-z1\.b},0,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `uzp {z0\.b-z1\.b},z0\.b,0'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `uzp {z1\.b-z2\.b},z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 1 -- `uzp {z0\.b-z2\.b},z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 1 -- `uzp {z0\.b-z3\.b},z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: unexpected characters following instruction at operand 2 -- `uzp {z0\.b-z1\.b},{z0\.b-z1\.b},{z0\.b,z1\.b}'
[^ :]+:[0-9]+: Error: operand mismatch -- `uzp {z0\.h-z1\.h},z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uzp {z0\.b-z1\.b}, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uzp {z0\.h-z1\.h}, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uzp {z0\.s-z1\.s}, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	uzp {z0\.d-z1\.d}, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `uzp {z0\.q-z3\.q},z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	uzp {z0\.b-z3\.b}, z0\.b, z0\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	uzp {z0\.h-z3\.h}, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    	uzp {z0\.s-z3\.s}, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	uzp {z0\.d-z3\.d}, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: unexpected characters following instruction at operand 2 -- `uzp {z0\.b-z3\.b},{z0\.b-z1\.b},{z2\.b-z3\.b}'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `uzp {z1\.b-z4\.b},{z0\.b-z3\.b}'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `uzp {z2\.b-z5\.b},{z0\.b-z3\.b}'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `uzp {z3\.b-z6\.b},{z0\.b-z3\.b}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `uzp {z0\.b-z3\.b},{z1\.b-z4\.b}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `uzp {z0\.b-z3\.b},{z2\.b-z5\.b}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `uzp {z0\.b-z3\.b},{z3\.b-z6\.b}'
