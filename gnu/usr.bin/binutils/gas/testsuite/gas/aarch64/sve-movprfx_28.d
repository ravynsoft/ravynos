#source: sve-movprfx_28.s
#warning_output: sve-movprfx_28.l
#as: -I$srcdir/$subdir --generate-missing-build-notes=no
#objdump: -Dr -M notes
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.* file format .*

Disassembly of section .*:

0+ <.*>:
[^:]+:	04912420 	movprfx	z0\.s, p1/m, z1\.s
[^:]+:	658aa440 	bfcvt	z0\.h, p1/m, z2\.s
[^:]+:	04902420 	movprfx	z0\.s, p1/z, z1\.s
[^:]+:	658aa440 	bfcvt	z0\.h, p1/m, z2\.s
[^:]+:	04512420 	movprfx	z0\.h, p1/m, z1\.h
[^:]+:	658aa440 	bfcvt	z0\.h, p1/m, z2\.s  // note: register size not compatible with previous `movprfx' at operand 1
[^:]+:	04502420 	movprfx	z0\.h, p1/z, z1\.h
[^:]+:	658aa440 	bfcvt	z0\.h, p1/m, z2\.s  // note: register size not compatible with previous `movprfx' at operand 1
[^:]+:	0420bc20 	movprfx	z0, z1
[^:]+:	658aa440 	bfcvt	z0\.h, p1/m, z2\.s
[^:]+:	0420bc20 	movprfx	z0, z1
[^:]+:	648aa440 	bfcvtnt	z0\.h, p1/m, z2\.s  // note: SVE `movprfx' compatible instruction expected
[^:]+:	04912420 	movprfx	z0\.s, p1/m, z1\.s
[^:]+:	648aa440 	bfcvtnt	z0\.h, p1/m, z2\.s  // note: SVE `movprfx' compatible instruction expected
[^:]+:	04902420 	movprfx	z0\.s, p1/z, z1\.s
[^:]+:	648aa440 	bfcvtnt	z0\.h, p1/m, z2\.s  // note: SVE `movprfx' compatible instruction expected
[^:]+:	04512420 	movprfx	z0\.h, p1/m, z1\.h
[^:]+:	648aa440 	bfcvtnt	z0\.h, p1/m, z2\.s  // note: SVE `movprfx' compatible instruction expected
[^:]+:	04502420 	movprfx	z0\.h, p1/z, z1\.h
[^:]+:	648aa440 	bfcvtnt	z0\.h, p1/m, z2\.s  // note: SVE `movprfx' compatible instruction expected
[^:]+:	d65f03c0 	ret
