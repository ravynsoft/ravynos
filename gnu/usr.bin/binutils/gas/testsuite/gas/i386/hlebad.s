# Check 32bit unsupported HLE instructions

	.allow_index_reg
	.text
_start:

# Tests for op imm8 al
	xacquire adc $100,%al
	xacquire lock adc $100,%al
	lock xacquire adc $100,%al
	xrelease adc $100,%al
	xrelease lock adc $100,%al
	lock xrelease adc $100,%al

# Tests for op imm16 ax
	xacquire adc $1000,%ax
	xacquire lock adc $1000,%ax
	lock xacquire adc $1000,%ax
	xrelease adc $1000,%ax
	xrelease lock adc $1000,%ax
	lock xrelease adc $1000,%ax

# Tests for op imm32 eax
	xacquire adc $10000000,%eax
	xacquire lock adc $10000000,%eax
	lock xacquire adc $10000000,%eax
	xrelease adc $10000000,%eax
	xrelease lock adc $10000000,%eax
	lock xrelease adc $10000000,%eax

# Tests for op imm8 regb/m8
	xacquire adcb $100,%cl
	xacquire lock adcb $100,%cl
	lock xacquire adcb $100,%cl
	xrelease adcb $100,%cl
	xrelease lock adcb $100,%cl
	lock xrelease adcb $100,%cl
	xacquire adcb $100,(%ecx)
	xrelease adcb $100,(%ecx)

# Tests for op imm16 regs/m16
	xacquire adcw $1000,%cx
	xacquire lock adcw $1000,%cx
	lock xacquire adcw $1000,%cx
	xrelease adcw $1000,%cx
	xrelease lock adcw $1000,%cx
	lock xrelease adcw $1000,%cx
	xacquire adcw $1000,(%ecx)
	xrelease adcw $1000,(%ecx)

# Tests for op imm32 regl/m32
	xacquire adcl $10000000,%ecx
	xacquire lock adcl $10000000,%ecx
	lock xacquire adcl $10000000,%ecx
	xrelease adcl $10000000,%ecx
	xrelease lock adcl $10000000,%ecx
	lock xrelease adcl $10000000,%ecx
	xacquire adcl $10000000,(%ecx)
	xrelease adcl $10000000,(%ecx)

# Tests for op imm8 regs/m16
	xacquire adcw $100,%cx
	xacquire lock adcw $100,%cx
	lock xacquire adcw $100,%cx
	xrelease adcw $100,%cx
	xrelease lock adcw $100,%cx
	lock xrelease adcw $100,%cx
	xacquire adcw $100,(%ecx)
	xrelease adcw $100,(%ecx)

# Tests for op imm8 regl/m32
	xacquire adcl $100,%ecx
	xacquire lock adcl $100,%ecx
	lock xacquire adcl $100,%ecx
	xrelease adcl $100,%ecx
	xrelease lock adcl $100,%ecx
	lock xrelease adcl $100,%ecx
	xacquire adcl $100,(%ecx)
	xrelease adcl $100,(%ecx)

# Tests for op imm8 regb/m8
	xacquire adcb $100,%cl
	xacquire lock adcb $100,%cl
	lock xacquire adcb $100,%cl
	xrelease adcb $100,%cl
	xrelease lock adcb $100,%cl
	lock xrelease adcb $100,%cl
	xacquire adcb $100,(%ecx)
	xrelease adcb $100,(%ecx)

# Tests for op regb regb/m8
# Tests for op regb/m8 regb
	xacquire adcb %al,%cl
	xacquire lock adcb %al,%cl
	lock xacquire adcb %al,%cl
	xrelease adcb %al,%cl
	xrelease lock adcb %al,%cl
	lock xrelease adcb %al,%cl
	xacquire adcb %al,(%ecx)
	xrelease adcb %al,(%ecx)
	xacquire adcb %cl,%al
	xacquire lock adcb %cl,%al
	lock xacquire adcb %cl,%al
	xrelease adcb %cl,%al
	xrelease lock adcb %cl,%al
	lock xrelease adcb %cl,%al
	xacquire adcb (%ecx),%al
	xacquire lock adcb (%ecx),%al
	lock xacquire adcb (%ecx),%al
	xrelease adcb (%ecx),%al
	xrelease lock adcb (%ecx),%al
	lock xrelease adcb (%ecx),%al

# Tests for op regs regs/m16
# Tests for op regs/m16 regs
	xacquire adcw %ax,%cx
	xacquire lock adcw %ax,%cx
	lock xacquire adcw %ax,%cx
	xrelease adcw %ax,%cx
	xrelease lock adcw %ax,%cx
	lock xrelease adcw %ax,%cx
	xacquire adcw %ax,(%ecx)
	xrelease adcw %ax,(%ecx)
	xacquire adcw %cx,%ax
	xacquire lock adcw %cx,%ax
	lock xacquire adcw %cx,%ax
	xrelease adcw %cx,%ax
	xrelease lock adcw %cx,%ax
	lock xrelease adcw %cx,%ax
	xacquire adcw (%ecx),%ax
	xacquire lock adcw (%ecx),%ax
	lock xacquire adcw (%ecx),%ax
	xrelease adcw (%ecx),%ax
	xrelease lock adcw (%ecx),%ax
	lock xrelease adcw (%ecx),%ax

# Tests for op regl regl/m32
# Tests for op regl/m32 regl
	xacquire adcl %eax,%ecx
	xacquire lock adcl %eax,%ecx
	lock xacquire adcl %eax,%ecx
	xrelease adcl %eax,%ecx
	xrelease lock adcl %eax,%ecx
	lock xrelease adcl %eax,%ecx
	xacquire adcl %eax,(%ecx)
	xrelease adcl %eax,(%ecx)
	xacquire adcl %ecx,%eax
	xacquire lock adcl %ecx,%eax
	lock xacquire adcl %ecx,%eax
	xrelease adcl %ecx,%eax
	xrelease lock adcl %ecx,%eax
	lock xrelease adcl %ecx,%eax
	xacquire adcl (%ecx),%eax
	xacquire lock adcl (%ecx),%eax
	lock xacquire adcl (%ecx),%eax
	xrelease adcl (%ecx),%eax
	xrelease lock adcl (%ecx),%eax
	lock xrelease adcl (%ecx),%eax

# Tests for op regs, regs/m16
	xacquire btcw %ax,%cx
	xacquire lock btcw %ax,%cx
	lock xacquire btcw %ax,%cx
	xrelease btcw %ax,%cx
	xrelease lock btcw %ax,%cx
	lock xrelease btcw %ax,%cx
	xacquire btcw %ax,(%ecx)
	xrelease btcw %ax,(%ecx)

# Tests for op regl regl/m32
	xacquire btcl %eax,%ecx
	xacquire lock btcl %eax,%ecx
	lock xacquire btcl %eax,%ecx
	xrelease btcl %eax,%ecx
	xrelease lock btcl %eax,%ecx
	lock xrelease btcl %eax,%ecx
	xacquire btcl %eax,(%ecx)
	xrelease btcl %eax,(%ecx)

# Tests for op regb/m8
	xacquire decb %cl
	xacquire lock decb %cl
	lock xacquire decb %cl
	xrelease decb %cl
	xrelease lock decb %cl
	lock xrelease decb %cl
	xacquire decb (%ecx)
	xrelease decb (%ecx)

# Tests for op regs/m16
	xacquire decw %cx
	xacquire lock decw %cx
	lock xacquire decw %cx
	xrelease decw %cx
	xrelease lock decw %cx
	lock xrelease decw %cx
	xacquire decw (%ecx)
	xrelease decw (%ecx)

# Tests for op regl/m32
	xacquire decl %ecx
	xacquire lock decl %ecx
	lock xacquire decl %ecx
	xrelease decl %ecx
	xrelease lock decl %ecx
	lock xrelease decl %ecx
	xacquire decl (%ecx)
	xrelease decl (%ecx)

# Tests for op m64
	xacquire cmpxchg8bq (%ecx)
	xrelease cmpxchg8bq (%ecx)

# Tests for op regb, regb/m8
	xacquire cmpxchgb %cl,%al
	xacquire lock cmpxchgb %cl,%al
	lock xacquire cmpxchgb %cl,%al
	xrelease cmpxchgb %cl,%al
	xrelease lock cmpxchgb %cl,%al
	lock xrelease cmpxchgb %cl,%al
	xacquire cmpxchgb %cl,(%ecx)
	xrelease cmpxchgb %cl,(%ecx)

	.intel_syntax noprefix

# Tests for op imm8 al
	xacquire adc al,100
	xacquire lock adc al,100
	lock xacquire adc al,100
	xrelease adc al,100
	xrelease lock adc al,100
	lock xrelease adc al,100

# Tests for op imm16 ax
	xacquire adc ax,1000
	xacquire lock adc ax,1000
	lock xacquire adc ax,1000
	xrelease adc ax,1000
	xrelease lock adc ax,1000
	lock xrelease adc ax,1000

# Tests for op imm32 eax
	xacquire adc eax,10000000
	xacquire lock adc eax,10000000
	lock xacquire adc eax,10000000
	xrelease adc eax,10000000
	xrelease lock adc eax,10000000
	lock xrelease adc eax,10000000

# Tests for op imm8 regb/m8
	xacquire adc cl,100
	xacquire lock adc cl,100
	lock xacquire adc cl,100
	xrelease adc cl,100
	xrelease lock adc cl,100
	lock xrelease adc cl,100
	xacquire adc BYTE PTR [ecx],100
	xrelease adc BYTE PTR [ecx],100

# Tests for op imm16 regs/m16
	xacquire adc cx,1000
	xacquire lock adc cx,1000
	lock xacquire adc cx,1000
	xrelease adc cx,1000
	xrelease lock adc cx,1000
	lock xrelease adc cx,1000
	xacquire adc WORD PTR [ecx],1000
	xrelease adc WORD PTR [ecx],1000

# Tests for op imm32 regl/m32
	xacquire adc ecx,10000000
	xacquire lock adc ecx,10000000
	lock xacquire adc ecx,10000000
	xrelease adc ecx,10000000
	xrelease lock adc ecx,10000000
	lock xrelease adc ecx,10000000
	xacquire adc DWORD PTR [ecx],10000000
	xrelease adc DWORD PTR [ecx],10000000

# Tests for op imm8 regs/m16
	xacquire adc cx,100
	xacquire lock adc cx,100
	lock xacquire adc cx,100
	xrelease adc cx,100
	xrelease lock adc cx,100
	lock xrelease adc cx,100
	xacquire adc WORD PTR [ecx],100
	xrelease adc WORD PTR [ecx],100

# Tests for op imm8 regl/m32
	xacquire adc ecx,100
	xacquire lock adc ecx,100
	lock xacquire adc ecx,100
	xrelease adc ecx,100
	xrelease lock adc ecx,100
	lock xrelease adc ecx,100
	xacquire adc DWORD PTR [ecx],100
	xrelease adc DWORD PTR [ecx],100

# Tests for op imm8 regb/m8
	xacquire adc cl,100
	xacquire lock adc cl,100
	lock xacquire adc cl,100
	xrelease adc cl,100
	xrelease lock adc cl,100
	lock xrelease adc cl,100
	xacquire adc BYTE PTR [ecx],100
	xrelease adc BYTE PTR [ecx],100

# Tests for op regb regb/m8
# Tests for op regb/m8 regb
	xacquire adc cl,al
	xacquire lock adc cl,al
	lock xacquire adc cl,al
	xrelease adc cl,al
	xrelease lock adc cl,al
	lock xrelease adc cl,al
	xacquire adc BYTE PTR [ecx],al
	xrelease adc BYTE PTR [ecx],al
	xacquire adc al,cl
	xacquire lock adc al,cl
	lock xacquire adc al,cl
	xrelease adc al,cl
	xrelease lock adc al,cl
	lock xrelease adc al,cl
	xacquire adc al,BYTE PTR [ecx]
	xacquire lock adc al,BYTE PTR [ecx]
	lock xacquire adc al,BYTE PTR [ecx]
	xrelease adc al,BYTE PTR [ecx]
	xrelease lock adc al,BYTE PTR [ecx]
	lock xrelease adc al,BYTE PTR [ecx]

# Tests for op regs regs/m16
# Tests for op regs/m16 regs
	xacquire adc cx,ax
	xacquire lock adc cx,ax
	lock xacquire adc cx,ax
	xrelease adc cx,ax
	xrelease lock adc cx,ax
	lock xrelease adc cx,ax
	xacquire adc WORD PTR [ecx],ax
	xrelease adc WORD PTR [ecx],ax
	xacquire adc ax,cx
	xacquire lock adc ax,cx
	lock xacquire adc ax,cx
	xrelease adc ax,cx
	xrelease lock adc ax,cx
	lock xrelease adc ax,cx
	xacquire adc ax,WORD PTR [ecx]
	xacquire lock adc ax,WORD PTR [ecx]
	lock xacquire adc ax,WORD PTR [ecx]
	xrelease adc ax,WORD PTR [ecx]
	xrelease lock adc ax,WORD PTR [ecx]
	lock xrelease adc ax,WORD PTR [ecx]

# Tests for op regl regl/m32
# Tests for op regl/m32 regl
	xacquire adc ecx,eax
	xacquire lock adc ecx,eax
	lock xacquire adc ecx,eax
	xrelease adc ecx,eax
	xrelease lock adc ecx,eax
	lock xrelease adc ecx,eax
	xacquire adc DWORD PTR [ecx],eax
	xrelease adc DWORD PTR [ecx],eax
	xacquire adc eax,ecx
	xacquire lock adc eax,ecx
	lock xacquire adc eax,ecx
	xrelease adc eax,ecx
	xrelease lock adc eax,ecx
	lock xrelease adc eax,ecx
	xacquire adc eax,DWORD PTR [ecx]
	xacquire lock adc eax,DWORD PTR [ecx]
	lock xacquire adc eax,DWORD PTR [ecx]
	xrelease adc eax,DWORD PTR [ecx]
	xrelease lock adc eax,DWORD PTR [ecx]
	lock xrelease adc eax,DWORD PTR [ecx]

# Tests for op regs, regs/m16
	xacquire btc cx,ax
	xacquire lock btc cx,ax
	lock xacquire btc cx,ax
	xrelease btc cx,ax
	xrelease lock btc cx,ax
	lock xrelease btc cx,ax
	xacquire btc WORD PTR [ecx],ax
	xrelease btc WORD PTR [ecx],ax

# Tests for op regl regl/m32
	xacquire btc ecx,eax
	xacquire lock btc ecx,eax
	lock xacquire btc ecx,eax
	xrelease btc ecx,eax
	xrelease lock btc ecx,eax
	lock xrelease btc ecx,eax
	xacquire btc DWORD PTR [ecx],eax
	xrelease btc DWORD PTR [ecx],eax

# Tests for op regb/m8
	xacquire dec cl
	xacquire lock dec cl
	lock xacquire dec cl
	xrelease dec cl
	xrelease lock dec cl
	lock xrelease dec cl
	xacquire dec BYTE PTR [ecx]
	xrelease dec BYTE PTR [ecx]

# Tests for op regs/m16
	xacquire dec cx
	xacquire lock dec cx
	lock xacquire dec cx
	xrelease dec cx
	xrelease lock dec cx
	lock xrelease dec cx
	xacquire dec WORD PTR [ecx]
	xrelease dec WORD PTR [ecx]

# Tests for op regl/m32
	xacquire dec ecx
	xacquire lock dec ecx
	lock xacquire dec ecx
	xrelease dec ecx
	xrelease lock dec ecx
	lock xrelease dec ecx
	xacquire dec DWORD PTR [ecx]
	xrelease dec DWORD PTR [ecx]

# Tests for op m64
	xacquire cmpxchg8b QWORD PTR [ecx]
	xrelease cmpxchg8b QWORD PTR [ecx]

# Tests for op regb, regb/m8
	xacquire cmpxchg al,cl
	xacquire lock cmpxchg al,cl
	lock xacquire cmpxchg al,cl
	xrelease cmpxchg al,cl
	xrelease lock cmpxchg al,cl
	lock xrelease cmpxchg al,cl
	xacquire cmpxchg BYTE PTR [ecx],cl
	xrelease cmpxchg BYTE PTR [ecx],cl
