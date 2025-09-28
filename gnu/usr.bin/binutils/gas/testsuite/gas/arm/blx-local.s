# objdump: -fdrw --prefix-addresses --show-raw-insn
# notarget: *-*-pe

  .text
  .arch armv5t
  .arm
one:
        blx	foo
	blx     foo2
	bl	foo
	bl	foo2
	blx	fooundefarm
	bl      fooundefarm
	blx     fooundefthumb
	bl      fooundefthumb
	
	.thumb
	.type foo, %function
	.thumb_func
foo:
	nop
	nop
fooundefthumb:
 	nop

 	.align 2
        .type foo2, %function
	.arm
foo2:
 	bleq  fooundefthumb @no relocs
 	beq   fooundefthumb @no relocs
 	b     fooundefthumb @no relocs
 	bleq  foo  @ R_ARM_PCREL_JUMP
 	beq   foo  @ R_ARM_PCREL_JUMP
 	b     foo  @ R_ARM_PCREL_JUMP
	nop
fooundefarm:
 	nop
