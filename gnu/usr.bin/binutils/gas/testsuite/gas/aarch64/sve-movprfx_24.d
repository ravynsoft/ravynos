#source: sve-movprfx_24.s
#warning_output: sve-movprfx_24.l
#as: -march=armv8-a+sve -I$srcdir/$subdir --generate-missing-build-notes=no
#objdump: -Dr -M notes
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.* file format .*

Disassembly of section .*:

0+ <.*>:
[^:]+:	04d12461 	movprfx	z1.d, p1/m, z3.d
[^:]+:	04db0441 	bic	z1.d, p1/m, z1.d, z2.d
[^:]+:	04d12461 	movprfx	z1.d, p1/m, z3.d
[^:]+:	04e23021 	bic	z1.d, z1.d, z2.d  // note: SVE `movprfx' compatible instruction expected
[^:]+:	0420bc61 	movprfx	z1, z3
[^:]+:	04e23021 	bic	z1.d, z1.d, z2.d  // note: SVE `movprfx' compatible instruction expected
[^:]+:	04d12461 	movprfx	z1.d, p1/m, z3.d
[^:]+:	0583e7a1 	and	z1.d, z1.d, #0xfffffffffffffff3  // note: predicated instruction expected after `movprfx'
[^:]+:	0420bc61 	movprfx	z1, z3
[^:]+:	0583e7a1 	and	z1.d, z1.d, #0xfffffffffffffff3
[^:]+:	0420bc61 	movprfx	z1, z3
[^:]+:	0543e7a1 	eor	z1.d, z1.d, #0xfffffffffffffff3
[^:]+:	0420bc61 	movprfx	z1, z3
[^:]+:	0503f021 	orr	z1.d, z1.d, #0xc
[^:]+:	d65f03c0 	ret
