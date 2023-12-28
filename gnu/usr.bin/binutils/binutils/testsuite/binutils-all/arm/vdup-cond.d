#PROG: objcopy
#source vdup-cond.s
#as: -mfpu=neon
#objdump: -d
#skip: *-*-pe *-wince-* *-*-coff
#name: Check if disassembler can handle conditional neon (vdup) instructions

.*: +file format .*arm.*

Disassembly of section \.vdups:

.+ <\.vdups>:
[^:]+:	0e800b10 	vdupeq.32	d0, r0
[^:]+:	1e800b10 	vdupne.32	d0, r0
[^:]+:	2e800b10 	vdupcs.32	d0, r0
[^:]+:	3e800b10 	vdupcc.32	d0, r0
[^:]+:	4e800b10 	vdupmi.32	d0, r0
[^:]+:	5e800b10 	vduppl.32	d0, r0
[^:]+:	6e800b10 	vdupvs.32	d0, r0
[^:]+:	7e800b10 	vdupvc.32	d0, r0
[^:]+:	8e800b10 	vduphi.32	d0, r0
[^:]+:	9e800b10 	vdupls.32	d0, r0
[^:]+:	ae800b10 	vdupge.32	d0, r0
[^:]+:	be800b10 	vduplt.32	d0, r0
[^:]+:	ce800b10 	vdupgt.32	d0, r0
[^:]+:	de800b10 	vduple.32	d0, r0
[^:]+:	ee800b10 	vdup.32	d0, r0
