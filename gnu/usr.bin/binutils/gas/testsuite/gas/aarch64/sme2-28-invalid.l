[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected a register at operand 1 -- `sqrshrn 0,{z0\.s-z3\.s},#1'
[^ :]+:[0-9]+: Error: expected '{' at operand 2 -- `sqrshrn z0\.b,0,#1'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `sqrshrn z0\.b,{z1\.s-z4\.s},#1'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `sqrshrn z0\.b,{z2\.s-z5\.s},#1'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `sqrshrn z0\.b,{z3\.s-z6\.s},#1'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `sqrshrn z0\.b,{z0\.s-z3\.s},#-1'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `sqrshrn z0\.b,{z0\.s-z3\.s},#0'
[^ :]+:[0-9]+: Error: immediate value out of range 1 to 32 at operand 3 -- `sqrshrn z0\.b,{z0\.s-z3\.s},#33'
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrshrn z0\.b,{z0\.d-z3\.d},#1'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrshrn z0\.b, {z0\.s-z3\.s}, #1
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqrshrn z0\.h, {z0\.d-z3\.d}, #1
[^ :]+:[0-9]+: Error: operand mismatch -- `sqrshrn z0\.b,{z0\.d-z3\.d},#65'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sqrshrn z0\.b, {z0\.s-z3\.s}, #65
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sqrshrn z0\.h, {z0\.d-z3\.d}, #65
