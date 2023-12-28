[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `bfcvt 0,{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Error: expected a register or register list at operand 2 -- `bfcvt z0\.h,0'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `bfcvt z0\.h,{z1\.s-z2\.s}'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 2 -- `bfcvtn z0\.h,{z0\.s-z2\.s}'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 2 -- `bfcvt z0\.h,{z0\.s-z3\.s}'
[^ :]+:[0-9]+: Error: operand mismatch -- `bfcvtn z0\.s,{z0\.s-z3\.s}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bfcvtn z0\.h, {z0\.s-z3\.s}
[^ :]+:[0-9]+: Error: operand mismatch -- `bfcvt z0\.s,{z0\.h-z3\.h}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bfcvt z0\.h, {z0\.s-z3\.s}
[^ :]+:[0-9]+: Error: operand mismatch -- `fcvt z0\.s,{z0\.h-z1\.h}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fcvt z0\.h, {z0\.s-z1\.s}
[^ :]+:[0-9]+: Error: operand mismatch -- `fcvt z0\.s,{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fcvt z0\.h, {z0\.s-z1\.s}
[^ :]+:[0-9]+: Error: operand mismatch -- `fcvt z0\.d,{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fcvt z0\.h, {z0\.s-z1\.s}
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `fcvt z0\.h,{z1\.s-z2\.s}'
