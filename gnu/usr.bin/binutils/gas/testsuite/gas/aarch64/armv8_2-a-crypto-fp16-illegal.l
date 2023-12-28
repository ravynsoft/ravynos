[^:]+: Assembler messages:
[^:]+:[0-9]+: Error: expected a scalar SIMD or floating-point register at operand 1 -- `sha512h X0,Q0,V1.2D'
[^:]+:[0-9]+: Error: operand mismatch -- `sha512h Q0,Q1,V2.16B'
[^:]+:[0-9]+: Info:    did you mean this\?
[^:]+:[0-9]+: Info:    	sha512h q0, q1, v2.2d
[^:]+:[0-9]+: Error: expected a scalar SIMD or floating-point register at operand 1 -- `sha512h2 X0,Q0,V1.2D'
[^:]+:[0-9]+: Error: operand mismatch -- `sha512h2 Q0,Q1,V2.16B'
[^:]+:[0-9]+: Info:    did you mean this\?
[^:]+:[0-9]+: Info:    	sha512h2 q0, q1, v2.2d
[^:]+:[0-9]+: Error: operand mismatch -- `sha512su0 V1.2D,v2.16B'
[^:]+:[0-9]+: Info:    did you mean this\?
[^:]+:[0-9]+: Info:    	sha512su0 v1.2d, v2.2d
[^:]+:[0-9]+: Error: invalid use of vector register at operand 1 -- `sha512su0 V0,V2.2D'
[^:]+:[0-9]+: Error: expected an Advanced SIMD vector register at operand 1 -- `sha512su1 X0,X1,X2'
[^:]+:[0-9]+: Error: operand mismatch -- `sha512su1 V1.2D,V2.16B,V2.2D'
[^:]+:[0-9]+: Info:    did you mean this\?
[^:]+:[0-9]+: Info:    	sha512su1 v1.2d, v2.2d, v2.2d
[^:]+:[0-9]+: Error: operand mismatch -- `eor3 V1.2D,V2.2D,V3.2D,V4.2D'
[^:]+:[0-9]+: Info:    did you mean this\?
[^:]+:[0-9]+: Info:    	eor3 v1.16b, v2.16b, v3.16b, v4.16b
[^:]+:[0-9]+: Error: operand mismatch -- `rax1 V0.4S,V2.4S,V3.4S'
[^:]+:[0-9]+: Info:    did you mean this\?
[^:]+:[0-9]+: Info:    	rax1 v0.2d, v2.2d, v3.2d
[^:]+:[0-9]+: Error: immediate value out of range 0 to 63 at operand 4 -- `xar v0.2d,v1.2d,v3.2d,128'
[^:]+:[0-9]+: Error: immediate value out of range 0 to 63 at operand 4 -- `xar v0.2d,v1.2d,v3.2d,-128'
