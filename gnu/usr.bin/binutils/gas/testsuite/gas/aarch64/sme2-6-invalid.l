[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 1 -- `cntp 0,pn0\.b,vlx2'
[^ :]+:[0-9]+: Error: expected an SVE predicate register at operand 2 -- `cntp x0,0,vlx2'
[^ :]+:[0-9]+: Error: operand 3 must be VLx2 or VLx4 -- `cntp x0,pn0\.b,0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 1 -- `cntp xsp,pn0\.b,vlx2'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 1 -- `cntp sp,pn0\.b,vlx2'
[^ :]+:[0-9]+: Error: operand mismatch -- `cntp w0,pn0\.b,vlx2'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	cntp x0, pn0\.b, vlx2
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	cntp x0, pn0\.h, vlx2
[^ :]+:[0-9]+: Info:    	cntp x0, pn0\.s, vlx2
[^ :]+:[0-9]+: Info:    	cntp x0, pn0\.d, vlx2
[^ :]+:[0-9]+: Error: expected an SVE predicate register at operand 3 -- `cntp x0,p0\.b,vlx2'
[^ :]+:[0-9]+: Error: expected an SVE predicate register at operand 2 -- `cntp x0,pn16\.b,vlx2'
[^ :]+:[0-9]+: Error: operand 3 must be VLx2 or VLx4 -- `cntp x0,pn0\.b,#0'
[^ :]+:[0-9]+: Error: operand 3 must be VLx2 or VLx4 -- `cntp x0,pn0\.b,vl'
[^ :]+:[0-9]+: Error: operand 3 must be VLx2 or VLx4 -- `cntp x0,pn0\.b,vlx3'
[^ :]+:[0-9]+: Error: operand mismatch -- `cntp x0,pn0,vlx2'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	cntp x0, pn0\.b, vlx2
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	cntp x0, pn0\.h, vlx2
[^ :]+:[0-9]+: Info:    	cntp x0, pn0\.s, vlx2
[^ :]+:[0-9]+: Info:    	cntp x0, pn0\.d, vlx2
[^ :]+:[0-9]+: Error: operand mismatch -- `cntp x0,pn0\.q,vlx2'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	cntp x0, pn0\.b, vlx2
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	cntp x0, pn0\.h, vlx2
[^ :]+:[0-9]+: Info:    	cntp x0, pn0\.s, vlx2
[^ :]+:[0-9]+: Info:    	cntp x0, pn0\.d, vlx2
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `pext 0,pn8\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE predicate-as-counter register at operand 2 -- `pext p0\.b,0'
[^ :]+:[0-9]+: Error: expected a predicate-as-mask rather than predicate-as-counter register at operand 1 -- `pext pn8\.b,pn0\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE predicate register at operand 1 -- `pext z0\.b,pn8\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE predicate-as-counter register at operand 2 -- `pext p8\.b,z8\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE predicate-as-counter register at operand 2 -- `pext p8\.b,x8'
[^ :]+:[0-9]+: Error: expected a predicate-as-counter rather than predicate-as-mask register at operand 2 -- `pext p8\.b,p8\[0\]'
[^ :]+:[0-9]+: Error: missing register index at operand 2 -- `pext p8\.b,pn8'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 2 -- `pext p8\.b,pn8\[-1\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 2 -- `pext p8\.b,pn8\[4\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 3 at operand 2 -- `pext p8\.b,pn8\[1<<32\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `pext p8\.b,pn8\.b\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	pext p8\.b, pn8\[0\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	pext p8\.h, pn8\[0\]
[^ :]+:[0-9]+: Info:    	pext p8\.s, pn8\[0\]
[^ :]+:[0-9]+: Info:    	pext p8\.d, pn8\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `pext p8\.q,pn8\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	pext p8\.b, pn8\[0\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	pext p8\.h, pn8\[0\]
[^ :]+:[0-9]+: Info:    	pext p8\.s, pn8\[0\]
[^ :]+:[0-9]+: Info:    	pext p8\.d, pn8\[0\]
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 1 -- `pext {p0\.b-p2\.b},pn8\[0\]'
[^ :]+:[0-9]+: Error: missing type suffix at operand 1 -- `pext {p0-p1},pn8\[0\]'
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 2 -- `pext {p0\.b-p1\.b},pn7\[0\]'
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 2 -- `pext {p0\.b-p1\.b},pn0\[0\]'
[^ :]+:[0-9]+: Error: missing register index at operand 2 -- `pext {p0\.b-p1\.b},pn8'
[^ :]+:[0-9]+: Error: expected a predicate-as-counter rather than predicate-as-mask register at operand 2 -- `pext {p0\.b-p1\.b},p0\[0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `pext {p0\.b-p1\.b},pn8\.b\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	pext {p0\.b-p1\.b}, pn8\[0\]
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	pext {p0\.h-p1\.h}, pn8\[0\]
[^ :]+:[0-9]+: Info:    	pext {p0\.s-p1\.s}, pn8\[0\]
[^ :]+:[0-9]+: Info:    	pext {p0\.d-p1\.d}, pn8\[0\]
[^ :]+:[0-9]+: Error: register element index out of range 0 to 1 at operand 2 -- `pext {p0\.b-p1\.b},pn8\[-1\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 1 at operand 2 -- `pext {p0\.b-p1\.b},pn8\[2\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 1 at operand 2 -- `pext {p0\.b-p1\.b},pn8\[1<<32\]'
[^ :]+:[0-9]+: Error: expected an SVE predicate register at operand 1 -- `ptrue 0'
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 1 -- `ptrue pn0\.b'
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 1 -- `ptrue pn7\.b'
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 1 -- `ptrue pn0\.h'
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 1 -- `ptrue pn7\.h'
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 1 -- `ptrue pn0\.s'
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 1 -- `ptrue pn7\.s'
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 1 -- `ptrue pn0\.d'
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 1 -- `ptrue pn7\.d'
[^ :]+:[0-9]+: Error: unexpected characters following instruction at operand 1 -- `ptrue pn8\.b,all'
[^ :]+:[0-9]+: Error: operand mismatch -- `ptrue pn8\.q'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	ptrue pn8\.b
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	ptrue pn8\.h
[^ :]+:[0-9]+: Info:    	ptrue pn8\.s
[^ :]+:[0-9]+: Info:    	ptrue pn8\.d
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `sel 0,pn8,{z0\.b-z1\.b},{z0\.b-z1\.b}'
[^ :]+:[0-9]+: Error: expected an SVE predicate-as-counter register at operand 2 -- `sel {z0\.b-z1\.b},0,{z0\.b-z1\.b},{z0\.b-z1\.b}'
[^ :]+:[0-9]+: Error: expected '{' at operand 3 -- `sel {z0\.b-z1\.b},pn8,0,{z0\.b-z1\.b}'
[^ :]+:[0-9]+: Error: expected '{' at operand 4 -- `sel {z0\.b-z1\.b},pn8,{z0\.b-z1\.b},0'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `sel {z1\.b-z2\.b},pn8,{z0\.b-z1\.b},{z0\.b-z1\.b}'
[^ :]+:[0-9]+: Error: expected a predicate-as-counter rather than predicate-as-mask register at operand 2 -- `sel {z0\.b-z1\.b},p8,{z0\.b-z1\.b},{z0\.b-z1\.b}'
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 2 -- `sel {z0\.b-z1\.b},pn7,{z0\.b-z1\.b},{z0\.b-z1\.b}'
[^ :]+:[0-9]+: Error: operand mismatch -- `sel {z0\.b-z1\.b},pn8/z,{z0\.b-z1\.b},{z0\.b-z1\.b}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sel {z0\.b-z1\.b}, pn8, {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sel {z0\.h-z1\.h}, pn8, {z0\.h-z1\.h}, {z0\.h-z1\.h}
[^ :]+:[0-9]+: Info:    	sel {z0\.s-z1\.s}, pn8, {z0\.s-z1\.s}, {z0\.s-z1\.s}
[^ :]+:[0-9]+: Info:    	sel {z0\.d-z1\.d}, pn8, {z0\.d-z1\.d}, {z0\.d-z1\.d}
[^ :]+:[0-9]+: Error: operand mismatch -- `sel {z0\.b-z1\.b},pn8/m,{z0\.b-z1\.b},{z0\.b-z1\.b}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sel {z0\.b-z1\.b}, pn8, {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sel {z0\.h-z1\.h}, pn8, {z0\.h-z1\.h}, {z0\.h-z1\.h}
[^ :]+:[0-9]+: Info:    	sel {z0\.s-z1\.s}, pn8, {z0\.s-z1\.s}, {z0\.s-z1\.s}
[^ :]+:[0-9]+: Info:    	sel {z0\.d-z1\.d}, pn8, {z0\.d-z1\.d}, {z0\.d-z1\.d}
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 2 -- `sel {z0\.b-z1\.b},pn0,{z0\.b-z1\.b},{z0\.b-z1\.b}'
[^ :]+:[0-9]+: Error: start register out of range at operand 3 -- `sel {z0\.b-z1\.b},pn8,{z11\.b-z12\.b},{z0\.b-z1\.b}'
[^ :]+:[0-9]+: Error: start register out of range at operand 4 -- `sel {z0\.b-z1\.b},pn8,{z0\.b-z1\.b},{z17\.b-z18\.b}'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `sel {z1\.b-z4\.b},pn8,{z0\.b-z3\.b},{z0\.b-z3\.b}'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `sel {z10\.b-z13\.b},pn8,{z0\.b-z3\.b},{z0\.b-z3\.b}'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `sel {z15\.b-z18\.b},pn8,{z0\.b-z3\.b},{z0\.b-z3\.b}'
[^ :]+:[0-9]+: Error: start register out of range at operand 3 -- `sel {z0\.b-z3\.b},pn8,{z1\.b-z4\.b},{z0\.b-z3\.b}'
[^ :]+:[0-9]+: Error: start register out of range at operand 3 -- `sel {z0\.b-z3\.b},pn8,{z22\.b-z25\.b},{z0\.b-z3\.b}'
[^ :]+:[0-9]+: Error: start register out of range at operand 3 -- `sel {z0\.b-z3\.b},pn8,{z27\.b-z30\.b},{z0\.b-z3\.b}'
[^ :]+:[0-9]+: Error: start register out of range at operand 4 -- `sel {z0\.b-z3\.b},pn8,{z0\.b-z3\.b},{z5\.b-z8\.b}'
[^ :]+:[0-9]+: Error: start register out of range at operand 4 -- `sel {z0\.b-z3\.b},pn8,{z0\.b-z3\.b},{z14\.b-z17\.b}'
[^ :]+:[0-9]+: Error: start register out of range at operand 4 -- `sel {z0\.b-z3\.b},pn8,{z0\.b-z3\.b},{z19\.b-z22\.b}'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 3 -- `sel {z0\.b-z1\.b},pn8,{z0\.b-z3\.b},{z0\.b-z3\.b}'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 3 -- `sel {z0\.b-z3\.b},pn8,{z0\.b-z1\.b},{z0\.b-z3\.b}'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 4 -- `sel {z0\.b-z3\.b},pn8,{z0\.b-z3\.b},{z0\.b-z1\.b}'
[^ :]+:[0-9]+: Error: expected a list of 2 or 4 registers at operand 1 -- `sel {z0\.b-z2\.b},pn8,{z0\.b-z3\.b},{z0\.b-z3\.b}'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 3 -- `sel {z0\.b-z3\.b},pn8,{z0\.b-z2\.b},{z0\.b-z3\.b}'
[^ :]+:[0-9]+: Error: expected a list of 4 registers at operand 4 -- `sel {z0\.b-z3\.b},pn8,{z0\.b-z3\.b},{z0\.b-z2\.b}'
[^ :]+:[0-9]+: Error: expected a list of 2 or 4 registers at operand 1 -- `sel {z0\.b-z2\.b},pn8,{z0\.b-z1\.b},{z0\.b-z1\.b}'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 3 -- `sel {z0\.b-z1\.b},pn8,{z0\.b-z2\.b},{z0\.b-z1\.b}'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 4 -- `sel {z0\.b-z1\.b},pn8,{z0\.b-z1\.b},{z0\.b-z2\.b}'
[^ :]+:[0-9]+: Error: operand mismatch -- `sel {z0\.q-z1\.q},pn8,{z0\.q-z1\.q},{z0\.q-z1\.q}'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sel {z0\.b-z1\.b}, pn8, {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	sel {z0\.h-z1\.h}, pn8, {z0\.h-z1\.h}, {z0\.h-z1\.h}
[^ :]+:[0-9]+: Info:    	sel {z0\.s-z1\.s}, pn8, {z0\.s-z1\.s}, {z0\.s-z1\.s}
[^ :]+:[0-9]+: Info:    	sel {z0\.d-z1\.d}, pn8, {z0\.d-z1\.d}, {z0\.d-z1\.d}
