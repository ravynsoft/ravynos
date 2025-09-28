#source: sve-movprfx_2.s
#warning_output: sve-movprfx_2.l
#as: -march=armv8-a+sve -I$srcdir/$subdir --generate-missing-build-notes=no
#objdump: -Dr -M notes
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.* file format .*

Disassembly of section .*:

0+ <.*>:
[^:]+:	2598e3e0 	ptrue	p0.s
[^:]+:	0420bc01 	movprfx	z1, z0
[^:]+:	0497a042 	neg	z2.s, p0/m, z2.s  // note: output register of preceding `movprfx' not used in current instruction at operand 1
[^:]+:	d65f03c0 	ret
