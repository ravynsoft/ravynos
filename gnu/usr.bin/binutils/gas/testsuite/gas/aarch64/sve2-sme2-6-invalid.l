[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sqrshrn 0,{z0\.s-z1\.s},#1'
[^ :]+:[0-9]+: Error: expected '{' at operand 2 -- `sqrshrn z0\.h,0,#1'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `sqrshrn z0\.h,{z1\.s-z2\.s},#1'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 2 -- `sqrshrn z0\.h,{z0\.s-z2\.s},#1'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 2 -- `sqrshrn z0\.h,{z0\.s-z3\.s},#1'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `sqrshrn z0\.h,{z0\.s-z1\.s},#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 16 at operand 3 -- `sqrshrn z0\.h,{z0\.s-z1\.s},#17'
[^ :]+:[0-9]+: Error: immediate operand required at operand 3 -- `sqrshrn z0\.h,{z0\.s-z1\.s},x0'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrshrn z0\.b,{z0\.h-z1\.h},#1'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrshrn z0\.b, {z0\.s-z1\.s}, #1
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqrshrn z0\.h, {z0\.d-z1\.d}, #1
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrshrn z0\.s,{z0\.d-z1\.d},#1'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrshrn z0\.h, {z0\.d-z1\.d}, #1
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqrshrn z0\.b, {z0\.s-z1\.s}, #1
[^ :]+:[0-9]+: Warning: SVE `movprfx' compatible instruction expected -- `sqrshrn z0\.h,{z2\.s-z3\.s},#1'
