[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `fadd za\.d\[w7,0\],{z0\.d-z1\.d}'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `fadd za\.d\[w12,0\],{z0\.d-z1\.d}'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `fadd za\.d\[w8,-1\],{z0\.d-z1\.d}'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `fadd za\.d\[w8,8\],{z0\.d-z1\.d}'
[^ :]+:[0-9]+: Error: expected a list of 2 or 4 registers at operand 2 -- `fadd za\.d\[w8,0\],{z0\.d-z2\.d}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `fadd za\.d\[w8,0\],{z1\.d-z2\.d}'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `fadd za\.d\[w7,0\],{z0\.d-z3\.d}'
[^ :]+:[0-9]+: Error: expected a selection register in the range w8-w11 at operand 1 -- `fadd za\.d\[w12,0\],{z0\.d-z3\.d}'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `fadd za\.d\[w8,-1\],{z0\.d-z3\.d}'
[^ :]+:[0-9]+: Error: immediate offset out of range 0 to 7 at operand 1 -- `fadd za\.d\[w8,8\],{z1\.d-z3\.d}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `fadd za\.d\[w8,0\],{z1\.d-z4\.d}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `fadd za\.d\[w8,0\],{z2\.d-z5\.d}'
[^ :]+:[0-9]+: Error: start register out of range at operand 2 -- `fadd za\.d\[w8,0\],{z3\.d-z6\.d}'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 2 -- `fadd za\.d\[w8,0,vgx4\],{z0\.d-z1\.d}'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 2 -- `fadd za\.d\[w8,0,vgx2\],{z0\.d-z3\.d}'
[^ :]+:[0-9]+: Error: operand mismatch -- `fadd za\[w8,0\],{z0\.d-z1\.d}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fadd za\.d\[w8, 0\], {z0\.d-z1\.d}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fadd za\.s\[w8, 0\], {z0\.s-z1\.s}
[^ :]+:[0-9]+: Error: missing type suffix at operand 2 -- `fadd za\.d\[w8,0\],{z0-z1}'
[^ :]+:[0-9]+: Error: operand mismatch -- `fadd za\.d\[w8,0\],{z0\.s-z1\.s}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fadd za\.s\[w8, 0\], {z0\.s-z1\.s}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fadd za\.d\[w8, 0\], {z0\.d-z1\.d}
