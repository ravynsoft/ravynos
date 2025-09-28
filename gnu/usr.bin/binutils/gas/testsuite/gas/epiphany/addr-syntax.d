#as:
#objdump: -dr
#name: addr-syntax

.*.o:     file format elf32-epiphany


Disassembly of section \.text:

00000000 \<\.text\>:
   0:	2bcc 01ff 	ldr r1,\[r2,-0x7ff\]
   4:	4c4c 0301 	ldr r2,\[r3\],-0x8
   8:	107c 2201 	strd r8,\[r4\],\+0x8
   c:	506c 2400 	ldrd r10,\[r12,\+0x0\]
  10:	587c 2400 	strd r10,\[lr\]
