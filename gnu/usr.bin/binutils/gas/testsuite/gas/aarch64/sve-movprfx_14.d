#source: sve-movprfx_14.s
#warning_output: sve-movprfx_14.l
#as: -march=armv8-a+sve -I$srcdir/$subdir --generate-missing-build-notes=no
#objdump: -Dr -M notes
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.* file format .*

Disassembly of section .*:

0+ <.*>:
[^:]+:	2598e3e0 	ptrue	p0.s
[^:]+:	04912461 	movprfx	z1.s, p1/m, z3.s
[^:]+:	0497a041 	neg	z1.s, p0/m, z2.s  // note: predicate register differs from that in preceding `movprfx' at operand 2
[^:]+:	d65f03c0 	ret

