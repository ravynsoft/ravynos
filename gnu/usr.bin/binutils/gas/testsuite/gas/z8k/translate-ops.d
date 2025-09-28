#as:
#objdump: -dr
#name: translate-ops

.*: +file format coff-z8k

Disassembly of section \.text:

0*00000000 <\.text>:
   0:	b828 0640      	trdb	@rr2,@rr4,r6
   4:	b82c 0640      	trdrb	@rr2,@rr4,r6
   8:	b8c0 07a0      	trib	@rr12,@rr10,r7
   c:	b8c4 08a0      	trirb	@rr12,@rr10,r8
  10:	b86a 0a80      	trtdb	@rr6,@rr8,r10
  14:	b88e 034e      	trtdrb	@rr8,@rr4,r3
  18:	b8a2 0c20      	trtib	@rr10,@rr2,r12
  1c:	b826 064e      	trtirb	@rr2,@rr4,r6
