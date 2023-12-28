# Check 32bit HLE instructions

	.allow_index_reg
	.text
_start:

# Tests for op imm8 regb/m8
	xacquire lock adcb $100,(%ecx)
	lock xacquire adcb $100,(%ecx)
	xrelease lock adcb $100,(%ecx)
	lock xrelease adcb $100,(%ecx)
	.byte 0xf0; .byte 0xf2; adcb $100,(%ecx)
	.byte 0xf0; .byte 0xf3; adcb $100,(%ecx)
	xacquire lock addb $100,(%ecx)
	lock xacquire addb $100,(%ecx)
	xrelease lock addb $100,(%ecx)
	lock xrelease addb $100,(%ecx)
	.byte 0xf0; .byte 0xf2; addb $100,(%ecx)
	.byte 0xf0; .byte 0xf3; addb $100,(%ecx)
	xacquire lock andb $100,(%ecx)
	lock xacquire andb $100,(%ecx)
	xrelease lock andb $100,(%ecx)
	lock xrelease andb $100,(%ecx)
	.byte 0xf0; .byte 0xf2; andb $100,(%ecx)
	.byte 0xf0; .byte 0xf3; andb $100,(%ecx)
	xrelease movb $100,(%ecx)
	xacquire lock orb $100,(%ecx)
	lock xacquire orb $100,(%ecx)
	xrelease lock orb $100,(%ecx)
	lock xrelease orb $100,(%ecx)
	.byte 0xf0; .byte 0xf2; orb $100,(%ecx)
	.byte 0xf0; .byte 0xf3; orb $100,(%ecx)
	xacquire lock sbbb $100,(%ecx)
	lock xacquire sbbb $100,(%ecx)
	xrelease lock sbbb $100,(%ecx)
	lock xrelease sbbb $100,(%ecx)
	.byte 0xf0; .byte 0xf2; sbbb $100,(%ecx)
	.byte 0xf0; .byte 0xf3; sbbb $100,(%ecx)
	xacquire lock subb $100,(%ecx)
	lock xacquire subb $100,(%ecx)
	xrelease lock subb $100,(%ecx)
	lock xrelease subb $100,(%ecx)
	.byte 0xf0; .byte 0xf2; subb $100,(%ecx)
	.byte 0xf0; .byte 0xf3; subb $100,(%ecx)
	xacquire lock xorb $100,(%ecx)
	lock xacquire xorb $100,(%ecx)
	xrelease lock xorb $100,(%ecx)
	lock xrelease xorb $100,(%ecx)
	.byte 0xf0; .byte 0xf2; xorb $100,(%ecx)
	.byte 0xf0; .byte 0xf3; xorb $100,(%ecx)

# Tests for op imm16 regs/m16
	xacquire lock adcw $1000,(%ecx)
	lock xacquire adcw $1000,(%ecx)
	xrelease lock adcw $1000,(%ecx)
	lock xrelease adcw $1000,(%ecx)
	.byte 0xf0; .byte 0xf2; adcw $1000,(%ecx)
	.byte 0xf0; .byte 0xf3; adcw $1000,(%ecx)
	xacquire lock addw $1000,(%ecx)
	lock xacquire addw $1000,(%ecx)
	xrelease lock addw $1000,(%ecx)
	lock xrelease addw $1000,(%ecx)
	.byte 0xf0; .byte 0xf2; addw $1000,(%ecx)
	.byte 0xf0; .byte 0xf3; addw $1000,(%ecx)
	xacquire lock andw $1000,(%ecx)
	lock xacquire andw $1000,(%ecx)
	xrelease lock andw $1000,(%ecx)
	lock xrelease andw $1000,(%ecx)
	.byte 0xf0; .byte 0xf2; andw $1000,(%ecx)
	.byte 0xf0; .byte 0xf3; andw $1000,(%ecx)
	xrelease movw $1000,(%ecx)
	xacquire lock orw $1000,(%ecx)
	lock xacquire orw $1000,(%ecx)
	xrelease lock orw $1000,(%ecx)
	lock xrelease orw $1000,(%ecx)
	.byte 0xf0; .byte 0xf2; orw $1000,(%ecx)
	.byte 0xf0; .byte 0xf3; orw $1000,(%ecx)
	xacquire lock sbbw $1000,(%ecx)
	lock xacquire sbbw $1000,(%ecx)
	xrelease lock sbbw $1000,(%ecx)
	lock xrelease sbbw $1000,(%ecx)
	.byte 0xf0; .byte 0xf2; sbbw $1000,(%ecx)
	.byte 0xf0; .byte 0xf3; sbbw $1000,(%ecx)
	xacquire lock subw $1000,(%ecx)
	lock xacquire subw $1000,(%ecx)
	xrelease lock subw $1000,(%ecx)
	lock xrelease subw $1000,(%ecx)
	.byte 0xf0; .byte 0xf2; subw $1000,(%ecx)
	.byte 0xf0; .byte 0xf3; subw $1000,(%ecx)
	xacquire lock xorw $1000,(%ecx)
	lock xacquire xorw $1000,(%ecx)
	xrelease lock xorw $1000,(%ecx)
	lock xrelease xorw $1000,(%ecx)
	.byte 0xf0; .byte 0xf2; xorw $1000,(%ecx)
	.byte 0xf0; .byte 0xf3; xorw $1000,(%ecx)

# Tests for op imm32 regl/m32
	xacquire lock adcl $10000000,(%ecx)
	lock xacquire adcl $10000000,(%ecx)
	xrelease lock adcl $10000000,(%ecx)
	lock xrelease adcl $10000000,(%ecx)
	.byte 0xf0; .byte 0xf2; adcl $10000000,(%ecx)
	.byte 0xf0; .byte 0xf3; adcl $10000000,(%ecx)
	xacquire lock addl $10000000,(%ecx)
	lock xacquire addl $10000000,(%ecx)
	xrelease lock addl $10000000,(%ecx)
	lock xrelease addl $10000000,(%ecx)
	.byte 0xf0; .byte 0xf2; addl $10000000,(%ecx)
	.byte 0xf0; .byte 0xf3; addl $10000000,(%ecx)
	xacquire lock andl $10000000,(%ecx)
	lock xacquire andl $10000000,(%ecx)
	xrelease lock andl $10000000,(%ecx)
	lock xrelease andl $10000000,(%ecx)
	.byte 0xf0; .byte 0xf2; andl $10000000,(%ecx)
	.byte 0xf0; .byte 0xf3; andl $10000000,(%ecx)
	xrelease movl $10000000,(%ecx)
	xacquire lock orl $10000000,(%ecx)
	lock xacquire orl $10000000,(%ecx)
	xrelease lock orl $10000000,(%ecx)
	lock xrelease orl $10000000,(%ecx)
	.byte 0xf0; .byte 0xf2; orl $10000000,(%ecx)
	.byte 0xf0; .byte 0xf3; orl $10000000,(%ecx)
	xacquire lock sbbl $10000000,(%ecx)
	lock xacquire sbbl $10000000,(%ecx)
	xrelease lock sbbl $10000000,(%ecx)
	lock xrelease sbbl $10000000,(%ecx)
	.byte 0xf0; .byte 0xf2; sbbl $10000000,(%ecx)
	.byte 0xf0; .byte 0xf3; sbbl $10000000,(%ecx)
	xacquire lock subl $10000000,(%ecx)
	lock xacquire subl $10000000,(%ecx)
	xrelease lock subl $10000000,(%ecx)
	lock xrelease subl $10000000,(%ecx)
	.byte 0xf0; .byte 0xf2; subl $10000000,(%ecx)
	.byte 0xf0; .byte 0xf3; subl $10000000,(%ecx)
	xacquire lock xorl $10000000,(%ecx)
	lock xacquire xorl $10000000,(%ecx)
	xrelease lock xorl $10000000,(%ecx)
	lock xrelease xorl $10000000,(%ecx)
	.byte 0xf0; .byte 0xf2; xorl $10000000,(%ecx)
	.byte 0xf0; .byte 0xf3; xorl $10000000,(%ecx)

# Tests for op imm8 regs/m16
	xacquire lock adcw $100,(%ecx)
	lock xacquire adcw $100,(%ecx)
	xrelease lock adcw $100,(%ecx)
	lock xrelease adcw $100,(%ecx)
	.byte 0xf0; .byte 0xf2; adcw $100,(%ecx)
	.byte 0xf0; .byte 0xf3; adcw $100,(%ecx)
	xacquire lock addw $100,(%ecx)
	lock xacquire addw $100,(%ecx)
	xrelease lock addw $100,(%ecx)
	lock xrelease addw $100,(%ecx)
	.byte 0xf0; .byte 0xf2; addw $100,(%ecx)
	.byte 0xf0; .byte 0xf3; addw $100,(%ecx)
	xacquire lock andw $100,(%ecx)
	lock xacquire andw $100,(%ecx)
	xrelease lock andw $100,(%ecx)
	lock xrelease andw $100,(%ecx)
	.byte 0xf0; .byte 0xf2; andw $100,(%ecx)
	.byte 0xf0; .byte 0xf3; andw $100,(%ecx)
	xacquire lock btcw $100,(%ecx)
	lock xacquire btcw $100,(%ecx)
	xrelease lock btcw $100,(%ecx)
	lock xrelease btcw $100,(%ecx)
	.byte 0xf0; .byte 0xf2; btcw $100,(%ecx)
	.byte 0xf0; .byte 0xf3; btcw $100,(%ecx)
	xacquire lock btrw $100,(%ecx)
	lock xacquire btrw $100,(%ecx)
	xrelease lock btrw $100,(%ecx)
	lock xrelease btrw $100,(%ecx)
	.byte 0xf0; .byte 0xf2; btrw $100,(%ecx)
	.byte 0xf0; .byte 0xf3; btrw $100,(%ecx)
	xacquire lock btsw $100,(%ecx)
	lock xacquire btsw $100,(%ecx)
	xrelease lock btsw $100,(%ecx)
	lock xrelease btsw $100,(%ecx)
	.byte 0xf0; .byte 0xf2; btsw $100,(%ecx)
	.byte 0xf0; .byte 0xf3; btsw $100,(%ecx)
	xrelease movw $100,(%ecx)
	xacquire lock orw $100,(%ecx)
	lock xacquire orw $100,(%ecx)
	xrelease lock orw $100,(%ecx)
	lock xrelease orw $100,(%ecx)
	.byte 0xf0; .byte 0xf2; orw $100,(%ecx)
	.byte 0xf0; .byte 0xf3; orw $100,(%ecx)
	xacquire lock sbbw $100,(%ecx)
	lock xacquire sbbw $100,(%ecx)
	xrelease lock sbbw $100,(%ecx)
	lock xrelease sbbw $100,(%ecx)
	.byte 0xf0; .byte 0xf2; sbbw $100,(%ecx)
	.byte 0xf0; .byte 0xf3; sbbw $100,(%ecx)
	xacquire lock subw $100,(%ecx)
	lock xacquire subw $100,(%ecx)
	xrelease lock subw $100,(%ecx)
	lock xrelease subw $100,(%ecx)
	.byte 0xf0; .byte 0xf2; subw $100,(%ecx)
	.byte 0xf0; .byte 0xf3; subw $100,(%ecx)
	xacquire lock xorw $100,(%ecx)
	lock xacquire xorw $100,(%ecx)
	xrelease lock xorw $100,(%ecx)
	lock xrelease xorw $100,(%ecx)
	.byte 0xf0; .byte 0xf2; xorw $100,(%ecx)
	.byte 0xf0; .byte 0xf3; xorw $100,(%ecx)

# Tests for op imm8 regl/m32
	xacquire lock adcl $100,(%ecx)
	lock xacquire adcl $100,(%ecx)
	xrelease lock adcl $100,(%ecx)
	lock xrelease adcl $100,(%ecx)
	.byte 0xf0; .byte 0xf2; adcl $100,(%ecx)
	.byte 0xf0; .byte 0xf3; adcl $100,(%ecx)
	xacquire lock addl $100,(%ecx)
	lock xacquire addl $100,(%ecx)
	xrelease lock addl $100,(%ecx)
	lock xrelease addl $100,(%ecx)
	.byte 0xf0; .byte 0xf2; addl $100,(%ecx)
	.byte 0xf0; .byte 0xf3; addl $100,(%ecx)
	xacquire lock andl $100,(%ecx)
	lock xacquire andl $100,(%ecx)
	xrelease lock andl $100,(%ecx)
	lock xrelease andl $100,(%ecx)
	.byte 0xf0; .byte 0xf2; andl $100,(%ecx)
	.byte 0xf0; .byte 0xf3; andl $100,(%ecx)
	xacquire lock btcl $100,(%ecx)
	lock xacquire btcl $100,(%ecx)
	xrelease lock btcl $100,(%ecx)
	lock xrelease btcl $100,(%ecx)
	.byte 0xf0; .byte 0xf2; btcl $100,(%ecx)
	.byte 0xf0; .byte 0xf3; btcl $100,(%ecx)
	xacquire lock btrl $100,(%ecx)
	lock xacquire btrl $100,(%ecx)
	xrelease lock btrl $100,(%ecx)
	lock xrelease btrl $100,(%ecx)
	.byte 0xf0; .byte 0xf2; btrl $100,(%ecx)
	.byte 0xf0; .byte 0xf3; btrl $100,(%ecx)
	xacquire lock btsl $100,(%ecx)
	lock xacquire btsl $100,(%ecx)
	xrelease lock btsl $100,(%ecx)
	lock xrelease btsl $100,(%ecx)
	.byte 0xf0; .byte 0xf2; btsl $100,(%ecx)
	.byte 0xf0; .byte 0xf3; btsl $100,(%ecx)
	xrelease movl $100,(%ecx)
	xacquire lock orl $100,(%ecx)
	lock xacquire orl $100,(%ecx)
	xrelease lock orl $100,(%ecx)
	lock xrelease orl $100,(%ecx)
	.byte 0xf0; .byte 0xf2; orl $100,(%ecx)
	.byte 0xf0; .byte 0xf3; orl $100,(%ecx)
	xacquire lock sbbl $100,(%ecx)
	lock xacquire sbbl $100,(%ecx)
	xrelease lock sbbl $100,(%ecx)
	lock xrelease sbbl $100,(%ecx)
	.byte 0xf0; .byte 0xf2; sbbl $100,(%ecx)
	.byte 0xf0; .byte 0xf3; sbbl $100,(%ecx)
	xacquire lock subl $100,(%ecx)
	lock xacquire subl $100,(%ecx)
	xrelease lock subl $100,(%ecx)
	lock xrelease subl $100,(%ecx)
	.byte 0xf0; .byte 0xf2; subl $100,(%ecx)
	.byte 0xf0; .byte 0xf3; subl $100,(%ecx)
	xacquire lock xorl $100,(%ecx)
	lock xacquire xorl $100,(%ecx)
	xrelease lock xorl $100,(%ecx)
	lock xrelease xorl $100,(%ecx)
	.byte 0xf0; .byte 0xf2; xorl $100,(%ecx)
	.byte 0xf0; .byte 0xf3; xorl $100,(%ecx)

# Tests for op imm8 regb/m8
	xacquire lock adcb $100,(%ecx)
	lock xacquire adcb $100,(%ecx)
	xrelease lock adcb $100,(%ecx)
	lock xrelease adcb $100,(%ecx)
	.byte 0xf0; .byte 0xf2; adcb $100,(%ecx)
	.byte 0xf0; .byte 0xf3; adcb $100,(%ecx)
	xacquire lock addb $100,(%ecx)
	lock xacquire addb $100,(%ecx)
	xrelease lock addb $100,(%ecx)
	lock xrelease addb $100,(%ecx)
	.byte 0xf0; .byte 0xf2; addb $100,(%ecx)
	.byte 0xf0; .byte 0xf3; addb $100,(%ecx)
	xacquire lock andb $100,(%ecx)
	lock xacquire andb $100,(%ecx)
	xrelease lock andb $100,(%ecx)
	lock xrelease andb $100,(%ecx)
	.byte 0xf0; .byte 0xf2; andb $100,(%ecx)
	.byte 0xf0; .byte 0xf3; andb $100,(%ecx)
	xrelease movb $100,(%ecx)
	xacquire lock orb $100,(%ecx)
	lock xacquire orb $100,(%ecx)
	xrelease lock orb $100,(%ecx)
	lock xrelease orb $100,(%ecx)
	.byte 0xf0; .byte 0xf2; orb $100,(%ecx)
	.byte 0xf0; .byte 0xf3; orb $100,(%ecx)
	xacquire lock sbbb $100,(%ecx)
	lock xacquire sbbb $100,(%ecx)
	xrelease lock sbbb $100,(%ecx)
	lock xrelease sbbb $100,(%ecx)
	.byte 0xf0; .byte 0xf2; sbbb $100,(%ecx)
	.byte 0xf0; .byte 0xf3; sbbb $100,(%ecx)
	xacquire lock subb $100,(%ecx)
	lock xacquire subb $100,(%ecx)
	xrelease lock subb $100,(%ecx)
	lock xrelease subb $100,(%ecx)
	.byte 0xf0; .byte 0xf2; subb $100,(%ecx)
	.byte 0xf0; .byte 0xf3; subb $100,(%ecx)
	xacquire lock xorb $100,(%ecx)
	lock xacquire xorb $100,(%ecx)
	xrelease lock xorb $100,(%ecx)
	lock xrelease xorb $100,(%ecx)
	.byte 0xf0; .byte 0xf2; xorb $100,(%ecx)
	.byte 0xf0; .byte 0xf3; xorb $100,(%ecx)

# Tests for op regb regb/m8
# Tests for op regb/m8 regb
	xacquire lock adcb %al,(%ecx)
	lock xacquire adcb %al,(%ecx)
	xrelease lock adcb %al,(%ecx)
	lock xrelease adcb %al,(%ecx)
	.byte 0xf0; .byte 0xf2; adcb %al,(%ecx)
	.byte 0xf0; .byte 0xf3; adcb %al,(%ecx)
	xacquire lock addb %al,(%ecx)
	lock xacquire addb %al,(%ecx)
	xrelease lock addb %al,(%ecx)
	lock xrelease addb %al,(%ecx)
	.byte 0xf0; .byte 0xf2; addb %al,(%ecx)
	.byte 0xf0; .byte 0xf3; addb %al,(%ecx)
	xacquire lock andb %al,(%ecx)
	lock xacquire andb %al,(%ecx)
	xrelease lock andb %al,(%ecx)
	lock xrelease andb %al,(%ecx)
	.byte 0xf0; .byte 0xf2; andb %al,(%ecx)
	.byte 0xf0; .byte 0xf3; andb %al,(%ecx)
	xrelease movb %al,(%ecx)
	xrelease movb %al,0x12345678
	xacquire lock orb %al,(%ecx)
	lock xacquire orb %al,(%ecx)
	xrelease lock orb %al,(%ecx)
	lock xrelease orb %al,(%ecx)
	.byte 0xf0; .byte 0xf2; orb %al,(%ecx)
	.byte 0xf0; .byte 0xf3; orb %al,(%ecx)
	xacquire lock sbbb %al,(%ecx)
	lock xacquire sbbb %al,(%ecx)
	xrelease lock sbbb %al,(%ecx)
	lock xrelease sbbb %al,(%ecx)
	.byte 0xf0; .byte 0xf2; sbbb %al,(%ecx)
	.byte 0xf0; .byte 0xf3; sbbb %al,(%ecx)
	xacquire lock subb %al,(%ecx)
	lock xacquire subb %al,(%ecx)
	xrelease lock subb %al,(%ecx)
	lock xrelease subb %al,(%ecx)
	.byte 0xf0; .byte 0xf2; subb %al,(%ecx)
	.byte 0xf0; .byte 0xf3; subb %al,(%ecx)
	xacquire lock xchgb %al,(%ecx)
	lock xacquire xchgb %al,(%ecx)
	xacquire xchgb %al,(%ecx)
	xrelease lock xchgb %al,(%ecx)
	lock xrelease xchgb %al,(%ecx)
	xrelease xchgb %al,(%ecx)
	.byte 0xf0; .byte 0xf2; xchgb %al,(%ecx)
	.byte 0xf0; .byte 0xf3; xchgb %al,(%ecx)
	xacquire lock xorb %al,(%ecx)
	lock xacquire xorb %al,(%ecx)
	xrelease lock xorb %al,(%ecx)
	lock xrelease xorb %al,(%ecx)
	.byte 0xf0; .byte 0xf2; xorb %al,(%ecx)
	.byte 0xf0; .byte 0xf3; xorb %al,(%ecx)

# Tests for op regs regs/m16
# Tests for op regs/m16 regs
	xacquire lock adcw %ax,(%ecx)
	lock xacquire adcw %ax,(%ecx)
	xrelease lock adcw %ax,(%ecx)
	lock xrelease adcw %ax,(%ecx)
	.byte 0xf0; .byte 0xf2; adcw %ax,(%ecx)
	.byte 0xf0; .byte 0xf3; adcw %ax,(%ecx)
	xacquire lock addw %ax,(%ecx)
	lock xacquire addw %ax,(%ecx)
	xrelease lock addw %ax,(%ecx)
	lock xrelease addw %ax,(%ecx)
	.byte 0xf0; .byte 0xf2; addw %ax,(%ecx)
	.byte 0xf0; .byte 0xf3; addw %ax,(%ecx)
	xacquire lock andw %ax,(%ecx)
	lock xacquire andw %ax,(%ecx)
	xrelease lock andw %ax,(%ecx)
	lock xrelease andw %ax,(%ecx)
	.byte 0xf0; .byte 0xf2; andw %ax,(%ecx)
	.byte 0xf0; .byte 0xf3; andw %ax,(%ecx)
	xrelease movw %ax,(%ecx)
	xrelease movw %ax,0x12345678
	xacquire lock orw %ax,(%ecx)
	lock xacquire orw %ax,(%ecx)
	xrelease lock orw %ax,(%ecx)
	lock xrelease orw %ax,(%ecx)
	.byte 0xf0; .byte 0xf2; orw %ax,(%ecx)
	.byte 0xf0; .byte 0xf3; orw %ax,(%ecx)
	xacquire lock sbbw %ax,(%ecx)
	lock xacquire sbbw %ax,(%ecx)
	xrelease lock sbbw %ax,(%ecx)
	lock xrelease sbbw %ax,(%ecx)
	.byte 0xf0; .byte 0xf2; sbbw %ax,(%ecx)
	.byte 0xf0; .byte 0xf3; sbbw %ax,(%ecx)
	xacquire lock subw %ax,(%ecx)
	lock xacquire subw %ax,(%ecx)
	xrelease lock subw %ax,(%ecx)
	lock xrelease subw %ax,(%ecx)
	.byte 0xf0; .byte 0xf2; subw %ax,(%ecx)
	.byte 0xf0; .byte 0xf3; subw %ax,(%ecx)
	xacquire lock xchgw %ax,(%ecx)
	lock xacquire xchgw %ax,(%ecx)
	xacquire xchgw %ax,(%ecx)
	xrelease lock xchgw %ax,(%ecx)
	lock xrelease xchgw %ax,(%ecx)
	xrelease xchgw %ax,(%ecx)
	.byte 0xf0; .byte 0xf2; xchgw %ax,(%ecx)
	.byte 0xf0; .byte 0xf3; xchgw %ax,(%ecx)
	xacquire lock xorw %ax,(%ecx)
	lock xacquire xorw %ax,(%ecx)
	xrelease lock xorw %ax,(%ecx)
	lock xrelease xorw %ax,(%ecx)
	.byte 0xf0; .byte 0xf2; xorw %ax,(%ecx)
	.byte 0xf0; .byte 0xf3; xorw %ax,(%ecx)

# Tests for op regl regl/m32
# Tests for op regl/m32 regl
	xacquire lock adcl %eax,(%ecx)
	lock xacquire adcl %eax,(%ecx)
	xrelease lock adcl %eax,(%ecx)
	lock xrelease adcl %eax,(%ecx)
	.byte 0xf0; .byte 0xf2; adcl %eax,(%ecx)
	.byte 0xf0; .byte 0xf3; adcl %eax,(%ecx)
	xacquire lock addl %eax,(%ecx)
	lock xacquire addl %eax,(%ecx)
	xrelease lock addl %eax,(%ecx)
	lock xrelease addl %eax,(%ecx)
	.byte 0xf0; .byte 0xf2; addl %eax,(%ecx)
	.byte 0xf0; .byte 0xf3; addl %eax,(%ecx)
	xacquire lock andl %eax,(%ecx)
	lock xacquire andl %eax,(%ecx)
	xrelease lock andl %eax,(%ecx)
	lock xrelease andl %eax,(%ecx)
	.byte 0xf0; .byte 0xf2; andl %eax,(%ecx)
	.byte 0xf0; .byte 0xf3; andl %eax,(%ecx)
	xrelease movl %eax,(%ecx)
	xrelease movl %eax,0x12345678
	xacquire lock orl %eax,(%ecx)
	lock xacquire orl %eax,(%ecx)
	xrelease lock orl %eax,(%ecx)
	lock xrelease orl %eax,(%ecx)
	.byte 0xf0; .byte 0xf2; orl %eax,(%ecx)
	.byte 0xf0; .byte 0xf3; orl %eax,(%ecx)
	xacquire lock sbbl %eax,(%ecx)
	lock xacquire sbbl %eax,(%ecx)
	xrelease lock sbbl %eax,(%ecx)
	lock xrelease sbbl %eax,(%ecx)
	.byte 0xf0; .byte 0xf2; sbbl %eax,(%ecx)
	.byte 0xf0; .byte 0xf3; sbbl %eax,(%ecx)
	xacquire lock subl %eax,(%ecx)
	lock xacquire subl %eax,(%ecx)
	xrelease lock subl %eax,(%ecx)
	lock xrelease subl %eax,(%ecx)
	.byte 0xf0; .byte 0xf2; subl %eax,(%ecx)
	.byte 0xf0; .byte 0xf3; subl %eax,(%ecx)
	xacquire lock xchgl %eax,(%ecx)
	lock xacquire xchgl %eax,(%ecx)
	xacquire xchgl %eax,(%ecx)
	xrelease lock xchgl %eax,(%ecx)
	lock xrelease xchgl %eax,(%ecx)
	xrelease xchgl %eax,(%ecx)
	.byte 0xf0; .byte 0xf2; xchgl %eax,(%ecx)
	.byte 0xf0; .byte 0xf3; xchgl %eax,(%ecx)
	xacquire lock xorl %eax,(%ecx)
	lock xacquire xorl %eax,(%ecx)
	xrelease lock xorl %eax,(%ecx)
	lock xrelease xorl %eax,(%ecx)
	.byte 0xf0; .byte 0xf2; xorl %eax,(%ecx)
	.byte 0xf0; .byte 0xf3; xorl %eax,(%ecx)

# Tests for op regs, regs/m16
	xacquire lock btcw %ax,(%ecx)
	lock xacquire btcw %ax,(%ecx)
	xrelease lock btcw %ax,(%ecx)
	lock xrelease btcw %ax,(%ecx)
	.byte 0xf0; .byte 0xf2; btcw %ax,(%ecx)
	.byte 0xf0; .byte 0xf3; btcw %ax,(%ecx)
	xacquire lock btrw %ax,(%ecx)
	lock xacquire btrw %ax,(%ecx)
	xrelease lock btrw %ax,(%ecx)
	lock xrelease btrw %ax,(%ecx)
	.byte 0xf0; .byte 0xf2; btrw %ax,(%ecx)
	.byte 0xf0; .byte 0xf3; btrw %ax,(%ecx)
	xacquire lock btsw %ax,(%ecx)
	lock xacquire btsw %ax,(%ecx)
	xrelease lock btsw %ax,(%ecx)
	lock xrelease btsw %ax,(%ecx)
	.byte 0xf0; .byte 0xf2; btsw %ax,(%ecx)
	.byte 0xf0; .byte 0xf3; btsw %ax,(%ecx)
	xacquire lock cmpxchgw %ax,(%ecx)
	lock xacquire cmpxchgw %ax,(%ecx)
	xrelease lock cmpxchgw %ax,(%ecx)
	lock xrelease cmpxchgw %ax,(%ecx)
	.byte 0xf0; .byte 0xf2; cmpxchgw %ax,(%ecx)
	.byte 0xf0; .byte 0xf3; cmpxchgw %ax,(%ecx)
	xacquire lock xaddw %ax,(%ecx)
	lock xacquire xaddw %ax,(%ecx)
	xrelease lock xaddw %ax,(%ecx)
	lock xrelease xaddw %ax,(%ecx)
	.byte 0xf0; .byte 0xf2; xaddw %ax,(%ecx)
	.byte 0xf0; .byte 0xf3; xaddw %ax,(%ecx)

# Tests for op regl regl/m32
	xacquire lock btcl %eax,(%ecx)
	lock xacquire btcl %eax,(%ecx)
	xrelease lock btcl %eax,(%ecx)
	lock xrelease btcl %eax,(%ecx)
	.byte 0xf0; .byte 0xf2; btcl %eax,(%ecx)
	.byte 0xf0; .byte 0xf3; btcl %eax,(%ecx)
	xacquire lock btrl %eax,(%ecx)
	lock xacquire btrl %eax,(%ecx)
	xrelease lock btrl %eax,(%ecx)
	lock xrelease btrl %eax,(%ecx)
	.byte 0xf0; .byte 0xf2; btrl %eax,(%ecx)
	.byte 0xf0; .byte 0xf3; btrl %eax,(%ecx)
	xacquire lock btsl %eax,(%ecx)
	lock xacquire btsl %eax,(%ecx)
	xrelease lock btsl %eax,(%ecx)
	lock xrelease btsl %eax,(%ecx)
	.byte 0xf0; .byte 0xf2; btsl %eax,(%ecx)
	.byte 0xf0; .byte 0xf3; btsl %eax,(%ecx)
	xacquire lock cmpxchgl %eax,(%ecx)
	lock xacquire cmpxchgl %eax,(%ecx)
	xrelease lock cmpxchgl %eax,(%ecx)
	lock xrelease cmpxchgl %eax,(%ecx)
	.byte 0xf0; .byte 0xf2; cmpxchgl %eax,(%ecx)
	.byte 0xf0; .byte 0xf3; cmpxchgl %eax,(%ecx)
	xacquire lock xaddl %eax,(%ecx)
	lock xacquire xaddl %eax,(%ecx)
	xrelease lock xaddl %eax,(%ecx)
	lock xrelease xaddl %eax,(%ecx)
	.byte 0xf0; .byte 0xf2; xaddl %eax,(%ecx)
	.byte 0xf0; .byte 0xf3; xaddl %eax,(%ecx)

# Tests for op regb/m8
	xacquire lock decb (%ecx)
	lock xacquire decb (%ecx)
	xrelease lock decb (%ecx)
	lock xrelease decb (%ecx)
	.byte 0xf0; .byte 0xf2; decb (%ecx)
	.byte 0xf0; .byte 0xf3; decb (%ecx)
	xacquire lock incb (%ecx)
	lock xacquire incb (%ecx)
	xrelease lock incb (%ecx)
	lock xrelease incb (%ecx)
	.byte 0xf0; .byte 0xf2; incb (%ecx)
	.byte 0xf0; .byte 0xf3; incb (%ecx)
	xacquire lock negb (%ecx)
	lock xacquire negb (%ecx)
	xrelease lock negb (%ecx)
	lock xrelease negb (%ecx)
	.byte 0xf0; .byte 0xf2; negb (%ecx)
	.byte 0xf0; .byte 0xf3; negb (%ecx)
	xacquire lock notb (%ecx)
	lock xacquire notb (%ecx)
	xrelease lock notb (%ecx)
	lock xrelease notb (%ecx)
	.byte 0xf0; .byte 0xf2; notb (%ecx)
	.byte 0xf0; .byte 0xf3; notb (%ecx)

# Tests for op regs/m16
	xacquire lock decw (%ecx)
	lock xacquire decw (%ecx)
	xrelease lock decw (%ecx)
	lock xrelease decw (%ecx)
	.byte 0xf0; .byte 0xf2; decw (%ecx)
	.byte 0xf0; .byte 0xf3; decw (%ecx)
	xacquire lock incw (%ecx)
	lock xacquire incw (%ecx)
	xrelease lock incw (%ecx)
	lock xrelease incw (%ecx)
	.byte 0xf0; .byte 0xf2; incw (%ecx)
	.byte 0xf0; .byte 0xf3; incw (%ecx)
	xacquire lock negw (%ecx)
	lock xacquire negw (%ecx)
	xrelease lock negw (%ecx)
	lock xrelease negw (%ecx)
	.byte 0xf0; .byte 0xf2; negw (%ecx)
	.byte 0xf0; .byte 0xf3; negw (%ecx)
	xacquire lock notw (%ecx)
	lock xacquire notw (%ecx)
	xrelease lock notw (%ecx)
	lock xrelease notw (%ecx)
	.byte 0xf0; .byte 0xf2; notw (%ecx)
	.byte 0xf0; .byte 0xf3; notw (%ecx)

# Tests for op regl/m32
	xacquire lock decl (%ecx)
	lock xacquire decl (%ecx)
	xrelease lock decl (%ecx)
	lock xrelease decl (%ecx)
	.byte 0xf0; .byte 0xf2; decl (%ecx)
	.byte 0xf0; .byte 0xf3; decl (%ecx)
	xacquire lock incl (%ecx)
	lock xacquire incl (%ecx)
	xrelease lock incl (%ecx)
	lock xrelease incl (%ecx)
	.byte 0xf0; .byte 0xf2; incl (%ecx)
	.byte 0xf0; .byte 0xf3; incl (%ecx)
	xacquire lock negl (%ecx)
	lock xacquire negl (%ecx)
	xrelease lock negl (%ecx)
	lock xrelease negl (%ecx)
	.byte 0xf0; .byte 0xf2; negl (%ecx)
	.byte 0xf0; .byte 0xf3; negl (%ecx)
	xacquire lock notl (%ecx)
	lock xacquire notl (%ecx)
	xrelease lock notl (%ecx)
	lock xrelease notl (%ecx)
	.byte 0xf0; .byte 0xf2; notl (%ecx)
	.byte 0xf0; .byte 0xf3; notl (%ecx)

# Tests for op m64
	xacquire lock cmpxchg8bq (%ecx)
	lock xacquire cmpxchg8bq (%ecx)
	xrelease lock cmpxchg8bq (%ecx)
	lock xrelease cmpxchg8bq (%ecx)
	.byte 0xf0; .byte 0xf2; cmpxchg8bq (%ecx)
	.byte 0xf0; .byte 0xf3; cmpxchg8bq (%ecx)

# Tests for op regb, regb/m8
	xacquire lock cmpxchgb %cl,(%ecx)
	lock xacquire cmpxchgb %cl,(%ecx)
	xrelease lock cmpxchgb %cl,(%ecx)
	lock xrelease cmpxchgb %cl,(%ecx)
	.byte 0xf0; .byte 0xf2; cmpxchgb %cl,(%ecx)
	.byte 0xf0; .byte 0xf3; cmpxchgb %cl,(%ecx)
	xacquire lock xaddb %cl,(%ecx)
	lock xacquire xaddb %cl,(%ecx)
	xrelease lock xaddb %cl,(%ecx)
	lock xrelease xaddb %cl,(%ecx)
	.byte 0xf0; .byte 0xf2; xaddb %cl,(%ecx)
	.byte 0xf0; .byte 0xf3; xaddb %cl,(%ecx)

	.intel_syntax noprefix

# Tests for op imm8 regb/m8
	xacquire lock adc BYTE PTR [ecx],100
	lock xacquire adc BYTE PTR [ecx],100
	xrelease lock adc BYTE PTR [ecx],100
	lock xrelease adc BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf2; adc BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf3; adc BYTE PTR [ecx],100
	xacquire lock add BYTE PTR [ecx],100
	lock xacquire add BYTE PTR [ecx],100
	xrelease lock add BYTE PTR [ecx],100
	lock xrelease add BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf2; add BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf3; add BYTE PTR [ecx],100
	xacquire lock and BYTE PTR [ecx],100
	lock xacquire and BYTE PTR [ecx],100
	xrelease lock and BYTE PTR [ecx],100
	lock xrelease and BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf2; and BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf3; and BYTE PTR [ecx],100
	xrelease mov BYTE PTR [ecx],100
	xacquire lock or BYTE PTR [ecx],100
	lock xacquire or BYTE PTR [ecx],100
	xrelease lock or BYTE PTR [ecx],100
	lock xrelease or BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf2; or BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf3; or BYTE PTR [ecx],100
	xacquire lock sbb BYTE PTR [ecx],100
	lock xacquire sbb BYTE PTR [ecx],100
	xrelease lock sbb BYTE PTR [ecx],100
	lock xrelease sbb BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf2; sbb BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf3; sbb BYTE PTR [ecx],100
	xacquire lock sub BYTE PTR [ecx],100
	lock xacquire sub BYTE PTR [ecx],100
	xrelease lock sub BYTE PTR [ecx],100
	lock xrelease sub BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf2; sub BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf3; sub BYTE PTR [ecx],100
	xacquire lock xor BYTE PTR [ecx],100
	lock xacquire xor BYTE PTR [ecx],100
	xrelease lock xor BYTE PTR [ecx],100
	lock xrelease xor BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf2; xor BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf3; xor BYTE PTR [ecx],100

# Tests for op imm16 regs/m16
	xacquire lock adc WORD PTR [ecx],1000
	lock xacquire adc WORD PTR [ecx],1000
	xrelease lock adc WORD PTR [ecx],1000
	lock xrelease adc WORD PTR [ecx],1000
	.byte 0xf0; .byte 0xf2; adc WORD PTR [ecx],1000
	.byte 0xf0; .byte 0xf3; adc WORD PTR [ecx],1000
	xacquire lock add WORD PTR [ecx],1000
	lock xacquire add WORD PTR [ecx],1000
	xrelease lock add WORD PTR [ecx],1000
	lock xrelease add WORD PTR [ecx],1000
	.byte 0xf0; .byte 0xf2; add WORD PTR [ecx],1000
	.byte 0xf0; .byte 0xf3; add WORD PTR [ecx],1000
	xacquire lock and WORD PTR [ecx],1000
	lock xacquire and WORD PTR [ecx],1000
	xrelease lock and WORD PTR [ecx],1000
	lock xrelease and WORD PTR [ecx],1000
	.byte 0xf0; .byte 0xf2; and WORD PTR [ecx],1000
	.byte 0xf0; .byte 0xf3; and WORD PTR [ecx],1000
	xrelease mov WORD PTR [ecx],1000
	xacquire lock or WORD PTR [ecx],1000
	lock xacquire or WORD PTR [ecx],1000
	xrelease lock or WORD PTR [ecx],1000
	lock xrelease or WORD PTR [ecx],1000
	.byte 0xf0; .byte 0xf2; or WORD PTR [ecx],1000
	.byte 0xf0; .byte 0xf3; or WORD PTR [ecx],1000
	xacquire lock sbb WORD PTR [ecx],1000
	lock xacquire sbb WORD PTR [ecx],1000
	xrelease lock sbb WORD PTR [ecx],1000
	lock xrelease sbb WORD PTR [ecx],1000
	.byte 0xf0; .byte 0xf2; sbb WORD PTR [ecx],1000
	.byte 0xf0; .byte 0xf3; sbb WORD PTR [ecx],1000
	xacquire lock sub WORD PTR [ecx],1000
	lock xacquire sub WORD PTR [ecx],1000
	xrelease lock sub WORD PTR [ecx],1000
	lock xrelease sub WORD PTR [ecx],1000
	.byte 0xf0; .byte 0xf2; sub WORD PTR [ecx],1000
	.byte 0xf0; .byte 0xf3; sub WORD PTR [ecx],1000
	xacquire lock xor WORD PTR [ecx],1000
	lock xacquire xor WORD PTR [ecx],1000
	xrelease lock xor WORD PTR [ecx],1000
	lock xrelease xor WORD PTR [ecx],1000
	.byte 0xf0; .byte 0xf2; xor WORD PTR [ecx],1000
	.byte 0xf0; .byte 0xf3; xor WORD PTR [ecx],1000

# Tests for op imm32 regl/m32
	xacquire lock adc DWORD PTR [ecx],10000000
	lock xacquire adc DWORD PTR [ecx],10000000
	xrelease lock adc DWORD PTR [ecx],10000000
	lock xrelease adc DWORD PTR [ecx],10000000
	.byte 0xf0; .byte 0xf2; adc DWORD PTR [ecx],10000000
	.byte 0xf0; .byte 0xf3; adc DWORD PTR [ecx],10000000
	xacquire lock add DWORD PTR [ecx],10000000
	lock xacquire add DWORD PTR [ecx],10000000
	xrelease lock add DWORD PTR [ecx],10000000
	lock xrelease add DWORD PTR [ecx],10000000
	.byte 0xf0; .byte 0xf2; add DWORD PTR [ecx],10000000
	.byte 0xf0; .byte 0xf3; add DWORD PTR [ecx],10000000
	xacquire lock and DWORD PTR [ecx],10000000
	lock xacquire and DWORD PTR [ecx],10000000
	xrelease lock and DWORD PTR [ecx],10000000
	lock xrelease and DWORD PTR [ecx],10000000
	.byte 0xf0; .byte 0xf2; and DWORD PTR [ecx],10000000
	.byte 0xf0; .byte 0xf3; and DWORD PTR [ecx],10000000
	xrelease mov DWORD PTR [ecx],10000000
	xacquire lock or DWORD PTR [ecx],10000000
	lock xacquire or DWORD PTR [ecx],10000000
	xrelease lock or DWORD PTR [ecx],10000000
	lock xrelease or DWORD PTR [ecx],10000000
	.byte 0xf0; .byte 0xf2; or DWORD PTR [ecx],10000000
	.byte 0xf0; .byte 0xf3; or DWORD PTR [ecx],10000000
	xacquire lock sbb DWORD PTR [ecx],10000000
	lock xacquire sbb DWORD PTR [ecx],10000000
	xrelease lock sbb DWORD PTR [ecx],10000000
	lock xrelease sbb DWORD PTR [ecx],10000000
	.byte 0xf0; .byte 0xf2; sbb DWORD PTR [ecx],10000000
	.byte 0xf0; .byte 0xf3; sbb DWORD PTR [ecx],10000000
	xacquire lock sub DWORD PTR [ecx],10000000
	lock xacquire sub DWORD PTR [ecx],10000000
	xrelease lock sub DWORD PTR [ecx],10000000
	lock xrelease sub DWORD PTR [ecx],10000000
	.byte 0xf0; .byte 0xf2; sub DWORD PTR [ecx],10000000
	.byte 0xf0; .byte 0xf3; sub DWORD PTR [ecx],10000000
	xacquire lock xor DWORD PTR [ecx],10000000
	lock xacquire xor DWORD PTR [ecx],10000000
	xrelease lock xor DWORD PTR [ecx],10000000
	lock xrelease xor DWORD PTR [ecx],10000000
	.byte 0xf0; .byte 0xf2; xor DWORD PTR [ecx],10000000
	.byte 0xf0; .byte 0xf3; xor DWORD PTR [ecx],10000000

# Tests for op imm8 regs/m16
	xacquire lock adc WORD PTR [ecx],100
	lock xacquire adc WORD PTR [ecx],100
	xrelease lock adc WORD PTR [ecx],100
	lock xrelease adc WORD PTR [ecx],100
	.byte 0xf0; .byte 0xf2; adc WORD PTR [ecx],100
	.byte 0xf0; .byte 0xf3; adc WORD PTR [ecx],100
	xacquire lock add WORD PTR [ecx],100
	lock xacquire add WORD PTR [ecx],100
	xrelease lock add WORD PTR [ecx],100
	lock xrelease add WORD PTR [ecx],100
	.byte 0xf0; .byte 0xf2; add WORD PTR [ecx],100
	.byte 0xf0; .byte 0xf3; add WORD PTR [ecx],100
	xacquire lock and WORD PTR [ecx],100
	lock xacquire and WORD PTR [ecx],100
	xrelease lock and WORD PTR [ecx],100
	lock xrelease and WORD PTR [ecx],100
	.byte 0xf0; .byte 0xf2; and WORD PTR [ecx],100
	.byte 0xf0; .byte 0xf3; and WORD PTR [ecx],100
	xacquire lock btc WORD PTR [ecx],100
	lock xacquire btc WORD PTR [ecx],100
	xrelease lock btc WORD PTR [ecx],100
	lock xrelease btc WORD PTR [ecx],100
	.byte 0xf0; .byte 0xf2; btc WORD PTR [ecx],100
	.byte 0xf0; .byte 0xf3; btc WORD PTR [ecx],100
	xacquire lock btr WORD PTR [ecx],100
	lock xacquire btr WORD PTR [ecx],100
	xrelease lock btr WORD PTR [ecx],100
	lock xrelease btr WORD PTR [ecx],100
	.byte 0xf0; .byte 0xf2; btr WORD PTR [ecx],100
	.byte 0xf0; .byte 0xf3; btr WORD PTR [ecx],100
	xacquire lock bts WORD PTR [ecx],100
	lock xacquire bts WORD PTR [ecx],100
	xrelease lock bts WORD PTR [ecx],100
	lock xrelease bts WORD PTR [ecx],100
	.byte 0xf0; .byte 0xf2; bts WORD PTR [ecx],100
	.byte 0xf0; .byte 0xf3; bts WORD PTR [ecx],100
	xrelease mov WORD PTR [ecx],100
	xacquire lock or WORD PTR [ecx],100
	lock xacquire or WORD PTR [ecx],100
	xrelease lock or WORD PTR [ecx],100
	lock xrelease or WORD PTR [ecx],100
	.byte 0xf0; .byte 0xf2; or WORD PTR [ecx],100
	.byte 0xf0; .byte 0xf3; or WORD PTR [ecx],100
	xacquire lock sbb WORD PTR [ecx],100
	lock xacquire sbb WORD PTR [ecx],100
	xrelease lock sbb WORD PTR [ecx],100
	lock xrelease sbb WORD PTR [ecx],100
	.byte 0xf0; .byte 0xf2; sbb WORD PTR [ecx],100
	.byte 0xf0; .byte 0xf3; sbb WORD PTR [ecx],100
	xacquire lock sub WORD PTR [ecx],100
	lock xacquire sub WORD PTR [ecx],100
	xrelease lock sub WORD PTR [ecx],100
	lock xrelease sub WORD PTR [ecx],100
	.byte 0xf0; .byte 0xf2; sub WORD PTR [ecx],100
	.byte 0xf0; .byte 0xf3; sub WORD PTR [ecx],100
	xacquire lock xor WORD PTR [ecx],100
	lock xacquire xor WORD PTR [ecx],100
	xrelease lock xor WORD PTR [ecx],100
	lock xrelease xor WORD PTR [ecx],100
	.byte 0xf0; .byte 0xf2; xor WORD PTR [ecx],100
	.byte 0xf0; .byte 0xf3; xor WORD PTR [ecx],100

# Tests for op imm8 regl/m32
	xacquire lock adc DWORD PTR [ecx],100
	lock xacquire adc DWORD PTR [ecx],100
	xrelease lock adc DWORD PTR [ecx],100
	lock xrelease adc DWORD PTR [ecx],100
	.byte 0xf0; .byte 0xf2; adc DWORD PTR [ecx],100
	.byte 0xf0; .byte 0xf3; adc DWORD PTR [ecx],100
	xacquire lock add DWORD PTR [ecx],100
	lock xacquire add DWORD PTR [ecx],100
	xrelease lock add DWORD PTR [ecx],100
	lock xrelease add DWORD PTR [ecx],100
	.byte 0xf0; .byte 0xf2; add DWORD PTR [ecx],100
	.byte 0xf0; .byte 0xf3; add DWORD PTR [ecx],100
	xacquire lock and DWORD PTR [ecx],100
	lock xacquire and DWORD PTR [ecx],100
	xrelease lock and DWORD PTR [ecx],100
	lock xrelease and DWORD PTR [ecx],100
	.byte 0xf0; .byte 0xf2; and DWORD PTR [ecx],100
	.byte 0xf0; .byte 0xf3; and DWORD PTR [ecx],100
	xacquire lock btc DWORD PTR [ecx],100
	lock xacquire btc DWORD PTR [ecx],100
	xrelease lock btc DWORD PTR [ecx],100
	lock xrelease btc DWORD PTR [ecx],100
	.byte 0xf0; .byte 0xf2; btc DWORD PTR [ecx],100
	.byte 0xf0; .byte 0xf3; btc DWORD PTR [ecx],100
	xacquire lock btr DWORD PTR [ecx],100
	lock xacquire btr DWORD PTR [ecx],100
	xrelease lock btr DWORD PTR [ecx],100
	lock xrelease btr DWORD PTR [ecx],100
	.byte 0xf0; .byte 0xf2; btr DWORD PTR [ecx],100
	.byte 0xf0; .byte 0xf3; btr DWORD PTR [ecx],100
	xacquire lock bts DWORD PTR [ecx],100
	lock xacquire bts DWORD PTR [ecx],100
	xrelease lock bts DWORD PTR [ecx],100
	lock xrelease bts DWORD PTR [ecx],100
	.byte 0xf0; .byte 0xf2; bts DWORD PTR [ecx],100
	.byte 0xf0; .byte 0xf3; bts DWORD PTR [ecx],100
	xrelease mov DWORD PTR [ecx],100
	xacquire lock or DWORD PTR [ecx],100
	lock xacquire or DWORD PTR [ecx],100
	xrelease lock or DWORD PTR [ecx],100
	lock xrelease or DWORD PTR [ecx],100
	.byte 0xf0; .byte 0xf2; or DWORD PTR [ecx],100
	.byte 0xf0; .byte 0xf3; or DWORD PTR [ecx],100
	xacquire lock sbb DWORD PTR [ecx],100
	lock xacquire sbb DWORD PTR [ecx],100
	xrelease lock sbb DWORD PTR [ecx],100
	lock xrelease sbb DWORD PTR [ecx],100
	.byte 0xf0; .byte 0xf2; sbb DWORD PTR [ecx],100
	.byte 0xf0; .byte 0xf3; sbb DWORD PTR [ecx],100
	xacquire lock sub DWORD PTR [ecx],100
	lock xacquire sub DWORD PTR [ecx],100
	xrelease lock sub DWORD PTR [ecx],100
	lock xrelease sub DWORD PTR [ecx],100
	.byte 0xf0; .byte 0xf2; sub DWORD PTR [ecx],100
	.byte 0xf0; .byte 0xf3; sub DWORD PTR [ecx],100
	xacquire lock xor DWORD PTR [ecx],100
	lock xacquire xor DWORD PTR [ecx],100
	xrelease lock xor DWORD PTR [ecx],100
	lock xrelease xor DWORD PTR [ecx],100
	.byte 0xf0; .byte 0xf2; xor DWORD PTR [ecx],100
	.byte 0xf0; .byte 0xf3; xor DWORD PTR [ecx],100

# Tests for op imm8 regb/m8
	xacquire lock adc BYTE PTR [ecx],100
	lock xacquire adc BYTE PTR [ecx],100
	xrelease lock adc BYTE PTR [ecx],100
	lock xrelease adc BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf2; adc BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf3; adc BYTE PTR [ecx],100
	xacquire lock add BYTE PTR [ecx],100
	lock xacquire add BYTE PTR [ecx],100
	xrelease lock add BYTE PTR [ecx],100
	lock xrelease add BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf2; add BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf3; add BYTE PTR [ecx],100
	xacquire lock and BYTE PTR [ecx],100
	lock xacquire and BYTE PTR [ecx],100
	xrelease lock and BYTE PTR [ecx],100
	lock xrelease and BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf2; and BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf3; and BYTE PTR [ecx],100
	xrelease mov BYTE PTR [ecx],100
	xacquire lock or BYTE PTR [ecx],100
	lock xacquire or BYTE PTR [ecx],100
	xrelease lock or BYTE PTR [ecx],100
	lock xrelease or BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf2; or BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf3; or BYTE PTR [ecx],100
	xacquire lock sbb BYTE PTR [ecx],100
	lock xacquire sbb BYTE PTR [ecx],100
	xrelease lock sbb BYTE PTR [ecx],100
	lock xrelease sbb BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf2; sbb BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf3; sbb BYTE PTR [ecx],100
	xacquire lock sub BYTE PTR [ecx],100
	lock xacquire sub BYTE PTR [ecx],100
	xrelease lock sub BYTE PTR [ecx],100
	lock xrelease sub BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf2; sub BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf3; sub BYTE PTR [ecx],100
	xacquire lock xor BYTE PTR [ecx],100
	lock xacquire xor BYTE PTR [ecx],100
	xrelease lock xor BYTE PTR [ecx],100
	lock xrelease xor BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf2; xor BYTE PTR [ecx],100
	.byte 0xf0; .byte 0xf3; xor BYTE PTR [ecx],100

# Tests for op regb regb/m8
# Tests for op regb/m8 regb
	xacquire lock adc BYTE PTR [ecx],al
	lock xacquire adc BYTE PTR [ecx],al
	xrelease lock adc BYTE PTR [ecx],al
	lock xrelease adc BYTE PTR [ecx],al
	.byte 0xf0; .byte 0xf2; adc BYTE PTR [ecx],al
	.byte 0xf0; .byte 0xf3; adc BYTE PTR [ecx],al
	xacquire lock add BYTE PTR [ecx],al
	lock xacquire add BYTE PTR [ecx],al
	xrelease lock add BYTE PTR [ecx],al
	lock xrelease add BYTE PTR [ecx],al
	.byte 0xf0; .byte 0xf2; add BYTE PTR [ecx],al
	.byte 0xf0; .byte 0xf3; add BYTE PTR [ecx],al
	xacquire lock and BYTE PTR [ecx],al
	lock xacquire and BYTE PTR [ecx],al
	xrelease lock and BYTE PTR [ecx],al
	lock xrelease and BYTE PTR [ecx],al
	.byte 0xf0; .byte 0xf2; and BYTE PTR [ecx],al
	.byte 0xf0; .byte 0xf3; and BYTE PTR [ecx],al
	xrelease mov BYTE PTR [ecx],al
	xacquire lock or BYTE PTR [ecx],al
	lock xacquire or BYTE PTR [ecx],al
	xrelease lock or BYTE PTR [ecx],al
	lock xrelease or BYTE PTR [ecx],al
	.byte 0xf0; .byte 0xf2; or BYTE PTR [ecx],al
	.byte 0xf0; .byte 0xf3; or BYTE PTR [ecx],al
	xacquire lock sbb BYTE PTR [ecx],al
	lock xacquire sbb BYTE PTR [ecx],al
	xrelease lock sbb BYTE PTR [ecx],al
	lock xrelease sbb BYTE PTR [ecx],al
	.byte 0xf0; .byte 0xf2; sbb BYTE PTR [ecx],al
	.byte 0xf0; .byte 0xf3; sbb BYTE PTR [ecx],al
	xacquire lock sub BYTE PTR [ecx],al
	lock xacquire sub BYTE PTR [ecx],al
	xrelease lock sub BYTE PTR [ecx],al
	lock xrelease sub BYTE PTR [ecx],al
	.byte 0xf0; .byte 0xf2; sub BYTE PTR [ecx],al
	.byte 0xf0; .byte 0xf3; sub BYTE PTR [ecx],al
	xacquire lock xchg BYTE PTR [ecx],al
	lock xacquire xchg BYTE PTR [ecx],al
	xacquire xchg BYTE PTR [ecx],al
	xrelease lock xchg BYTE PTR [ecx],al
	lock xrelease xchg BYTE PTR [ecx],al
	xrelease xchg BYTE PTR [ecx],al
	.byte 0xf0; .byte 0xf2; xchg BYTE PTR [ecx],al
	.byte 0xf0; .byte 0xf3; xchg BYTE PTR [ecx],al
	xacquire lock xor BYTE PTR [ecx],al
	lock xacquire xor BYTE PTR [ecx],al
	xrelease lock xor BYTE PTR [ecx],al
	lock xrelease xor BYTE PTR [ecx],al
	.byte 0xf0; .byte 0xf2; xor BYTE PTR [ecx],al
	.byte 0xf0; .byte 0xf3; xor BYTE PTR [ecx],al

# Tests for op regs regs/m16
# Tests for op regs/m16 regs
	xacquire lock adc WORD PTR [ecx],ax
	lock xacquire adc WORD PTR [ecx],ax
	xrelease lock adc WORD PTR [ecx],ax
	lock xrelease adc WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf2; adc WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf3; adc WORD PTR [ecx],ax
	xacquire lock add WORD PTR [ecx],ax
	lock xacquire add WORD PTR [ecx],ax
	xrelease lock add WORD PTR [ecx],ax
	lock xrelease add WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf2; add WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf3; add WORD PTR [ecx],ax
	xacquire lock and WORD PTR [ecx],ax
	lock xacquire and WORD PTR [ecx],ax
	xrelease lock and WORD PTR [ecx],ax
	lock xrelease and WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf2; and WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf3; and WORD PTR [ecx],ax
	xrelease mov WORD PTR [ecx],ax
	xacquire lock or WORD PTR [ecx],ax
	lock xacquire or WORD PTR [ecx],ax
	xrelease lock or WORD PTR [ecx],ax
	lock xrelease or WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf2; or WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf3; or WORD PTR [ecx],ax
	xacquire lock sbb WORD PTR [ecx],ax
	lock xacquire sbb WORD PTR [ecx],ax
	xrelease lock sbb WORD PTR [ecx],ax
	lock xrelease sbb WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf2; sbb WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf3; sbb WORD PTR [ecx],ax
	xacquire lock sub WORD PTR [ecx],ax
	lock xacquire sub WORD PTR [ecx],ax
	xrelease lock sub WORD PTR [ecx],ax
	lock xrelease sub WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf2; sub WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf3; sub WORD PTR [ecx],ax
	xacquire lock xchg WORD PTR [ecx],ax
	lock xacquire xchg WORD PTR [ecx],ax
	xacquire xchg WORD PTR [ecx],ax
	xrelease lock xchg WORD PTR [ecx],ax
	lock xrelease xchg WORD PTR [ecx],ax
	xrelease xchg WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf2; xchg WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf3; xchg WORD PTR [ecx],ax
	xacquire lock xor WORD PTR [ecx],ax
	lock xacquire xor WORD PTR [ecx],ax
	xrelease lock xor WORD PTR [ecx],ax
	lock xrelease xor WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf2; xor WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf3; xor WORD PTR [ecx],ax

# Tests for op regl regl/m32
# Tests for op regl/m32 regl
	xacquire lock adc DWORD PTR [ecx],eax
	lock xacquire adc DWORD PTR [ecx],eax
	xrelease lock adc DWORD PTR [ecx],eax
	lock xrelease adc DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf2; adc DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf3; adc DWORD PTR [ecx],eax
	xacquire lock add DWORD PTR [ecx],eax
	lock xacquire add DWORD PTR [ecx],eax
	xrelease lock add DWORD PTR [ecx],eax
	lock xrelease add DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf2; add DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf3; add DWORD PTR [ecx],eax
	xacquire lock and DWORD PTR [ecx],eax
	lock xacquire and DWORD PTR [ecx],eax
	xrelease lock and DWORD PTR [ecx],eax
	lock xrelease and DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf2; and DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf3; and DWORD PTR [ecx],eax
	xrelease mov DWORD PTR [ecx],eax
	xacquire lock or DWORD PTR [ecx],eax
	lock xacquire or DWORD PTR [ecx],eax
	xrelease lock or DWORD PTR [ecx],eax
	lock xrelease or DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf2; or DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf3; or DWORD PTR [ecx],eax
	xacquire lock sbb DWORD PTR [ecx],eax
	lock xacquire sbb DWORD PTR [ecx],eax
	xrelease lock sbb DWORD PTR [ecx],eax
	lock xrelease sbb DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf2; sbb DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf3; sbb DWORD PTR [ecx],eax
	xacquire lock sub DWORD PTR [ecx],eax
	lock xacquire sub DWORD PTR [ecx],eax
	xrelease lock sub DWORD PTR [ecx],eax
	lock xrelease sub DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf2; sub DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf3; sub DWORD PTR [ecx],eax
	xacquire lock xchg DWORD PTR [ecx],eax
	lock xacquire xchg DWORD PTR [ecx],eax
	xacquire xchg DWORD PTR [ecx],eax
	xrelease lock xchg DWORD PTR [ecx],eax
	lock xrelease xchg DWORD PTR [ecx],eax
	xrelease xchg DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf2; xchg DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf3; xchg DWORD PTR [ecx],eax
	xacquire lock xor DWORD PTR [ecx],eax
	lock xacquire xor DWORD PTR [ecx],eax
	xrelease lock xor DWORD PTR [ecx],eax
	lock xrelease xor DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf2; xor DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf3; xor DWORD PTR [ecx],eax

# Tests for op regs, regs/m16
	xacquire lock btc WORD PTR [ecx],ax
	lock xacquire btc WORD PTR [ecx],ax
	xrelease lock btc WORD PTR [ecx],ax
	lock xrelease btc WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf2; btc WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf3; btc WORD PTR [ecx],ax
	xacquire lock btr WORD PTR [ecx],ax
	lock xacquire btr WORD PTR [ecx],ax
	xrelease lock btr WORD PTR [ecx],ax
	lock xrelease btr WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf2; btr WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf3; btr WORD PTR [ecx],ax
	xacquire lock bts WORD PTR [ecx],ax
	lock xacquire bts WORD PTR [ecx],ax
	xrelease lock bts WORD PTR [ecx],ax
	lock xrelease bts WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf2; bts WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf3; bts WORD PTR [ecx],ax
	xacquire lock cmpxchg WORD PTR [ecx],ax
	lock xacquire cmpxchg WORD PTR [ecx],ax
	xrelease lock cmpxchg WORD PTR [ecx],ax
	lock xrelease cmpxchg WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf2; cmpxchg WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf3; cmpxchg WORD PTR [ecx],ax
	xacquire lock xadd WORD PTR [ecx],ax
	lock xacquire xadd WORD PTR [ecx],ax
	xrelease lock xadd WORD PTR [ecx],ax
	lock xrelease xadd WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf2; xadd WORD PTR [ecx],ax
	.byte 0xf0; .byte 0xf3; xadd WORD PTR [ecx],ax

# Tests for op regl regl/m32
	xacquire lock btc DWORD PTR [ecx],eax
	lock xacquire btc DWORD PTR [ecx],eax
	xrelease lock btc DWORD PTR [ecx],eax
	lock xrelease btc DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf2; btc DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf3; btc DWORD PTR [ecx],eax
	xacquire lock btr DWORD PTR [ecx],eax
	lock xacquire btr DWORD PTR [ecx],eax
	xrelease lock btr DWORD PTR [ecx],eax
	lock xrelease btr DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf2; btr DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf3; btr DWORD PTR [ecx],eax
	xacquire lock bts DWORD PTR [ecx],eax
	lock xacquire bts DWORD PTR [ecx],eax
	xrelease lock bts DWORD PTR [ecx],eax
	lock xrelease bts DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf2; bts DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf3; bts DWORD PTR [ecx],eax
	xacquire lock cmpxchg DWORD PTR [ecx],eax
	lock xacquire cmpxchg DWORD PTR [ecx],eax
	xrelease lock cmpxchg DWORD PTR [ecx],eax
	lock xrelease cmpxchg DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf2; cmpxchg DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf3; cmpxchg DWORD PTR [ecx],eax
	xacquire lock xadd DWORD PTR [ecx],eax
	lock xacquire xadd DWORD PTR [ecx],eax
	xrelease lock xadd DWORD PTR [ecx],eax
	lock xrelease xadd DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf2; xadd DWORD PTR [ecx],eax
	.byte 0xf0; .byte 0xf3; xadd DWORD PTR [ecx],eax

# Tests for op regb/m8
	xacquire lock dec BYTE PTR [ecx]
	lock xacquire dec BYTE PTR [ecx]
	xrelease lock dec BYTE PTR [ecx]
	lock xrelease dec BYTE PTR [ecx]
	.byte 0xf0; .byte 0xf2; dec BYTE PTR [ecx]
	.byte 0xf0; .byte 0xf3; dec BYTE PTR [ecx]
	xacquire lock inc BYTE PTR [ecx]
	lock xacquire inc BYTE PTR [ecx]
	xrelease lock inc BYTE PTR [ecx]
	lock xrelease inc BYTE PTR [ecx]
	.byte 0xf0; .byte 0xf2; inc BYTE PTR [ecx]
	.byte 0xf0; .byte 0xf3; inc BYTE PTR [ecx]
	xacquire lock neg BYTE PTR [ecx]
	lock xacquire neg BYTE PTR [ecx]
	xrelease lock neg BYTE PTR [ecx]
	lock xrelease neg BYTE PTR [ecx]
	.byte 0xf0; .byte 0xf2; neg BYTE PTR [ecx]
	.byte 0xf0; .byte 0xf3; neg BYTE PTR [ecx]
	xacquire lock not BYTE PTR [ecx]
	lock xacquire not BYTE PTR [ecx]
	xrelease lock not BYTE PTR [ecx]
	lock xrelease not BYTE PTR [ecx]
	.byte 0xf0; .byte 0xf2; not BYTE PTR [ecx]
	.byte 0xf0; .byte 0xf3; not BYTE PTR [ecx]

# Tests for op regs/m16
	xacquire lock dec WORD PTR [ecx]
	lock xacquire dec WORD PTR [ecx]
	xrelease lock dec WORD PTR [ecx]
	lock xrelease dec WORD PTR [ecx]
	.byte 0xf0; .byte 0xf2; dec WORD PTR [ecx]
	.byte 0xf0; .byte 0xf3; dec WORD PTR [ecx]
	xacquire lock inc WORD PTR [ecx]
	lock xacquire inc WORD PTR [ecx]
	xrelease lock inc WORD PTR [ecx]
	lock xrelease inc WORD PTR [ecx]
	.byte 0xf0; .byte 0xf2; inc WORD PTR [ecx]
	.byte 0xf0; .byte 0xf3; inc WORD PTR [ecx]
	xacquire lock neg WORD PTR [ecx]
	lock xacquire neg WORD PTR [ecx]
	xrelease lock neg WORD PTR [ecx]
	lock xrelease neg WORD PTR [ecx]
	.byte 0xf0; .byte 0xf2; neg WORD PTR [ecx]
	.byte 0xf0; .byte 0xf3; neg WORD PTR [ecx]
	xacquire lock not WORD PTR [ecx]
	lock xacquire not WORD PTR [ecx]
	xrelease lock not WORD PTR [ecx]
	lock xrelease not WORD PTR [ecx]
	.byte 0xf0; .byte 0xf2; not WORD PTR [ecx]
	.byte 0xf0; .byte 0xf3; not WORD PTR [ecx]

# Tests for op regl/m32
	xacquire lock dec DWORD PTR [ecx]
	lock xacquire dec DWORD PTR [ecx]
	xrelease lock dec DWORD PTR [ecx]
	lock xrelease dec DWORD PTR [ecx]
	.byte 0xf0; .byte 0xf2; dec DWORD PTR [ecx]
	.byte 0xf0; .byte 0xf3; dec DWORD PTR [ecx]
	xacquire lock inc DWORD PTR [ecx]
	lock xacquire inc DWORD PTR [ecx]
	xrelease lock inc DWORD PTR [ecx]
	lock xrelease inc DWORD PTR [ecx]
	.byte 0xf0; .byte 0xf2; inc DWORD PTR [ecx]
	.byte 0xf0; .byte 0xf3; inc DWORD PTR [ecx]
	xacquire lock neg DWORD PTR [ecx]
	lock xacquire neg DWORD PTR [ecx]
	xrelease lock neg DWORD PTR [ecx]
	lock xrelease neg DWORD PTR [ecx]
	.byte 0xf0; .byte 0xf2; neg DWORD PTR [ecx]
	.byte 0xf0; .byte 0xf3; neg DWORD PTR [ecx]
	xacquire lock not DWORD PTR [ecx]
	lock xacquire not DWORD PTR [ecx]
	xrelease lock not DWORD PTR [ecx]
	lock xrelease not DWORD PTR [ecx]
	.byte 0xf0; .byte 0xf2; not DWORD PTR [ecx]
	.byte 0xf0; .byte 0xf3; not DWORD PTR [ecx]

# Tests for op m64
	xacquire lock cmpxchg8b QWORD PTR [ecx]
	lock xacquire cmpxchg8b QWORD PTR [ecx]
	xrelease lock cmpxchg8b QWORD PTR [ecx]
	lock xrelease cmpxchg8b QWORD PTR [ecx]
	.byte 0xf0; .byte 0xf2; cmpxchg8b QWORD PTR [ecx]
	.byte 0xf0; .byte 0xf3; cmpxchg8b QWORD PTR [ecx]

# Tests for op regb, regb/m8
	xacquire lock cmpxchg BYTE PTR [ecx],cl
	lock xacquire cmpxchg BYTE PTR [ecx],cl
	xrelease lock cmpxchg BYTE PTR [ecx],cl
	lock xrelease cmpxchg BYTE PTR [ecx],cl
	.byte 0xf0; .byte 0xf2; cmpxchg BYTE PTR [ecx],cl
	.byte 0xf0; .byte 0xf3; cmpxchg BYTE PTR [ecx],cl
	xacquire lock xadd BYTE PTR [ecx],cl
	lock xacquire xadd BYTE PTR [ecx],cl
	xrelease lock xadd BYTE PTR [ecx],cl
	lock xrelease xadd BYTE PTR [ecx],cl
	.byte 0xf0; .byte 0xf2; xadd BYTE PTR [ecx],cl
	.byte 0xf0; .byte 0xf3; xadd BYTE PTR [ecx],cl
