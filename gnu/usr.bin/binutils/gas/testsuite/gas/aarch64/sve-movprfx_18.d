#source: sve-movprfx_18.s
#warning_output: sve-movprfx_18.l
#as: -march=armv8-a+sve -I$srcdir/$subdir --generate-missing-build-notes=no
#objdump: -Dr -M notes
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.* file format .*

Disassembly of section .*:

0+ <.*>:
[^:]+:	2598e3e0 	ptrue	p0.s
[^:]+:	04912461 	movprfx	z1.s, p1/m, z3.s
[^:]+:	25a0c141 	add	z1.s, z1.s, #10  // note: predicated instruction expected after `movprfx'
[^:]+:	d65f03c0 	ret
