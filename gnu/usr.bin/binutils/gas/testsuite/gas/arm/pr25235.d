#skip: *-*-pe *-*-wince *-*-vxworks
#objdump: -dr
#name: PR25235: Thumb forward references error

.*: +file format .*arm.*

Disassembly of section .text:

00000000 <f1>:
   0:	46c0      	nop			@ \(mov r8, r8\)
   2:	46c0      	nop			@ \(mov r8, r8\)

00000004 <f2>:
   4:	f2af 0107 	subw	r1, pc, #7
   8:	f20f 0305 	addw	r3, pc, #5
   c:	a401      	add	r4, pc, #4	@ \(adr r4, 14 <f4>\)
   e:	46c0      	nop			@ \(mov r8, r8\)

00000010 <f3>:
  10:	46c0      	nop			@ \(mov r8, r8\)
  12:	46c0      	nop			@ \(mov r8, r8\)

00000014 <f4>:
  14:	e1a00000 	nop			@ \(mov r0, r0\)
