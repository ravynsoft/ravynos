[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 1 -- `whilege {p0\.b},x0,x0'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `whilege {p1\.b-p2\.b},x0,x0'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 1 -- `whilege {p0\.b-p2\.b},x0,x0'
[^ :]+:[0-9]+: Error: expected a list of 2 registers at operand 1 -- `whilege {p0\.b-p3\.b},x0,x0'
[^ :]+:[0-9]+: Error: start register out of range at operand 1 -- `whilege {p15\.b-p0\.b},x0,x0'
[^ :]+:[0-9]+: Error: the register list must have a stride of 1 at operand 1 -- `whilege {p0\.b,p8\.b},x0,x0'
[^ :]+:[0-9]+: Error: expected a predicate-as-mask rather than predicate-as-counter register at operand 1 -- `whilege {pn0\.b-pn1\.b},x0,x0'
[^ :]+:[0-9]+: Error: missing type suffix at operand 1 -- `whilege {p0-p1},x0,x0'
[^ :]+:[0-9]+: Error: operand mismatch -- `whilege {p0\.q-p1\.q},x0,x0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	whilege {p0\.b-p1\.b}, x0, x0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	whilege {p0\.h-p1\.h}, x0, x0
[^ :]+:[0-9]+: Info:    	whilege {p0\.s-p1\.s}, x0, x0
[^ :]+:[0-9]+: Info:    	whilege {p0\.d-p1\.d}, x0, x0
[^ :]+:[0-9]+: Error: operand mismatch -- `whilege {p0\.b-p1\.b},w0,w0'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	whilege {p0\.b-p1\.b}, x0, x0
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	whilege {p0\.h-p1\.h}, x0, x0
[^ :]+:[0-9]+: Info:    	whilege {p0\.s-p1\.s}, x0, x0
[^ :]+:[0-9]+: Info:    	whilege {p0\.d-p1\.d}, x0, x0
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `whilege {p0\.b-p1\.b},sp,x0'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 3 -- `whilege {p0\.b-p1\.b},x0,sp'
