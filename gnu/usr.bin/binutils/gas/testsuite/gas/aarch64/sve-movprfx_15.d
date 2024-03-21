#source: sve-movprfx_15.s
#warning_output: sve-movprfx_15.l
#as: -march=armv8-a+sve -I$srcdir/$subdir --generate-missing-build-notes=no
#objdump: -Dr -M notes
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.* file format .*

Disassembly of section .*:

0+ <.*>:
[^:]+:	2598e3e0 	ptrue	p0.s
[^:]+:	04912461 	movprfx	z1.s, p1/m, z3.s
[^:]+:	04623061 	orr	z1.d, z3.d, z2.d  // note: SVE `movprfx' compatible instruction expected
[^:]+:	d65f03c0 	ret
