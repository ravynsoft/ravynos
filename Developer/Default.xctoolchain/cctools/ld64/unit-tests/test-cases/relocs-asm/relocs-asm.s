/*
 * Copyright (c) 2005-2010 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

		
		
#if __arm__
	.text
	.align 2
		
	.globl _test_loads
_test_loads:
    @ PIC load of a
    ldr r0, L100
L0:
    ldr r0, [pc, r0]

    @ PIC load of c
    ldr r0, L100+4
L1:
    ldr r0, [pc, r0]

	@ sorta-absolute load of a
	ldr r0, L100+8
	ldr r0, [r0, #0]
	
	@ sorta-absolute load of c
	ldr r0, L100+12
	ldr r0, [r0, #0]

	@ sorta-absolute load of external
	ldr r0, L100+16
	ldr r0, [r0, #0]

	@ PIC load of a + addend ??
    bx lr

L100:
    .long _a-(L0+8)
    .long _c-(L1+8)
    .long _a
    .long _c
    .long _ax

_test_calls:
	@ call internal
	bl	_test_branches
	
	@ call internal + addend
	bl	_test_branches+0x19000

	@ call external
	bl	_external
	
	@ call external + addend
	bl	_external+0x19000
	

_test_branches:
	@ call internal
	bne	_test_calls
	
	@ call internal + addend
	bne	_test_calls+16

	@ call internal - addend
	bne	_test_calls-16

	@ call external
	bne	_external
	
	@ call external + addend
	bne	_external+16

	nop
	bl	  1f
1:	nop


	.globl	_test_weak
	.weak_definition _test_weak
_test_weak:
	nop
	nop

	.globl	_test_hidden_weak
	.private_extern _test_hidden_weak
	.weak_definition _test_hidden_weak
_test_hidden_weak:
	nop
	nop


_test_weak_call:
	bl	_test_weak
	bl	_test_weak+4


_test_weak_hidden_pointer_call:
	ldr		r12,L3
	add		r12, pc, r12
	nop
	bx		r12
L101:	
	.long	_test_hidden_weak - L101 
	
	
	.text
_pointer_diffs:
1:	.long _foo-1b
	.long _foo+10-1b
	.long _test_branches-1b
	.long _test_branches+3-1b
	.long (_test_branches - _test_loads) + -2097152
	.long (_test_calls - _test_loads) + -2097152 


	.text
	.code 32
_arm1: 
	bx	lr
_arm2: 
	bx	lr
	.weak_definition _arm3
	.globl _arm3
	.private_extern _arm3
_arm3: 
	bx	lr
	.weak_definition _arm4
	.globl _arm4
	.private_extern _arm4
_arm4: 
	bx	lr

	.code 16
	.thumb_func _thumb1
_thumb1: 
	bx	lr
	.thumb_func _thumb2
_thumb2: 
	bx	lr
	.weak_definition _thumb3
	.globl _thumb3
	.private_extern _thumb3
	.thumb_func _thumb3
_thumb3: 
	bx	lr
	.weak_definition _thumb4
	.globl _thumb4
	.private_extern _thumb4
	.thumb_func _thumb4
_thumb4: 
	bx	lr

	.thumb_func _thumb_func_ref_test
_thumb_func_ref_test: 
	push	{r7, lr}
	add	r7, sp, #0
	ldr	r3, L6
L2:	add	r3, pc
	ldr	r3, L7
L3:	add	r3, pc
	ldr	r3, L8
L4:	add	r3, pc
	ldr	r3, L9
L5:	add	r3, pc
	pop	{r7, pc}
	.align 2
L6:	.long	_thumb1-(L2+4)
L7:	.long	_thumb2-(L3+4)
L7a:.long	_thumb3-(L3+4)
L7b:.long	_thumb4-(L3+4)
L8:	.long	_arm1-(L4+4)
L9:	.long	_arm2-(L5+4)
L9a:.long	_arm3-(L5+4)
L9b:.long	_arm4-(L5+4)

	.code 32
	.align 2
_arm_func_ref_test: 
	push	{r7, lr}
	add	r7, sp, #0
	ldr	r3, L16
L12:add	r3, pc
	ldr	r3, L17
L13:add	r3, pc
	ldr	r3, L18
L14:add	r3, pc
	ldr	r3, L19
L15:add	r3, pc
	pop	{r7, pc}
	.align 2
L16:	.long	_thumb1-(L12+8)
L17:	.long	_thumb2-(L13+8)
L17a:	.long	_thumb3-(L13+8)
L17b:	.long	_thumb4-(L13+8)
L18:	.long	_arm1-(L14+8)
L19:	.long	_arm2-(L15+8)
L19a:	.long	_arm3-(L15+8)
L19b:	.long	_arm4-(L15+8)

	.section	__DATA,__const
_myVTable:
		.long	_thumb1
		.long	_thumb2
		.long	_thumb3
		.long	_arm1
		.long	_arm2

#if __ARM_ARCH_7A__
	.text
		.align 2
_arm16tests:
       movw    r0, :lower16:_datahilo16
       movt    r0, :upper16:_datahilo16
       movw    r0, :lower16:_datahilo16+4
       movt    r0, :upper16:_datahilo16+4
       movw    r0, :lower16:_datahilo16alt
       movt    r0, :upper16:_datahilo16alt
       movw    r0, :lower16:_datahilo16alt+61440
       movt    r0, :upper16:_datahilo16alt+61440
       movw    r0, :lower16:_datahilo16alt+2048
       movt    r0, :upper16:_datahilo16alt+2048
       movw    r0, :lower16:_datahilo16alt+1792
       movt    r0, :upper16:_datahilo16alt+1792
       movw    r0, :lower16:_datahilo16alt+165
       movt    r0, :upper16:_datahilo16alt+165
       movw    r0, :lower16:_thumbTarget
       movt    r0, :upper16:_thumbTarget
       movw    r0, :lower16:_externalTarget
       movt    r0, :upper16:_externalTarget
       movw    r0, :lower16:_externalTarget+61447
       movt    r0, :upper16:_externalTarget+61447
Lpicbase:
       movw    r0, :lower16:_datahilo16 - Lpicbase
       movt    r0, :upper16:_datahilo16 - Lpicbase
       movw    r0, :lower16:_datahilo16+4 - Lpicbase
       movt    r0, :upper16:_datahilo16+4 - Lpicbase
       movw    r0, :lower16:_datahilo16alt - Lpicbase
       movt    r0, :upper16:_datahilo16alt - Lpicbase
       movw    r0, :lower16:_datahilo16alt+61440 - Lpicbase
       movt    r0, :upper16:_datahilo16alt+61440 - Lpicbase
       movw    r0, :lower16:_datahilo16alt+2048 - Lpicbase
       movt    r0, :upper16:_datahilo16alt+2048 - Lpicbase
       movw    r0, :lower16:_datahilo16alt+1792 - Lpicbase
       movt    r0, :upper16:_datahilo16alt+1792 - Lpicbase
       movw    r0, :lower16:_datahilo16alt+165 - Lpicbase
       movt    r0, :upper16:_datahilo16alt+165 - Lpicbase
       movw    r0, :lower16:_thumbTarget - Lpicbase
       movt    r0, :upper16:_thumbTarget - Lpicbase
       bx      lr
	   
	.code 16
	.thumb_func _thumb16tests
_thumb16tests:
       movw    r0, :lower16:_datahilo16
       movt    r0, :upper16:_datahilo16
       movw    r0, :lower16:_datahilo16+4
       movt    r0, :upper16:_datahilo16+4
       movw    r0, :lower16:_datahilo16alt
       movt    r0, :upper16:_datahilo16alt
       movw    r0, :lower16:_datahilo16alt+61440
       movt    r0, :upper16:_datahilo16alt+61440
       movw    r0, :lower16:_datahilo16alt+2048
       movt    r0, :upper16:_datahilo16alt+2048
       movw    r0, :lower16:_datahilo16alt+1792
       movt    r0, :upper16:_datahilo16alt+1792
       movw    r0, :lower16:_datahilo16alt+165
       movt    r0, :upper16:_datahilo16alt+165
       movw    r0, :lower16:_thumbTarget
       movt    r0, :upper16:_thumbTarget
       movw    r0, :lower16:_externalTarget
       movt    r0, :upper16:_externalTarget
       movw    r0, :lower16:_externalTarget+61447
       movt    r0, :upper16:_externalTarget+61447
Lpicbase2:
       movw    r0, :lower16:_datahilo16 - Lpicbase2
       movt    r0, :upper16:_datahilo16 - Lpicbase2
       movw    r0, :lower16:_datahilo16+4 - Lpicbase2
       movt    r0, :upper16:_datahilo16+4 - Lpicbase2
       movw    r0, :lower16:_datahilo16alt - Lpicbase2
       movt    r0, :upper16:_datahilo16alt - Lpicbase2
       movw    r0, :lower16:_datahilo16alt+61440 - Lpicbase2
       movt    r0, :upper16:_datahilo16alt+61440 - Lpicbase2
       movw    r0, :lower16:_datahilo16alt+2048 - Lpicbase2
       movt    r0, :upper16:_datahilo16alt+2048 - Lpicbase2
       movw    r0, :lower16:_datahilo16alt+1792 - Lpicbase2
       movt    r0, :upper16:_datahilo16alt+1792 - Lpicbase2
       movw    r0, :lower16:_datahilo16alt+165 - Lpicbase2
       movt    r0, :upper16:_datahilo16alt+165 - Lpicbase2
       movw    r0, :lower16:_thumbTarget - Lpicbase2
       movt    r0, :upper16:_thumbTarget - Lpicbase2
       bx      lr

	.code 16
	.thumb_func _thumbTarget
_thumbTarget:
        nop
        bx  lr

	.data
_datahilo16:	.long 0
_datahilo16alt:	.long 0



#endif
	
#endif

#if __ppc__ || __ppc64__

	.text
	.align 2
		
	.globl _test_loads
_test_loads:
	stmw r30,-8(r1)
	stwu r1,-48(r1)
Lpicbase:

	; PIC load of a 
	addis r2,r10,ha16(_a-Lpicbase)
	lwz r2,lo16(_a-Lpicbase)(r2)

	; PIC load of c 
	addis r2,r10,ha16(_c-Lpicbase)
	lwz r2,lo16(_c-Lpicbase)(r2)

	; absolute load of a
	lis r2,ha16(_a)
	lwz r2,lo16(_a)(r2)

	; absolute load of c
	lis r2,ha16(_c)
	lwz r2,lo16(_c)(r2)

	; absolute load of external
	lis r2,ha16(_ax)
	lwz r2,lo16(_ax)(r2)

	; absolute lea of external
	lis r2,hi16(_ax)
	ori r2,r2,lo16(_ax)


	; PIC load of a + addend
	addis r2,r10,ha16(_a+0x19000-Lpicbase)
	lwz r2,lo16(_a+0x19000-Lpicbase)(r2)

	; absolute load of a + addend
	lis r2,ha16(_a+0x19000)
	lwz r2,lo16(_a+0x19000)(r2)

	; lea of a + addend
	lis r2,ha16(_a+0x19000)
	addi r2,r2,lo16(_a+0x19000)

	; alt lea of a + addend
	lis r2,hi16(_a+0x19000)
	ori r2,r2,lo16(_a+0x19000)

	; absolute load of external + addend
	lis r2,ha16(_ax+0x19000)
	lwz r2,lo16(_ax+0x19000)(r2)

	; absolute lea of external + addend
	lis r2,hi16(_ax+0x19000)
	ori r2,r2,lo16(_ax+0x19000)


	; PIC load of a + addend
	addis r2,r10,ha16(_a+0x09000-Lpicbase)
	lwz r2,lo16(_a+0x09000-Lpicbase)(r2)

	; absolute load of a + addend
	lis r2,ha16(_a+0x09000)
	lwz r2,lo16(_a+0x09000)(r2)

	; lea of a + addend
	lis r2,ha16(_a+0x09000)
	addi r2,r2,lo16(_a+0x09000)

	; alt lea of a + addend
	lis r2,hi16(_a+0x09000)
	ori r2,r2,lo16(_a+0x09000)

	; absolute load of external + addend
	lis r2,ha16(_ax+0x09000)
	lwz r2,lo16(_ax+0x09000)(r2)

	; absolute lea of external + addend
	lis r2,hi16(_ax+0x09000)
	ori r2,r2,lo16(_ax+0x09000)

	blr


_test_calls:
	; call internal
	bl	_test_branches
	
	; call internal + addend
	bl	_test_branches+0x19000

	; call external
	bl	_external
	
	; call external + addend
	bl	_external+0x19000
	

_test_branches:
	; call internal
	bne	_test_calls
	
	; call internal + addend
	bne	_test_calls+16

	; call external
	bne	_external
	
	; call external + addend
	bne	_external+16

	.globl	_test_weak
	.weak_definition _test_weak
_test_weak:
	nop
	nop
	
_test_weak_call:
	bl	_test_weak
	bl	_test_weak+4

#endif



#if __i386__
	.text
	.align 2

Ltest_data:
	.long	1
	.long	2
	.long	3

	.globl _test_loads
_test_loads:
	pushl	%ebp
Lpicbase:

	# PIC load of a 
	movl	_a-Lpicbase(%ebx), %eax
	
	# absolute load of a
	movl	_a, %eax

	# absolute load of external
	movl	_ax, %eax

	# absolute lea of external
	leal	_ax, %eax


	# PIC load of a + addend
	movl	_a-Lpicbase+0x19000(%ebx), %eax

	# absolute load of a + addend
	movl	_a+0x19000(%ebx), %eax

	# absolute load of external + addend
	movl	_ax+0x19000(%ebx), %eax

	# absolute lea of external + addend
	leal	_ax+0x1900, %eax

	# absolute load of _test_data with negative addend and local label
	movl	Ltest_data-16(%edi),%eax
	movq	Ltest_data-16(%edi),%mm4
	
	ret


_test_calls:
	# call internal
	call	_test_branches
	
	# call internal + addend
	call	_test_branches+0x19000

	# 16-bit call internal
	callw	_test_branches
	
	# 16-bit call internal + addend
	callw	_test_branches+13

	# call external
	call	_external
	
	# call external + addend
	call	_external+0x19000
	

_test_branches:
	# call internal
	jne	_test_calls
	
	# call internal + addend
	jne	_test_calls+16

	# call external
	jne	_external
	
	# call external + addend
	jne	_external+16
	
_pointer_diffs:
	nop
	call	_get_ret_eax	
1:	movl _foo-1b(%eax),%esi
	movl _foo+10-1b(%eax),%esi
	movl _test_branches-1b(%eax),%esi
	movl _test_branches+3-1b(%eax),%esi
	cmpl $(( (_test_branches - _test_loads) + -2097152 )),(%esp)
	cmpl $(( (_test_calls - _test_loads) + -2097152 )),(%esp)

	
_word_relocs:
	callw	_pointer_diffs

_byte_relocs:
    mov          $100, %ecx 
c_1:  
    loop         c_1
    mov          $100, %ecx
c_2:
    sub          $(1), %ecx
    jcxz         c_2

	.globl	_test_weak
	.weak_definition _test_weak
_test_weak:
	nop
	nop

_test_weak_call:
	call	_test_weak
	call	_test_weak+1
	
#endif



#if __x86_64__
	.text
	.align 2
	
	.globl _test_loads
_test_loads:

	# PIC load of a 
	movl	_a(%rip), %eax
	
	# PIC load of a + addend
	movl	_a+0x1234(%rip), %eax

	# PIC lea
	leaq	_a(%rip), %rax

	# PIC lea through GOT
	movq	_a@GOTPCREL(%rip), %rax
	
	# PIC access of GOT
  	pushq	_a@GOTPCREL(%rip)

	# PIC lea external through GOT
	movq	_ax@GOTPCREL(%rip), %rax
	
	# PIC external access of GOT
  	pushq	_ax@GOTPCREL(%rip)

	# 1-byte store
  	movb  $0x12, _a(%rip)
  	movb  $0x12, _a+2(%rip)
  	movb  $0x12, L0(%rip)

	# 4-byte store
  	movl  $0x12345678, _a(%rip)
  	movl  $0x12345678, _a+4(%rip)
  	movl  $0x12345678, L0(%rip)
	
	# test local labels
	lea L1(%rip), %rax		
  	movl L0(%rip), %eax		

	ret


_test_calls:
	# call internal
	call	_test_branches
	
	# call internal + addend
	call	_test_branches+0x19000

	# call external
	call	_external
	
	# call external + addend
	call	_external+0x19000
	

_test_branches:
	# call internal
	jne	_test_calls
	
	# call internal + addend
	jne	_test_calls+16

	# call external
	jne	_external
	
	# call external + addend
	jne	_external+16
	
_byte_relocs:
	# nonsense loop that creates byte branch relocation
    mov          $100, %ecx 
c_1:
    loop         _byte_relocs
    nop

	.globl	_test_weak
	.weak_definition _test_weak
_test_weak:
	nop
	nop

_test_weak_call:
	call	_test_weak
	call	_test_weak+1

#endif



	# test that pointer-diff relocs are preserved
	.text
	.align 2
_test_diffs:
Llocal2:
	.long 0
	.long Llocal2-_test_branches
	.long . - _test_branches
	.long . - _test_branches + 8
	.long _test_branches - .
	.long _test_branches - . + 8
	.long _test_branches - . - 8
	.long 0
	.long 0
#if __ppc64__
	.quad Llocal2-_test_branches
#endif

_foo: nop
Lfoo: nop

	.align 2	
_distance_from_foo:
	.long	0
	.long	. - _foo
	.long	. - 8 - _foo
	
	
_distance_to_foo:
	.long	_foo - .
	.long	_foo - . + 4
	

_distance_to_here:	
	.long	_foo - _distance_to_here
	.long	_foo - _distance_to_here - 4 
	.long	_foo - _distance_to_here - 12 
	.long	Lfoo - _distance_to_here
Ltohere:
	.long	Lfoo - Ltohere
	.long	Lfoo - Ltohere - 4
	.long	0


#if __x86_64__
	.data
L0:  .quad _test_branches
_prev:
	.quad _test_branches+4
L1:	.quad _test_branches - _test_diffs
  	.quad _test_branches - _test_diffs + 4
  	.long _test_branches - _test_diffs
#	.long LCL0-.				### assembler bug: should SUB/UNSIGNED with content= LCL0-24, or single pc-rel SIGNED reloc with content = LCL0-.+4
  	.quad L1
  	.quad L0					
  	.quad _test_branches - .
  	.quad _test_branches - L1
  	.quad L1 - _prev			
	.quad _prev+100 - _test_branches
 #tests support for 32-bit absolute pointers
	.long _prev
	.long L1

# the following generates: _foo cannot be undefined in a subtraction expression
# but it should be ok (it will be a linker error if _foo and _bar are not in same linkage unit)
#	.quad _foo - _bar	### assembler bug

	.section __DATA,__data2
LCL0: .long 2


#endif


	.data
_a:	
	.long	0

_b:
#if __ppc__ || __i386__ || __arm__
	.long	_test_calls
	.long	_test_calls+16
	.long	_external
	.long	_external+16
	.long	_test_weak
	.long	_test_weak+16
	.long	Lother - . + 0x4000000
Lother: 
	.long	 0
#elif __ppc64__ || __x86_64__
	.quad	_test_calls
	.quad	_test_calls+16
	.quad	_external
	.quad	_external+16
	.quad	_test_weak
	.quad	_test_weak+16
	.quad	Lother - . + 0x4000000
Lother: 
	.quad	 0
#endif

	# test that reloc sizes are the same
Llocal3:
	.long	0
	
Llocal4:
	.long 0
	
	.long Llocal4-Llocal3
	
Lfiller:
	.space	0x9000
_c:
	.long	0

