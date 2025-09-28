#source: sve-movprfx_25.s
#warning_output: sve-movprfx_25.l
#as: -march=armv8-a+sve -I$srcdir/$subdir --generate-missing-build-notes=no
#objdump: -Dr -M notes
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.* file format .*

Disassembly of section .*:

0+ <.*>:
[^:]+:	04d12461 	movprfx	z1.d, p1/m, z3.d
[^:]+:	05d14181 	mov	z1.d, p1/m, #12
[^:]+:	04d12461 	movprfx	z1.d, p1/m, z3.d
[^:]+:	05d14001 	mov	z1.d, p1/m, #0
[^:]+:	04d12461 	movprfx	z1.d, p1/m, z3.d
[^:]+:	05d14001 	mov	z1.d, p1/m, #0
[^:]+:	04d12461 	movprfx	z1.d, p1/m, z3.d
[^:]+:	05d94181 	mov	z1.d, p9/m, #12  // note: predicate register differs from that in preceding `movprfx' at operand 2
[^:]+:	04d12461 	movprfx	z1.d, p1/m, z3.d
[^:]+:	05d10181 	mov	z1.d, p1/z, #12  // note: merging predicate expected due to preceding `movprfx' at operand 2
[^:]+:	04d12461 	movprfx	z1.d, p1/m, z3.d
[^:]+:	05e8a441 	mov	z1.d, p1/m, x2
[^:]+:	04d12461 	movprfx	z1.d, p1/m, z3.d
[^:]+:	05e8a421 	mov	z1.d, p1/m, x1
[^:]+:	04d12461 	movprfx	z1.d, p1/m, z3.d
[^:]+:	05e08441 	mov	z1.d, p1/m, d2
[^:]+:	04d12461 	movprfx	z1.d, p1/m, z3.d
[^:]+:	05e08421 	mov	z1.d, p1/m, d1  // note: output register of preceding `movprfx' used as input at operand 3
[^:]+:	d65f03c0 	ret
