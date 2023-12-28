[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected a ZA tile at operand 1 -- `bmopa 0,p0/m,p0/m,z0\.s,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE predicate register at operand 2 -- `bmopa za0\.s,0,p0/m,z0\.s,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE predicate register at operand 3 -- `bmopa za0\.s,p0/m,0,z0\.s,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 4 -- `bmopa za0\.s,p0/m,p0/m,0,z0\.s'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 5 -- `bmopa za0\.s,p0/m,p0/m,z0\.s,0'
[^ :]+:[0-9]+: Error: operand mismatch -- `bmopa za0\.b,p0/m,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bmopa za0\.s, p0/m, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: operand mismatch -- `bmopa za0\.b,p0/m,p0/m,z0\.s,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bmopa za0\.s, p0/m, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: operand mismatch -- `bmopa za0\.s,p0/m,p0/m,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	bmopa za0\.s, p0/m, p0/m, z0\.s, z0\.s
[^ :]+:[0-9]+: Error: ZA tile number out of range at operand 1 -- `bmopa za4\.s,p0/m,p0/m,z0\.s,z0\.s'
[^ :]+:[0-9]+: Error: p0-p7 expected at operand 2 -- `bmopa za0\.s,p8/m,p0/m,z0\.s,z0\.s'
[^ :]+:[0-9]+: Error: ZA tile number out of range at operand 1 -- `bmopa za4\.s,p0/m,p8/m,z0\.s,z0\.s'
