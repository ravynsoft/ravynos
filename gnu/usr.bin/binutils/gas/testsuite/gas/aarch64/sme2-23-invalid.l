[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `fcvtzs 0,{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Error: expected '{' at operand 2 -- `fcvtzs {z0\.s,z1\.s},0'
[^ :]+:[0-9]+: Error: operand mismatch -- `fcvtzs {z0\.s,z1\.s},{z0\.h-z1\.h}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fcvtzs {z0\.s-z1\.s}, {z0\.s-z1\.s}
[^ :]+:[0-9]+: Error: operand mismatch -- `fcvtzs {z30\.h,z31\.h},{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fcvtzs {z30\.s-z31\.s}, {z0\.s-z1\.s}
[^ :]+:[0-9]+: Error: operand mismatch -- `fcvtzs {z0\.d,z1\.d},{z30\.d-z31\.d}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fcvtzs {z0\.s-z1\.s}, {z30\.s-z31\.s}
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `fcvtzs {z1\.s,z2\.s},{z30\.s-z31\.s}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `fcvtzs {z0\.s,z1\.s},{z29\.s-z30\.s}'
