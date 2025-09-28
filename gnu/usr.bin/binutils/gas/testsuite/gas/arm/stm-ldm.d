#objdump: -dr --prefix-addresses --show-raw-insn
#name: STM and LDM
#warning: writeback of base register when in register list is UNPREDICTABLE

# Test the `STM*' and `LDM*' instructions

.*: +file format .*arm.*

Disassembly of section .text:
0+000 <.*> e92d0001 	stmfd	sp!, {r0}
0+004 <.*> e92d0002 	stmfd	sp!, {r1}
0+008 <.*> e92d0004 	stmfd	sp!, {r2}
0+00c <.*> e92d0008 	stmfd	sp!, {r3}
0+010 <.*> e92d0010 	stmfd	sp!, {r4}
0+014 <.*> e92d0020 	stmfd	sp!, {r5}
0+018 <.*> e92d0040 	stmfd	sp!, {r6}
0+01c <.*> e92d0080 	stmfd	sp!, {r7}
0+020 <.*> e92d0100 	stmfd	sp!, {r8}
0+024 <.*> e92d0200 	stmfd	sp!, {r9}
0+028 <.*> e92d0400 	stmfd	sp!, {sl}
0+02c <.*> e92d0800 	stmfd	sp!, {fp}
0+030 <.*> e92d1000 	stmfd	sp!, {ip}
0+034 <.*> e92d2000 	stmfd	sp!, {sp}
0+038 <.*> e92d4000 	stmfd	sp!, {lr}
0+03c <.*> e92d8000 	stmfd	sp!, {pc}
0+040 <.*> e92d000e 	push	{r1, r2, r3}
0+044 <.*> e8bd000e 	pop	{r1, r2, r3}
0+048 <.*> e8bd0001 	ldmfd	sp!, {r0}
0+04c <.*> e8bd0002 	ldmfd	sp!, {r1}
0+050 <.*> e8bd0004 	ldmfd	sp!, {r2}
0+054 <.*> e8bd0008 	ldmfd	sp!, {r3}
0+058 <.*> e8bd0010 	ldmfd	sp!, {r4}
0+05c <.*> e8bd0020 	ldmfd	sp!, {r5}
0+060 <.*> e8bd0040 	ldmfd	sp!, {r6}
0+064 <.*> e8bd0080 	ldmfd	sp!, {r7}
0+068 <.*> e8bd0100 	ldmfd	sp!, {r8}
0+06c <.*> e8bd0200 	ldmfd	sp!, {r9}
0+070 <.*> e8bd0400 	ldmfd	sp!, {sl}
0+074 <.*> e8bd0800 	ldmfd	sp!, {fp}
0+078 <.*> e8bd1000 	ldmfd	sp!, {ip}
0+07c <.*> e8bd2000 	ldmfd	sp!, {sp}
0+080 <.*> e8bd4000 	ldmfd	sp!, {lr}
0+084 <.*> e8bd8000 	ldmfd	sp!, {pc}
