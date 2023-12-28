[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 1 -- `sqcvtn 0,{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Error: expected '{' at operand 2 -- `sqcvtn z0\.h,0'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `sqcvtn z0\.h,{z1\.s-z2\.s}'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 2 -- `sqcvtun z0\.h,{z0\.s-z2\.s}'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 2 -- `sqcvtn z0\.h,{z0\.s-z3\.s}'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqcvtun z0\.s,{z0\.s-z3\.s}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqcvtun z0\.b, {z0\.s-z3\.s}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqcvtun z0\.h, {z0\.d-z3\.d}
[^ :]+:[0-9]+: Error: operand mismatch -- `sqcvtn z0\.s,{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqcvtn z0\.b, {z0\.s-z3\.s}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqcvtn z0\.h, {z0\.d-z3\.d}
[^ :]+:[0-9]+: Error: operand mismatch -- `sqcvtun z0\.b,{z0\.h-z1\.h}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqcvtun z0\.b, {z0\.s-z1\.s}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqcvtun z0\.h, {z0\.d-z1\.d}
[^ :]+:[0-9]+: Error: operand mismatch -- `sqcvtn z0\.s,{z0\.d-z1\.d}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqcvtn z0\.h, {z0\.d-z1\.d}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqcvtn z0\.b, {z0\.s-z1\.s}
[^ :]+:[0-9]+: Warning: SVE `movprfx' compatible instruction expected -- `sqcvtn z0\.h,{z2\.s-z3\.s}'
