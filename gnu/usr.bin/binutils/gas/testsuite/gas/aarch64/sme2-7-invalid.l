[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `whilege 0,x0,x0,vlx2'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `whilege pn8\.b,0,x0,vlx2'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 3 -- `whilege pn8\.b,x0,0,vlx2'
[^ :]+:[0-9]+: Error: operand 4 must be VLx2 or VLx4 -- `whilege pn8\.b,x0,x0,0'
[^ :]+:[0-9]+: Error: pn8-pn15 expected at operand 1 -- `whilege pn0\.b,x0,x0,vlx2'
[^ :]+:[0-9]+: Error: unexpected characters following instruction at operand 3 -- `whilege p8\.b,x0,x0,vlx2'
[^ :]+:[0-9]+: Error: expected an SVE predicate register at operand 1 -- `whilege z8\.b,x0,x0,vlx2'
[^ :]+:[0-9]+: Error: comma expected between operands at operand 4 -- `whilege pn8\.b,x0,x0'
[^ :]+:[0-9]+: Error: operand mismatch -- `whilege pn8\.b,w0,w0,vlx2'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	whilege pn8\.b, x0, x0, vlx2
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	whilege pn8\.h, x0, x0, vlx2
[^ :]+:[0-9]+: Info:    	whilege pn8\.s, x0, x0, vlx2
[^ :]+:[0-9]+: Info:    	whilege pn8\.d, x0, x0, vlx2
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `whilege pn8\.b,sp,x0,vlx2'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 3 -- `whilege pn8\.b,x0,sp,vlx2'
[^ :]+:[0-9]+: Error: operand 4 must be VLx2 or VLx4 -- `whilege pn8\.b,x0,x0,#0'
[^ :]+:[0-9]+: Error: operand 4 must be VLx2 or VLx4 -- `whilege pn8\.b,x0,x0,1'
