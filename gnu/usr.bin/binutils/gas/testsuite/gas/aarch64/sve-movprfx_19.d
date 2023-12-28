#source: sve-movprfx_19.s
#as: -march=armv8-a+sve -I$srcdir/$subdir --generate-missing-build-notes=no
#objdump: -Dr -M notes
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.* file format .*

Disassembly of section .*:

0+ <.*>:
[^:]+:	2598e3e0 	ptrue	p0.s
[^:]+:	0420bc61 	movprfx	z1, z3
[^:]+:	04800441 	add	z1.s, p1/m, z1.s, z2.s
[^:]+:	d65f03c0 	ret
