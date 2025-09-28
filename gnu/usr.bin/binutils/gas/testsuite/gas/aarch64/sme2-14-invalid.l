[^ :]+: Assembler messages:
[^ :]+:[0-9]+: Error: expected a ZA array vector at operand 1 -- `sumlall 0,z0\.b,z0\.b\[0\]'
[^ :]+:[0-9]+: Error: expected a register or register list at operand 2 -- `sumlall za\.s\[w8,0:3\],0,z0\.b\[0\]'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sumlall za\.s\[w8,0:3\],z0\.b,0'
[^ :]+:[0-9]+: Error: operand 3 must be an indexed SVE vector register -- `sumlall za\.s\[w8,0:3\],z0\.b,z0\.b'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sumlall za\.s\[w8,0:3\],{z0\.b-z1\.b},{z0\.b-z1\.b}'
[^ :]+:[0-9]+: Error: expected an SVE vector register at operand 3 -- `sumlall za\.s\[w8,0:3\],{z0\.b-z3\.b},{z0\.b-z3\.b}'
