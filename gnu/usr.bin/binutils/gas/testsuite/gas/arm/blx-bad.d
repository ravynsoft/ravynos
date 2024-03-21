#objdump: -drw --show-raw-insn
#name: BLX encoding
# This test is only valid on ELF based ports.
#notarget: *-*-pe *-*-wince

.*:     file format .*arm.*


Disassembly of section .text:

00000000 <ARM>:
   0:	e1a00000 	nop			@ \(mov r0, r0\)

00000004 <THUMB>:
   4:	f7ff effc 	blx	0 <ARM>
   8:	46c0      	nop			@ \(mov r8, r8\)
   a:	f7ff effa 	blx	0 <ARM>
   e:	46c0      	nop			@ \(mov r8, r8\)
  10:	f7ff eff6 	blx	0 <ARM>
  14:	f7ff eff5 			@ <UNDEFINED> instruction: 0xf7ffeff5
  18:	46c0      	nop			@ \(mov r8, r8\)
  1a:	f7ff eff1 			@ <UNDEFINED> instruction: 0xf7ffeff1
  1e:	f7ff eff0 	blx	0 <ARM>
  22:	46c0      	nop			@ \(mov r8, r8\)
