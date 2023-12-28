#source: sve-movprfx_27.s
#as: -march=armv8-a+sve -I$srcdir/$subdir --generate-missing-build-notes=no
#objdump: -Dr -M notes
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.* file format .*

Disassembly of section .*:

0+ <.*>:
[^:]+:	0420bc20 	movprfx	z0, z1
[^:]+:	0590ce00 	fmov	z0.s, p0/m, #1.0+(e\+00)?
[^:]+:	0420bc20 	movprfx	z0, z1
[^:]+:	0590ce00 	fmov	z0.s, p0/m, #1.0+(e\+00)?
[^:]+:	d65f03c0 	ret
