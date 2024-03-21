# Source file used to test the add and addi instructions.
	
foo:
	add	r4,r4,r4
	addi	r4,r4,0x7fff
	addi	r4,r4,-0x8000
	addi	r4,r4,0x0
	addi	r4,r4,-0x01
	subi	r4,r4,0x01
	addi	r4,r4,0x3456

# should disassemble to add r0,0,r0
	nop
