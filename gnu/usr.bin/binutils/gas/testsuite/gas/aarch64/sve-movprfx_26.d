#source: sve-movprfx_26.s
#warning_output: sve-movprfx_26.l
#as: -march=armv8-a+sve -I$srcdir/$subdir --generate-missing-build-notes=no
#objdump: -Dr -M notes
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.* file format .*

Disassembly of section .*:

0+ <.*>:
[^:]+:	04912420 	movprfx	z0.s, p1/m, z1.s
[^:]+:	65caa440 	fcvt	z0.s, p1/m, z2.d  // note: register size not compatible with previous `movprfx' at operand 1
[^:]+:	04d12420 	movprfx	z0.d, p1/m, z1.d
[^:]+:	65caa440 	fcvt	z0.s, p1/m, z2.d
[^:]+:	04912420 	movprfx	z0.s, p1/m, z1.s
[^:]+:	65cba440 	fcvt	z0.d, p1/m, z2.s  // note: register size not compatible with previous `movprfx' at operand 1
[^:]+:	04d12420 	movprfx	z0.d, p1/m, z1.d
[^:]+:	65cba440 	fcvt	z0.d, p1/m, z2.s
[^:]+:	04912420 	movprfx	z0.s, p1/m, z1.s
[^:]+:	65d8a440 	fcvtzs	z0.s, p1/m, z2.d  // note: register size not compatible with previous `movprfx' at operand 1
[^:]+:	04d12420 	movprfx	z0.d, p1/m, z1.d
[^:]+:	65d8a440 	fcvtzs	z0.s, p1/m, z2.d
[^:]+:	04912420 	movprfx	z0.s, p1/m, z1.s
[^:]+:	65dca440 	fcvtzs	z0.d, p1/m, z2.s  // note: register size not compatible with previous `movprfx' at operand 1
[^:]+:	04d12420 	movprfx	z0.d, p1/m, z1.d
[^:]+:	65dca440 	fcvtzs	z0.d, p1/m, z2.s
[^:]+:	04912420 	movprfx	z0.s, p1/m, z1.s
[^:]+:	65d9a440 	fcvtzu	z0.s, p1/m, z2.d  // note: register size not compatible with previous `movprfx' at operand 1
[^:]+:	04d12420 	movprfx	z0.d, p1/m, z1.d
[^:]+:	65d9a440 	fcvtzu	z0.s, p1/m, z2.d
[^:]+:	04912420 	movprfx	z0.s, p1/m, z1.s
[^:]+:	65dda440 	fcvtzu	z0.d, p1/m, z2.s  // note: register size not compatible with previous `movprfx' at operand 1
[^:]+:	04d12420 	movprfx	z0.d, p1/m, z1.d
[^:]+:	65dda440 	fcvtzu	z0.d, p1/m, z2.s
[^:]+:	04912420 	movprfx	z0.s, p1/m, z1.s
[^:]+:	65d4a440 	scvtf	z0.s, p1/m, z2.d  // note: register size not compatible with previous `movprfx' at operand 1
[^:]+:	04d12420 	movprfx	z0.d, p1/m, z1.d
[^:]+:	65d4a440 	scvtf	z0.s, p1/m, z2.d
[^:]+:	04912420 	movprfx	z0.s, p1/m, z1.s
[^:]+:	65d0a440 	scvtf	z0.d, p1/m, z2.s  // note: register size not compatible with previous `movprfx' at operand 1
[^:]+:	04d12420 	movprfx	z0.d, p1/m, z1.d
[^:]+:	65d0a440 	scvtf	z0.d, p1/m, z2.s
[^:]+:	04912420 	movprfx	z0.s, p1/m, z1.s
[^:]+:	65d5a440 	ucvtf	z0.s, p1/m, z2.d  // note: register size not compatible with previous `movprfx' at operand 1
[^:]+:	04d12420 	movprfx	z0.d, p1/m, z1.d
[^:]+:	65d5a440 	ucvtf	z0.s, p1/m, z2.d
[^:]+:	04912420 	movprfx	z0.s, p1/m, z1.s
[^:]+:	65d1a440 	ucvtf	z0.d, p1/m, z2.s  // note: register size not compatible with previous `movprfx' at operand 1
[^:]+:	04d12420 	movprfx	z0.d, p1/m, z1.d
[^:]+:	65d1a440 	ucvtf	z0.d, p1/m, z2.s
[^:]+:	04112420 	movprfx	z0.b, p1/m, z1.b
[^:]+:	041b8440 	lsl	z0.b, p1/m, z0.b, z2.d
[^:]+:	04d12420 	movprfx	z0.d, p1/m, z1.d
[^:]+:	041b8440 	lsl	z0.b, p1/m, z0.b, z2.d  // note: register size not compatible with previous `movprfx' at operand 1
[^:]+:	04112420 	movprfx	z0.b, p1/m, z1.b
[^:]+:	04198440 	lsr	z0.b, p1/m, z0.b, z2.d
[^:]+:	04d12420 	movprfx	z0.d, p1/m, z1.d
[^:]+:	04198440 	lsr	z0.b, p1/m, z0.b, z2.d  // note: register size not compatible with previous `movprfx' at operand 1
[^:]+:	04112420 	movprfx	z0.b, p1/m, z1.b
[^:]+:	04188440 	asr	z0.b, p1/m, z0.b, z2.d
[^:]+:	04d12420 	movprfx	z0.d, p1/m, z1.d
[^:]+:	04188440 	asr	z0.b, p1/m, z0.b, z2.d  // note: register size not compatible with previous `movprfx' at operand 1
[^:]+:	d65f03c0 	ret
