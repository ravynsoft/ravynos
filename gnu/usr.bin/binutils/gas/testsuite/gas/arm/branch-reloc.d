#name: Inter-section branch relocations
#This test is only valid on EABI based ports.
#target: *-*-*eabi* *-*-nacl*
#as: -march=armv5t
#objdump: -rd
#warning_output: branch-reloc.l

# Test the generation of relocation for inter-section branches

.*: +file format.*arm.*


Disassembly of section .text:

00000000 <arm_glob_sym1-0x4>:
   0:	e1a00000 	nop			@ \(mov r0, r0\)

00000004 <arm_glob_sym1>:
   4:	ebfffffe 	bl	46 <thumb_glob_sym1>
			4: R_ARM_CALL	thumb_glob_sym1
   8:	ebfffffe 	bl	100 <thumb_glob_sym2>
			8: R_ARM_CALL	thumb_glob_sym2
   c:	fa00000c 	blx	44 <thumb_sym1>
  10:	ebfffffe 	bl	4 <arm_glob_sym1>
			10: R_ARM_CALL	arm_glob_sym1
  14:	ebfffffe 	bl	13c <arm_glob_sym2>
			14: R_ARM_CALL	arm_glob_sym2
  18:	eb000007 	bl	3c <arm_sym1>
  1c:	fafffffe 	blx	46 <thumb_glob_sym1>
			1c: R_ARM_CALL	thumb_glob_sym1
  20:	fafffffe 	blx	100 <thumb_glob_sym2>
			20: R_ARM_CALL	thumb_glob_sym2
  24:	fa000006 	blx	44 <thumb_sym1>
  28:	fafffffe 	blx	4 <arm_glob_sym1>
			28: R_ARM_CALL	arm_glob_sym1
  2c:	fafffffe 	blx	13c <arm_glob_sym2>
			2c: R_ARM_CALL	arm_glob_sym2
  30:	eb000001 	bl	3c <arm_sym1>
  34:	e1a00000 	nop			@ \(mov r0, r0\)
  38:	e12fff1e 	bx	lr

0000003c <arm_sym1>:
  3c:	e1a00000 	nop			@ \(mov r0, r0\)
  40:	e12fff1e 	bx	lr

00000044 <thumb_sym1>:
  44:	4770      	bx	lr

00000046 <thumb_glob_sym1>:
  46:	4770      	bx	lr

Disassembly of section foo:

00000000 <thumb_glob_sym2-0x100>:
	...

00000100 <thumb_glob_sym2>:
 100:	f7ff fffe 	bl	4 <thumb_glob_sym2-0xfc>
			100: R_ARM_THM_CALL	arm_glob_sym1
 104:	f7ff fffe 	bl	13c <arm_glob_sym2>
			104: R_ARM_THM_CALL	arm_glob_sym2
 108:	f000 e816 	blx	138 <arm_sym2>
 10c:	f7ff fffe 	bl	46 <thumb_glob_sym2-0xba>
			10c: R_ARM_THM_CALL	thumb_glob_sym1
 110:	f7ff fffe 	bl	100 <thumb_glob_sym2>
			110: R_ARM_THM_CALL	thumb_glob_sym2
 114:	f000 f80e 	bl	134 <thumb_sym2>
 118:	f7ff effe 	blx	4 <thumb_glob_sym2-0xfc>
			118: R_ARM_THM_CALL	arm_glob_sym1
 11c:	f7ff effe 	blx	13c <arm_glob_sym2>
			11c: R_ARM_THM_CALL	arm_glob_sym2
 120:	f000 e80a 	blx	138 <arm_sym2>
 124:	f7ff effe 	blx	46 <thumb_glob_sym2-0xba>
			124: R_ARM_THM_CALL	thumb_glob_sym1
 128:	f7ff effe 	blx	100 <thumb_glob_sym2>
			128: R_ARM_THM_CALL	thumb_glob_sym2
 12c:	f000 f802 	bl	134 <thumb_sym2>
 130:	46c0      	nop			@ \(mov r8, r8\)
 132:	4770      	bx	lr

00000134 <thumb_sym2>:
 134:	46c0      	nop			@ \(mov r8, r8\)
 136:	4770      	bx	lr

00000138 <arm_sym2>:
 138:	e12fff1e 	bx	lr

0000013c <arm_glob_sym2>:
 13c:	e12fff1e 	bx	lr
