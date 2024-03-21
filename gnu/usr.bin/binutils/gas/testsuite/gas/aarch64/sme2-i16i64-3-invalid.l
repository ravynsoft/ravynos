[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: register element index out of range 0 to 1 at operand 3 -- `sdot za\.d\[w8,0\],{z0\.h-z1\.h},z0\.h\[-1\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 1 at operand 3 -- `sdot za\.d\[w8,0\],{z0\.h-z1\.h},z0\.h\[2\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 1 at operand 3 -- `sdot za\.d\[w8,0\],{z0\.h-z3\.h},z0\.h\[-1\]'
[^ :]+:[0-9]+: Error: register element index out of range 0 to 1 at operand 3 -- `sdot za\.d\[w8,0\],{z0\.h-z3\.h},z0\.h\[2\]'
[^ :]+:[0-9]+: Error: operand mismatch -- `sudot za\.d\[w8,0\],{z0\.h-z1\.h},z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sudot za\.s\[w8, 0\], {z0\.b-z1\.b}, z0\.b\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `sudot za\.d\[w8,0\],{z0\.h-z3\.h},z0\.h\[0\]'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sudot za\.s\[w8, 0\], {z0\.b-z3\.b}, z0\.b\[0\]
[^ :]+:[0-9]+: Error: operand mismatch -- `sudot za\.d\[w8,0\],{z0\.h-z1\.h},z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sudot za\.s\[w8, 0\], {z0\.b-z1\.b}, z0\.b
[^ :]+:[0-9]+: Error: operand mismatch -- `sudot za\.d\[w8,0\],{z0\.h-z3\.h},z0\.h'
[^ :]+:[0-9]+: Info:    did you mean this\?
[^ :]+:[0-9]+: Info:    	sudot za\.s\[w8, 0\], {z0\.b-z3\.b}, z0\.b
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sudot za\.d\[w8,0\],{z0\.h-z1\.h},{z0\.h-z1\.h}'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sudot za\.d\[w8,0\],{z0\.h-z3\.h},{z0\.h-z3\.h}'
