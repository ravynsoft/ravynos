#source: sve-movprfx_23.s
#warning_output: sve-movprfx_23.l
#as: -march=armv8-a+sve -I$srcdir/$subdir --generate-missing-build-notes=no
#objdump: -Dr -M notes
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.* file format .*

Disassembly of section .*:

0+ <.*>:
[^:]+:	04512461 	movprfx	z1.h, p1/m, z3.h
[^:]+:	256c8021 	incp	z1.h, p1.h  // note: merging predicate expected due to preceding `movprfx' at operand 2
[^:]+:	04912461 	movprfx	z1.s, p1/m, z3.s
[^:]+:	25ac8021 	incp	z1.s, p1.s  // note: merging predicate expected due to preceding `movprfx' at operand 2
[^:]+:	04d12461 	movprfx	z1.d, p1/m, z3.d
[^:]+:	25ec8021 	incp	z1.d, p1.d  // note: merging predicate expected due to preceding `movprfx' at operand 2
[^:]+:	04512461 	movprfx	z1.h, p1/m, z3.h
[^:]+:	256d8021 	decp	z1.h, p1.h  // note: merging predicate expected due to preceding `movprfx' at operand 2
[^:]+:	04912461 	movprfx	z1.s, p1/m, z3.s
[^:]+:	25ad8021 	decp	z1.s, p1.s  // note: merging predicate expected due to preceding `movprfx' at operand 2
[^:]+:	04d12461 	movprfx	z1.d, p1/m, z3.d
[^:]+:	25ed8021 	decp	z1.d, p1.d  // note: merging predicate expected due to preceding `movprfx' at operand 2
[^:]+:	04512461 	movprfx	z1.h, p1/m, z3.h
[^:]+:	25688021 	sqincp	z1.h, p1.h  // note: merging predicate expected due to preceding `movprfx' at operand 2
[^:]+:	04912461 	movprfx	z1.s, p1/m, z3.s
[^:]+:	25a88021 	sqincp	z1.s, p1.s  // note: merging predicate expected due to preceding `movprfx' at operand 2
[^:]+:	04d12461 	movprfx	z1.d, p1/m, z3.d
[^:]+:	25e88021 	sqincp	z1.d, p1.d  // note: merging predicate expected due to preceding `movprfx' at operand 2
[^:]+:	04512461 	movprfx	z1.h, p1/m, z3.h
[^:]+:	256a8021 	sqdecp	z1.h, p1.h  // note: merging predicate expected due to preceding `movprfx' at operand 2
[^:]+:	04912461 	movprfx	z1.s, p1/m, z3.s
[^:]+:	25aa8021 	sqdecp	z1.s, p1.s  // note: merging predicate expected due to preceding `movprfx' at operand 2
[^:]+:	04d12461 	movprfx	z1.d, p1/m, z3.d
[^:]+:	25ea8021 	sqdecp	z1.d, p1.d  // note: merging predicate expected due to preceding `movprfx' at operand 2
[^:]+:	04112461 	movprfx	z1.b, p1/m, z3.b
[^:]+:	05288421 	clasta	z1.b, p1, z1.b, z1.b  // note: merging predicate expected due to preceding `movprfx' at operand 2
[^:]+:	04512461 	movprfx	z1.h, p1/m, z3.h
[^:]+:	05688421 	clasta	z1.h, p1, z1.h, z1.h  // note: merging predicate expected due to preceding `movprfx' at operand 2
[^:]+:	04912461 	movprfx	z1.s, p1/m, z3.s
[^:]+:	05a88421 	clasta	z1.s, p1, z1.s, z1.s  // note: merging predicate expected due to preceding `movprfx' at operand 2
[^:]+:	04d12461 	movprfx	z1.d, p1/m, z3.d
[^:]+:	05e88421 	clasta	z1.d, p1, z1.d, z1.d  // note: merging predicate expected due to preceding `movprfx' at operand 2
[^:]+:	04112461 	movprfx	z1.b, p1/m, z3.b
[^:]+:	05298421 	clastb	z1.b, p1, z1.b, z1.b  // note: merging predicate expected due to preceding `movprfx' at operand 2
[^:]+:	04512461 	movprfx	z1.h, p1/m, z3.h
[^:]+:	05698421 	clastb	z1.h, p1, z1.h, z1.h  // note: merging predicate expected due to preceding `movprfx' at operand 2
[^:]+:	04912461 	movprfx	z1.s, p1/m, z3.s
[^:]+:	05a98421 	clastb	z1.s, p1, z1.s, z1.s  // note: merging predicate expected due to preceding `movprfx' at operand 2
[^:]+:	04d12461 	movprfx	z1.d, p1/m, z3.d
[^:]+:	05e98421 	clastb	z1.d, p1, z1.d, z1.d  // note: merging predicate expected due to preceding `movprfx' at operand 2
[^:]+:	d65f03c0 	ret
