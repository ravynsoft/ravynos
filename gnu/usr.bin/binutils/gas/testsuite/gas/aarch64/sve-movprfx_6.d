#source: sve-movprfx_6.s
#warning_output: sve-movprfx_6.l
#as: -march=armv8-a+sve -I$srcdir/$subdir --generate-missing-build-notes=no
#objdump: -Dr -M notes
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.* file format .*

Disassembly of section .*:

0+ <.*>:
[^:]+:	2598e3e0 	ptrue	p0.s
[^:]+:	0420bc01 	movprfx	z1, z0
[^:]+:	0420bc62 	movprfx	z2, z3  // note: instruction opens new dependency sequence without ending previous one
[^:]+:	0497a042 	neg	z2.s, p0/m, z2.s  // note: output register of preceding `movprfx' used as input at operand 3
[^:]+:	d65f03c0 	ret
