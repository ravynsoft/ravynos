
#ifdef _WIN64
#	define START_PROC(x) .seh_proc x
#	define END_PROC(x) .seh_endproc
#	define FRAME_OFFSET(x) .seh_stackalloc x
#	define FIRST_ARGUMENT_STR "%rcx"
#	define FIRST_ARGUMENT %rcx
#	define SECOND_ARGUMENT %rdx
#	define THIRD_ARGUMENT %r8
#else
#	define START_PROC(x) .cfi_startproc
#	define END_PROC(x) .cfi_endproc
#	define FRAME_OFFSET(x) .cfi_adjust_cfa_offset x
#	define FIRST_ARGUMENT_STR "%rdi"
#	define FIRST_ARGUMENT %rdi
#	define SECOND_ARGUMENT %rsi
#	define THIRD_ARGUMENT %rdx
#endif

.macro MSGSEND fnname receiver, sel
	START_PROC(\fnname)                   # Start emitting unwind data.  We
	                                      # don't actually care about any of
	                                      # the stuff except the slow call,
	                                      # because that's the only one that
	                                      # can throw.

	test  \receiver, \receiver            # If the receiver is nil
	jz    4f                              # return nil
	movq  $SMALLOBJ_MASK, %r10            # Load the small object mask
	test  \receiver, %r10                 # Check if the receiver is a small object
	jnz   6f                              # Get the small object class

	mov   (\receiver), %r10               # Load the dtable from the class
1:	                                      # classLoaded
	mov   DTABLE_OFFSET(%r10), %r10       # Load the dtable from the class into r10
	mov   %rax, -8(%rsp)                  # %rax contains information for variadic calls
	mov   %rbx, -16(%rsp)                 # On the fast path, spill into the red zone
	mov   (\sel), %eax                    # Load the selector index into %eax
	mov   SHIFT_OFFSET(%r10), %r11d       # Load the shift (dtable size) into r11
	cmpl  $8, %r11d                       # If this is a small dtable, jump to the small dtable handlers
	je    2f 
	cmpl  $0, %r11d
	je    3f 

	movl  %eax, %r11d
	shrl  $16, %r11d
	movq  DATA_OFFSET(%r10, %r11, 8), %r10
2:                                        # dtable16:
	movzbl %ah, %ebx
	movq  DATA_OFFSET(%r10, %rbx, 8), %r10
3:                                       # dtable8:
	movzbl %al, %ebx
	mov   -8(%rsp), %rax
	movq  DATA_OFFSET(%r10, %rbx, 8), %r10
	mov   -16(%rsp), %rbx
	test  %r10, %r10
	jz    5f                             # Nil slot - invoke some kind of forwarding mechanism
	mov   SLOT_OFFSET(%r10), %r10

7:
#ifdef WITH_TRACING

	push  %r12
	push  %r13
	push  %r10

	mov   (\sel), %r11                    # Load the selector index
	lea   tracing_dtable(%rip), %r10
	mov   (%r10), %r10

	mov   SHIFT_OFFSET(%r10), %r13        # Load the shift (dtable size)
	mov   DATA_OFFSET(%r10), %r12         # load the address of the start of the array
	pop   %r10
	cmpl  $8, %r13d                       # If this is a small dtable, jump to the small dtable handlers
	je    10f 
	cmpl  $0, %r13d
	je    11f 

	mov   %r11, %r13
	and   $0xff0000, %r13
	shrl  $13, %r13d                      # Right shift 16, but then left shift by 3 *sizeof(void*)
	add   %r13, %r12
	mov   (%r12), %r12
	mov   DATA_OFFSET(%r12), %r12
10:                                        # dtable16:
	mov   %r11, %r13
	and   $0xff00, %r13
	shrl  $5, %r13d
	add   %r13, %r12
	mov   (%r12), %r12
	mov   DATA_OFFSET(%r12), %r12
11:                                       # dtable8:
	mov   %r11, %r13
	and   $0xff, %r13
	shll  $3, %r13d
	add   %r13, %r12
	mov   (%r12), %r11
	pop   %r13
	pop   %r12
	test  %r11, %r11
	jz    12f

	push  %rax                           # We need to preserve all registers that may contain arguments:
	push  %rdi
	push  %rsi
	push  %rdx
	push  %rcx
	push  %r8
	push  %r9
	push  %r10
	push  %r11
	mov   \receiver, %rdi                # Arg 0 is receiver
	mov   \sel, %rsi                     # Arg 1 is selector 
	mov   %r10, %rdx                     # Arg 2 is IMP
	mov   $0, %rcx                       # Arg 3 is entry / exit (0/1)
	mov   $0, %r8                        # Arg 4 is return value (0 on entry)

	call  *%r11                          # Call the tracing function
	cmpq  $0, %rax
	jz    13f                            # If it returns 0, don't call the end-tracing function.
	cmpq  $1, %rax                       # If it returns 1, do call the tracing function
	jne   14f                            # Any other value is an interposition
	                                     # function to call instead of the method

	call  pushTraceReturnStack           # rax now contains a thread-local buffer for storing returns

	pop   %r11                           # Restore all of the argument registers
	pop   %r10                           # except rax, which we'll need before the call
	pop   %r9
	pop   %r8
	pop   %rcx
	pop   %rdx
	pop   %rsi
	pop   %rdi

	mov   \receiver, (%rax)              # Store the receiver in TLS
	mov   \sel, 8(%rax)                  # Store the selector in TLS
	mov   %r10, 16(%rax)                 # Store the method in TLS
	mov   %r11, 24(%rax)                 # Store the tracing function in TLS
	mov   8(%rsp), %r11                  # r11 now contains the return address
	mov   %r11, 32(%rax)                 # Store the method-return address in TLS

	pop   %rax
	pop   %r11                           # r11 now contains the return address, but we don't care

	call  *%r10                          # Call the IMP.  The stack should now be in the same state
	                                     # that it was on entry into this function

	push  %rax                           # Now we are free to clobber argument
	push  %rdx                           # registers, but we must preserve return registers...

	call  popTraceReturnStack            # rax now contains a thread-local buffer for storing returns
	
	push  %rax                           # save the return value, because we'll need it after the tracing function call
	mov   (%rax), %rdi                   # Load the receiver into arg 0
	mov   8(%rax), %rsi                  # Load the selector into arg 1
	mov   16(%rax), %rdx                 # Load the IMP into arg 3
	mov   $1, %rcx                       # Arg 4 is 1 (tracing on exit)
	mov   %rax, %r8                      # Arg 5 is the return result

	mov   24(%rax), %r11                 # Reload the address of the tracing function

	call  *%r11                          # Call the tracing function
	pop   %rax                           # Reload the real return address
	mov   32(%rax), %r11
	pop   %rdx                           # Reload saved values
	pop   %rax
	jmp   *%r11                          # Simulate a return by jumping to the cached return address

13:                                      # Skip tracing on exit and just tail-call the method
	pop   %r11
	pop   %r10
	pop   %r9
	pop   %r8
	pop   %rcx
	pop   %rdx
	pop   %rsi
	pop   %rdi
	pop   %rax
	jmp   *%r10

14: 
	mov   %rax, %r10
	pop   %r9
	pop   %r9
	pop   %r9
	pop   %r8
	pop   %rcx
	pop   %rdx
	pop   %rsi
	pop   %rdi
	pop   %rax

12:
#endif // WITH_TRACING
#ifdef _WIN64
	mov   %r10, %rax
	jmp   *__guard_dispatch_icall_fptr(%rip)
#else
	jmp   *%r10
#endif
4:                                       # returnNil:
	                                     # Both of the return registers are
	                                     # callee-save on x86-64, so we can
	                                     # return 0 in both in the same code:
	xor   %rax, %rax                     # Return 0 as an integer
	pxor  %xmm0, %xmm0                   # Return 0 as a floating point value
	ret
5:                                       # slowSend:
	push  %rax                           # We need to preserve all registers that may contain arguments:
	push  %rbx
	push  %rcx
	push  %r8
	push  %r9
	
	sub $0x98, %rsp
	movups	%xmm0, 0x80(%rsp)
	movups	%xmm1, 0x70(%rsp)
	movups	%xmm2, 0x60(%rsp)
	movups	%xmm3, 0x50(%rsp)
	movups	%xmm4, 0x40(%rsp)
	movups	%xmm5, 0x30(%rsp)
	movups	%xmm6, 0x20(%rsp)
	movups	%xmm7, 0x10(%rsp)

#rdi rsi rdx
	# We're (potentially) modifying the self argument with the lookup, so we don't want to be 
.ifc "\receiver", FIRST_ARGUMENT_STR
	push  FIRST_ARGUMENT
	mov   %rsp, FIRST_ARGUMENT
	push  SECOND_ARGUMENT                # Save _cmd (not preserved across calls)
	push  THIRD_ARGUMENT
.else
	push  FIRST_ARGUMENT                 # Save the sret pointer
	push  SECOND_ARGUMENT                # Save self where it can be modified
	mov   %rsp, FIRST_ARGUMENT
	push  THIRD_ARGUMENT
	mov   THIRD_ARGUMENT, SECOND_ARGUMENT # move _cmd to where the callee expects it to be
.endif

	FRAME_OFFSET(0xD8)
	call  CDECL(slowMsgLookup)           # Call the slow lookup function
	mov   %rax, %r10                     # Load the returned IMP

	pop   THIRD_ARGUMENT
	pop   SECOND_ARGUMENT
	pop   FIRST_ARGUMENT

	movups	0x80(%rsp), %xmm0
	movups	0x70(%rsp), %xmm1
	movups	0x60(%rsp), %xmm2
	movups	0x50(%rsp), %xmm3
	movups	0x40(%rsp), %xmm4
	movups	0x30(%rsp), %xmm5
	movups	0x20(%rsp), %xmm6
	movups	0x10(%rsp), %xmm7
	add   $0x98, %rsp

	pop   %r9
	pop   %r8
	pop   %rcx
	pop   %rbx
	pop   %rax
	jmp   7b
6:                                        # smallObject:
	and   \receiver, %r10                 # Find the small int type
	lea   CDECL(SmallObjectClasses)(%rip), %r11
	mov   (%r11, %r10, 8), %r10
	jmp   1b 
	END_PROC(\fnname)
.endm
#ifdef _WIN64
.text
.def objc_msgSend;
.scl 2;
.type 32;
.endef
.def objc_msgSend_fpret;
.scl 2;
.type 32;
.endef
.def objc_msgSend_stret;
.scl 2;
.type 32;
.endef
.globl CDECL(objc_msgSend_fpret)
TYPE_DIRECTIVE(CDECL(objc_msgSend_fpret), @function)
.globl CDECL(objc_msgSend)
TYPE_DIRECTIVE(CDECL(objc_msgSend), @function)
CDECL(objc_msgSend_fpret):
CDECL(objc_msgSend):
	MSGSEND objc_msgSend, %rcx, %rdx
.globl CDECL(objc_msgSend_stret)
TYPE_DIRECTIVE(CDECL(objc_msgSend_stret), @function)
CDECL(objc_msgSend_stret):
	MSGSEND objc_msgSend_stret, %rdx, %r8
.section        .drectve,"yn"
.ascii  " /EXPORT:objc_msgSend"
.ascii  " /EXPORT:objc_msgSend_fpret"
.ascii  " /EXPORT:objc_msgSend_stret"
#else
.globl CDECL(objc_msgSend)
TYPE_DIRECTIVE(CDECL(objc_msgSend), @function)
.globl CDECL(objc_msgSend_fpret)
TYPE_DIRECTIVE(CDECL(objc_msgSend_fpret), @function)
CDECL(objc_msgSend_fpret):
CDECL(objc_msgSend):
	MSGSEND objc_msgSend, %rdi, %rsi
.globl CDECL(objc_msgSend_stret)
TYPE_DIRECTIVE(CDECL(objc_msgSend_stret), @function)
CDECL(objc_msgSend_stret):
	MSGSEND objc_msgSend_stret, %rsi, %rdx
#endif
