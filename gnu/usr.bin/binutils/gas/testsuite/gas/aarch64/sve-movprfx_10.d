#source: sve-movprfx_10.s
#warning_output: sve-movprfx_10.l
#as: -march=armv8-a+sve -I$srcdir/$subdir --generate-missing-build-notes=no
#objdump: -Dr -M notes
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.* file format .*

Disassembly of section .*:

0+ <.*>:
[^:]+:	2598e3e0 	ptrue	p0.s
[^:]+:	04912401 	movprfx	z1.s, p1/m, z0.s
[^:]+:	04d7a441 	neg	z1.d, p1/m, z2.d  // note: register size not compatible with previous `movprfx' at operand 1
[^:]+:	d65f03c0 	ret
