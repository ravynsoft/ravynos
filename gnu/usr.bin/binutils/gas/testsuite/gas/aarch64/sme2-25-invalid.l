[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqcvt 0,{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Error: expected '{' at operand 2 -- `sqcvt z0\.h,0'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqcvt z0\.s,{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqcvt z0\.b, {z0\.s-z1\.s}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqcvt z0\.h, {z0\.d-z1\.d}
[^ :]+:[0-9]+: Error: operand mismatch -- `sqcvt z0\.b,{z0\.d-z1\.d}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqcvt z0\.b, {z0\.s-z1\.s}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqcvt z0\.h, {z0\.d-z1\.d}
[^ :]+:[0-9]+: Error: operand mismatch -- `sqcvt z0\.s,{z0\.d-z1\.d}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqcvt z0\.h, {z0\.d-z1\.d}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqcvt z0\.b, {z0\.s-z1\.s}
[^ :]+:[0-9]+: Error: operand mismatch -- `sqcvt z0\.s,{z0\.s-z3\.s}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqcvt z0\.b, {z0\.s-z3\.s}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqcvt z0\.h, {z0\.d-z3\.d}
[^ :]+:[0-9]+: Error: operand mismatch -- `sqcvt z0\.b,{z0\.d-z3\.d}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqcvt z0\.b, {z0\.s-z3\.s}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqcvt z0\.h, {z0\.d-z3\.d}
[^ :]+:[0-9]+: Error: operand mismatch -- `sqcvt z0\.s,{z0\.d-z3\.d}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqcvt z0\.h, {z0\.d-z3\.d}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqcvt z0\.b, {z0\.s-z3\.s}
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 2 -- `sqcvt z0\.h,{z0\.s-z2\.s}'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 2 -- `sqcvt z0\.h,{z0\.s-z3\.s}'
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 at operand 2 -- `sqcvt z0\.h,{z0\.s,z8\.s}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `sqcvt z0\.h,{z1\.s-z2\.s}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `sqcvt z0\.h,{z31\.s,z0\.s}'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 2 -- `sqcvt z0\.b,{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 2 -- `sqcvt z0\.b,{z0\.s-z2\.s}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `sqcvt z0\.b,{z1\.s-z4\.s}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `sqcvt z0\.b,{z2\.s-z5\.s}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `sqcvt z0\.b,{z3\.s-z6\.s}'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 2 -- `sqcvt z0\.h,{z0\.d-z1\.d}'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 2 -- `sqcvt z0\.h,{z0\.d-z2\.d}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `sqcvt z0\.h,{z1\.d-z4\.d}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `sqcvt z0\.h,{z2\.d-z5\.d}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `sqcvt z0\.h,{z3\.d-z6\.d}'
