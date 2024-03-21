[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `rprfm pldkeep,0,\[x0\]'
[^ :]+:[0-9]+: Error: invalid addressing mode at operand 3 -- `rprfm pldkeep,x0,0'
[^ :]+:[0-9]+: Error: operand 1 must be a range prefetch operation specifier -- `rprfm pldl1keep,x0,\[x0\]'
[^ :]+:[0-9]+: Error: operand 1 must be a range prefetch operation specifier -- `rprfm #-1,x0,\[x0\]'
[^ :]+:[0-9]+: Error: operand 1 must be a range prefetch operation specifier -- `rprfm #64,x0,\[x0\]'
[^ :]+:[0-9]+: Error: expected an integer or zero register at operand 2 -- `rprfm #1,sp,\[x0\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `rprfm #1,w0,\[x0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	rprfm pstkeep, x0, \[x0\]
[^ :]+:[0-9]+: Error: invalid base register at operand 3 -- `rprfm #1,x0,\[xzr\]'
