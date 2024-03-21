# Check 64bit HLE instructions

	.allow_index_reg
	.text
_start:


# Tests for op imm32 rax

# Tests for op imm8 regb/m8
	xacquire lock adcb $100,(%rcx)
	lock xacquire adcb $100,(%rcx)
	xrelease lock adcb $100,(%rcx)
	lock xrelease adcb $100,(%rcx)
	.byte 0xf0; .byte 0xf2; adcb $100,(%rcx)
	.byte 0xf0; .byte 0xf3; adcb $100,(%rcx)
	xacquire lock addb $100,(%rcx)
	lock xacquire addb $100,(%rcx)
	xrelease lock addb $100,(%rcx)
	lock xrelease addb $100,(%rcx)
	.byte 0xf0; .byte 0xf2; addb $100,(%rcx)
	.byte 0xf0; .byte 0xf3; addb $100,(%rcx)
	xacquire lock andb $100,(%rcx)
	lock xacquire andb $100,(%rcx)
	xrelease lock andb $100,(%rcx)
	lock xrelease andb $100,(%rcx)
	.byte 0xf0; .byte 0xf2; andb $100,(%rcx)
	.byte 0xf0; .byte 0xf3; andb $100,(%rcx)
	xrelease movb $100,(%rcx)
	xacquire lock orb $100,(%rcx)
	lock xacquire orb $100,(%rcx)
	xrelease lock orb $100,(%rcx)
	lock xrelease orb $100,(%rcx)
	.byte 0xf0; .byte 0xf2; orb $100,(%rcx)
	.byte 0xf0; .byte 0xf3; orb $100,(%rcx)
	xacquire lock sbbb $100,(%rcx)
	lock xacquire sbbb $100,(%rcx)
	xrelease lock sbbb $100,(%rcx)
	lock xrelease sbbb $100,(%rcx)
	.byte 0xf0; .byte 0xf2; sbbb $100,(%rcx)
	.byte 0xf0; .byte 0xf3; sbbb $100,(%rcx)
	xacquire lock subb $100,(%rcx)
	lock xacquire subb $100,(%rcx)
	xrelease lock subb $100,(%rcx)
	lock xrelease subb $100,(%rcx)
	.byte 0xf0; .byte 0xf2; subb $100,(%rcx)
	.byte 0xf0; .byte 0xf3; subb $100,(%rcx)
	xacquire lock xorb $100,(%rcx)
	lock xacquire xorb $100,(%rcx)
	xrelease lock xorb $100,(%rcx)
	lock xrelease xorb $100,(%rcx)
	.byte 0xf0; .byte 0xf2; xorb $100,(%rcx)
	.byte 0xf0; .byte 0xf3; xorb $100,(%rcx)

# Tests for op imm16 regs/m16
	xacquire lock adcw $1000,(%rcx)
	lock xacquire adcw $1000,(%rcx)
	xrelease lock adcw $1000,(%rcx)
	lock xrelease adcw $1000,(%rcx)
	.byte 0xf0; .byte 0xf2; adcw $1000,(%rcx)
	.byte 0xf0; .byte 0xf3; adcw $1000,(%rcx)
	xacquire lock addw $1000,(%rcx)
	lock xacquire addw $1000,(%rcx)
	xrelease lock addw $1000,(%rcx)
	lock xrelease addw $1000,(%rcx)
	.byte 0xf0; .byte 0xf2; addw $1000,(%rcx)
	.byte 0xf0; .byte 0xf3; addw $1000,(%rcx)
	xacquire lock andw $1000,(%rcx)
	lock xacquire andw $1000,(%rcx)
	xrelease lock andw $1000,(%rcx)
	lock xrelease andw $1000,(%rcx)
	.byte 0xf0; .byte 0xf2; andw $1000,(%rcx)
	.byte 0xf0; .byte 0xf3; andw $1000,(%rcx)
	xrelease movw $1000,(%rcx)
	xacquire lock orw $1000,(%rcx)
	lock xacquire orw $1000,(%rcx)
	xrelease lock orw $1000,(%rcx)
	lock xrelease orw $1000,(%rcx)
	.byte 0xf0; .byte 0xf2; orw $1000,(%rcx)
	.byte 0xf0; .byte 0xf3; orw $1000,(%rcx)
	xacquire lock sbbw $1000,(%rcx)
	lock xacquire sbbw $1000,(%rcx)
	xrelease lock sbbw $1000,(%rcx)
	lock xrelease sbbw $1000,(%rcx)
	.byte 0xf0; .byte 0xf2; sbbw $1000,(%rcx)
	.byte 0xf0; .byte 0xf3; sbbw $1000,(%rcx)
	xacquire lock subw $1000,(%rcx)
	lock xacquire subw $1000,(%rcx)
	xrelease lock subw $1000,(%rcx)
	lock xrelease subw $1000,(%rcx)
	.byte 0xf0; .byte 0xf2; subw $1000,(%rcx)
	.byte 0xf0; .byte 0xf3; subw $1000,(%rcx)
	xacquire lock xorw $1000,(%rcx)
	lock xacquire xorw $1000,(%rcx)
	xrelease lock xorw $1000,(%rcx)
	lock xrelease xorw $1000,(%rcx)
	.byte 0xf0; .byte 0xf2; xorw $1000,(%rcx)
	.byte 0xf0; .byte 0xf3; xorw $1000,(%rcx)

# Tests for op imm32 regl/m32
	xacquire lock adcl $10000000,(%rcx)
	lock xacquire adcl $10000000,(%rcx)
	xrelease lock adcl $10000000,(%rcx)
	lock xrelease adcl $10000000,(%rcx)
	.byte 0xf0; .byte 0xf2; adcl $10000000,(%rcx)
	.byte 0xf0; .byte 0xf3; adcl $10000000,(%rcx)
	xacquire lock addl $10000000,(%rcx)
	lock xacquire addl $10000000,(%rcx)
	xrelease lock addl $10000000,(%rcx)
	lock xrelease addl $10000000,(%rcx)
	.byte 0xf0; .byte 0xf2; addl $10000000,(%rcx)
	.byte 0xf0; .byte 0xf3; addl $10000000,(%rcx)
	xacquire lock andl $10000000,(%rcx)
	lock xacquire andl $10000000,(%rcx)
	xrelease lock andl $10000000,(%rcx)
	lock xrelease andl $10000000,(%rcx)
	.byte 0xf0; .byte 0xf2; andl $10000000,(%rcx)
	.byte 0xf0; .byte 0xf3; andl $10000000,(%rcx)
	xrelease movl $10000000,(%rcx)
	xacquire lock orl $10000000,(%rcx)
	lock xacquire orl $10000000,(%rcx)
	xrelease lock orl $10000000,(%rcx)
	lock xrelease orl $10000000,(%rcx)
	.byte 0xf0; .byte 0xf2; orl $10000000,(%rcx)
	.byte 0xf0; .byte 0xf3; orl $10000000,(%rcx)
	xacquire lock sbbl $10000000,(%rcx)
	lock xacquire sbbl $10000000,(%rcx)
	xrelease lock sbbl $10000000,(%rcx)
	lock xrelease sbbl $10000000,(%rcx)
	.byte 0xf0; .byte 0xf2; sbbl $10000000,(%rcx)
	.byte 0xf0; .byte 0xf3; sbbl $10000000,(%rcx)
	xacquire lock subl $10000000,(%rcx)
	lock xacquire subl $10000000,(%rcx)
	xrelease lock subl $10000000,(%rcx)
	lock xrelease subl $10000000,(%rcx)
	.byte 0xf0; .byte 0xf2; subl $10000000,(%rcx)
	.byte 0xf0; .byte 0xf3; subl $10000000,(%rcx)
	xacquire lock xorl $10000000,(%rcx)
	lock xacquire xorl $10000000,(%rcx)
	xrelease lock xorl $10000000,(%rcx)
	lock xrelease xorl $10000000,(%rcx)
	.byte 0xf0; .byte 0xf2; xorl $10000000,(%rcx)
	.byte 0xf0; .byte 0xf3; xorl $10000000,(%rcx)

# Tests for op imm32 regq/m64
	xacquire lock adcq $10000000,(%rcx)
	lock xacquire adcq $10000000,(%rcx)
	xrelease lock adcq $10000000,(%rcx)
	lock xrelease adcq $10000000,(%rcx)
	.byte 0xf0; .byte 0xf2; adcq $10000000,(%rcx)
	.byte 0xf0; .byte 0xf3; adcq $10000000,(%rcx)
	xacquire lock addq $10000000,(%rcx)
	lock xacquire addq $10000000,(%rcx)
	xrelease lock addq $10000000,(%rcx)
	lock xrelease addq $10000000,(%rcx)
	.byte 0xf0; .byte 0xf2; addq $10000000,(%rcx)
	.byte 0xf0; .byte 0xf3; addq $10000000,(%rcx)
	xacquire lock andq $10000000,(%rcx)
	lock xacquire andq $10000000,(%rcx)
	xrelease lock andq $10000000,(%rcx)
	lock xrelease andq $10000000,(%rcx)
	.byte 0xf0; .byte 0xf2; andq $10000000,(%rcx)
	.byte 0xf0; .byte 0xf3; andq $10000000,(%rcx)
	xrelease movq $10000000,(%rcx)
	xacquire lock orq $10000000,(%rcx)
	lock xacquire orq $10000000,(%rcx)
	xrelease lock orq $10000000,(%rcx)
	lock xrelease orq $10000000,(%rcx)
	.byte 0xf0; .byte 0xf2; orq $10000000,(%rcx)
	.byte 0xf0; .byte 0xf3; orq $10000000,(%rcx)
	xacquire lock sbbq $10000000,(%rcx)
	lock xacquire sbbq $10000000,(%rcx)
	xrelease lock sbbq $10000000,(%rcx)
	lock xrelease sbbq $10000000,(%rcx)
	.byte 0xf0; .byte 0xf2; sbbq $10000000,(%rcx)
	.byte 0xf0; .byte 0xf3; sbbq $10000000,(%rcx)
	xacquire lock subq $10000000,(%rcx)
	lock xacquire subq $10000000,(%rcx)
	xrelease lock subq $10000000,(%rcx)
	lock xrelease subq $10000000,(%rcx)
	.byte 0xf0; .byte 0xf2; subq $10000000,(%rcx)
	.byte 0xf0; .byte 0xf3; subq $10000000,(%rcx)
	xacquire lock xorq $10000000,(%rcx)
	lock xacquire xorq $10000000,(%rcx)
	xrelease lock xorq $10000000,(%rcx)
	lock xrelease xorq $10000000,(%rcx)
	.byte 0xf0; .byte 0xf2; xorq $10000000,(%rcx)
	.byte 0xf0; .byte 0xf3; xorq $10000000,(%rcx)

# Tests for op imm8 regs/m16
	xacquire lock adcw $100,(%rcx)
	lock xacquire adcw $100,(%rcx)
	xrelease lock adcw $100,(%rcx)
	lock xrelease adcw $100,(%rcx)
	.byte 0xf0; .byte 0xf2; adcw $100,(%rcx)
	.byte 0xf0; .byte 0xf3; adcw $100,(%rcx)
	xacquire lock addw $100,(%rcx)
	lock xacquire addw $100,(%rcx)
	xrelease lock addw $100,(%rcx)
	lock xrelease addw $100,(%rcx)
	.byte 0xf0; .byte 0xf2; addw $100,(%rcx)
	.byte 0xf0; .byte 0xf3; addw $100,(%rcx)
	xacquire lock andw $100,(%rcx)
	lock xacquire andw $100,(%rcx)
	xrelease lock andw $100,(%rcx)
	lock xrelease andw $100,(%rcx)
	.byte 0xf0; .byte 0xf2; andw $100,(%rcx)
	.byte 0xf0; .byte 0xf3; andw $100,(%rcx)
	xacquire lock btcw $100,(%rcx)
	lock xacquire btcw $100,(%rcx)
	xrelease lock btcw $100,(%rcx)
	lock xrelease btcw $100,(%rcx)
	.byte 0xf0; .byte 0xf2; btcw $100,(%rcx)
	.byte 0xf0; .byte 0xf3; btcw $100,(%rcx)
	xacquire lock btrw $100,(%rcx)
	lock xacquire btrw $100,(%rcx)
	xrelease lock btrw $100,(%rcx)
	lock xrelease btrw $100,(%rcx)
	.byte 0xf0; .byte 0xf2; btrw $100,(%rcx)
	.byte 0xf0; .byte 0xf3; btrw $100,(%rcx)
	xacquire lock btsw $100,(%rcx)
	lock xacquire btsw $100,(%rcx)
	xrelease lock btsw $100,(%rcx)
	lock xrelease btsw $100,(%rcx)
	.byte 0xf0; .byte 0xf2; btsw $100,(%rcx)
	.byte 0xf0; .byte 0xf3; btsw $100,(%rcx)
	xrelease movw $100,(%rcx)
	xacquire lock orw $100,(%rcx)
	lock xacquire orw $100,(%rcx)
	xrelease lock orw $100,(%rcx)
	lock xrelease orw $100,(%rcx)
	.byte 0xf0; .byte 0xf2; orw $100,(%rcx)
	.byte 0xf0; .byte 0xf3; orw $100,(%rcx)
	xacquire lock sbbw $100,(%rcx)
	lock xacquire sbbw $100,(%rcx)
	xrelease lock sbbw $100,(%rcx)
	lock xrelease sbbw $100,(%rcx)
	.byte 0xf0; .byte 0xf2; sbbw $100,(%rcx)
	.byte 0xf0; .byte 0xf3; sbbw $100,(%rcx)
	xacquire lock subw $100,(%rcx)
	lock xacquire subw $100,(%rcx)
	xrelease lock subw $100,(%rcx)
	lock xrelease subw $100,(%rcx)
	.byte 0xf0; .byte 0xf2; subw $100,(%rcx)
	.byte 0xf0; .byte 0xf3; subw $100,(%rcx)
	xacquire lock xorw $100,(%rcx)
	lock xacquire xorw $100,(%rcx)
	xrelease lock xorw $100,(%rcx)
	lock xrelease xorw $100,(%rcx)
	.byte 0xf0; .byte 0xf2; xorw $100,(%rcx)
	.byte 0xf0; .byte 0xf3; xorw $100,(%rcx)

# Tests for op imm8 regl/m32
	xacquire lock adcl $100,(%rcx)
	lock xacquire adcl $100,(%rcx)
	xrelease lock adcl $100,(%rcx)
	lock xrelease adcl $100,(%rcx)
	.byte 0xf0; .byte 0xf2; adcl $100,(%rcx)
	.byte 0xf0; .byte 0xf3; adcl $100,(%rcx)
	xacquire lock addl $100,(%rcx)
	lock xacquire addl $100,(%rcx)
	xrelease lock addl $100,(%rcx)
	lock xrelease addl $100,(%rcx)
	.byte 0xf0; .byte 0xf2; addl $100,(%rcx)
	.byte 0xf0; .byte 0xf3; addl $100,(%rcx)
	xacquire lock andl $100,(%rcx)
	lock xacquire andl $100,(%rcx)
	xrelease lock andl $100,(%rcx)
	lock xrelease andl $100,(%rcx)
	.byte 0xf0; .byte 0xf2; andl $100,(%rcx)
	.byte 0xf0; .byte 0xf3; andl $100,(%rcx)
	xacquire lock btcl $100,(%rcx)
	lock xacquire btcl $100,(%rcx)
	xrelease lock btcl $100,(%rcx)
	lock xrelease btcl $100,(%rcx)
	.byte 0xf0; .byte 0xf2; btcl $100,(%rcx)
	.byte 0xf0; .byte 0xf3; btcl $100,(%rcx)
	xacquire lock btrl $100,(%rcx)
	lock xacquire btrl $100,(%rcx)
	xrelease lock btrl $100,(%rcx)
	lock xrelease btrl $100,(%rcx)
	.byte 0xf0; .byte 0xf2; btrl $100,(%rcx)
	.byte 0xf0; .byte 0xf3; btrl $100,(%rcx)
	xacquire lock btsl $100,(%rcx)
	lock xacquire btsl $100,(%rcx)
	xrelease lock btsl $100,(%rcx)
	lock xrelease btsl $100,(%rcx)
	.byte 0xf0; .byte 0xf2; btsl $100,(%rcx)
	.byte 0xf0; .byte 0xf3; btsl $100,(%rcx)
	xrelease movl $100,(%rcx)
	xacquire lock orl $100,(%rcx)
	lock xacquire orl $100,(%rcx)
	xrelease lock orl $100,(%rcx)
	lock xrelease orl $100,(%rcx)
	.byte 0xf0; .byte 0xf2; orl $100,(%rcx)
	.byte 0xf0; .byte 0xf3; orl $100,(%rcx)
	xacquire lock sbbl $100,(%rcx)
	lock xacquire sbbl $100,(%rcx)
	xrelease lock sbbl $100,(%rcx)
	lock xrelease sbbl $100,(%rcx)
	.byte 0xf0; .byte 0xf2; sbbl $100,(%rcx)
	.byte 0xf0; .byte 0xf3; sbbl $100,(%rcx)
	xacquire lock subl $100,(%rcx)
	lock xacquire subl $100,(%rcx)
	xrelease lock subl $100,(%rcx)
	lock xrelease subl $100,(%rcx)
	.byte 0xf0; .byte 0xf2; subl $100,(%rcx)
	.byte 0xf0; .byte 0xf3; subl $100,(%rcx)
	xacquire lock xorl $100,(%rcx)
	lock xacquire xorl $100,(%rcx)
	xrelease lock xorl $100,(%rcx)
	lock xrelease xorl $100,(%rcx)
	.byte 0xf0; .byte 0xf2; xorl $100,(%rcx)
	.byte 0xf0; .byte 0xf3; xorl $100,(%rcx)

# Tests for op imm8 regq/m64
	xacquire lock adcq $100,(%rcx)
	lock xacquire adcq $100,(%rcx)
	xrelease lock adcq $100,(%rcx)
	lock xrelease adcq $100,(%rcx)
	.byte 0xf0; .byte 0xf2; adcq $100,(%rcx)
	.byte 0xf0; .byte 0xf3; adcq $100,(%rcx)
	xacquire lock addq $100,(%rcx)
	lock xacquire addq $100,(%rcx)
	xrelease lock addq $100,(%rcx)
	lock xrelease addq $100,(%rcx)
	.byte 0xf0; .byte 0xf2; addq $100,(%rcx)
	.byte 0xf0; .byte 0xf3; addq $100,(%rcx)
	xacquire lock andq $100,(%rcx)
	lock xacquire andq $100,(%rcx)
	xrelease lock andq $100,(%rcx)
	lock xrelease andq $100,(%rcx)
	.byte 0xf0; .byte 0xf2; andq $100,(%rcx)
	.byte 0xf0; .byte 0xf3; andq $100,(%rcx)
	xacquire lock btcq $100,(%rcx)
	lock xacquire btcq $100,(%rcx)
	xrelease lock btcq $100,(%rcx)
	lock xrelease btcq $100,(%rcx)
	.byte 0xf0; .byte 0xf2; btcq $100,(%rcx)
	.byte 0xf0; .byte 0xf3; btcq $100,(%rcx)
	xacquire lock btrq $100,(%rcx)
	lock xacquire btrq $100,(%rcx)
	xrelease lock btrq $100,(%rcx)
	lock xrelease btrq $100,(%rcx)
	.byte 0xf0; .byte 0xf2; btrq $100,(%rcx)
	.byte 0xf0; .byte 0xf3; btrq $100,(%rcx)
	xacquire lock btsq $100,(%rcx)
	lock xacquire btsq $100,(%rcx)
	xrelease lock btsq $100,(%rcx)
	lock xrelease btsq $100,(%rcx)
	.byte 0xf0; .byte 0xf2; btsq $100,(%rcx)
	.byte 0xf0; .byte 0xf3; btsq $100,(%rcx)
	xrelease movq $100,(%rcx)
	xacquire lock orq $100,(%rcx)
	lock xacquire orq $100,(%rcx)
	xrelease lock orq $100,(%rcx)
	lock xrelease orq $100,(%rcx)
	.byte 0xf0; .byte 0xf2; orq $100,(%rcx)
	.byte 0xf0; .byte 0xf3; orq $100,(%rcx)
	xacquire lock sbbq $100,(%rcx)
	lock xacquire sbbq $100,(%rcx)
	xrelease lock sbbq $100,(%rcx)
	lock xrelease sbbq $100,(%rcx)
	.byte 0xf0; .byte 0xf2; sbbq $100,(%rcx)
	.byte 0xf0; .byte 0xf3; sbbq $100,(%rcx)
	xacquire lock subq $100,(%rcx)
	lock xacquire subq $100,(%rcx)
	xrelease lock subq $100,(%rcx)
	lock xrelease subq $100,(%rcx)
	.byte 0xf0; .byte 0xf2; subq $100,(%rcx)
	.byte 0xf0; .byte 0xf3; subq $100,(%rcx)
	xacquire lock xorq $100,(%rcx)
	lock xacquire xorq $100,(%rcx)
	xrelease lock xorq $100,(%rcx)
	lock xrelease xorq $100,(%rcx)
	.byte 0xf0; .byte 0xf2; xorq $100,(%rcx)
	.byte 0xf0; .byte 0xf3; xorq $100,(%rcx)

# Tests for op imm8 regb/m8
	xacquire lock adcb $100,(%rcx)
	lock xacquire adcb $100,(%rcx)
	xrelease lock adcb $100,(%rcx)
	lock xrelease adcb $100,(%rcx)
	.byte 0xf0; .byte 0xf2; adcb $100,(%rcx)
	.byte 0xf0; .byte 0xf3; adcb $100,(%rcx)
	xacquire lock addb $100,(%rcx)
	lock xacquire addb $100,(%rcx)
	xrelease lock addb $100,(%rcx)
	lock xrelease addb $100,(%rcx)
	.byte 0xf0; .byte 0xf2; addb $100,(%rcx)
	.byte 0xf0; .byte 0xf3; addb $100,(%rcx)
	xacquire lock andb $100,(%rcx)
	lock xacquire andb $100,(%rcx)
	xrelease lock andb $100,(%rcx)
	lock xrelease andb $100,(%rcx)
	.byte 0xf0; .byte 0xf2; andb $100,(%rcx)
	.byte 0xf0; .byte 0xf3; andb $100,(%rcx)
	xrelease movb $100,(%rcx)
	xacquire lock orb $100,(%rcx)
	lock xacquire orb $100,(%rcx)
	xrelease lock orb $100,(%rcx)
	lock xrelease orb $100,(%rcx)
	.byte 0xf0; .byte 0xf2; orb $100,(%rcx)
	.byte 0xf0; .byte 0xf3; orb $100,(%rcx)
	xacquire lock sbbb $100,(%rcx)
	lock xacquire sbbb $100,(%rcx)
	xrelease lock sbbb $100,(%rcx)
	lock xrelease sbbb $100,(%rcx)
	.byte 0xf0; .byte 0xf2; sbbb $100,(%rcx)
	.byte 0xf0; .byte 0xf3; sbbb $100,(%rcx)
	xacquire lock subb $100,(%rcx)
	lock xacquire subb $100,(%rcx)
	xrelease lock subb $100,(%rcx)
	lock xrelease subb $100,(%rcx)
	.byte 0xf0; .byte 0xf2; subb $100,(%rcx)
	.byte 0xf0; .byte 0xf3; subb $100,(%rcx)
	xacquire lock xorb $100,(%rcx)
	lock xacquire xorb $100,(%rcx)
	xrelease lock xorb $100,(%rcx)
	lock xrelease xorb $100,(%rcx)
	.byte 0xf0; .byte 0xf2; xorb $100,(%rcx)
	.byte 0xf0; .byte 0xf3; xorb $100,(%rcx)

# Tests for op regb regb/m8
# Tests for op regb/m8 regb
	xacquire lock adcb %al,(%rcx)
	lock xacquire adcb %al,(%rcx)
	xrelease lock adcb %al,(%rcx)
	lock xrelease adcb %al,(%rcx)
	.byte 0xf0; .byte 0xf2; adcb %al,(%rcx)
	.byte 0xf0; .byte 0xf3; adcb %al,(%rcx)
	xacquire lock addb %al,(%rcx)
	lock xacquire addb %al,(%rcx)
	xrelease lock addb %al,(%rcx)
	lock xrelease addb %al,(%rcx)
	.byte 0xf0; .byte 0xf2; addb %al,(%rcx)
	.byte 0xf0; .byte 0xf3; addb %al,(%rcx)
	xacquire lock andb %al,(%rcx)
	lock xacquire andb %al,(%rcx)
	xrelease lock andb %al,(%rcx)
	lock xrelease andb %al,(%rcx)
	.byte 0xf0; .byte 0xf2; andb %al,(%rcx)
	.byte 0xf0; .byte 0xf3; andb %al,(%rcx)
	xrelease movb %al,(%rcx)
	xrelease movb %al,0x12345678
	xrelease addr32 movb %al,0x87654321
	xacquire lock orb %al,(%rcx)
	lock xacquire orb %al,(%rcx)
	xrelease lock orb %al,(%rcx)
	lock xrelease orb %al,(%rcx)
	.byte 0xf0; .byte 0xf2; orb %al,(%rcx)
	.byte 0xf0; .byte 0xf3; orb %al,(%rcx)
	xacquire lock sbbb %al,(%rcx)
	lock xacquire sbbb %al,(%rcx)
	xrelease lock sbbb %al,(%rcx)
	lock xrelease sbbb %al,(%rcx)
	.byte 0xf0; .byte 0xf2; sbbb %al,(%rcx)
	.byte 0xf0; .byte 0xf3; sbbb %al,(%rcx)
	xacquire lock subb %al,(%rcx)
	lock xacquire subb %al,(%rcx)
	xrelease lock subb %al,(%rcx)
	lock xrelease subb %al,(%rcx)
	.byte 0xf0; .byte 0xf2; subb %al,(%rcx)
	.byte 0xf0; .byte 0xf3; subb %al,(%rcx)
	xacquire lock xchgb %al,(%rcx)
	lock xacquire xchgb %al,(%rcx)
	xacquire xchgb %al,(%rcx)
	xrelease lock xchgb %al,(%rcx)
	lock xrelease xchgb %al,(%rcx)
	xrelease xchgb %al,(%rcx)
	.byte 0xf0; .byte 0xf2; xchgb %al,(%rcx)
	.byte 0xf0; .byte 0xf3; xchgb %al,(%rcx)
	xacquire lock xorb %al,(%rcx)
	lock xacquire xorb %al,(%rcx)
	xrelease lock xorb %al,(%rcx)
	lock xrelease xorb %al,(%rcx)
	.byte 0xf0; .byte 0xf2; xorb %al,(%rcx)
	.byte 0xf0; .byte 0xf3; xorb %al,(%rcx)

# Tests for op regs regs/m16
# Tests for op regs/m16 regs
	xacquire lock adcw %ax,(%rcx)
	lock xacquire adcw %ax,(%rcx)
	xrelease lock adcw %ax,(%rcx)
	lock xrelease adcw %ax,(%rcx)
	.byte 0xf0; .byte 0xf2; adcw %ax,(%rcx)
	.byte 0xf0; .byte 0xf3; adcw %ax,(%rcx)
	xacquire lock addw %ax,(%rcx)
	lock xacquire addw %ax,(%rcx)
	xrelease lock addw %ax,(%rcx)
	lock xrelease addw %ax,(%rcx)
	.byte 0xf0; .byte 0xf2; addw %ax,(%rcx)
	.byte 0xf0; .byte 0xf3; addw %ax,(%rcx)
	xacquire lock andw %ax,(%rcx)
	lock xacquire andw %ax,(%rcx)
	xrelease lock andw %ax,(%rcx)
	lock xrelease andw %ax,(%rcx)
	.byte 0xf0; .byte 0xf2; andw %ax,(%rcx)
	.byte 0xf0; .byte 0xf3; andw %ax,(%rcx)
	xrelease movw %ax,(%rcx)
	xrelease movw %ax,0x12345678
	xrelease addr32 movw %ax,0x87654321
	xacquire lock orw %ax,(%rcx)
	lock xacquire orw %ax,(%rcx)
	xrelease lock orw %ax,(%rcx)
	lock xrelease orw %ax,(%rcx)
	.byte 0xf0; .byte 0xf2; orw %ax,(%rcx)
	.byte 0xf0; .byte 0xf3; orw %ax,(%rcx)
	xacquire lock sbbw %ax,(%rcx)
	lock xacquire sbbw %ax,(%rcx)
	xrelease lock sbbw %ax,(%rcx)
	lock xrelease sbbw %ax,(%rcx)
	.byte 0xf0; .byte 0xf2; sbbw %ax,(%rcx)
	.byte 0xf0; .byte 0xf3; sbbw %ax,(%rcx)
	xacquire lock subw %ax,(%rcx)
	lock xacquire subw %ax,(%rcx)
	xrelease lock subw %ax,(%rcx)
	lock xrelease subw %ax,(%rcx)
	.byte 0xf0; .byte 0xf2; subw %ax,(%rcx)
	.byte 0xf0; .byte 0xf3; subw %ax,(%rcx)
	xacquire lock xchgw %ax,(%rcx)
	lock xacquire xchgw %ax,(%rcx)
	xacquire xchgw %ax,(%rcx)
	xrelease lock xchgw %ax,(%rcx)
	lock xrelease xchgw %ax,(%rcx)
	xrelease xchgw %ax,(%rcx)
	.byte 0xf0; .byte 0xf2; xchgw %ax,(%rcx)
	.byte 0xf0; .byte 0xf3; xchgw %ax,(%rcx)
	xacquire lock xorw %ax,(%rcx)
	lock xacquire xorw %ax,(%rcx)
	xrelease lock xorw %ax,(%rcx)
	lock xrelease xorw %ax,(%rcx)
	.byte 0xf0; .byte 0xf2; xorw %ax,(%rcx)
	.byte 0xf0; .byte 0xf3; xorw %ax,(%rcx)

# Tests for op regl regl/m32
# Tests for op regl/m32 regl
	xacquire lock adcl %eax,(%rcx)
	lock xacquire adcl %eax,(%rcx)
	xrelease lock adcl %eax,(%rcx)
	lock xrelease adcl %eax,(%rcx)
	.byte 0xf0; .byte 0xf2; adcl %eax,(%rcx)
	.byte 0xf0; .byte 0xf3; adcl %eax,(%rcx)
	xacquire lock addl %eax,(%rcx)
	lock xacquire addl %eax,(%rcx)
	xrelease lock addl %eax,(%rcx)
	lock xrelease addl %eax,(%rcx)
	.byte 0xf0; .byte 0xf2; addl %eax,(%rcx)
	.byte 0xf0; .byte 0xf3; addl %eax,(%rcx)
	xacquire lock andl %eax,(%rcx)
	lock xacquire andl %eax,(%rcx)
	xrelease lock andl %eax,(%rcx)
	lock xrelease andl %eax,(%rcx)
	.byte 0xf0; .byte 0xf2; andl %eax,(%rcx)
	.byte 0xf0; .byte 0xf3; andl %eax,(%rcx)
	xrelease movl %eax,(%rcx)
	xrelease movl %eax,0x12345678
	xrelease addr32 movl %eax,0x87654321
	xacquire lock orl %eax,(%rcx)
	lock xacquire orl %eax,(%rcx)
	xrelease lock orl %eax,(%rcx)
	lock xrelease orl %eax,(%rcx)
	.byte 0xf0; .byte 0xf2; orl %eax,(%rcx)
	.byte 0xf0; .byte 0xf3; orl %eax,(%rcx)
	xacquire lock sbbl %eax,(%rcx)
	lock xacquire sbbl %eax,(%rcx)
	xrelease lock sbbl %eax,(%rcx)
	lock xrelease sbbl %eax,(%rcx)
	.byte 0xf0; .byte 0xf2; sbbl %eax,(%rcx)
	.byte 0xf0; .byte 0xf3; sbbl %eax,(%rcx)
	xacquire lock subl %eax,(%rcx)
	lock xacquire subl %eax,(%rcx)
	xrelease lock subl %eax,(%rcx)
	lock xrelease subl %eax,(%rcx)
	.byte 0xf0; .byte 0xf2; subl %eax,(%rcx)
	.byte 0xf0; .byte 0xf3; subl %eax,(%rcx)
	xacquire lock xchgl %eax,(%rcx)
	lock xacquire xchgl %eax,(%rcx)
	xacquire xchgl %eax,(%rcx)
	xrelease lock xchgl %eax,(%rcx)
	lock xrelease xchgl %eax,(%rcx)
	xrelease xchgl %eax,(%rcx)
	.byte 0xf0; .byte 0xf2; xchgl %eax,(%rcx)
	.byte 0xf0; .byte 0xf3; xchgl %eax,(%rcx)
	xacquire lock xorl %eax,(%rcx)
	lock xacquire xorl %eax,(%rcx)
	xrelease lock xorl %eax,(%rcx)
	lock xrelease xorl %eax,(%rcx)
	.byte 0xf0; .byte 0xf2; xorl %eax,(%rcx)
	.byte 0xf0; .byte 0xf3; xorl %eax,(%rcx)

# Tests for op regq regq/m64
# Tests for op regq/m64 regq
	xacquire lock adcq %rax,(%rcx)
	lock xacquire adcq %rax,(%rcx)
	xrelease lock adcq %rax,(%rcx)
	lock xrelease adcq %rax,(%rcx)
	.byte 0xf0; .byte 0xf2; adcq %rax,(%rcx)
	.byte 0xf0; .byte 0xf3; adcq %rax,(%rcx)
	xacquire lock addq %rax,(%rcx)
	lock xacquire addq %rax,(%rcx)
	xrelease lock addq %rax,(%rcx)
	lock xrelease addq %rax,(%rcx)
	.byte 0xf0; .byte 0xf2; addq %rax,(%rcx)
	.byte 0xf0; .byte 0xf3; addq %rax,(%rcx)
	xacquire lock andq %rax,(%rcx)
	lock xacquire andq %rax,(%rcx)
	xrelease lock andq %rax,(%rcx)
	lock xrelease andq %rax,(%rcx)
	.byte 0xf0; .byte 0xf2; andq %rax,(%rcx)
	.byte 0xf0; .byte 0xf3; andq %rax,(%rcx)
	xrelease movq %rax,(%rcx)
	xrelease movq %rax,0x12345678
	xrelease addr32 movq %rax,0x87654321
	xacquire lock orq %rax,(%rcx)
	lock xacquire orq %rax,(%rcx)
	xrelease lock orq %rax,(%rcx)
	lock xrelease orq %rax,(%rcx)
	.byte 0xf0; .byte 0xf2; orq %rax,(%rcx)
	.byte 0xf0; .byte 0xf3; orq %rax,(%rcx)
	xacquire lock sbbq %rax,(%rcx)
	lock xacquire sbbq %rax,(%rcx)
	xrelease lock sbbq %rax,(%rcx)
	lock xrelease sbbq %rax,(%rcx)
	.byte 0xf0; .byte 0xf2; sbbq %rax,(%rcx)
	.byte 0xf0; .byte 0xf3; sbbq %rax,(%rcx)
	xacquire lock subq %rax,(%rcx)
	lock xacquire subq %rax,(%rcx)
	xrelease lock subq %rax,(%rcx)
	lock xrelease subq %rax,(%rcx)
	.byte 0xf0; .byte 0xf2; subq %rax,(%rcx)
	.byte 0xf0; .byte 0xf3; subq %rax,(%rcx)
	xacquire lock xchgq %rax,(%rcx)
	lock xacquire xchgq %rax,(%rcx)
	xacquire xchgq %rax,(%rcx)
	xrelease lock xchgq %rax,(%rcx)
	lock xrelease xchgq %rax,(%rcx)
	xrelease xchgq %rax,(%rcx)
	.byte 0xf0; .byte 0xf2; xchgq %rax,(%rcx)
	.byte 0xf0; .byte 0xf3; xchgq %rax,(%rcx)
	xacquire lock xorq %rax,(%rcx)
	lock xacquire xorq %rax,(%rcx)
	xrelease lock xorq %rax,(%rcx)
	lock xrelease xorq %rax,(%rcx)
	.byte 0xf0; .byte 0xf2; xorq %rax,(%rcx)
	.byte 0xf0; .byte 0xf3; xorq %rax,(%rcx)

# Tests for op regs, regs/m16
	xacquire lock btcw %ax,(%rcx)
	lock xacquire btcw %ax,(%rcx)
	xrelease lock btcw %ax,(%rcx)
	lock xrelease btcw %ax,(%rcx)
	.byte 0xf0; .byte 0xf2; btcw %ax,(%rcx)
	.byte 0xf0; .byte 0xf3; btcw %ax,(%rcx)
	xacquire lock btrw %ax,(%rcx)
	lock xacquire btrw %ax,(%rcx)
	xrelease lock btrw %ax,(%rcx)
	lock xrelease btrw %ax,(%rcx)
	.byte 0xf0; .byte 0xf2; btrw %ax,(%rcx)
	.byte 0xf0; .byte 0xf3; btrw %ax,(%rcx)
	xacquire lock btsw %ax,(%rcx)
	lock xacquire btsw %ax,(%rcx)
	xrelease lock btsw %ax,(%rcx)
	lock xrelease btsw %ax,(%rcx)
	.byte 0xf0; .byte 0xf2; btsw %ax,(%rcx)
	.byte 0xf0; .byte 0xf3; btsw %ax,(%rcx)
	xacquire lock cmpxchgw %ax,(%rcx)
	lock xacquire cmpxchgw %ax,(%rcx)
	xrelease lock cmpxchgw %ax,(%rcx)
	lock xrelease cmpxchgw %ax,(%rcx)
	.byte 0xf0; .byte 0xf2; cmpxchgw %ax,(%rcx)
	.byte 0xf0; .byte 0xf3; cmpxchgw %ax,(%rcx)
	xacquire lock xaddw %ax,(%rcx)
	lock xacquire xaddw %ax,(%rcx)
	xrelease lock xaddw %ax,(%rcx)
	lock xrelease xaddw %ax,(%rcx)
	.byte 0xf0; .byte 0xf2; xaddw %ax,(%rcx)
	.byte 0xf0; .byte 0xf3; xaddw %ax,(%rcx)

# Tests for op regl regl/m32
	xacquire lock btcl %eax,(%rcx)
	lock xacquire btcl %eax,(%rcx)
	xrelease lock btcl %eax,(%rcx)
	lock xrelease btcl %eax,(%rcx)
	.byte 0xf0; .byte 0xf2; btcl %eax,(%rcx)
	.byte 0xf0; .byte 0xf3; btcl %eax,(%rcx)
	xacquire lock btrl %eax,(%rcx)
	lock xacquire btrl %eax,(%rcx)
	xrelease lock btrl %eax,(%rcx)
	lock xrelease btrl %eax,(%rcx)
	.byte 0xf0; .byte 0xf2; btrl %eax,(%rcx)
	.byte 0xf0; .byte 0xf3; btrl %eax,(%rcx)
	xacquire lock btsl %eax,(%rcx)
	lock xacquire btsl %eax,(%rcx)
	xrelease lock btsl %eax,(%rcx)
	lock xrelease btsl %eax,(%rcx)
	.byte 0xf0; .byte 0xf2; btsl %eax,(%rcx)
	.byte 0xf0; .byte 0xf3; btsl %eax,(%rcx)
	xacquire lock cmpxchgl %eax,(%rcx)
	lock xacquire cmpxchgl %eax,(%rcx)
	xrelease lock cmpxchgl %eax,(%rcx)
	lock xrelease cmpxchgl %eax,(%rcx)
	.byte 0xf0; .byte 0xf2; cmpxchgl %eax,(%rcx)
	.byte 0xf0; .byte 0xf3; cmpxchgl %eax,(%rcx)
	xacquire lock xaddl %eax,(%rcx)
	lock xacquire xaddl %eax,(%rcx)
	xrelease lock xaddl %eax,(%rcx)
	lock xrelease xaddl %eax,(%rcx)
	.byte 0xf0; .byte 0xf2; xaddl %eax,(%rcx)
	.byte 0xf0; .byte 0xf3; xaddl %eax,(%rcx)

# Tests for op regq regq/m64
	xacquire lock btcq %rax,(%rcx)
	lock xacquire btcq %rax,(%rcx)
	xrelease lock btcq %rax,(%rcx)
	lock xrelease btcq %rax,(%rcx)
	.byte 0xf0; .byte 0xf2; btcq %rax,(%rcx)
	.byte 0xf0; .byte 0xf3; btcq %rax,(%rcx)
	xacquire lock btrq %rax,(%rcx)
	lock xacquire btrq %rax,(%rcx)
	xrelease lock btrq %rax,(%rcx)
	lock xrelease btrq %rax,(%rcx)
	.byte 0xf0; .byte 0xf2; btrq %rax,(%rcx)
	.byte 0xf0; .byte 0xf3; btrq %rax,(%rcx)
	xacquire lock btsq %rax,(%rcx)
	lock xacquire btsq %rax,(%rcx)
	xrelease lock btsq %rax,(%rcx)
	lock xrelease btsq %rax,(%rcx)
	.byte 0xf0; .byte 0xf2; btsq %rax,(%rcx)
	.byte 0xf0; .byte 0xf3; btsq %rax,(%rcx)
	xacquire lock cmpxchgq %rax,(%rcx)
	lock xacquire cmpxchgq %rax,(%rcx)
	xrelease lock cmpxchgq %rax,(%rcx)
	lock xrelease cmpxchgq %rax,(%rcx)
	.byte 0xf0; .byte 0xf2; cmpxchgq %rax,(%rcx)
	.byte 0xf0; .byte 0xf3; cmpxchgq %rax,(%rcx)
	xacquire lock xaddq %rax,(%rcx)
	lock xacquire xaddq %rax,(%rcx)
	xrelease lock xaddq %rax,(%rcx)
	lock xrelease xaddq %rax,(%rcx)
	.byte 0xf0; .byte 0xf2; xaddq %rax,(%rcx)
	.byte 0xf0; .byte 0xf3; xaddq %rax,(%rcx)

# Tests for op regb/m8
	xacquire lock decb (%rcx)
	lock xacquire decb (%rcx)
	xrelease lock decb (%rcx)
	lock xrelease decb (%rcx)
	.byte 0xf0; .byte 0xf2; decb (%rcx)
	.byte 0xf0; .byte 0xf3; decb (%rcx)
	xacquire lock incb (%rcx)
	lock xacquire incb (%rcx)
	xrelease lock incb (%rcx)
	lock xrelease incb (%rcx)
	.byte 0xf0; .byte 0xf2; incb (%rcx)
	.byte 0xf0; .byte 0xf3; incb (%rcx)
	xacquire lock negb (%rcx)
	lock xacquire negb (%rcx)
	xrelease lock negb (%rcx)
	lock xrelease negb (%rcx)
	.byte 0xf0; .byte 0xf2; negb (%rcx)
	.byte 0xf0; .byte 0xf3; negb (%rcx)
	xacquire lock notb (%rcx)
	lock xacquire notb (%rcx)
	xrelease lock notb (%rcx)
	lock xrelease notb (%rcx)
	.byte 0xf0; .byte 0xf2; notb (%rcx)
	.byte 0xf0; .byte 0xf3; notb (%rcx)

# Tests for op regs/m16
	xacquire lock decw (%rcx)
	lock xacquire decw (%rcx)
	xrelease lock decw (%rcx)
	lock xrelease decw (%rcx)
	.byte 0xf0; .byte 0xf2; decw (%rcx)
	.byte 0xf0; .byte 0xf3; decw (%rcx)
	xacquire lock incw (%rcx)
	lock xacquire incw (%rcx)
	xrelease lock incw (%rcx)
	lock xrelease incw (%rcx)
	.byte 0xf0; .byte 0xf2; incw (%rcx)
	.byte 0xf0; .byte 0xf3; incw (%rcx)
	xacquire lock negw (%rcx)
	lock xacquire negw (%rcx)
	xrelease lock negw (%rcx)
	lock xrelease negw (%rcx)
	.byte 0xf0; .byte 0xf2; negw (%rcx)
	.byte 0xf0; .byte 0xf3; negw (%rcx)
	xacquire lock notw (%rcx)
	lock xacquire notw (%rcx)
	xrelease lock notw (%rcx)
	lock xrelease notw (%rcx)
	.byte 0xf0; .byte 0xf2; notw (%rcx)
	.byte 0xf0; .byte 0xf3; notw (%rcx)

# Tests for op regl/m32
	xacquire lock decl (%rcx)
	lock xacquire decl (%rcx)
	xrelease lock decl (%rcx)
	lock xrelease decl (%rcx)
	.byte 0xf0; .byte 0xf2; decl (%rcx)
	.byte 0xf0; .byte 0xf3; decl (%rcx)
	xacquire lock incl (%rcx)
	lock xacquire incl (%rcx)
	xrelease lock incl (%rcx)
	lock xrelease incl (%rcx)
	.byte 0xf0; .byte 0xf2; incl (%rcx)
	.byte 0xf0; .byte 0xf3; incl (%rcx)
	xacquire lock negl (%rcx)
	lock xacquire negl (%rcx)
	xrelease lock negl (%rcx)
	lock xrelease negl (%rcx)
	.byte 0xf0; .byte 0xf2; negl (%rcx)
	.byte 0xf0; .byte 0xf3; negl (%rcx)
	xacquire lock notl (%rcx)
	lock xacquire notl (%rcx)
	xrelease lock notl (%rcx)
	lock xrelease notl (%rcx)
	.byte 0xf0; .byte 0xf2; notl (%rcx)
	.byte 0xf0; .byte 0xf3; notl (%rcx)

# Tests for op regq/m64
	xacquire lock decq (%rcx)
	lock xacquire decq (%rcx)
	xrelease lock decq (%rcx)
	lock xrelease decq (%rcx)
	.byte 0xf0; .byte 0xf2; decq (%rcx)
	.byte 0xf0; .byte 0xf3; decq (%rcx)
	xacquire lock incq (%rcx)
	lock xacquire incq (%rcx)
	xrelease lock incq (%rcx)
	lock xrelease incq (%rcx)
	.byte 0xf0; .byte 0xf2; incq (%rcx)
	.byte 0xf0; .byte 0xf3; incq (%rcx)
	xacquire lock negq (%rcx)
	lock xacquire negq (%rcx)
	xrelease lock negq (%rcx)
	lock xrelease negq (%rcx)
	.byte 0xf0; .byte 0xf2; negq (%rcx)
	.byte 0xf0; .byte 0xf3; negq (%rcx)
	xacquire lock notq (%rcx)
	lock xacquire notq (%rcx)
	xrelease lock notq (%rcx)
	lock xrelease notq (%rcx)
	.byte 0xf0; .byte 0xf2; notq (%rcx)
	.byte 0xf0; .byte 0xf3; notq (%rcx)

# Tests for op m64
	xacquire lock cmpxchg8bq (%rcx)
	lock xacquire cmpxchg8bq (%rcx)
	xrelease lock cmpxchg8bq (%rcx)
	lock xrelease cmpxchg8bq (%rcx)
	.byte 0xf0; .byte 0xf2; cmpxchg8bq (%rcx)
	.byte 0xf0; .byte 0xf3; cmpxchg8bq (%rcx)

# Tests for op regb, regb/m8
	xacquire lock cmpxchgb %cl,(%rcx)
	lock xacquire cmpxchgb %cl,(%rcx)
	xrelease lock cmpxchgb %cl,(%rcx)
	lock xrelease cmpxchgb %cl,(%rcx)
	.byte 0xf0; .byte 0xf2; cmpxchgb %cl,(%rcx)
	.byte 0xf0; .byte 0xf3; cmpxchgb %cl,(%rcx)
	xacquire lock xaddb %cl,(%rcx)
	lock xacquire xaddb %cl,(%rcx)
	xrelease lock xaddb %cl,(%rcx)
	lock xrelease xaddb %cl,(%rcx)
	.byte 0xf0; .byte 0xf2; xaddb %cl,(%rcx)
	.byte 0xf0; .byte 0xf3; xaddb %cl,(%rcx)

	.intel_syntax noprefix


# Tests for op imm32 rax

# Tests for op imm8 regb/m8
	xacquire lock adc BYTE PTR [rcx],100
	lock xacquire adc BYTE PTR [rcx],100
	xrelease lock adc BYTE PTR [rcx],100
	lock xrelease adc BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf2; adc BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf3; adc BYTE PTR [rcx],100
	xacquire lock add BYTE PTR [rcx],100
	lock xacquire add BYTE PTR [rcx],100
	xrelease lock add BYTE PTR [rcx],100
	lock xrelease add BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf2; add BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf3; add BYTE PTR [rcx],100
	xacquire lock and BYTE PTR [rcx],100
	lock xacquire and BYTE PTR [rcx],100
	xrelease lock and BYTE PTR [rcx],100
	lock xrelease and BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf2; and BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf3; and BYTE PTR [rcx],100
	xrelease mov BYTE PTR [rcx],100
	xacquire lock or BYTE PTR [rcx],100
	lock xacquire or BYTE PTR [rcx],100
	xrelease lock or BYTE PTR [rcx],100
	lock xrelease or BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf2; or BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf3; or BYTE PTR [rcx],100
	xacquire lock sbb BYTE PTR [rcx],100
	lock xacquire sbb BYTE PTR [rcx],100
	xrelease lock sbb BYTE PTR [rcx],100
	lock xrelease sbb BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf2; sbb BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf3; sbb BYTE PTR [rcx],100
	xacquire lock sub BYTE PTR [rcx],100
	lock xacquire sub BYTE PTR [rcx],100
	xrelease lock sub BYTE PTR [rcx],100
	lock xrelease sub BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf2; sub BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf3; sub BYTE PTR [rcx],100
	xacquire lock xor BYTE PTR [rcx],100
	lock xacquire xor BYTE PTR [rcx],100
	xrelease lock xor BYTE PTR [rcx],100
	lock xrelease xor BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf2; xor BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf3; xor BYTE PTR [rcx],100

# Tests for op imm16 regs/m16
	xacquire lock adc WORD PTR [rcx],1000
	lock xacquire adc WORD PTR [rcx],1000
	xrelease lock adc WORD PTR [rcx],1000
	lock xrelease adc WORD PTR [rcx],1000
	.byte 0xf0; .byte 0xf2; adc WORD PTR [rcx],1000
	.byte 0xf0; .byte 0xf3; adc WORD PTR [rcx],1000
	xacquire lock add WORD PTR [rcx],1000
	lock xacquire add WORD PTR [rcx],1000
	xrelease lock add WORD PTR [rcx],1000
	lock xrelease add WORD PTR [rcx],1000
	.byte 0xf0; .byte 0xf2; add WORD PTR [rcx],1000
	.byte 0xf0; .byte 0xf3; add WORD PTR [rcx],1000
	xacquire lock and WORD PTR [rcx],1000
	lock xacquire and WORD PTR [rcx],1000
	xrelease lock and WORD PTR [rcx],1000
	lock xrelease and WORD PTR [rcx],1000
	.byte 0xf0; .byte 0xf2; and WORD PTR [rcx],1000
	.byte 0xf0; .byte 0xf3; and WORD PTR [rcx],1000
	xrelease mov WORD PTR [rcx],1000
	xacquire lock or WORD PTR [rcx],1000
	lock xacquire or WORD PTR [rcx],1000
	xrelease lock or WORD PTR [rcx],1000
	lock xrelease or WORD PTR [rcx],1000
	.byte 0xf0; .byte 0xf2; or WORD PTR [rcx],1000
	.byte 0xf0; .byte 0xf3; or WORD PTR [rcx],1000
	xacquire lock sbb WORD PTR [rcx],1000
	lock xacquire sbb WORD PTR [rcx],1000
	xrelease lock sbb WORD PTR [rcx],1000
	lock xrelease sbb WORD PTR [rcx],1000
	.byte 0xf0; .byte 0xf2; sbb WORD PTR [rcx],1000
	.byte 0xf0; .byte 0xf3; sbb WORD PTR [rcx],1000
	xacquire lock sub WORD PTR [rcx],1000
	lock xacquire sub WORD PTR [rcx],1000
	xrelease lock sub WORD PTR [rcx],1000
	lock xrelease sub WORD PTR [rcx],1000
	.byte 0xf0; .byte 0xf2; sub WORD PTR [rcx],1000
	.byte 0xf0; .byte 0xf3; sub WORD PTR [rcx],1000
	xacquire lock xor WORD PTR [rcx],1000
	lock xacquire xor WORD PTR [rcx],1000
	xrelease lock xor WORD PTR [rcx],1000
	lock xrelease xor WORD PTR [rcx],1000
	.byte 0xf0; .byte 0xf2; xor WORD PTR [rcx],1000
	.byte 0xf0; .byte 0xf3; xor WORD PTR [rcx],1000

# Tests for op imm32 regl/m32
	xacquire lock adc DWORD PTR [rcx],10000000
	lock xacquire adc DWORD PTR [rcx],10000000
	xrelease lock adc DWORD PTR [rcx],10000000
	lock xrelease adc DWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf2; adc DWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf3; adc DWORD PTR [rcx],10000000
	xacquire lock add DWORD PTR [rcx],10000000
	lock xacquire add DWORD PTR [rcx],10000000
	xrelease lock add DWORD PTR [rcx],10000000
	lock xrelease add DWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf2; add DWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf3; add DWORD PTR [rcx],10000000
	xacquire lock and DWORD PTR [rcx],10000000
	lock xacquire and DWORD PTR [rcx],10000000
	xrelease lock and DWORD PTR [rcx],10000000
	lock xrelease and DWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf2; and DWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf3; and DWORD PTR [rcx],10000000
	xrelease mov DWORD PTR [rcx],10000000
	xacquire lock or DWORD PTR [rcx],10000000
	lock xacquire or DWORD PTR [rcx],10000000
	xrelease lock or DWORD PTR [rcx],10000000
	lock xrelease or DWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf2; or DWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf3; or DWORD PTR [rcx],10000000
	xacquire lock sbb DWORD PTR [rcx],10000000
	lock xacquire sbb DWORD PTR [rcx],10000000
	xrelease lock sbb DWORD PTR [rcx],10000000
	lock xrelease sbb DWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf2; sbb DWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf3; sbb DWORD PTR [rcx],10000000
	xacquire lock sub DWORD PTR [rcx],10000000
	lock xacquire sub DWORD PTR [rcx],10000000
	xrelease lock sub DWORD PTR [rcx],10000000
	lock xrelease sub DWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf2; sub DWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf3; sub DWORD PTR [rcx],10000000
	xacquire lock xor DWORD PTR [rcx],10000000
	lock xacquire xor DWORD PTR [rcx],10000000
	xrelease lock xor DWORD PTR [rcx],10000000
	lock xrelease xor DWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf2; xor DWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf3; xor DWORD PTR [rcx],10000000

# Tests for op imm32 regq/m64
	xacquire lock adc QWORD PTR [rcx],10000000
	lock xacquire adc QWORD PTR [rcx],10000000
	xrelease lock adc QWORD PTR [rcx],10000000
	lock xrelease adc QWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf2; adc QWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf3; adc QWORD PTR [rcx],10000000
	xacquire lock add QWORD PTR [rcx],10000000
	lock xacquire add QWORD PTR [rcx],10000000
	xrelease lock add QWORD PTR [rcx],10000000
	lock xrelease add QWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf2; add QWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf3; add QWORD PTR [rcx],10000000
	xacquire lock and QWORD PTR [rcx],10000000
	lock xacquire and QWORD PTR [rcx],10000000
	xrelease lock and QWORD PTR [rcx],10000000
	lock xrelease and QWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf2; and QWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf3; and QWORD PTR [rcx],10000000
	xrelease mov QWORD PTR [rcx],10000000
	xacquire lock or QWORD PTR [rcx],10000000
	lock xacquire or QWORD PTR [rcx],10000000
	xrelease lock or QWORD PTR [rcx],10000000
	lock xrelease or QWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf2; or QWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf3; or QWORD PTR [rcx],10000000
	xacquire lock sbb QWORD PTR [rcx],10000000
	lock xacquire sbb QWORD PTR [rcx],10000000
	xrelease lock sbb QWORD PTR [rcx],10000000
	lock xrelease sbb QWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf2; sbb QWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf3; sbb QWORD PTR [rcx],10000000
	xacquire lock sub QWORD PTR [rcx],10000000
	lock xacquire sub QWORD PTR [rcx],10000000
	xrelease lock sub QWORD PTR [rcx],10000000
	lock xrelease sub QWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf2; sub QWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf3; sub QWORD PTR [rcx],10000000
	xacquire lock xor QWORD PTR [rcx],10000000
	lock xacquire xor QWORD PTR [rcx],10000000
	xrelease lock xor QWORD PTR [rcx],10000000
	lock xrelease xor QWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf2; xor QWORD PTR [rcx],10000000
	.byte 0xf0; .byte 0xf3; xor QWORD PTR [rcx],10000000

# Tests for op imm8 regs/m16
	xacquire lock adc WORD PTR [rcx],100
	lock xacquire adc WORD PTR [rcx],100
	xrelease lock adc WORD PTR [rcx],100
	lock xrelease adc WORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; adc WORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; adc WORD PTR [rcx],100
	xacquire lock add WORD PTR [rcx],100
	lock xacquire add WORD PTR [rcx],100
	xrelease lock add WORD PTR [rcx],100
	lock xrelease add WORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; add WORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; add WORD PTR [rcx],100
	xacquire lock and WORD PTR [rcx],100
	lock xacquire and WORD PTR [rcx],100
	xrelease lock and WORD PTR [rcx],100
	lock xrelease and WORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; and WORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; and WORD PTR [rcx],100
	xacquire lock btc WORD PTR [rcx],100
	lock xacquire btc WORD PTR [rcx],100
	xrelease lock btc WORD PTR [rcx],100
	lock xrelease btc WORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; btc WORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; btc WORD PTR [rcx],100
	xacquire lock btr WORD PTR [rcx],100
	lock xacquire btr WORD PTR [rcx],100
	xrelease lock btr WORD PTR [rcx],100
	lock xrelease btr WORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; btr WORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; btr WORD PTR [rcx],100
	xacquire lock bts WORD PTR [rcx],100
	lock xacquire bts WORD PTR [rcx],100
	xrelease lock bts WORD PTR [rcx],100
	lock xrelease bts WORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; bts WORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; bts WORD PTR [rcx],100
	xrelease mov WORD PTR [rcx],100
	xacquire lock or WORD PTR [rcx],100
	lock xacquire or WORD PTR [rcx],100
	xrelease lock or WORD PTR [rcx],100
	lock xrelease or WORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; or WORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; or WORD PTR [rcx],100
	xacquire lock sbb WORD PTR [rcx],100
	lock xacquire sbb WORD PTR [rcx],100
	xrelease lock sbb WORD PTR [rcx],100
	lock xrelease sbb WORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; sbb WORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; sbb WORD PTR [rcx],100
	xacquire lock sub WORD PTR [rcx],100
	lock xacquire sub WORD PTR [rcx],100
	xrelease lock sub WORD PTR [rcx],100
	lock xrelease sub WORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; sub WORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; sub WORD PTR [rcx],100
	xacquire lock xor WORD PTR [rcx],100
	lock xacquire xor WORD PTR [rcx],100
	xrelease lock xor WORD PTR [rcx],100
	lock xrelease xor WORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; xor WORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; xor WORD PTR [rcx],100

# Tests for op imm8 regl/m32
	xacquire lock adc DWORD PTR [rcx],100
	lock xacquire adc DWORD PTR [rcx],100
	xrelease lock adc DWORD PTR [rcx],100
	lock xrelease adc DWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; adc DWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; adc DWORD PTR [rcx],100
	xacquire lock add DWORD PTR [rcx],100
	lock xacquire add DWORD PTR [rcx],100
	xrelease lock add DWORD PTR [rcx],100
	lock xrelease add DWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; add DWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; add DWORD PTR [rcx],100
	xacquire lock and DWORD PTR [rcx],100
	lock xacquire and DWORD PTR [rcx],100
	xrelease lock and DWORD PTR [rcx],100
	lock xrelease and DWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; and DWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; and DWORD PTR [rcx],100
	xacquire lock btc DWORD PTR [rcx],100
	lock xacquire btc DWORD PTR [rcx],100
	xrelease lock btc DWORD PTR [rcx],100
	lock xrelease btc DWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; btc DWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; btc DWORD PTR [rcx],100
	xacquire lock btr DWORD PTR [rcx],100
	lock xacquire btr DWORD PTR [rcx],100
	xrelease lock btr DWORD PTR [rcx],100
	lock xrelease btr DWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; btr DWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; btr DWORD PTR [rcx],100
	xacquire lock bts DWORD PTR [rcx],100
	lock xacquire bts DWORD PTR [rcx],100
	xrelease lock bts DWORD PTR [rcx],100
	lock xrelease bts DWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; bts DWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; bts DWORD PTR [rcx],100
	xrelease mov DWORD PTR [rcx],100
	xacquire lock or DWORD PTR [rcx],100
	lock xacquire or DWORD PTR [rcx],100
	xrelease lock or DWORD PTR [rcx],100
	lock xrelease or DWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; or DWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; or DWORD PTR [rcx],100
	xacquire lock sbb DWORD PTR [rcx],100
	lock xacquire sbb DWORD PTR [rcx],100
	xrelease lock sbb DWORD PTR [rcx],100
	lock xrelease sbb DWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; sbb DWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; sbb DWORD PTR [rcx],100
	xacquire lock sub DWORD PTR [rcx],100
	lock xacquire sub DWORD PTR [rcx],100
	xrelease lock sub DWORD PTR [rcx],100
	lock xrelease sub DWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; sub DWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; sub DWORD PTR [rcx],100
	xacquire lock xor DWORD PTR [rcx],100
	lock xacquire xor DWORD PTR [rcx],100
	xrelease lock xor DWORD PTR [rcx],100
	lock xrelease xor DWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; xor DWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; xor DWORD PTR [rcx],100

# Tests for op imm8 regq/m64
	xacquire lock adc QWORD PTR [rcx],100
	lock xacquire adc QWORD PTR [rcx],100
	xrelease lock adc QWORD PTR [rcx],100
	lock xrelease adc QWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; adc QWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; adc QWORD PTR [rcx],100
	xacquire lock add QWORD PTR [rcx],100
	lock xacquire add QWORD PTR [rcx],100
	xrelease lock add QWORD PTR [rcx],100
	lock xrelease add QWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; add QWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; add QWORD PTR [rcx],100
	xacquire lock and QWORD PTR [rcx],100
	lock xacquire and QWORD PTR [rcx],100
	xrelease lock and QWORD PTR [rcx],100
	lock xrelease and QWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; and QWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; and QWORD PTR [rcx],100
	xacquire lock btc QWORD PTR [rcx],100
	lock xacquire btc QWORD PTR [rcx],100
	xrelease lock btc QWORD PTR [rcx],100
	lock xrelease btc QWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; btc QWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; btc QWORD PTR [rcx],100
	xacquire lock btr QWORD PTR [rcx],100
	lock xacquire btr QWORD PTR [rcx],100
	xrelease lock btr QWORD PTR [rcx],100
	lock xrelease btr QWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; btr QWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; btr QWORD PTR [rcx],100
	xacquire lock bts QWORD PTR [rcx],100
	lock xacquire bts QWORD PTR [rcx],100
	xrelease lock bts QWORD PTR [rcx],100
	lock xrelease bts QWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; bts QWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; bts QWORD PTR [rcx],100
	xrelease mov QWORD PTR [rcx],100
	xacquire lock or QWORD PTR [rcx],100
	lock xacquire or QWORD PTR [rcx],100
	xrelease lock or QWORD PTR [rcx],100
	lock xrelease or QWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; or QWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; or QWORD PTR [rcx],100
	xacquire lock sbb QWORD PTR [rcx],100
	lock xacquire sbb QWORD PTR [rcx],100
	xrelease lock sbb QWORD PTR [rcx],100
	lock xrelease sbb QWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; sbb QWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; sbb QWORD PTR [rcx],100
	xacquire lock sub QWORD PTR [rcx],100
	lock xacquire sub QWORD PTR [rcx],100
	xrelease lock sub QWORD PTR [rcx],100
	lock xrelease sub QWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; sub QWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; sub QWORD PTR [rcx],100
	xacquire lock xor QWORD PTR [rcx],100
	lock xacquire xor QWORD PTR [rcx],100
	xrelease lock xor QWORD PTR [rcx],100
	lock xrelease xor QWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf2; xor QWORD PTR [rcx],100
	.byte 0xf0; .byte 0xf3; xor QWORD PTR [rcx],100

# Tests for op imm8 regb/m8
	xacquire lock adc BYTE PTR [rcx],100
	lock xacquire adc BYTE PTR [rcx],100
	xrelease lock adc BYTE PTR [rcx],100
	lock xrelease adc BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf2; adc BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf3; adc BYTE PTR [rcx],100
	xacquire lock add BYTE PTR [rcx],100
	lock xacquire add BYTE PTR [rcx],100
	xrelease lock add BYTE PTR [rcx],100
	lock xrelease add BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf2; add BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf3; add BYTE PTR [rcx],100
	xacquire lock and BYTE PTR [rcx],100
	lock xacquire and BYTE PTR [rcx],100
	xrelease lock and BYTE PTR [rcx],100
	lock xrelease and BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf2; and BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf3; and BYTE PTR [rcx],100
	xrelease mov BYTE PTR [rcx],100
	xacquire lock or BYTE PTR [rcx],100
	lock xacquire or BYTE PTR [rcx],100
	xrelease lock or BYTE PTR [rcx],100
	lock xrelease or BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf2; or BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf3; or BYTE PTR [rcx],100
	xacquire lock sbb BYTE PTR [rcx],100
	lock xacquire sbb BYTE PTR [rcx],100
	xrelease lock sbb BYTE PTR [rcx],100
	lock xrelease sbb BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf2; sbb BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf3; sbb BYTE PTR [rcx],100
	xacquire lock sub BYTE PTR [rcx],100
	lock xacquire sub BYTE PTR [rcx],100
	xrelease lock sub BYTE PTR [rcx],100
	lock xrelease sub BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf2; sub BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf3; sub BYTE PTR [rcx],100
	xacquire lock xor BYTE PTR [rcx],100
	lock xacquire xor BYTE PTR [rcx],100
	xrelease lock xor BYTE PTR [rcx],100
	lock xrelease xor BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf2; xor BYTE PTR [rcx],100
	.byte 0xf0; .byte 0xf3; xor BYTE PTR [rcx],100

# Tests for op regb regb/m8
# Tests for op regb/m8 regb
	xacquire lock adc BYTE PTR [rcx],al
	lock xacquire adc BYTE PTR [rcx],al
	xrelease lock adc BYTE PTR [rcx],al
	lock xrelease adc BYTE PTR [rcx],al
	.byte 0xf0; .byte 0xf2; adc BYTE PTR [rcx],al
	.byte 0xf0; .byte 0xf3; adc BYTE PTR [rcx],al
	xacquire lock add BYTE PTR [rcx],al
	lock xacquire add BYTE PTR [rcx],al
	xrelease lock add BYTE PTR [rcx],al
	lock xrelease add BYTE PTR [rcx],al
	.byte 0xf0; .byte 0xf2; add BYTE PTR [rcx],al
	.byte 0xf0; .byte 0xf3; add BYTE PTR [rcx],al
	xacquire lock and BYTE PTR [rcx],al
	lock xacquire and BYTE PTR [rcx],al
	xrelease lock and BYTE PTR [rcx],al
	lock xrelease and BYTE PTR [rcx],al
	.byte 0xf0; .byte 0xf2; and BYTE PTR [rcx],al
	.byte 0xf0; .byte 0xf3; and BYTE PTR [rcx],al
	xrelease mov BYTE PTR [rcx],al
	xacquire lock or BYTE PTR [rcx],al
	lock xacquire or BYTE PTR [rcx],al
	xrelease lock or BYTE PTR [rcx],al
	lock xrelease or BYTE PTR [rcx],al
	.byte 0xf0; .byte 0xf2; or BYTE PTR [rcx],al
	.byte 0xf0; .byte 0xf3; or BYTE PTR [rcx],al
	xacquire lock sbb BYTE PTR [rcx],al
	lock xacquire sbb BYTE PTR [rcx],al
	xrelease lock sbb BYTE PTR [rcx],al
	lock xrelease sbb BYTE PTR [rcx],al
	.byte 0xf0; .byte 0xf2; sbb BYTE PTR [rcx],al
	.byte 0xf0; .byte 0xf3; sbb BYTE PTR [rcx],al
	xacquire lock sub BYTE PTR [rcx],al
	lock xacquire sub BYTE PTR [rcx],al
	xrelease lock sub BYTE PTR [rcx],al
	lock xrelease sub BYTE PTR [rcx],al
	.byte 0xf0; .byte 0xf2; sub BYTE PTR [rcx],al
	.byte 0xf0; .byte 0xf3; sub BYTE PTR [rcx],al
	xacquire lock xchg BYTE PTR [rcx],al
	lock xacquire xchg BYTE PTR [rcx],al
	xacquire xchg BYTE PTR [rcx],al
	xrelease lock xchg BYTE PTR [rcx],al
	lock xrelease xchg BYTE PTR [rcx],al
	xrelease xchg BYTE PTR [rcx],al
	.byte 0xf0; .byte 0xf2; xchg BYTE PTR [rcx],al
	.byte 0xf0; .byte 0xf3; xchg BYTE PTR [rcx],al
	xacquire lock xor BYTE PTR [rcx],al
	lock xacquire xor BYTE PTR [rcx],al
	xrelease lock xor BYTE PTR [rcx],al
	lock xrelease xor BYTE PTR [rcx],al
	.byte 0xf0; .byte 0xf2; xor BYTE PTR [rcx],al
	.byte 0xf0; .byte 0xf3; xor BYTE PTR [rcx],al

# Tests for op regs regs/m16
# Tests for op regs/m16 regs
	xacquire lock adc WORD PTR [rcx],ax
	lock xacquire adc WORD PTR [rcx],ax
	xrelease lock adc WORD PTR [rcx],ax
	lock xrelease adc WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf2; adc WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf3; adc WORD PTR [rcx],ax
	xacquire lock add WORD PTR [rcx],ax
	lock xacquire add WORD PTR [rcx],ax
	xrelease lock add WORD PTR [rcx],ax
	lock xrelease add WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf2; add WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf3; add WORD PTR [rcx],ax
	xacquire lock and WORD PTR [rcx],ax
	lock xacquire and WORD PTR [rcx],ax
	xrelease lock and WORD PTR [rcx],ax
	lock xrelease and WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf2; and WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf3; and WORD PTR [rcx],ax
	xrelease mov WORD PTR [rcx],ax
	xacquire lock or WORD PTR [rcx],ax
	lock xacquire or WORD PTR [rcx],ax
	xrelease lock or WORD PTR [rcx],ax
	lock xrelease or WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf2; or WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf3; or WORD PTR [rcx],ax
	xacquire lock sbb WORD PTR [rcx],ax
	lock xacquire sbb WORD PTR [rcx],ax
	xrelease lock sbb WORD PTR [rcx],ax
	lock xrelease sbb WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf2; sbb WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf3; sbb WORD PTR [rcx],ax
	xacquire lock sub WORD PTR [rcx],ax
	lock xacquire sub WORD PTR [rcx],ax
	xrelease lock sub WORD PTR [rcx],ax
	lock xrelease sub WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf2; sub WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf3; sub WORD PTR [rcx],ax
	xacquire lock xchg WORD PTR [rcx],ax
	lock xacquire xchg WORD PTR [rcx],ax
	xacquire xchg WORD PTR [rcx],ax
	xrelease lock xchg WORD PTR [rcx],ax
	lock xrelease xchg WORD PTR [rcx],ax
	xrelease xchg WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf2; xchg WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf3; xchg WORD PTR [rcx],ax
	xacquire lock xor WORD PTR [rcx],ax
	lock xacquire xor WORD PTR [rcx],ax
	xrelease lock xor WORD PTR [rcx],ax
	lock xrelease xor WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf2; xor WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf3; xor WORD PTR [rcx],ax

# Tests for op regl regl/m32
# Tests for op regl/m32 regl
	xacquire lock adc DWORD PTR [rcx],eax
	lock xacquire adc DWORD PTR [rcx],eax
	xrelease lock adc DWORD PTR [rcx],eax
	lock xrelease adc DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf2; adc DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf3; adc DWORD PTR [rcx],eax
	xacquire lock add DWORD PTR [rcx],eax
	lock xacquire add DWORD PTR [rcx],eax
	xrelease lock add DWORD PTR [rcx],eax
	lock xrelease add DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf2; add DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf3; add DWORD PTR [rcx],eax
	xacquire lock and DWORD PTR [rcx],eax
	lock xacquire and DWORD PTR [rcx],eax
	xrelease lock and DWORD PTR [rcx],eax
	lock xrelease and DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf2; and DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf3; and DWORD PTR [rcx],eax
	xrelease mov DWORD PTR [rcx],eax
	xacquire lock or DWORD PTR [rcx],eax
	lock xacquire or DWORD PTR [rcx],eax
	xrelease lock or DWORD PTR [rcx],eax
	lock xrelease or DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf2; or DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf3; or DWORD PTR [rcx],eax
	xacquire lock sbb DWORD PTR [rcx],eax
	lock xacquire sbb DWORD PTR [rcx],eax
	xrelease lock sbb DWORD PTR [rcx],eax
	lock xrelease sbb DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf2; sbb DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf3; sbb DWORD PTR [rcx],eax
	xacquire lock sub DWORD PTR [rcx],eax
	lock xacquire sub DWORD PTR [rcx],eax
	xrelease lock sub DWORD PTR [rcx],eax
	lock xrelease sub DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf2; sub DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf3; sub DWORD PTR [rcx],eax
	xacquire lock xchg DWORD PTR [rcx],eax
	lock xacquire xchg DWORD PTR [rcx],eax
	xacquire xchg DWORD PTR [rcx],eax
	xrelease lock xchg DWORD PTR [rcx],eax
	lock xrelease xchg DWORD PTR [rcx],eax
	xrelease xchg DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf2; xchg DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf3; xchg DWORD PTR [rcx],eax
	xacquire lock xor DWORD PTR [rcx],eax
	lock xacquire xor DWORD PTR [rcx],eax
	xrelease lock xor DWORD PTR [rcx],eax
	lock xrelease xor DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf2; xor DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf3; xor DWORD PTR [rcx],eax

# Tests for op regq regq/m64
# Tests for op regq/m64 regq
	xacquire lock adc QWORD PTR [rcx],rax
	lock xacquire adc QWORD PTR [rcx],rax
	xrelease lock adc QWORD PTR [rcx],rax
	lock xrelease adc QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf2; adc QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf3; adc QWORD PTR [rcx],rax
	xacquire lock add QWORD PTR [rcx],rax
	lock xacquire add QWORD PTR [rcx],rax
	xrelease lock add QWORD PTR [rcx],rax
	lock xrelease add QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf2; add QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf3; add QWORD PTR [rcx],rax
	xacquire lock and QWORD PTR [rcx],rax
	lock xacquire and QWORD PTR [rcx],rax
	xrelease lock and QWORD PTR [rcx],rax
	lock xrelease and QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf2; and QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf3; and QWORD PTR [rcx],rax
	xrelease mov QWORD PTR [rcx],rax
	xacquire lock or QWORD PTR [rcx],rax
	lock xacquire or QWORD PTR [rcx],rax
	xrelease lock or QWORD PTR [rcx],rax
	lock xrelease or QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf2; or QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf3; or QWORD PTR [rcx],rax
	xacquire lock sbb QWORD PTR [rcx],rax
	lock xacquire sbb QWORD PTR [rcx],rax
	xrelease lock sbb QWORD PTR [rcx],rax
	lock xrelease sbb QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf2; sbb QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf3; sbb QWORD PTR [rcx],rax
	xacquire lock sub QWORD PTR [rcx],rax
	lock xacquire sub QWORD PTR [rcx],rax
	xrelease lock sub QWORD PTR [rcx],rax
	lock xrelease sub QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf2; sub QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf3; sub QWORD PTR [rcx],rax
	xacquire lock xchg QWORD PTR [rcx],rax
	lock xacquire xchg QWORD PTR [rcx],rax
	xacquire xchg QWORD PTR [rcx],rax
	xrelease lock xchg QWORD PTR [rcx],rax
	lock xrelease xchg QWORD PTR [rcx],rax
	xrelease xchg QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf2; xchg QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf3; xchg QWORD PTR [rcx],rax
	xacquire lock xor QWORD PTR [rcx],rax
	lock xacquire xor QWORD PTR [rcx],rax
	xrelease lock xor QWORD PTR [rcx],rax
	lock xrelease xor QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf2; xor QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf3; xor QWORD PTR [rcx],rax

# Tests for op regs, regs/m16
	xacquire lock btc WORD PTR [rcx],ax
	lock xacquire btc WORD PTR [rcx],ax
	xrelease lock btc WORD PTR [rcx],ax
	lock xrelease btc WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf2; btc WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf3; btc WORD PTR [rcx],ax
	xacquire lock btr WORD PTR [rcx],ax
	lock xacquire btr WORD PTR [rcx],ax
	xrelease lock btr WORD PTR [rcx],ax
	lock xrelease btr WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf2; btr WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf3; btr WORD PTR [rcx],ax
	xacquire lock bts WORD PTR [rcx],ax
	lock xacquire bts WORD PTR [rcx],ax
	xrelease lock bts WORD PTR [rcx],ax
	lock xrelease bts WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf2; bts WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf3; bts WORD PTR [rcx],ax
	xacquire lock cmpxchg WORD PTR [rcx],ax
	lock xacquire cmpxchg WORD PTR [rcx],ax
	xrelease lock cmpxchg WORD PTR [rcx],ax
	lock xrelease cmpxchg WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf2; cmpxchg WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf3; cmpxchg WORD PTR [rcx],ax
	xacquire lock xadd WORD PTR [rcx],ax
	lock xacquire xadd WORD PTR [rcx],ax
	xrelease lock xadd WORD PTR [rcx],ax
	lock xrelease xadd WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf2; xadd WORD PTR [rcx],ax
	.byte 0xf0; .byte 0xf3; xadd WORD PTR [rcx],ax

# Tests for op regl regl/m32
	xacquire lock btc DWORD PTR [rcx],eax
	lock xacquire btc DWORD PTR [rcx],eax
	xrelease lock btc DWORD PTR [rcx],eax
	lock xrelease btc DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf2; btc DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf3; btc DWORD PTR [rcx],eax
	xacquire lock btr DWORD PTR [rcx],eax
	lock xacquire btr DWORD PTR [rcx],eax
	xrelease lock btr DWORD PTR [rcx],eax
	lock xrelease btr DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf2; btr DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf3; btr DWORD PTR [rcx],eax
	xacquire lock bts DWORD PTR [rcx],eax
	lock xacquire bts DWORD PTR [rcx],eax
	xrelease lock bts DWORD PTR [rcx],eax
	lock xrelease bts DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf2; bts DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf3; bts DWORD PTR [rcx],eax
	xacquire lock cmpxchg DWORD PTR [rcx],eax
	lock xacquire cmpxchg DWORD PTR [rcx],eax
	xrelease lock cmpxchg DWORD PTR [rcx],eax
	lock xrelease cmpxchg DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf2; cmpxchg DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf3; cmpxchg DWORD PTR [rcx],eax
	xacquire lock xadd DWORD PTR [rcx],eax
	lock xacquire xadd DWORD PTR [rcx],eax
	xrelease lock xadd DWORD PTR [rcx],eax
	lock xrelease xadd DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf2; xadd DWORD PTR [rcx],eax
	.byte 0xf0; .byte 0xf3; xadd DWORD PTR [rcx],eax

# Tests for op regq regq/m64
	xacquire lock btc QWORD PTR [rcx],rax
	lock xacquire btc QWORD PTR [rcx],rax
	xrelease lock btc QWORD PTR [rcx],rax
	lock xrelease btc QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf2; btc QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf3; btc QWORD PTR [rcx],rax
	xacquire lock btr QWORD PTR [rcx],rax
	lock xacquire btr QWORD PTR [rcx],rax
	xrelease lock btr QWORD PTR [rcx],rax
	lock xrelease btr QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf2; btr QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf3; btr QWORD PTR [rcx],rax
	xacquire lock bts QWORD PTR [rcx],rax
	lock xacquire bts QWORD PTR [rcx],rax
	xrelease lock bts QWORD PTR [rcx],rax
	lock xrelease bts QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf2; bts QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf3; bts QWORD PTR [rcx],rax
	xacquire lock cmpxchg QWORD PTR [rcx],rax
	lock xacquire cmpxchg QWORD PTR [rcx],rax
	xrelease lock cmpxchg QWORD PTR [rcx],rax
	lock xrelease cmpxchg QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf2; cmpxchg QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf3; cmpxchg QWORD PTR [rcx],rax
	xacquire lock xadd QWORD PTR [rcx],rax
	lock xacquire xadd QWORD PTR [rcx],rax
	xrelease lock xadd QWORD PTR [rcx],rax
	lock xrelease xadd QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf2; xadd QWORD PTR [rcx],rax
	.byte 0xf0; .byte 0xf3; xadd QWORD PTR [rcx],rax

# Tests for op regb/m8
	xacquire lock dec BYTE PTR [rcx]
	lock xacquire dec BYTE PTR [rcx]
	xrelease lock dec BYTE PTR [rcx]
	lock xrelease dec BYTE PTR [rcx]
	.byte 0xf0; .byte 0xf2; dec BYTE PTR [rcx]
	.byte 0xf0; .byte 0xf3; dec BYTE PTR [rcx]
	xacquire lock inc BYTE PTR [rcx]
	lock xacquire inc BYTE PTR [rcx]
	xrelease lock inc BYTE PTR [rcx]
	lock xrelease inc BYTE PTR [rcx]
	.byte 0xf0; .byte 0xf2; inc BYTE PTR [rcx]
	.byte 0xf0; .byte 0xf3; inc BYTE PTR [rcx]
	xacquire lock neg BYTE PTR [rcx]
	lock xacquire neg BYTE PTR [rcx]
	xrelease lock neg BYTE PTR [rcx]
	lock xrelease neg BYTE PTR [rcx]
	.byte 0xf0; .byte 0xf2; neg BYTE PTR [rcx]
	.byte 0xf0; .byte 0xf3; neg BYTE PTR [rcx]
	xacquire lock not BYTE PTR [rcx]
	lock xacquire not BYTE PTR [rcx]
	xrelease lock not BYTE PTR [rcx]
	lock xrelease not BYTE PTR [rcx]
	.byte 0xf0; .byte 0xf2; not BYTE PTR [rcx]
	.byte 0xf0; .byte 0xf3; not BYTE PTR [rcx]

# Tests for op regs/m16
	xacquire lock dec WORD PTR [rcx]
	lock xacquire dec WORD PTR [rcx]
	xrelease lock dec WORD PTR [rcx]
	lock xrelease dec WORD PTR [rcx]
	.byte 0xf0; .byte 0xf2; dec WORD PTR [rcx]
	.byte 0xf0; .byte 0xf3; dec WORD PTR [rcx]
	xacquire lock inc WORD PTR [rcx]
	lock xacquire inc WORD PTR [rcx]
	xrelease lock inc WORD PTR [rcx]
	lock xrelease inc WORD PTR [rcx]
	.byte 0xf0; .byte 0xf2; inc WORD PTR [rcx]
	.byte 0xf0; .byte 0xf3; inc WORD PTR [rcx]
	xacquire lock neg WORD PTR [rcx]
	lock xacquire neg WORD PTR [rcx]
	xrelease lock neg WORD PTR [rcx]
	lock xrelease neg WORD PTR [rcx]
	.byte 0xf0; .byte 0xf2; neg WORD PTR [rcx]
	.byte 0xf0; .byte 0xf3; neg WORD PTR [rcx]
	xacquire lock not WORD PTR [rcx]
	lock xacquire not WORD PTR [rcx]
	xrelease lock not WORD PTR [rcx]
	lock xrelease not WORD PTR [rcx]
	.byte 0xf0; .byte 0xf2; not WORD PTR [rcx]
	.byte 0xf0; .byte 0xf3; not WORD PTR [rcx]

# Tests for op regl/m32
	xacquire lock dec DWORD PTR [rcx]
	lock xacquire dec DWORD PTR [rcx]
	xrelease lock dec DWORD PTR [rcx]
	lock xrelease dec DWORD PTR [rcx]
	.byte 0xf0; .byte 0xf2; dec DWORD PTR [rcx]
	.byte 0xf0; .byte 0xf3; dec DWORD PTR [rcx]
	xacquire lock inc DWORD PTR [rcx]
	lock xacquire inc DWORD PTR [rcx]
	xrelease lock inc DWORD PTR [rcx]
	lock xrelease inc DWORD PTR [rcx]
	.byte 0xf0; .byte 0xf2; inc DWORD PTR [rcx]
	.byte 0xf0; .byte 0xf3; inc DWORD PTR [rcx]
	xacquire lock neg DWORD PTR [rcx]
	lock xacquire neg DWORD PTR [rcx]
	xrelease lock neg DWORD PTR [rcx]
	lock xrelease neg DWORD PTR [rcx]
	.byte 0xf0; .byte 0xf2; neg DWORD PTR [rcx]
	.byte 0xf0; .byte 0xf3; neg DWORD PTR [rcx]
	xacquire lock not DWORD PTR [rcx]
	lock xacquire not DWORD PTR [rcx]
	xrelease lock not DWORD PTR [rcx]
	lock xrelease not DWORD PTR [rcx]
	.byte 0xf0; .byte 0xf2; not DWORD PTR [rcx]
	.byte 0xf0; .byte 0xf3; not DWORD PTR [rcx]

# Tests for op regq/m64
	xacquire lock dec QWORD PTR [rcx]
	lock xacquire dec QWORD PTR [rcx]
	xrelease lock dec QWORD PTR [rcx]
	lock xrelease dec QWORD PTR [rcx]
	.byte 0xf0; .byte 0xf2; dec QWORD PTR [rcx]
	.byte 0xf0; .byte 0xf3; dec QWORD PTR [rcx]
	xacquire lock inc QWORD PTR [rcx]
	lock xacquire inc QWORD PTR [rcx]
	xrelease lock inc QWORD PTR [rcx]
	lock xrelease inc QWORD PTR [rcx]
	.byte 0xf0; .byte 0xf2; inc QWORD PTR [rcx]
	.byte 0xf0; .byte 0xf3; inc QWORD PTR [rcx]
	xacquire lock neg QWORD PTR [rcx]
	lock xacquire neg QWORD PTR [rcx]
	xrelease lock neg QWORD PTR [rcx]
	lock xrelease neg QWORD PTR [rcx]
	.byte 0xf0; .byte 0xf2; neg QWORD PTR [rcx]
	.byte 0xf0; .byte 0xf3; neg QWORD PTR [rcx]
	xacquire lock not QWORD PTR [rcx]
	lock xacquire not QWORD PTR [rcx]
	xrelease lock not QWORD PTR [rcx]
	lock xrelease not QWORD PTR [rcx]
	.byte 0xf0; .byte 0xf2; not QWORD PTR [rcx]
	.byte 0xf0; .byte 0xf3; not QWORD PTR [rcx]

# Tests for op m64
	xacquire lock cmpxchg8b QWORD PTR [rcx]
	lock xacquire cmpxchg8b QWORD PTR [rcx]
	xrelease lock cmpxchg8b QWORD PTR [rcx]
	lock xrelease cmpxchg8b QWORD PTR [rcx]
	.byte 0xf0; .byte 0xf2; cmpxchg8b QWORD PTR [rcx]
	.byte 0xf0; .byte 0xf3; cmpxchg8b QWORD PTR [rcx]

# Tests for op regb, regb/m8
	xacquire lock cmpxchg BYTE PTR [rcx],cl
	lock xacquire cmpxchg BYTE PTR [rcx],cl
	xrelease lock cmpxchg BYTE PTR [rcx],cl
	lock xrelease cmpxchg BYTE PTR [rcx],cl
	.byte 0xf0; .byte 0xf2; cmpxchg BYTE PTR [rcx],cl
	.byte 0xf0; .byte 0xf3; cmpxchg BYTE PTR [rcx],cl
	xacquire lock xadd BYTE PTR [rcx],cl
	lock xacquire xadd BYTE PTR [rcx],cl
	xrelease lock xadd BYTE PTR [rcx],cl
	lock xrelease xadd BYTE PTR [rcx],cl
	.byte 0xf0; .byte 0xf2; xadd BYTE PTR [rcx],cl
	.byte 0xf0; .byte 0xf3; xadd BYTE PTR [rcx],cl
