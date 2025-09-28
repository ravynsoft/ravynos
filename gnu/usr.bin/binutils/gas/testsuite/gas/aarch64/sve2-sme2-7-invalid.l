[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected a register or register list at operand 1 -- `fclamp 0,z0\.h,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `fclamp z0\.h,0,z0\.h'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `fclamp z0\.h,z0\.h,0'
[^ :]+:[0-9]+: Error: operand mismatch -- `fclamp z0\.b,z0\.b,z0\.b'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fclamp z0\.h, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fclamp z0\.s, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	fclamp z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 2 -- `fclamp z0\.h,{z0\.h,z0\.h}'
[^ :]+:[0-9]+: Error: operand mismatch -- `fclamp z0\.s,z0\.h,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fclamp z0\.h, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fclamp z0\.s, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	fclamp z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `fclamp z0\.h,z0\.s,z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fclamp z0\.h, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fclamp z0\.s, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	fclamp z0\.d, z0\.d, z0\.d
[^ :]+:[0-9]+: Error: operand mismatch -- `fclamp z0\.h,z0\.h,z0\.s'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	fclamp z0\.h, z0\.h, z0\.h
[^ :]+:[0-9]+: Info:    other valid variant\(s\):
[^ :]+:[0-9]+: Info:    	fclamp z0\.s, z0\.s, z0\.s
[^ :]+:[0-9]+: Info:    	fclamp z0\.d, z0\.d, z0\.d
