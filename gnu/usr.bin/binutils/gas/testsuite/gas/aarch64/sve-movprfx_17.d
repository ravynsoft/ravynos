#source: sve-movprfx_17.s
#warning_output: sve-movprfx_17.l
#as: -march=armv8-a+sve -I$srcdir/$subdir --generate-missing-build-notes=no
#objdump: -Dr -M notes
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.* file format .*

Disassembly of section .*:

0+ <.*>:
[^:]+:	2598e3e0 	ptrue	p0.s
[^:]+:	04912461 	movprfx	z1.s, p1/m, z3.s
[^:]+:	65c9a441 	fcvt	z1.d, p1/m, z2.h  // note: register size not compatible with previous `movprfx' at operand 1
[^:]+:	d65f03c0 	ret
